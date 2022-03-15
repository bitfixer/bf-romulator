#include "xmodem.h"
#include <Arduino.h>
#include <LittleFS.h>

#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define ETB 0x17
#define CAN 0x18

uint8_t calc_checksum(char* ptr, int count)
{
    uint8_t checksum = 0;
    for (int i = 0; i < count; i++)
    {
        checksum += ptr[i];
    }
    return checksum;
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
    uint8_t packet[132];

    File fp = LittleFS.open(fname, "r");

    // wait until we see a NAK
    char b = 0x00;
    while (b != NAK) {
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
       
        uint8_t checksum = calc_checksum((char*)&packet[3], 128);
        packet[131] = checksum;
        sendPacketUntilAck(packet, 132);
        packetNumber++;
    }

    // done sending data, now end the transmission
    packet[0] = EOT;
    sendPacketUntilAck(packet, 1);
}

void xmodemRecvFile(const char* fname)
{
    // testing
    File fp = LittleFS.open(fname, "w");
    uint8_t packet[132];
    memset(packet, 0, 132);

    // keep sending 
    while (packet[0] != SOH)
    {
        packet[0] = NAK;
        Serial.write(packet, 1);
        Serial.readBytes(packet, 132);
    }

    bool done = false;
    int bytesRead = 0;
    int repeatedPackets = 0;
    uint8_t prevPacketNumber = 0;
    uint8_t expectedPacketNumber = 1;
    while (!done)
    {
        if (packet[0] == EOT)
        {
            packet[0] = ACK;
            Serial.write(packet, 1);
            done = true;
            continue;
        }

        // first check if expected packet number matches
        uint8_t packetNum = packet[1];
        uint8_t invPacketNum = packet[2];
        uint8_t checksum = calc_checksum((char*)&packet[3], 128);
        // check checksum value
        if (checksum == packet[131] && packetNum + invPacketNum == 0xFF)
        {
            if (packetNum == expectedPacketNumber)
            {
                // write to file
                bytesRead += 128;
                fp.write(&packet[3], 128);
                packet[0] = ACK;
                Serial.write(packet, 1);

                prevPacketNumber = packetNum;
                expectedPacketNumber = prevPacketNumber + 1;
            }
            else if (packetNum == prevPacketNumber)
            {
                // last packet repeated, ack and continue
                repeatedPackets++;
                packet[0] = ACK;
                Serial.write(packet, 1);
            }
        }
        else
        {
            packet[0] = NAK;
            Serial.write(packet, 1);
        }

        Serial.readBytes(packet, 132);
    }

    fp.close();
    delay(1000);
    Serial.printf("read %d bytes, repeated packets %d\r\n", bytesRead, repeatedPackets);
}