#include <Arduino.h>

#include "function.h"
#include "config.h"

// local
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

DISPLAYMENU DisplayMenu;

uint8_t mac[6];
bool knobHold;

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
    MODE Currentmode;

    if (Currentmode != input)
        return;
    
    digitalWrite(MAX485_MODE_PIN, input);

    DMXSerial.init(input ? DMXReceiver : DMXController);
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

    PACKET Newpacket;

    setMode(DisplayMenu.mode);
    
    Newpacket.mode = DisplayMenu.mode;
    Newpacket.universe = DisplayMenu.mode;

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
            if (((now * 2) / FLASH_RATE) % 2)
            {
                display.print(DisplayMenu.mode ? F("TX") : F("RX"));
            }
            // digitalWrite(15, (((now * 2) / FLASH_RATE) % 2));
        
            if ((now - lastKnobActivate) > 3000)
            {
                DisplayMenu.mode = (DisplayMenu.mode == TRANSMITTER) ? RECEIVER : TRANSMITTER;
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
        display.print(DisplayMenu.mode ? F("TX") : F("RX"));
    }

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

