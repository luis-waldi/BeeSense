/**
 * BeeSense Dashboard Server
 * 
 * Empfängt Tracking-Daten vom ESP32 und pusht sie live ans Dashboard.
 * 
 * Starten mit: node server.js
 * Dashboard öffnen: http://localhost:8080
 */

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 8080;

// Aktuelle Tracking-Daten
let trackingData = {
    einflug: 0,
    ausflug: 0,
    timestamp: Date.now(),
    lastUpdate: null
};

// Server-Sent Events Clients (für Live-Updates)
const sseClients = new Set();

// MIME Types für statische Dateien
const mimeTypes = {
    '.html': 'text/html',
    '.css': 'text/css',
    '.js': 'application/javascript',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.svg': 'image/svg+xml',
    '.json': 'application/json'
};

// HTTP Server
const server = http.createServer((req, res) => {
    // CORS Headers für lokale Entwicklung
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    // OPTIONS Request (CORS Preflight)
    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    const url = new URL(req.url, `http://localhost:${PORT}`);
    
    // API: Tracking-Daten empfangen (POST vom ESP32)
    if (url.pathname === '/api/tracking' && req.method === 'POST') {
        let body = '';
        req.on('data', chunk => body += chunk);
        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                
                // Daten aktualisieren
                trackingData = {
                    einflug: data.einflug || 0,
                    ausflug: data.ausflug || 0,
                    timestamp: data.timestamp || Date.now(),
                    lastUpdate: new Date().toISOString()
                };

                console.log(`[${new Date().toLocaleTimeString()}] Tracking empfangen:`, trackingData);

                // Alle SSE Clients benachrichtigen
                broadcastToClients(trackingData);

                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: true, message: 'Daten empfangen' }));
            } catch (e) {
                console.error('Fehler beim Parsen:', e.message);
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: false, error: 'Invalid JSON' }));
            }
        });
        return;
    }

    // API: Aktuelle Daten abrufen (GET)
    if (url.pathname === '/api/tracking' && req.method === 'GET') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(trackingData));
        return;
    }

    // API: Server-Sent Events für Live-Updates
    if (url.pathname === '/api/events') {
        res.writeHead(200, {
            'Content-Type': 'text/event-stream',
            'Cache-Control': 'no-cache',
            'Connection': 'keep-alive'
        });

        // Initiale Daten senden
        res.write(`data: ${JSON.stringify(trackingData)}\n\n`);

        // Client zur Liste hinzufügen
        sseClients.add(res);
        console.log(`[SSE] Client verbunden. Aktive Clients: ${sseClients.size}`);

        // Cleanup bei Disconnect
        req.on('close', () => {
            sseClients.delete(res);
            console.log(`[SSE] Client getrennt. Aktive Clients: ${sseClients.size}`);
        });
        return;
    }

    // Statische Dateien servieren
    let filePath = url.pathname;
    if (filePath === '/') filePath = '/index.html';
    
    const fullPath = path.join(__dirname, filePath);
    const ext = path.extname(fullPath);
    const contentType = mimeTypes[ext] || 'application/octet-stream';

    fs.readFile(fullPath, (err, data) => {
        if (err) {
            if (err.code === 'ENOENT') {
                res.writeHead(404);
                res.end('Datei nicht gefunden: ' + filePath);
            } else {
                res.writeHead(500);
                res.end('Server Fehler');
            }
            return;
        }

        res.writeHead(200, { 'Content-Type': contentType });
        res.end(data);
    });
});

// Broadcast an alle SSE Clients
function broadcastToClients(data) {
    const message = `data: ${JSON.stringify(data)}\n\n`;
    sseClients.forEach(client => {
        try {
            client.write(message);
        } catch (e) {
            sseClients.delete(client);
        }
    });
}

// Server starten
server.listen(PORT, '0.0.0.0', () => {
    console.log('');
    console.log('🐝 BeeSense Dashboard Server gestartet');
    console.log('=====================================');
    console.log(`📊 Dashboard:     http://localhost:${PORT}`);
    console.log(`📡 API Endpoint:  http://localhost:${PORT}/api/tracking`);
    console.log(`🔴 Live Events:   http://localhost:${PORT}/api/events`);
    console.log('');
    console.log('Warte auf Daten vom ESP32...');
    console.log('');
});
