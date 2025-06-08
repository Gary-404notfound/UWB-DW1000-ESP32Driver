#include <Arduino.h>

#include <SPI.h>
#include "DW1000Ranging.h"

#define TAG_ADDR "7D:00:22:EA:82:60:3B:9B"

 // connection pins
const uint8_t SPI_SCK  = 6;
const uint8_t SPI_MISO = 7;
const uint8_t SPI_MOSI = 8;
const uint8_t PIN_RST  = 2;
const uint8_t PIN_IRQ  = 5;
const uint8_t PIN_SS   = 9;
long int runtime = 0;

struct Link
{
    uint16_t anchor_addr;
    float range;
    float dbm;
    struct Link *next;
};

struct Link *uwb_data;


struct Link *init_link()
{
#ifdef DEBUG
    Serial.println("init_link");
#endif
    struct Link *p = (struct Link *)malloc(sizeof(struct Link));
    p->next = NULL;
    p->anchor_addr = 0;
    p->range = 0.0;

    return p;
}

void add_link(struct Link *p, uint16_t addr)
{
#ifdef DEBUG
    Serial.println("add_link");
#endif
    struct Link *temp = p;
    // Find struct Link end
    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    Serial.println("add_link:find struct Link end");
    // Create a anchor
    struct Link *a = (struct Link *)malloc(sizeof(struct Link));
    a->anchor_addr = addr;
    a->range = 0.0;
    a->dbm = 0.0;
    a->next = NULL;

    // Add anchor to end of struct Link
    temp->next = a;

    return;
}

struct Link *find_link(struct Link *p, uint16_t addr)
{
#ifdef DEBUG
    Serial.println("find_link");
#endif
    if (addr == 0)
    {
        Serial.println("find_link:Input addr is 0");
        return NULL;
    }

    if (p->next == NULL)
    {
        Serial.println("find_link:Link is empty");
        return NULL;
    }

    struct Link *temp = p;
    // Find target struct Link or struct Link end
    while (temp->next != NULL)
    {
        temp = temp->next;
        if (temp->anchor_addr == addr)
        {
            // Serial.println("find_link:Find addr");
            return temp;
        }
    }

    Serial.println("find_link:Can't find addr");
    return NULL;
}

void fresh_link(struct Link *p, uint16_t addr, float range, float dbm)
{
#ifdef DEBUG
    Serial.println("fresh_link");
#endif
    struct Link *temp = find_link(p, addr);
    if (temp != NULL)
    {

        temp->range = range;
        temp->dbm = dbm;
        return;
    }
    else
    {
        Serial.println("fresh_link:Fresh fail");
        return;
    }
}

void print_link(struct Link *p)
{
#ifdef DEBUG
    Serial.println("print_link");
#endif
    struct Link *temp = p;

    while (temp->next != NULL)
    {
        // Serial.println("Dev %d:%d m", temp->next->anchor_addr, temp->next->range);
        Serial.println(temp->next->anchor_addr, HEX);
        Serial.println(temp->next->range);
        Serial.println(temp->next->dbm);
        temp = temp->next;
    }

    return;
}

void delete_link(struct Link *p, uint16_t addr)
{
#ifdef DEBUG
    Serial.println("delete_link");
#endif
    if (addr == 0)
        return;

    struct Link *temp = p;
    while (temp->next != NULL)
    {
        if (temp->next->anchor_addr == addr)
        {
            struct Link *del = temp->next;
            temp->next = del->next;
            free(del);
            return;
        }
        temp = temp->next;
    }
    return;
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

    fresh_link(uwb_data, DW1000Ranging.getDistantDevice()->getShortAddress(), DW1000Ranging.getDistantDevice()->getRange(), DW1000Ranging.getDistantDevice()->getRXPower());
}

void newDevice(DW1000Device *device)
{
    Serial.print("ranging init; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);

    add_link(uwb_data, device->getShortAddress());
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);

    delete_link(uwb_data, device->getShortAddress());
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("begin config");
    //init the configuration
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    // SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
    //define the sketch as anchor. It will be great to dynamically change the type of module
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    //Enable the filter to smooth the distance
    //DW1000Ranging.useRangeFilter(true);

    //we start the module as a tag
    DW1000Ranging.startAsTag(TAG_ADDR, DW1000.MODE_LONGDATA_FAST_ACCURACY, false);
    Serial.println("end config");
    uwb_data = init_link();
}

void loop()
{
    DW1000Ranging.loop();
    // if ((millis() - runtime) > 1000)
    // {
    //     display_uwb(uwb_data);
    //     runtime = millis();
    // }
}

void display_uwb(struct Link *p)
{
    struct Link *temp = p;
    if (temp->next == NULL){
        return;
    };
    while (temp->next != NULL){
        temp = temp->next;

        // Serial.println("Dev %d:%d m", temp->next->anchor_addr, temp->next->range);
        Serial.println(temp->anchor_addr, HEX);
        Serial.println(temp->range);
    }
}
