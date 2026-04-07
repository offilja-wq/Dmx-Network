#pragma once

#include <Arduino.h>

#include "config.h"

// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <esp_now.h>

void begin();

void setMode(MODE input);

void switchMode();

void handleReceive(const uint8_t *mac, const uint8_t *data, int len);

void handleSend();

void send(PACKET *packet);

void updateDisplay();