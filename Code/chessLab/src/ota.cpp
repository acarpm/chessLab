#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "ota.h"
#include "wifi_credentials.h"
#include "logger.h"

#define OTA_HOSTNAME  "chesslab"
#define OTA_PASSWORD  AP_PASSWORD   // même mot de passe que l'AP

static bool wifi_connect_sta() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    LOG("[WiFi] Connexion à '%s'", WIFI_SSID);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= WIFI_STA_TIMEOUT_MS) {
            Serial.println(" timeout.");
            return false;
        }
        delay(500);
        LOG(".");
    }
    LOG("\n[WiFi] STA connecté — IP: %s\n", WiFi.localIP().toString().c_str());
    // Désactiver le sleep WiFi — réduit fortement le bruit ADC sur ESP32
    WiFi.setSleep(false);
    return true;
}

static void wifi_start_ap() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.setSleep(false);
    LOG("[WiFi] Mode AP — SSID: '%s'  IP: %s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
}

void ota_init() {
    if (!wifi_connect_sta()) {
        wifi_start_ap();
    }

    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        Serial.println("[OTA] Début de la mise à jour...");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] Terminé. Redémarrage.");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        LOG("[OTA] %u%%\r", progress * 100 / total);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        LOG("[OTA] Erreur [%u]\n", error);
    });

    MDNS.begin(OTA_HOSTNAME);
    ArduinoOTA.begin();

    LOG("[OTA] Prêt — hostname: %s.local\n", OTA_HOSTNAME);
    LOG("[OTA] IP directe : %s\n",
        WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString().c_str()
                                  : WiFi.localIP().toString().c_str());
}

void ota_update() {
    ArduinoOTA.handle();
}
