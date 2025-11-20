
#include "UARTProtocol.h"



uint8_t UARTProtocol::calcChecksum(const uint8_t* data, uint8_t len) {
    uint8_t chk = 0;
    for (uint8_t i = 0; i < len; ++i) {
        chk ^= data[i];
    }
    return chk;
}
void UARTProtocol::receiveByte(uint8_t byte) {
    switch (state) {
        case WAIT_START:
            if (byte == START_BYTE) state = READ_LENGTH;
            break;

        case READ_LENGTH:
            length = byte;
            index = 0;
            state = READ_DATA;
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
                processMessage(buffer, length);
            }
            state = WAIT_START;
            break;
    }
}
void UARTProtocol::processMessageOut(const uint8_t* data, uint8_t len) {
    // Example: Echo back with "OK: " prefix
    const char* prefix = "OK: ";
    uint8_t out[UART_BUFFER_SIZE];
    uint8_t prefixLen = strlen(prefix);

    memcpy(out, prefix, prefixLen);
    memcpy(out + prefixLen, data, len);

    sendMessage(out, prefixLen + len);
}

void UARTProtocol::sendMessage(const uint8_t* data, uint8_t len) {
    uint8_t chk = calcChecksum(data, len);
    Serial.write(START_BYTE);
    Serial.write(len);
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
