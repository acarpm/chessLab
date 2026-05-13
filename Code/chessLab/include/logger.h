#pragma once
#include <stdarg.h>

void logger_init();
void logger_update();
void logger_printf(const char* fmt, ...);

// Macro de remplacement — remplace Serial.printf partout
#define LOG(fmt, ...) logger_printf(fmt, ##__VA_ARGS__)
