/*
 * LIN Annex Slave Example - Respond to Master Commands
 * 
 * This example demonstrates LIN slave mode operation:
 * - Wait for messages from master
 * - Process received commands
 * - Send response data
 * 
 * Hardware Setup:
 * - TX pin: 1 (Arduino Uno hardware UART)
 * - RX pin: 0 (Arduino Uno hardware UART)
 * - Break pin: 3 (GPIO for LIN break generation)
 * - LIN transceiver connected to TX/RX
 * 
 * Note: Run the master example on another Arduino
 */

#include <LinAnnex.h>

// Create LIN slave instance with Serial, break pin 3
LINAnnex lin(Serial, 3, 13);

// Message IDs (must match master)
#define MSG_COMMAND     0x10  // Slave receives commands
#define MSG_STATUS      0x20  // Slave sends status
#define MSG_CONTROL     0x30  // Slave receives control

// Slave state variables
uint8_t slave_status[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
uint8_t pwm_level = 0;
bool slave_enabled = false;

void setup() {
    // Initialize debug serial (not the LIN serial)
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("LIN Annex Slave Example");
    Serial.println("----------------------");
    
    // Initialize as slave at 19200 baud
    uint16_t status = lin.beginSlave(19200);
    if (status != LIN_OK) {
        Serial.println("ERROR: Failed to initialize LIN slave!");
        Serial.print("Status: 0x");
        Serial.println(status, HEX);
        while (1);  // Halt
    }
    
    // Enable LIN 2.0 enhanced checksum
    lin.setEnhancedChecksum(true);
    
    Serial.println("LIN Slave initialized successfully!");
    Serial.println("Baud: 19200, Enhanced Checksum: Enabled");
    Serial.println("Waiting for master commands...");
    Serial.println();
}

void loop() {
    // Wait for message from master (2000ms timeout)
    LIN_Message msg;
    uint16_t result = lin.readMessageWait(&msg, 2000);
    
    if (result == LIN_OK) {
        Serial.print("<< Received message: ID=0x");
        Serial.print(msg.id, HEX);
        Serial.print(", Length=");
        Serial.print(msg.length);
        Serial.print(" bytes");
        
        if (msg.length > 0) {
            Serial.print(", Data: ");
            for (int i = 0; i < msg.length; i++) {
                Serial.print("0x");
                if (msg.data[i] < 0x10) Serial.print("0");
                Serial.print(msg.data[i], HEX);
                if (i < msg.length - 1) Serial.print(" ");
            }
        }
        Serial.println();
        
        // Process command based on message ID
        processCommand(&msg);
        
    } else if (result == LIN_TIMEOUT) {
        // Timeout - no message received, continue waiting
        Serial.println("(Waiting for master...)");
        
    } else {
        // Error occurred
        Serial.print("ERROR: Failed to read message (0x");
        Serial.print(result, HEX);
        Serial.println(")");
        
        // Print error details
        if (result & LIN_CHECKSUM_ERROR) {
            Serial.println("  - Checksum error detected");
        }
        if (result & LIN_PARITY_ERROR) {
            Serial.println("  - Parity error detected");
        }
        if (result & LIN_SYNC_ERROR) {
            Serial.println("  - Sync error detected");
        }
    }
}

/**
 * Process received command based on message ID
 */
void processCommand(LIN_Message *msg) {
    switch (msg->id) {
        case MSG_COMMAND:
            handleCommandMessage(msg);
            break;
            
        case MSG_STATUS:
            handleStatusRequest(msg);
            break;
            
        case MSG_CONTROL:
            handleControlMessage(msg);
            break;
            
        default:
            Serial.print("   WARNING: Unknown message ID 0x");
            Serial.println(msg->id, HEX);
            break;
    }
}

/**
 * Handle command message from master
 */
void handleCommandMessage(LIN_Message *msg) {
    Serial.println(">> Processing command message:");
    
    if (msg->length >= 1) {
        uint8_t cmd = msg->data[0];
        Serial.print("   Command byte: 0x");
        Serial.println(cmd, HEX);
        
        // Example: Decode command
        if (cmd == 0xAA) {
            Serial.println("   -> Action: Start operation");
            slave_enabled = true;
        } else if (cmd == 0x55) {
            Serial.println("   -> Action: Stop operation");
            slave_enabled = false;
        } else {
            Serial.println("   -> Action: Unknown command");
        }
        
        // Prepare and send acknowledgment if needed
        // (Master will send next message to fetch data)
    }
}

/**
 * Handle status request from master
 * Master has sent header, now we send response
 */
void handleStatusRequest(LIN_Message *msg) {
    Serial.println(">> Master requesting status, sending response:");
    
    // Update status buffer with current state
    slave_status[0] = slave_enabled ? 0x01 : 0x00;
    slave_status[1] = pwm_level;
    slave_status[2] = getSensorReading();
    slave_status[3] = getErrorFlags();
    
    // Send response
    uint16_t result = lin.sendResponse(slave_status, 4);
    
    if (result == LIN_OK) {
        Serial.print("   OK: Status sent - State=");
        Serial.print(slave_enabled ? "ON" : "OFF");
        Serial.print(", PWM=");
        Serial.print((100 * pwm_level) / 255);
        Serial.print("%, Sensor=");
        Serial.print(slave_status[2]);
        Serial.print(", Errors=0x");
        Serial.println(slave_status[3], HEX);
    } else {
        Serial.print("   ERROR: Failed to send response (0x");
        Serial.print(result, HEX);
        Serial.println(")");
    }
}

/**
 * Handle control message from master
 * Master sends mode, PWM, control flags, etc.
 */
void handleControlMessage(LIN_Message *msg) {
    Serial.println(">> Processing control message:");
    
    if (msg->length >= 3) {
        uint8_t mode = msg->data[0];
        pwm_level = msg->data[1];
        uint8_t enable = msg->data[2];
        
        Serial.print("   Mode: 0x");
        Serial.print(mode, HEX);
        Serial.print(", PWM: ");
        Serial.print((100 * pwm_level) / 255);
        Serial.print("%, Enable: ");
        Serial.println(enable ? "Yes" : "No");
        
        // Apply control settings
        applyControl(mode, pwm_level, enable);
        
        Serial.println("   -> Control settings applied");
    }
}

/**
 * Get simulated sensor reading
 */
uint8_t getSensorReading() {
    // Simulate sensor reading (0-255)
    static uint8_t reading = 100;
    reading += 5;  // Simulate changing value
    if (reading > 200) reading = 100;
    return reading;
}

/**
 * Get error flags
 */
uint8_t getErrorFlags() {
    // No errors in this example
    return 0x00;
}

/**
 * Apply control settings
 */
void applyControl(uint8_t mode, uint8_t pwm, uint8_t enable) {
    // In a real application, this would:
    // - Set GPIO output levels based on mode
    // - Adjust PWM output
    // - Enable/disable the device
    
    slave_enabled = (enable != 0);
    pwm_level = pwm;
    
    // Example: Set a GPIO output
    // digitalWrite(LED_PIN, slave_enabled ? HIGH : LOW);
    // analogWrite(PWM_PIN, pwm_level);
}

/**
 * Optional: Print slave status
 */
void printSlaveStatus() {
    Serial.println("\n>> LIN Slave Status:");
    Serial.print("   Mode: ");
    Serial.println(lin.isMaster() ? "Master" : "Slave");
    Serial.print("   Enabled: ");
    Serial.println(slave_enabled ? "Yes" : "No");
    Serial.print("   PWM Level: ");
    Serial.print((100 * pwm_level) / 255);
    Serial.println("%");
}
