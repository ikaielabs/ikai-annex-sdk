#include "LinAnnex.h"
#include <Arduino.h>

// LIN Protocol constants
#define LIN_SYNC_BYTE       0x55
#define LIN_MAX_FRAME_ID    0x3F
#define LIN_MAX_DATA_LEN    8
#define LIN_MIN_TIMEOUT_MS  1

/**
 * @brief Constructor - Initialize LINAnnex object
 * @param serial Reference to HardwareSerial object
 * @param tx_pin TX pin for break generation
 * @param break_width Break width in bit times (13-15 typical)
 */
LINAnnex::LINAnnex(HardwareSerial &serial, uint8_t tx_pin, uint8_t rx_pin, uint8_t break_width)
    : _serial(serial),
      _tx_pin(tx_pin),
      _rx_pin(rx_pin),
      _break_width(break_width),
      _baudrate(9600),
      _flags(0),
      _last_status(LIN_OK),
      _last_request_id(0),
      _initialized(false),
      _break_detected(false) {
}

/**
 * @brief Compute protected ID with parity bits (LIN 1.x/2.0)
 * Parity bits P0 and P1 are calculated based on the 6-bit ID
 */
uint8_t LINAnnex::computeProtectedId(uint8_t id) {
    uint8_t p0 = ((id >> 0U) ^ (id >> 1U) ^ (id >> 2U) ^ (id >> 4U)) & 0x01;
    uint8_t p1 = (~((id >> 1U) ^ (id >> 3U) ^ (id >> 4U) ^ (id >> 5U)) & 0x01);
    return (uint8_t)(id | (p0 << 6) | (p1 << 7));
}

/**
 * @brief Validate protected ID and extract raw ID
 */
bool LINAnnex::validateProtectedId(uint8_t pid, uint8_t *id) {
    if (!id) return false;
    uint8_t raw = pid & 0x3F;
    uint8_t expected = computeProtectedId(raw);
    if (expected != pid) {
        _last_status |= LIN_PARITY_ERROR;
        return false;
    }
    *id = raw;
    return true;
}

/**
 * @brief Compute LIN checksum (classic or enhanced)
 * Classic: sum of data bytes + inversion
 * Enhanced: sum of data bytes + ID + inversion
 */
uint8_t LINAnnex::computeChecksum(uint8_t id, const uint8_t *data, uint8_t length, bool enhanced) {
    uint16_t sum = 0;

    if (enhanced)
        sum += id;

    for (uint8_t i = 0; i < length; i++)
        sum += data[i];

    // Checksum calculation with carry propagation
    sum = (uint8_t)(sum & 0xFF) + (uint8_t)(sum >> 8);
    sum = (uint8_t)(sum & 0xFF) + (uint8_t)(sum >> 8);

    return (uint8_t)(~sum);
}

/**
 * @brief Safe microsecond delay (handles values > 16383 for 16MHz Arduino)
 */
void LINAnnex::delayMicroseconds_Safe(uint32_t us) {
    uint32_t milliseconds = us / 1000;
    uint32_t microseconds = us % 1000;
    
    if (milliseconds > 0) {
        delay(milliseconds);
    }
    if (microseconds > 0) {
        delayMicroseconds(microseconds);
    }
}

/**
 * @brief Send LIN break signal
 * Sets TX pin low for break duration, then releases
 */
void LINAnnex::sendBreak() {
    // Calculate break duration in microseconds
    // Break = 13 bits at specified baudrate
    uint32_t break_us = (uint32_t)(((_break_width * 1000000UL) / _baudrate) + 1);
    
    // Flush any pending data
    _serial.flush();

    // End Serial communication 
    _serial.end();

    // GPIO to take control of TX pin to generate break condition.
    // Set TX pin as output and pull low for break
    pinMode(_tx_pin, OUTPUT);
    digitalWrite(_tx_pin, LOW);
    
    delayMicroseconds_Safe(break_us);
    
    // Release TX pin (go high)
    digitalWrite(_tx_pin, HIGH);
    
    // Delimiter: one bit time high
    uint32_t bit_us = (1000000UL / _baudrate) + 1;
    delayMicroseconds_Safe(bit_us);
    
    // Let UART take over TX line, Start/Restart the UART
    _serial.begin(_baudrate);
}

/**
 * @brief Detect LIN break signal
 * Monitor RX pin for break detection, then releases
 */
void LINAnnex::detectBreak() {
    // Calculate break duration in microseconds
    // Break = 13 bits at specified baudrate
    uint32_t break_us = (uint32_t)(((_break_width * 1000000UL) / _baudrate) + 1);
    
    // Flush any pending data
    _serial.flush();

    // End Serial communication 
    _serial.end();

    while (!_break_detected) {

        // GPIO to take control of TX pin to generate break condition.
        // Set TX pin as output and pull low for break
        pinMode(_rx_pin, INPUT);

        while(digitalRead(_rx_pin) == HIGH);

        uint32_t start = micros();
        
        while(digitalRead(_rx_pin) == LOW);

        // TODO: Actual bit time checking for exact break period. Currently not working
        // Simple HIGH to LOW transition is checked.
        // uint32_t low_duration_us = (micros() - start);
        // if((low_duration_us - 100) >= break_us)
        _break_detected = true;

    }
    
    // Let UART take over RX line, Start/Restart the UART
    _serial.begin(_baudrate);
}

/**
 * @brief Send LIN frame header (sync byte + protected ID)
 */
uint16_t LINAnnex::sendHeader(uint8_t id) {
    if (id > LIN_MAX_FRAME_ID) {
        return (_last_status = LIN_ERROR);
    }

    // Send sync byte
    _serial.write(LIN_SYNC_BYTE);
    
    // Send protected ID
    uint8_t pid = computeProtectedId(id);
    _serial.write(pid);
    
    return (_last_status = LIN_OK);
}

/**
 * @brief Read bytes from serial with timeout
 */
uint16_t LINAnnex::readBytes(uint8_t *buffer, uint8_t length, uint32_t timeout_ms) {
    if (!buffer || length == 0) {
        return (_last_status = LIN_ERROR);
    }

    uint32_t start = millis();
    uint8_t received = 0;

    while (received < length) {
        int c = _serial.read();
        if (c >= 0) {
            buffer[received++] = (uint8_t)c;
            start = millis();      // reset timeout on each successful byte (see note below)
            continue;
        }

        if (timeout_ms > 0 && (millis() - start) >= timeout_ms) {
            _last_status |= LIN_TIMEOUT;
            break;
        }
    }

    if (received == 0) {
        return (_last_status = LIN_TIMEOUT);
    }
    if (received < length) {
        return (_last_status = LIN_TIMEOUT); // partial frame
    }

    return (_last_status = LIN_OK);
}

/**
 * @brief Read LIN frame header (sync + protected ID)
 */
uint16_t LINAnnex::readHeader(LIN_Message *msg, uint32_t timeout_ms) {
    if (!msg) {
        return (_last_status = LIN_ERROR);
    }

    uint8_t byte = 0;
    uint8_t sync_count = 0;

    // Look for sync byte
    uint32_t start = millis();
    while (byte != LIN_SYNC_BYTE) {
        if (readBytes(&byte, 1, 10) != LIN_OK) {
            if (sync_count++ > 100) {
                return (_last_status = LIN_SYNC_ERROR);
            }
            continue;
        }
        if ((millis() - start) > timeout_ms && timeout_ms > 0) {
            return (_last_status = LIN_TIMEOUT);
        }
    }

    // Read protected ID
    uint8_t pid = 0;
    if (readBytes(&pid, 1, 100) != LIN_OK) {
        return (_last_status = LIN_PARITY_ERROR);
    }

    // Validate and extract ID
    uint8_t raw_id = 0;
    if (!validateProtectedId(pid, &raw_id)) {
        return (_last_status = LIN_PARITY_ERROR);
    }

    msg->id = raw_id;
    msg->length = 0;
    msg->status = LIN_NO_DATA;
    _last_request_id = raw_id;
    return (_last_status = LIN_OK);
}

/**
 * @brief Read LIN response (data + checksum)
 */
uint16_t LINAnnex::readResponse(LIN_Message *msg, uint8_t dlc, uint32_t timeout_ms) {
    if (!msg) {
        return (_last_status = LIN_ERROR);
    }

    uint8_t buffer[9] = {0};
    uint32_t received = 0;
    uint8_t total_len = dlc + 1U;
    
    // Read (dlc + 1 (checksum)) byte
    uint32_t start = millis();
    while (received < total_len) {
        if (readBytes(&(buffer[received++]), 1, 10) != LIN_OK) {
            continue;
        }

        if (timeout_ms > 0 && (millis() - start) >= timeout_ms) {
            break;
        }
    }

    digitalWrite(10, LOW);

    if (received < 2) {
        msg->status = LIN_NO_DATA;
        return (_last_status = LIN_NO_DATA);
    }

    // Extract data length (last byte is checksum)
    uint8_t data_len = received - 1;
    if (data_len > LIN_MAX_DATA_LEN) {
        data_len = LIN_MAX_DATA_LEN;
    }

    // Verify checksum
    uint8_t checksum = buffer[received - 1];
    uint8_t frame_id = (msg->id != 0) ? msg->id : _last_request_id;
    uint8_t expected = computeChecksum(frame_id, buffer, data_len,
                                       (_flags & LIN_ENHANCED_CHECKSUM) != 0);
    
    if (checksum != expected) {
        msg->status = LIN_CHECKSUM_ERROR;
        return (_last_status = LIN_CHECKSUM_ERROR);
    }

    // Copy data
    memcpy(msg->data, buffer, data_len);
    msg->id = frame_id;
    msg->length = data_len;
    msg->checksum = checksum;
    msg->status = LIN_OK;

    return (_last_status = LIN_OK);
}

/**
 * @brief Initialize LIN driver for master mode
 */
uint16_t LINAnnex::beginMaster(uint32_t baudrate) {
    _baudrate = baudrate;
    _flags = LIN_MASTER;
    
    //_serial.begin(_baudrate);
    
    _initialized = true;
    return (_last_status = LIN_OK);
}

/**
 * @brief Initialize LIN driver for slave mode
 */
uint16_t LINAnnex::beginSlave(uint32_t baudrate) {
    _baudrate = baudrate;
    _flags = LIN_SLAVE;
    
    _serial.begin(_baudrate);
    
    _initialized = true;
    return (_last_status = LIN_OK);
}

/**
 * @brief Close/deinitialize LIN driver
 */
void LINAnnex::end() {
    if (_initialized) {
        _serial.end();
        _initialized = false;
        _flags = 0;
    }
}

/**
 * @brief Write a LIN message (master mode)
 */
uint16_t LINAnnex::writeMessage(uint8_t id, const uint8_t *data, uint8_t length) {
    if (!_initialized || !(_flags & LIN_MASTER)) {
        return (_last_status = LIN_NOT_INITIALIZED);
    }

    if (length > LIN_MAX_DATA_LEN) {
        return (_last_status = LIN_ERROR);
    }

    if (!data || length == 0) {
        return (_last_status = LIN_ERROR);
    }

    // Send break signal
    sendBreak();

    // Send header
    if (sendHeader(id) != LIN_OK) {
        return _last_status;
    }

    // Send data
    for (uint8_t i = 0; i < length; i++) {
        _serial.write(data[i]);
    }

    // Compute and send checksum
    uint8_t checksum = computeChecksum(id, data, length, 
                                       (_flags & LIN_ENHANCED_CHECKSUM) != 0);
    _serial.write(checksum);

    _last_request_id = id;
    return (_last_status = LIN_OK);
}

/**
 * @brief Request a message from slave (master mode)
 */
uint16_t LINAnnex::requestMessage(uint8_t id) {
    if (!_initialized || !(_flags & LIN_MASTER)) {
        return (_last_status = LIN_NOT_INITIALIZED);
    }

    if (id > LIN_MAX_FRAME_ID) {
        return (_last_status = LIN_ERROR);
    }

    // Send break signal
    sendBreak();

    // Send header
    if (sendHeader(id) != LIN_OK) {
        return _last_status;
    }

    _last_request_id = id;
    return (_last_status = LIN_OK);
}

/**
 * @brief Send slave response to master
 */
uint16_t LINAnnex::sendResponse(const uint8_t *data, uint8_t length) {
    if (!_initialized || !(_flags & LIN_SLAVE)) {
        return (_last_status = LIN_NOT_INITIALIZED);
    }

    if (length > LIN_MAX_DATA_LEN) {
        return (_last_status = LIN_ERROR);
    }

    if (!data) {
        return (_last_status = LIN_ERROR);
    }

    if (length == 0) {
        return (_last_status = LIN_OK);
    }

    // Send data
    for (uint8_t i = 0; i < length; i++) {
        _serial.write(data[i]);
    }

    // Compute and send checksum
    uint8_t checksum = computeChecksum(_last_request_id, data, length,
                                       (_flags & LIN_ENHANCED_CHECKSUM) != 0);
    _serial.write(checksum);

    return (_last_status = LIN_OK);
}

/**
 * @brief Read a LIN message with timeout
 */
uint16_t LINAnnex::readMessageWait(LIN_Message *msg, uint8_t dlc, uint32_t timeout_ms) {
    if (!_initialized || !msg) {
        return (_last_status = LIN_NOT_INITIALIZED);
    }

    memset(msg, 0, sizeof(LIN_Message));

    if (_flags & LIN_MASTER) {
        // Master reads response to previous request
        return readResponse(msg, dlc, timeout_ms);
    } else if (_flags & LIN_SLAVE) {

        detectBreak();

        // Slave reads command header and the following data payload
        if (readHeader(msg, timeout_ms) != LIN_OK) {
            return _last_status;
        }
        return readResponse(msg, dlc, timeout_ms);
    }

    return (_last_status = LIN_ERROR);
}

/**
 * @brief Set mode (master or slave)
 */
void LINAnnex::setMode(bool is_master) {
    if (is_master) {
        _flags |= LIN_MASTER;
        _flags &= ~LIN_SLAVE;
    } else {
        _flags |= LIN_SLAVE;
        _flags &= ~LIN_MASTER;
    }
}

/**
 * @brief Enable/disable enhanced checksum
 */
void LINAnnex::setEnhancedChecksum(bool enable) {
    if (enable) {
        _flags |= LIN_ENHANCED_CHECKSUM;
    } else {
        _flags &= ~LIN_ENHANCED_CHECKSUM;
    }
}

/**
 * @brief Flush serial buffers
 */
void LINAnnex::flushBuffers() {
    _serial.flush();
}
