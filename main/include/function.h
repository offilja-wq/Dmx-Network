#pragma once

#include <Arduino.h>

#include "config.h"

// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <esp_now.h>

void begin();

void send(Packet *packet);

void handleReceive(const uint8_t *mac, const uint8_t *data, int len);

void handleNetwork(const uint8_t *mac, const Packet *packet);

void updateData();

void updateDisplay(MODE mode, uint8_t CHANNEL, bool dmxInput);