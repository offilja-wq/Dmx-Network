#pragma once

#include <Arduino.h>

// ESP NOW
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

const uint8_t NETWORK_CHANNEL = 2;
const uint8_t BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef enum : bool
{
    SENDER,
    RECEIVER
} MODE;

typedef struct
{
    MODE mode;
    uint8_t universe;
    uint8_t data[512];
} Packet;

// MAX485
#define MAX485_MODE_PIN 2

// Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_ADDRESS 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32