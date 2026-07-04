#ifndef LIN_ANNEX_H
#define LIN_ANNEX_H

#include <stdint.h>
#include <string.h>
#include "HardwareSerial.h"

/**
 * @brief LIN Frame flags and error codes
 */
#define LIN_OK                      0x00
#define LIN_ERROR                   0x01
#define LIN_NOT_INITIALIZED         0x02
#define LIN_BIT_ERROR               0x04  // Bit error when transmitting
#define LIN_SYNC_ERROR              0x08  // Sync byte error
#define LIN_PARITY_ERROR            0x10  // ID parity error
#define LIN_CHECKSUM_ERROR          0x20  // Checksum error
#define LIN_NO_DATA                 0x40  // No data, only a header
#define LIN_TIMEOUT                 0x80  // Read timeout

/**
 * @brief LIN Configuration flags
 */
#define LIN_ENHANCED_CHECKSUM       0x01  // Use enhanced checksum (LIN 2.0)
#define LIN_WAKEUP_FRAME            0x02  // Wake-up frame received
#define LIN_VARIABLE_DLC            0x04  // Variable message length based on ID
#define LIN_MASTER                  0x08  // Master mode
#define LIN_SLAVE                   0x10  // Slave mode

/**
 * @brief LIN Message structure
 */
typedef struct {
    uint8_t id;                 // Frame ID (0-63)
    uint8_t data[8];            // Data payload
    uint8_t length;             // Data length (0-8)
    uint8_t checksum;           // Calculated checksum
    uint16_t status;            // Status flags
} LIN_Message;

/**
 * @class LINAnnex
 * @brief Arduino-compatible LIN Bus driver using Serial communication
 * 
 * This class provides interface for LIN bus communication (both master and slave modes).
 * LIN is a single-wire serial protocol with break signal generation for synchronization.
 */
class LINAnnex {
public:
    /**
     * @brief Constructor for LINAnnex
     * @param serial Reference to HardwareSerial object (Serial, Serial1, etc.)
     * @param tx_pin Pin used for LIN TX (for break signal generation)
     * @param break_width Break width in bit times (13-15 bits typical)
     */
    LINAnnex(HardwareSerial &serial, uint8_t tx_pin, uint8_t rx_pin, uint8_t break_width = 13);

    /**
     * @brief Initialize the LIN driver for master mode
     * @param baudrate LIN baud rate (9600, 19200, 38400)
     * @return LIN_OK on success, LIN_ERROR otherwise
     */
    uint16_t beginMaster(uint32_t baudrate);

    /**
     * @brief Initialize the LIN driver for slave mode
     * @param baudrate LIN baud rate (9600, 19200, 38400)
     * @return LIN_OK on success, LIN_ERROR otherwise
     */
    uint16_t beginSlave(uint32_t baudrate);

    /**
     * @brief Close/deinitialize the LIN driver
     */
    void end();

    /**
     * @brief Send a LIN message (master only)
     * Generates break signal, sends header with protected ID, data, and checksum
     * @param id Frame ID (0-63)
     * @param data Pointer to data buffer
     * @param length Data length (0-8)
     * @return LIN_OK on success, error code otherwise
     */
    uint16_t writeMessage(uint8_t id, const uint8_t *data, uint8_t length);

    /**
     * @brief Request a LIN message from slave (master only)
     * Generates break signal and header, waits for slave response
     * @param id Frame ID to request (0-63)
     * @return LIN_OK on success, error code otherwise
     */
    uint16_t requestMessage(uint8_t id);

    /**
     * @brief Read a LIN message with timeout (blocking)
     * @param msg Pointer to LIN_Message structure to store result
     * @param dlc Data length. Valid values [1..8]. 
     * @param timeout_ms Timeout in milliseconds (0 = no wait)
     * @return LIN_OK on success, error code otherwise
     */
    uint16_t readMessageWait(LIN_Message *msg, uint8_t dlc, uint32_t timeout_ms);

    /**
     * @brief Send a response message (slave only)
     * Sends data and checksum for the last received message ID
     * @param data Pointer to data buffer
     * @param length Data length (0-8)
     * @return LIN_OK on success, error code otherwise
     */
    uint16_t sendResponse(const uint8_t *data, uint8_t length);

    /**
     * @brief Set mode (master or slave)
     * @param is_master True for master, false for slave
     */
    void setMode(bool is_master);

    /**
     * @brief Enable/disable enhanced checksum (LIN 2.0)
     * @param enable True for enhanced, false for classic
     */
    void setEnhancedChecksum(bool enable);

    /**
     * @brief Get current mode
     * @return True if master, false if slave
     */
    bool isMaster() const { return (_flags & LIN_MASTER) != 0; }

    /**
     * @brief Get last error status
     * @return Status flags from last operation
     */
    uint16_t getLastStatus() const { return _last_status; }

    /**
     * @brief Clear any pending data in serial buffers
     */
    void flushBuffers();

    /**
     * @brief Check if data is available to read
     * @return Number of bytes available
     */
    int available() const;

private:
    HardwareSerial &_serial;
    uint8_t _tx_pin;
    uint8_t _rx_pin;
    uint8_t _break_width;
    uint32_t _baudrate;
    uint16_t _flags;
    uint16_t _last_status;
    uint8_t _last_request_id;
    bool _initialized;
    bool _break_detected;
    // volatile uint32_t _fall_time_us;

    // LIN Protocol helper functions
    uint8_t computeProtectedId(uint8_t id);
    bool validateProtectedId(uint8_t pid, uint8_t *id);
    uint8_t computeChecksum(uint8_t id, const uint8_t *data, uint8_t length, bool enhanced);
    
    // Communication helpers
    void sendBreak();
    void detectBreak();
    // void pinISR();
    // void handlePinChange();
    uint16_t sendHeader(uint8_t id);
    uint16_t readBytes(uint8_t *buffer, uint8_t length, uint32_t timeout_ms);
    uint16_t readResponse(LIN_Message *msg, uint8_t dlc, uint32_t timeout_ms);
    uint16_t readHeader(LIN_Message *msg, uint32_t timeout_ms);
    
    // Timing helper
    void delayMicroseconds_Safe(uint32_t us);
};

#endif /* LIN_ANNEX_H */

