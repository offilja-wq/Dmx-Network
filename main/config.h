#pragma once

#include <Arduino.h>

// ESP NOW
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

const uint8_t NETWORK_CHANNEL = 2;
const uint8_t BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct
{
    bool mode; // 0: RECEIVER 1:TRANSMITTER - dont use an enum!
    uint8_t universe;
    uint8_t data[512];
} PACKET;

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

#define FLASH_RATE 500

typedef struct
{
    bool mode; // 0: RECEIVER 1:TRANSMITTER - dont use an enum!
    uint8_t liveUniverse;
    uint8_t selectUniverse;
    bool liveDmxSignal;
} DISPLAYMENU;

// Rotary encoder
#define ENCODER_A_PIN 50
#define ENCODER_B_PIN 51
#define ENCODER_KNOB 52
