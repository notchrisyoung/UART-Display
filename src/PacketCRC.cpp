#include "PacketCRC.h"

PacketCRC::PacketCRC(const uint8_t& polynomial, const uint8_t& crcLen){
    poly      = polynomial;
    crcLen_   = crcLen;
    tableLen_ = pow(2, crcLen);
    csTable   = new uint8_t[tableLen_];

    generateTable();
}

PacketCRC::~PacketCRC(){
    delete[] csTable;
}

void PacketCRC::generateTable(){
    for (int i = 0; i < tableLen_; ++i){
        int curr = i;
        for (int j = 0; j < 8; ++j){
            if ((curr & 0x80) != 0)
                curr = (curr << 1) ^ (int)poly;
            else
                curr <<= 1;
        }
        csTable[i] = (byte)curr;
    }
}

void PacketCRC::printTable()
{
    for (int i = 0; i < tableLen_; i++){
        Serial.print(csTable[i], HEX);
        if ((i + 1) % 16)
            Serial.print(' ');
        else
            Serial.println();
    }
}

uint8_t PacketCRC::calculate(const uint8_t& val){
    if (val < tableLen_)
        return csTable[val];
    return 0;
}

uint8_t PacketCRC::calculate(uint8_t arr[], uint16_t len){
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
        crc = csTable[crc ^ arr[i]];
    return crc;
}

uint8_t PacketCRC::calculate(const uint8_t arr[], uint16_t len){
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
        crc = csTable[crc ^ arr[i]];
    return crc;
}