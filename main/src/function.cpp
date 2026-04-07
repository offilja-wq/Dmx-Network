#include <Arduino.h>

#include "config.h"
#include "function.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
DISPLAYMENU DisplayMenu;

MODE currentMode;
uint8_t mac[6];

void begin()
{
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(NETWORK_CHANNEL, WIFI_SECOND_CHAN_NONE);
    WiFi.setSleep(false);
    
    if (esp_now_init() != ESP_OK)
    {
        return;
    }

    esp_now_peer_info_t broadcastPeer = {};
    broadcastPeer.channel = NETWORK_CHANNEL;
    broadcastPeer.encrypt = false;
    memcpy(broadcastPeer.peer_addr, BROADCAST_ADDRESS, sizeof(BROADCAST_ADDRESS));

    if ((esp_now_add_peer(&broadcastPeer) != ESP_OK )|| (esp_now_init() != ESP_OK))
    {
        return;
    }

    WiFi.macAddress(mac);

    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len)
                             {handleReceive(mac, data, len); });


    // uart_set_pin(1, MAX485_DI, MAX485_RO, 18, 19);

    pinMode(MAX485_MODE_PIN, OUTPUT);
    pinMode(15, OUTPUT);

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

void setMode(MODE input)
{
    if (currentMode == input)
        return;
    
    digitalWrite(MAX485_MODE_PIN, input);

    // // DMXSerial.init(input ? DMXReceiver : DMXController);

    currentMode = input;
}

void switchMode()
{    
    // MODE Newmode = (TRANSMITTER == currentMode ? RECEIVER : TRANSMITTER);

    digitalWrite(MAX485_MODE_PIN, (TRANSMITTER == currentMode ? RECEIVER : TRANSMITTER));

    DMXSerial.init(TRANSMITTER == currentMode ? DMXReceiver : DMXController);

    currentMode = (TRANSMITTER == currentMode ? RECEIVER : TRANSMITTER);
}

void handleReceive(const uint8_t *mac, const uint8_t *data, int len)
{    
    if ((currentMode != RECEIVER) || (sizeof(PACKET) != len))
        return;
    
    PACKET *packet = (PACKET *)data;

    if ((packet->mode == TRANSMITTER) && (packet->universe == DisplayMenu.liveUniverse))
    {
        for (int i = 1; i <= 512; i++)
        {
            DMXSerial.write(3, packet->data[i]);
        }
    }
}

void handleSend()
{
    if (currentMode != TRANSMITTER)
        return;

    PACKET Newpacket;
    
    Newpacket.mode = currentMode;
    Newpacket.universe = DisplayMenu.liveUniverse;

    for (int i = 1; i <= 512; i++)
    {
        Newpacket.data[i] = DMXSerial.read(i);
    }

    send(&Newpacket);
}

void send(PACKET *packet)
{
    esp_now_send(BROADCAST_ADDRESS, (uint8_t *)packet, sizeof(PACKET));
}

void updateDisplay()
{
    unsigned long now = millis();
    unsigned long lastKnobActivate;

    bool readyForPush;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);

    if (digitalRead(ENCODER_A_PIN))
    {

    }
    
    if (digitalRead(ENCODER_B_PIN))
    {
        
    }

    if (digitalRead(ENCODER_KNOB))
    {
        if (readyForPush)
        {
            lastKnobActivate = now;
            readyForPush = false;
        }
        else
        {
            if ((((now - lastKnobActivate) * 4) / FLASH_RATE) % 2)
            {
                display.print(currentMode ? F("TX") : F("RX"));
            }

            if ((now - lastKnobActivate) > 3000)
            {
                switchMode();

                readyForPush = true;
            }
        }
    }
    else
    {
        readyForPush = true;
        display.print(currentMode ? F("TX") : F("RX"));
    }

    digitalWrite(15, (((now * 2) / FLASH_RATE) % 2));

    display.setCursor(0, 16);

    if (DisplayMenu.liveDmxSignal)
    {
        display.print(F("ACTIVE"));
    }
    else if ((now / 500) % 2)
    {
        display.print(F("NO DATA"));
    }
    
    display.drawRoundRect(((SCREEN_WIDTH)-(SCREEN_WIDTH / 3)), 0, (SCREEN_WIDTH / 3), SCREEN_HEIGHT, 4, SSD1306_WHITE);

    display.setTextSize(3);
    display.setCursor(100, 4);

    if (DisplayMenu.selectUniverse == DisplayMenu.liveUniverse)
    {
        display.print(DisplayMenu.liveUniverse);
    }
    else if ((now / FLASH_RATE) % 2)
    {
        display.print(DisplayMenu.selectUniverse);
    }

    display.display();
}