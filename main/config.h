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
    RECEIVER,
    TRANSMITTER
} MODE;

typedef struct
{
    MODE mode;
    uint8_t universe;
    uint8_t data[512];
} PACKET;

// MAX485
#include <DMXSerial.h>
// #include <uart.h>

// SSD1306
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
    uint8_t liveUniverse;
    uint8_t selectUniverse;
    bool liveDmxSignal;
} DISPLAYMENU;


// GPIO

// MAX485: RO(18), DI(17)
#define MAX485_MODE_PIN 6

#define ENCODER_A_PIN 27
#define ENCODER_B_PIN 26
#define ENCODER_KNOB 1

// SSD1306: SDA(33), SCL(35)