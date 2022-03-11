#include "XmodemCRC.h"
#include <Arduino.h>
#include <LittleFS.h>

#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define ETB 0x17
#define CAN 0x18

int calcrc(char *ptr, int count)
{
    int  crc;
    char i;

    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while(--i);
    }
    return (crc);
}

void sendPacketUntilAck(uint8_t* packet, int size)
{
    char b = 0;
    bool done = false;
    while (!done) 
    {
        Serial.write(packet, size);
        while (b != ACK && b != NAK)
        {
            Serial.readBytes(&b, 1);
        }
        if (b == ACK)
        {
            done = true;
        }
    }
}

void xmodemSendFile(const char* fname)
{
    uint8_t packet[133];

    File fp = LittleFS.open(fname, "r");

    // wait until we see a 'C'
    char b = 0x00;
    while (b != 'C') {
        Serial.readBytes(&b, 1);
    }

    // receiver indicated ready to receive
    // send packets
    bool done = false;
    uint8_t packetNumber = 1;
    while (!done) 
    {
        // read next packet
        memset(&packet[3], 0, 128);
        size_t bytesRead = fp.readBytes((char*)&packet[3], 128);
        if (bytesRead == 0)
        {
            done = true;
            continue;
        }

        // send SOH (start of header)
        packet[0] = SOH;
        packet[1] = packetNumber;
        packet[2] = ~packetNumber;

        // generate crc
        int crc = calcrc((char*)&packet[3], 128);
        packet[131] = (crc & 0xFF00) >> 8;
        packet[132] = crc & 0x00FF;

        sendPacketUntilAck(packet, 133);
    }

    // done sending data, now end the transmission
    packet[0] = EOT;
    sendPacketUntilAck(packet, 1);

    packet[0] = ETB;
    sendPacketUntilAck(packet, 1);
}

void xmodemRecvFile(const char* fname)
{

}