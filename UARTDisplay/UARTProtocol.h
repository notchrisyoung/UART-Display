#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <Arduino.h>

#define UART_BUFFER_SIZE 128
#define START_BYTE 0x02
#define END_BYTE   0x03

class UARTProtocol {
public:
    UARTProtocol(){};
    void begin(unsigned long baud = 115200) { Serial.begin(baud); }
    void process();
    void sendMessage(const uint8_t* data, uint8_t len);

    void setCallback(void (*cb)(const uint8_t*, uint8_t));

private:

    enum State { WAIT_START, READ_LENGTH, READ_DATA, READ_CHK, WAIT_END } state;
    uint8_t buffer[UART_BUFFER_SIZE];
    uint8_t length;
    uint8_t index;
    uint8_t checksum;

    uint8_t calcChecksum(const uint8_t* data, uint8_t len);
    void receiveByte(uint8_t byte);
    void processMessage(const uint8_t* data, uint8_t len);
    void processMessageOut(const uint8_t* data, uint8_t len);

    void (*messageCallback)(const uint8_t*, uint8_t) = nullptr;
};

#endif