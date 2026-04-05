#pragma once

#include <Arduino.h>

#include "config.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

void begin();

void send(Packet *packet);

void handleReceive(const uint8_t *mac, const uint8_t *data, int len);

void handleNetwork(const uint8_t *mac, const Packet *packet);

char *macToString(const uint8_t *mac);

void stringToMac(const char *str, uint8_t *mac);

char *packetToString(const void *packet, size_t size);