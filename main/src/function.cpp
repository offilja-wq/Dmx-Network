#include <Arduino.h>

#include "function.h"
#include "config.h"

// local
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

DISPLAYMENU DisplayMenu;

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

    // Check band activatie
    if ((esp_now_add_peer(&broadcastPeer) != ESP_OK )|| (esp_now_init() != ESP_OK))
    {
        return;
    }

    WiFi.macAddress(mac);

    // Zet data in register met activatie functie
    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len)
                             {handleReceive(mac, data, len); });

    pinMode(MAX485_MODE_PIN, OUTPUT);

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

void setMode(MODE input)
{
    MODE Currentmode;

    if (Currentmode != input)
        return;
    
    digitalWrite(MAX485_MODE_PIN, input);

    DMXSerial.init(input ? DMXReceiver : DMXController);
}

void send(PACKET *packet)
{
    esp_now_send(BROADCAST_ADDRESS, (uint8_t *)packet, sizeof(PACKET));
}

void handleReceive(const uint8_t *mac, const uint8_t *data, int len)
{    
    if ((DisplayMenu.mode != RECEIVER) || (sizeof(PACKET) != len))
        return;
    
    PACKET *packet = (PACKET *)data;

    if ((packet->mode == TRANSMITTER) && (packet->universe == DisplayMenu.liveUniverse))
    {
        setMode(DisplayMenu.mode);

        for (int i = 1; i <= 512; i++)
        {
            DMXSerial.write(3, packet->data[i]);
        }
    }
}

void handleSend()
{
    if (DisplayMenu.mode != TRANSMITTER)
        return;
    
    digitalWrite(MAX485_MODE_PIN, DisplayMenu.mode);

    if (DisplayMenu.mode = RECEIVER)
    {
        for (int i = 0; i <= 512; i++)
        {

        }
    }
    else
    {

    }
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
    {

    }
    
    if (digitalRead(ENCODER_B_PIN))
    {
        
    }
    
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
                DisplayMenu.mode = DisplayMenu.mode ? RECEIVER : TRANSMITTER;
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

