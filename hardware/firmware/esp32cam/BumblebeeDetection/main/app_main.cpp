#include "esp_log.h"
#include "dl_model_base.hpp"
#include "dl_image_define.hpp"
#include "dl_image_preprocessor.hpp"
#include "dl_detect_base.hpp"
#include "dl_detect_yolo11_postprocessor.hpp"
#include "dl_image_jpeg.hpp"
#include "bsp/esp-bsp.h"
#include <esp_system.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "include/ClassificationPostProcessor.hpp"
// #include "include/classification_category_name.hpp"
#include "esp_camera.h"

#include "sd_card.hpp"

#include <dirent.h>
#include <vector>
#include <limits>

#include "include/camera_pins.h"

// Support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

extern const uint8_t example_jpg_start[] asm("_binary_example_jpg_start");
extern const uint8_t example_jpg_end[] asm("_binary_example_jpg_end");

// Passe Modellpfad an dein quantisiertes Hummelmodell an
extern const uint8_t espdl_model[] asm("_binary_yolov8_bumblebee_quantized_espdl_start");
static const char *model_path = (const char *)espdl_model;



// Detection-Inferenz für YOLOv8
static std::list<dl::detect::result_t> run_detection(dl::image::img_t &input_img) {
    char dir[64];
    snprintf(dir, sizeof(dir), "%s/espdl_models", CONFIG_BSP_SD_MOUNT_POINT);

    dl::Model *model = nullptr;
    dl::image::ImagePreprocessor *m_image_preprocessor = nullptr;
    dl::detect::yolo11PostProcessor *m_postprocessor = nullptr;
    std::list<dl::detect::result_t> results;

    model = new dl::Model(model_path, dir, static_cast<fbs::model_location_type_t>(0));
    if (!model) {
        ESP_LOGE("MODEL", "Failed to create model");
        return results;
    }
    model->minimize();
    vTaskDelay(pdMS_TO_TICKS(10));

    m_image_preprocessor = new dl::image::ImagePreprocessor(model, {123.675, 116.28, 103.53}, {58.395, 57.12, 57.375}, DL_IMAGE_CAP_RGB565_BIG_ENDIAN);
    if (!m_image_preprocessor) {
        ESP_LOGE("PREPROCESSOR", "Failed to create image preprocessor");
        delete model;
        return results;
    }
    m_image_preprocessor->preprocess(input_img);
    model->run(dl::RUNTIME_MODE_MULTI_CORE);

    // Score-Threshold, NMS-Threshold, Top-K, Stages (hier leer, falls nicht benötigt)
    m_postprocessor = new dl::detect::yolo11PostProcessor(model, 0.3f, 0.5f, 10, {});
    m_postprocessor->set_resize_scale_x(m_image_preprocessor->get_resize_scale_x());
    m_postprocessor->set_resize_scale_y(m_image_preprocessor->get_resize_scale_y());
    m_postprocessor->postprocess();
    results = m_postprocessor->get_result(input_img.width, input_img.height);

    // Ressourcen freigeben
    delete m_postprocessor;
    delete m_image_preprocessor;
    delete model;

    return results;
}

// Camera Module pin mapping
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,

    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000, // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, // PIXFORMAT_RGB565 , PIXFORMAT_JPEG
    .frame_size = FRAMESIZE_QVGA,     // [<<320x240>> (QVGA, 4:3); FRAMESIZE_320X320, 240x176 (HQVGA, 15:11); 400x296 (CIF, 50:37)],FRAMESIZE_QVGA,FRAMESIZE_VGA

    .jpeg_quality = 8, // 0-63 lower number means higher quality.  Reduce quality if stack overflow in cam_task
    .fb_count = 2,     // if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .sccb_i2c_port = 0 // optional
};

static esp_err_t init_camera(void) {
    // Initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE("CAM", "Camera Init Failed");
    }
    return err;
}

static bool capture_image(dl::image::img_t &output_img) {
    ESP_LOGI("CAM", "Taking picture...");
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE("CAM", "Failed to capture image");
        return false;
    }

    // Use pic->buf to access the image
    ESP_LOGI("CAM", "Picture taken! Its size was: %zu bytes", pic->len);
    ESP_LOGW("image_dim", "Height: %d, Width: %d, Len: %zu", pic->height, pic->width, pic->len);

    // Allocate memory and copy the image data before returning the frame buffer
    output_img.height = pic->height;
    output_img.width = pic->width;
    output_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;

    // Allocate memory for the image data
    output_img.data = malloc(pic->len);
    if (!output_img.data) {
        ESP_LOGE("MEM", "Memory allocation failed");
        esp_camera_fb_return(pic);
        return false;
    }

    // Copy the image data
    memcpy(output_img.data, pic->buf, pic->len);

    esp_camera_fb_return(pic);
    return true;
}


extern "C" void app_main(void) {
    ESP_LOGI("SD", "Mounting SD card...");
    bool mounted = sdcard::init();
    if (!mounted) {
        ESP_LOGE("SD", "SD card init/mount failed");
        return;
    }

    if (ESP_OK != init_camera()) {
        ESP_LOGE("APP", "Camera initialization failed");
        return;
    }
    
    const char *out_dir = "/sdcard/classification";
    sdcard::create_dir(out_dir);

    while (true) {
        ESP_LOGI("MEM", "Free heap at start of loop: %lu bytes", esp_get_free_heap_size());

        dl::image::img_t img;
        if (!capture_image(img)) {
            ESP_LOGE("CAM", "Could not take picture");
            vTaskDelay(pdMS_TO_TICKS(2000));
            return;
        }

        // Convert RGB565 (camera) → RGB888 (needed for inference)
        dl::image::img_t rgb888_img;
        rgb888_img.height = img.height;
        rgb888_img.width = img.width;
        rgb888_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
        rgb888_img.data = malloc(img.height * img.width * 3);
        if (!rgb888_img.data) {
            ESP_LOGE("MEM", "Failed to allocate RGB888 buffer");
            free(img.data);
            continue;
        }

        dl::image::convert_img(img, rgb888_img, 0, nullptr, {});

        // Free camera frame buffer
        free(img.data);


        // --- Detection-Inferenz ---
        auto results = run_detection(rgb888_img);
        bool found_bumblebee = false;
        for (const auto &det : results) {
            // Annahme: Klasse 0 = background, Klasse 1 = bumblebee
            if (det.category == 1 && det.score > 0.3f) {
                ESP_LOGI("DETECT", "Hummel erkannt! Score: %.2f, Box: [%d,%d,%d,%d]", det.score, det.box[0], det.box[1], det.box[2], det.box[3]);
                found_bumblebee = true;
            }
        }
        if (!found_bumblebee) {
            ESP_LOGI("DETECT", "Keine Hummel erkannt.");
        }

        // Optional: Bild speichern, falls gewünscht
        // if (!sdcard::save_classified_jpeg(rgb888_img, best, out_dir)) {
        //     ESP_LOGE("SD", "Failed to save classified JPEG");
        // }

        free(rgb888_img.data);

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 Sekunde warten
    }
}
