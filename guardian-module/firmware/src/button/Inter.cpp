#include "Inter.h"
#include <Adafruit_MCP23X17.h>

extern Adafruit_MCP23X17 mcp;

Inter::Inter(int gpio)
{
    pin = gpio;
}
void Inter::init()
{
    mcp.pinMode(pin, INPUT_PULLUP);
}
Inter::Mode Inter::getMode()
{
    if (mcp.digitalRead(pin) == LOW)
        return Inter::LOCAL;
    else
        return Inter::REMOTE;
}
