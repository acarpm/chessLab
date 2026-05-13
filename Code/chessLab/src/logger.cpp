#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include "logger.h"

#define TELNET_PORT  23
#define MAX_CLIENTS   2

static WiFiServer  telnet_server(TELNET_PORT);
static WiFiClient  clients[MAX_CLIENTS];

void logger_init() {
    telnet_server.begin();
    telnet_server.setNoDelay(true);
    Serial.printf("[Logger] Telnet prêt sur port %d\n", TELNET_PORT);
}

void logger_update() {
    // Accepte les nouveaux clients dans un slot libre
    if (telnet_server.hasClient()) {
        WiFiClient incoming = telnet_server.accept();
        bool placed = false;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i] || !clients[i].connected()) {
                clients[i] = incoming;
                placed = true;
                Serial.printf("[Logger] Client Telnet connecté (%d/%d)\n", i + 1, MAX_CLIENTS);
                break;
            }
        }
        if (!placed) {
            incoming.println("ChessLab: serveur plein, réessayez.");
            incoming.stop();
        }
    }
}

void logger_printf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Toujours vers USB serial
    Serial.print(buf);

    // Vers chaque client Telnet connecté
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i].connected())
            clients[i].print(buf);
    }
}
