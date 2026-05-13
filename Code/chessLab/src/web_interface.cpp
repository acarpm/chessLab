#include "board_config.h"
#include "web_interface.h"

// TODO: implémenter WiFi AP + serveur WebSocket
// Bibliothèques suggérées : WiFi.h (Arduino ESP32), WebSocketsServer (lib_deps)
//
// Architecture prévue :
//   web_init()    → WiFi.softAP("ChessLab", password)
//                 → serveur WebSocket sur port 81
//   web_update()  → webSocket.loop()
//   web_broadcast_state() → sérialise dev[8][8] en JSON et envoie à tous les clients
//
// Format JSON envisagé :
//   { "board": [[0,0,...],[0,150,...], ...] }

void web_init() {
    // non implémenté
}

void web_update() {
    // non implémenté
}

void web_broadcast_state(int dev[BOARD_SIZE][BOARD_SIZE]) {
    (void)dev;
    // non implémenté
}
