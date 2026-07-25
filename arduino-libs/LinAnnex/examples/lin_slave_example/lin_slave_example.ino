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
#include <SoftwareSerial.h>

SoftwareSerial linAnnexSerial(10, 11); // RX, TX -> For prints

// Create LIN slave instance with Serial, tx pin 1, break width 13 bits
LINAnnex lin(Serial, 1, 0, 13);

// Message ID used by the master example
#define MSG_COMMAND 0x17U

// Expected payload from the master
static const uint8_t expected_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

void setup() {

    linAnnexSerial.begin(115200);

    linAnnexSerial.println("Lin Responder Start");
    uint16_t status = lin.beginSlave(10000);
    if (status != LIN_OK) {
        linAnnexSerial.println("Lin Init Failed");
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

        linAnnexSerial.print("Received Data: ");
        for (uint8_t i = 0; i < msg.length; i++) {
            linAnnexSerial.print(msg.data[i], HEX); // Print the number without starting a new line
            if (i < msg.length - 1) {
                linAnnexSerial.print(", ");
            }
        }
        linAnnexSerial.println();
        if(match) {
            linAnnexSerial.println("Received Data Matched: OK");
        } else {
            linAnnexSerial.println("Received Data not Matched: FAIL");
        }
    }
}
