#include <Arduino.h>

#include "config.h"
#include "function.h"

void setup()
{
    begin();
    // Serial.begin(115200);
}

void loop()
{
    handleSend();

    updateDisplay();
}