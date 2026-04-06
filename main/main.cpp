#include <Arduino.h>

#include "config.h"
#include "function.h"

void setup()
{
    begin();
}

void loop()
{
    handleSend();

    updateDisplay();
}