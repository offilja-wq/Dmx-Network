#pragma once

#include <Arduino.h>

#include "config.h"

// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <esp_now.h>

void begin();

void send(PACKET *packet);

void handleReceive(const uint8_t *mac, const uint8_t *data, int len);

void handleNetwork(const uint8_t *mac, const PACKET *packet);

void updateData();

void updateDisplay();

void handleEncoder();