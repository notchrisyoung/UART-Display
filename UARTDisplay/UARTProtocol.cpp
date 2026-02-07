
#include "UARTProtocol.h"

uint8_t UARTProtocol::calcChecksum(const uint8_t* data, uint16_t len) {
    return crc.calculate(data, len);
}


void UARTProtocol::receiveByte(uint8_t byte) {
    switch (state) {
        case WAIT_START:
            if (byte == START_BYTE) {
                state = READ_LENGTH;
                index = 0;
            }
            break;

        case READ_LENGTH:
            buffer[index++] = byte;
            if (index >= 2) {
                length = (buffer[0] << 8) | buffer[1];
                index = 0;
                state = READ_DATA;
            }
            break;

        case READ_DATA:
            if (index < UART_BUFFER_SIZE) {
                buffer[index++] = byte;
            }
            if (index >= length) {
                state = READ_CHK;
            }
            break;

        case READ_CHK:
            checksum = byte;
            state = WAIT_END;
            break;

        case WAIT_END:
            if (byte == END_BYTE && checksum == calcChecksum(buffer, length)) {
                Serial.write(MESSAGE_OK);
                processMessage(buffer, length);
            } else {
                Serial.write(MESSAGE_ERR);
            }
            state = WAIT_START;
            break;
    }
}

void UARTProtocol::sendMessage(const uint8_t* data, uint16_t len) {
    uint8_t chk = calcChecksum(data, len);
    Serial.write(START_BYTE);
    Serial.write((len >> 8) & 0xFF);
    Serial.write(len & 0xFF); 
    Serial.write(data, len);
    Serial.write(chk);
    Serial.write(END_BYTE);
}

void UARTProtocol::process() {
    while (Serial.available()) {
        receiveByte(Serial.read());
    }
}

void UARTProtocol::processMessage(const uint8_t* data, uint8_t len) {
    if (messageCallback) {
        messageCallback(data, len);
    }
}

void UARTProtocol::setCallback(void (*cb)(const uint8_t*, uint8_t)) {
    messageCallback = cb;
}
