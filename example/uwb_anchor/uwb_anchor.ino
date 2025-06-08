/*

For ESP32 UWB Pro with Display

*/

#include <SPI.h>
#include "DW1000Ranging.h"

#include <Wire.h>

#define ANCHOR_ADD "86:17:5B:D5:A9:9A:E2:9C"

 // connection pins
const uint8_t SPI_SCK  = 6;
const uint8_t SPI_MISO = 7;
const uint8_t SPI_MOSI = 8;
const uint8_t PIN_RST  = 2;
const uint8_t PIN_IRQ  = 5;
const uint8_t PIN_SS   = 9;

void setup()
{
    Serial.begin(115200);

    delay(1000);

    // init the configuration
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); // Reset, CS, IRQ pin

    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachBlinkDevice(newBlink);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_FAST_ACCURACY, false);
}

void loop()
{
    DW1000Ranging.loop();
}

void newRange()
{
    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}
