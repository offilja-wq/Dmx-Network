#include <Arduino.h>

#include "function.h"
#include "config.h"

// local
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

DISPLAYMENU DisplayMenu;

uint8_t mac[6];

void begin()
{
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

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

    // Check band activatie
    if (esp_now_add_peer(&broadcastPeer) != ESP_OK)
    {
        return;
    }

    WiFi.macAddress(mac);

    // Zet data in register met activatie functie
    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len)
                             {handleReceive(mac, data, len); });
}

void send(PACKET *packet)
{
    esp_now_send(BROADCAST_ADDRESS, (uint8_t *)packet, sizeof(PACKET));
}

void handleReceive(const uint8_t *mac, const uint8_t *data, int len)
{
    if (sizeof(PACKET) != len)
    {
        return;
    }
    
    PACKET *packet = (PACKET *)data;
    handleNetwork(mac, packet);
}

void handleNetwork(const uint8_t *mac, const PACKET *packet)
{
	// handleResponseBand((InputData*)packet->data);
	// updateStrip((InputData*)packet->data);
}

void updateData()
{

}

void updateDisplay()
{
    unsigned long now = millis();
    unsigned long lastKnobActivate;

    bool knobHold;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);

    if (digitalRead(ENCODER_A_PIN))
    
    if (digitalRead(ENCODER_B_PIN))
    
    if (digitalRead(ENCODER_KNOB))
    {
        if (knobHold)
        {
            if (((now - lastKnobActivate) / 250) % 2)
            {
                display.println(DisplayMenu.mode ? F("TX") : F("RX"));
            }
            else
            {
                display.println(F(" "));
            }

            if ((now - lastKnobActivate) >= 3000)
            {
                DisplayMenu.mode = !DisplayMenu.mode;
                knobHold = false;
            }
        }
        else
        {
            knobHold = true;
            lastKnobActivate = now;
        }
    }
    else
    {
        knobHold = false;
        display.println(DisplayMenu.mode ? F("TX") : F("RX"));
    }

    if (DisplayMenu.liveDmxSignal)
    {
        display.print(F("ACTIVE"));
    }
    else
    {
        display.print(((now / 500) % 2) ? F("NO DATA") : F(" "));
    }
    
    display.drawRoundRect(((SCREEN_WIDTH)-(SCREEN_WIDTH / 3)), 0, (SCREEN_WIDTH / 3), SCREEN_HEIGHT, 4, SSD1306_WHITE);

    display.setTextSize(3);
    display.setCursor(100, 4);

    if (DisplayMenu.selectUniverse == DisplayMenu.liveUniverse)
    {
        display.print(DisplayMenu.liveUniverse);
    }
    else if ((now / 500) % 2)
    {
        display.print(DisplayMenu.selectUniverse);
    }

    display.display();
}

void handleEncoder()
{
}

