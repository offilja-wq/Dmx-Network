#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

const uint8_t NETWORK_CHANNEL = 2;
const uint8_t BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct
{
    uint8_t identity;
    uint8_t data[512];
} Packet;