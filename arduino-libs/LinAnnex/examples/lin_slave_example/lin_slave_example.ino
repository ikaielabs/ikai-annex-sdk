/*
 * LIN Annex Slave Example - Listen for a Master Frame
 *
 * This example waits for a LIN message from the master. When the received
 * payload matches the expected sequence, the onboard UNO LED is turned on.
 *
 * Hardware Setup:
 * - TX pin: 1 (Arduino Uno hardware UART)
 * - RX pin: 0 (Arduino Uno hardware UART)
 * - LIN transceiver connected to TX/RX
 */

#include <LinAnnex.h>

// Create LIN slave instance with Serial, tx pin 1, break width 13 bits
LINAnnex lin(Serial, 1, 0, 13);

// Message ID used by the master example
#define MSG_COMMAND 0x17U

// Expected payload from the master
static const uint8_t expected_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(10, OUTPUT);

    uint16_t status = lin.beginSlave(10000);
    if (status != LIN_OK) {
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }

    lin.setEnhancedChecksum(false);
}

void loop() {
    LIN_Message msg;
    uint16_t result = lin.readMessageWait(&msg, 8, 2000);

    if (result == LIN_OK && msg.id == MSG_COMMAND && msg.length == sizeof(expected_data)) {
        bool match = true;
        for (uint8_t i = 0; i < msg.length; ++i) {
            if (msg.data[i] != expected_data[i]) {
                match = false;
                break;
            }
        }

        digitalWrite(LED_BUILTIN, match ? HIGH : LOW);
    }
}
