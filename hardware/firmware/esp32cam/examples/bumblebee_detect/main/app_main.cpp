#include "dl_image_jpeg.hpp"
#include "bumblebee_detect.hpp"
#include "esp_log.h"
#include "bsp/esp-bsp.h"

extern const uint8_t bumblebee_jpg_start[] asm("_binary_bumblebee_jpg_start");
extern const uint8_t bumblebee_jpg_end[] asm("_binary_bumblebee_jpg_end");
const char *TAG = "bumblebee_detect";

extern "C" void app_main(void)
{
#if CONFIG_BUMBLEBEE_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

    dl::image::jpeg_img_t jpeg_img = {.data = (void *)bumblebee_jpg_start, .data_len = (size_t)(bumblebee_jpg_end - bumblebee_jpg_start)};
    auto img = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);

    BumblebeeDetect *detect = new BumblebeeDetect();
    auto &detect_results = detect->run(img);
    int result_count = 0;
    for (const auto &res : detect_results) {
        if (res.category == 0 && res.score > 0.3f) {
            ESP_LOGI(TAG,
                     "[category: %d, score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
                     res.category,
                     res.score,
                     res.box[0],
                     res.box[1],
                     res.box[2],
                     res.box[3]);
            ++result_count;
        }
    }
    if (result_count == 0) {
        ESP_LOGI(TAG, "Detection done, nothing detected (class 0 & score > 0.3)");
    } else {
        ESP_LOGI(TAG, "Detection done, results: %d", result_count);
    }
    delete detect;
    heap_caps_free(img.data);

#if CONFIG_BUMBLEBEE_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif
}
