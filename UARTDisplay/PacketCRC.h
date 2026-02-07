#ifndef PACKET_CRC_H
#define PACKET_CRC_H

#include <Arduino.h>

class PacketCRC
{
public:
    uint8_t poly = 0;

    PacketCRC(const uint8_t& polynomial = 0x9B, const uint8_t& crcLen = 8);
    ~PacketCRC();

    void generateTable();
    void printTable();
    uint8_t calculate(const uint8_t& val);
    uint8_t calculate(uint8_t arr[], uint16_t len);
    uint8_t calculate(const uint8_t arr[], uint16_t len);

private:
    uint16_t tableLen_;
    uint8_t  crcLen_;
    uint8_t* csTable;
};

#endif
