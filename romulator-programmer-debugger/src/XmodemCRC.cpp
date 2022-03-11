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
    uint8_t packet[133];

    bool useCRC = false;
    File fp = LittleFS.open(fname, "r");

    // wait until we see a 'C'
    char b = 0x00;
    while (b != 'C' && b != NAK) {
        Serial.readBytes(&b, 1);
    }

    if (b == 'C')
    {
        useCRC = true;
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

        if (useCRC) 
        {
            // generate crc
            int crc = calcrc((char*)&packet[3], 128);
            packet[131] = (crc & 0xFF00) >> 8;
            packet[132] = crc & 0x00FF;
            sendPacketUntilAck(packet, 133);
        }
        else
        {
            uint8_t checksum = calc_checksum((char*)&packet[3], 128);
            packet[131] = checksum;
            sendPacketUntilAck(packet, 132);
        }
        packetNumber++;
    }

    // done sending data, now end the transmission
    packet[0] = EOT;
    sendPacketUntilAck(packet, 1);

    packet[0] = ETB;
    sendPacketUntilAck(packet, 1);
}

void xmodemRecvFile(const char* fname)
{
    // testing
    File fp = LittleFS.open(fname, "w");

    uint8_t packet[133];

    packet[0] = 'C';
    Serial.write(packet, 1);
    int tries = 3;
    bool isCRC = false;
    while (tries > 0) 
    {
        if (Serial.readBytes(packet, 133) == 133)
        {
            isCRC = true;
            break;
        }
        else
        {
            tries--;
        }
    }

    if (!isCRC)
    {
        packet[0] = NAK;
        while (packet[0] != SOH)
        {
            Serial.write(packet, 1);
            Serial.readBytes(packet, 132);
        }
    }

    bool done = false;
    int bytesRead = 0;
    while (!done)
    {
        if (packet[0] == EOT)
        {
            packet[0] = ACK;
            Serial.write(packet, 1);
            done = true;
            continue;
        }

        if (isCRC) 
        {
            
        }
        else
        {
            uint8_t checksum = calc_checksum((char*)&packet[3], 128);
            // check checksum value
            if (checksum == packet[131])
            {
                // write to file
                bytesRead += 128;
                fp.write(&packet[3], 128);
                packet[0] = ACK;
                Serial.write(packet, 1);
            }
            else
            {
                packet[0] = NAK;
                Serial.write(packet, 1);
            }

            Serial.readBytes(packet, 132);
        }
    }

    fp.close();
    delay(5000);
    Serial.printf("read %d bytes\n", bytesRead);
}