#include <Arduino.h>

#include "function.h"
#include "config.h"

// local
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

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

void send(Packet *packet)
{
    esp_now_send(BROADCAST_ADDRESS, (uint8_t *)packet, sizeof(Packet));
}

void handleReceive(const uint8_t *mac, const uint8_t *data, int len)
{
    if (sizeof(Packet) != len)
    {
        return;
    }
    
    Packet *packet = (Packet *)data;
    handleNetwork(mac, packet);
}

void handleNetwork(const uint8_t *mac, const Packet *packet)
{
	// handleResponseBand((InputData*)packet->data);
	// updateStrip((InputData*)packet->data);
}

void updateData()
{

}

void updateDisplay(MODE mode, uint8_t CHANNEL, bool dmxInput)
{
    
}

