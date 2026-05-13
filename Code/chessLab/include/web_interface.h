#pragma once
#include "board_config.h"

// Point d'entrée : démarre le WiFi AP et le serveur WebSocket
void web_init();

// À appeler dans loop() — traite les clients WebSocket connectés
void web_update();

// Envoie l'état du plateau (JSON) à tous les clients connectés
void web_broadcast_state(int dev[BOARD_SIZE][BOARD_SIZE]);
