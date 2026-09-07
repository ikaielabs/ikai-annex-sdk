# LIN Add-on Board - Arduino Library

An Arduino-compatible driver for Local Interconnect Network (LIN) bus communication. This library provides a simple, class-based interface supporting both master and slave operation modes using Arduino's native Serial library.

## Features

- **Master and Slave Modes**: Support for LIN master and slave operation
- **Native Serial Integration**: Uses Arduino's HardwareSerial library
- **LIN 1.x and 2.0 Support**: Classic and enhanced checksums
- **Automatic Break Generation**: Proper LIN break signal timing
- **Protected ID**: Automatic parity bit calculation and validation
- **Error Detection**: Comprehensive error handling and status reporting
- **Simple API**: Object-oriented C++ interface

## LIN Bus Overview

LIN (Local Interconnect Network) is a single-wire serial protocol used for low-speed, cost-effective vehicle communication. Key features:
- Single wire (plus ground)
- Low cost compared to CAN
- Master-slave architecture
- Baud rates: Less than 20000 bps typical
- 6-bit frame IDs (0-63)
- Up to 8 data bytes per frame

## Hardware Requirements

- Arduino board with hardware UART (Uno, Mega, Leonardo, etc.)
- LIN Board
- LIN bus connection

### Typical Connections

```
Arduino Pin 0 (RX) ←→ LIN Transceiver RX
Arduino Pin 1 (TX) ←→ LIN Transceiver TX
```

## Installation

1. Copy the `LIN` folder to your Arduino `libraries` folder
2. Restart Arduino IDE
3. Library will appear under **Sketch → Include Library → LIN Annex**

## Quick Start - Master Mode

```cpp
#include <InxLinBoard.h>

// Create LIN instance (Serial, TX pin for break, break width in bits)
InxLinBoard lin(Serial, 1, 13);

void setup() {
    // Initialize as master at 10000 baud
    if (lin.beginMaster(10000) != INX_LIN_BOARD_STATUS_OK) {
        Serial.println("LIN initialization failed!");
        return;
    }
    
    lin.setEnhancedChecksum(false);  // Use LIN 1.0 checksum
}

void loop() {
    // Write message to LIN bus (ID=0x10, data=[0x01, 0x02])
    uint8_t data[] = {0x01, 0x02};
    lin.writeMessage(0x10, data, 2);
    
    // Request response from slave
    lin.requestMessage(0x20);
    
    // Read response
    Inx_Lin_Board_Msg msg;
    if (lin.readMessageWait(&msg, 100) == INX_LIN_BOARD_STATUS_OK) {
        Serial.print("Received ID: ");
        Serial.println(msg.id);
    }
    
    delay(100);
}
```

## Quick Start - Slave Mode

```cpp
#include <InxLinBoard.h>

InxLinBoard lin(Serial, 1);

void setup() {
    // Initialize as slave at 10000 baud
    lin.beginSlave(10000);
    lin.setEnhancedChecksum(false);
}

void loop() {
    // Wait for command from master
    Inx_Lin_Board_Msg msg;
    if (lin.readMessageWait(&msg, 1000) == INX_LIN_BOARD_STATUS_OK) {
        // Process command and send response
        uint8_t response[] = {0xAA, 0xBB};
        lin.sendResponse(response, 2);
    }
}
```

## API Reference

### Constructor

```cpp
InxLinBoard(HardwareSerial &serial, uint8_t tx_pin, uint8_t break_width = 13)
```

- `serial`: Reference to HardwareSerial object (Serial, Serial1, Serial2, etc.)
- `tx_pin`: Tx pin for break signal generation
- `break_width`: Break duration in bit times (default: 13 bits)

### Initialization

```cpp
uint16_t beginMaster(uint32_t baudrate)
```

Initialize as LIN master at specified baud rate. Returns `INX_LIN_BOARD_STATUS_OK` on success.

```cpp
uint16_t beginSlave(uint32_t baudrate)
```

Initialize as LIN slave at specified baud rate. Returns `INX_LIN_BOARD_STATUS_OK` on success.

```cpp
void end()
```

Deinitialize the LIN driver and release serial resources.

### Master Functions

```cpp
uint16_t writeMessage(uint8_t id, const uint8_t *data, uint8_t length)
```

Send a complete LIN message (master as transmitter).

```cpp
uint16_t requestMessage(uint8_t id)
```

Request a message from slave (send header only, wait for response).

### Slave Functions

```cpp
uint16_t sendResponse(const uint8_t *data, uint8_t length)
```

Send response data after receiving a request header.

### Common Functions

```cpp
uint16_t readMessage(Inx_Lin_Board_Msg *msg)
```

Read a LIN message (no timeout). Master reads response, slave reads request header.

```cpp
uint16_t readMessageWait(Inx_Lin_Board_Msg *msg, uint32_t timeout_ms)
```

Read a LIN message with timeout in milliseconds.

### Configuration

```cpp
void setMode(bool is_master)
```

Switch between master (true) and slave (false) modes dynamically.

```cpp
void setEnhancedChecksum(bool enable)
```

Enable LIN 2.0 enhanced checksum (true) or classic LIN 1.x (false).

```cpp
bool isMaster() const
```

Check if currently in master mode.

```cpp
uint16_t getLastStatus() const
```

Get status flags from last operation.

### Utility Functions

```cpp
void flushBuffers()
```

Clear any pending data in serial buffers.

```cpp
int available() const
```

Check number of bytes available to read.

## Inx_Lin_Board_Msg Structure

```cpp
typedef struct {
    uint8_t id;             // Frame ID (0-63)
    uint8_t data[8];        // Data payload
    uint8_t length;         // Data length (0-8)
    uint8_t checksum;       // Calculated checksum
    uint16_t status;        // Status flags
} Inx_Lin_Board_Msg;
```

## Status Codes

| Code | Description |
|------|-------------|
| `INX_LIN_BOARD_STATUS_OK` | Operation successful |
| `INX_LIN_BOARD_STATUS_ERR` | General operation error |
| `INX_LIN_BOARD_STATUS_NOT_INITIALIZED` | Driver not initialized |
| `INX_LIN_BOARD_STATUS_BIT_ERR` | Bit transmission error |
| `INX_LIN_BOARD_STATUS_SYNC_ERR` | Sync byte error |
| `INX_LIN_BOARD_STATUS_PARITY_ERR` | ID parity validation failed |
| `INX_LIN_BOARD_STATUS_CHKSUM_ERR` | Data checksum mismatch |
| `INX_LIN_BOARD_STATUS_NODATA` | Header received but no data |
| `INX_LIN_BOARD_STATUS_TIMEOUT` | Operation timeout |

## Configuration Flags

```cpp
#define INX_LIN_BOARD_ENHANCED_CHKSUM   0x01  // LIN 2.0 checksum
#define INX_LIN_BOARD_WAKEUP_FRAME        0x02  // Wake-up frame support
#define INX_LIN_BOARD_VAR_DLC        0x04  // Variable data length
#define INX_LIN_BOARD_MASTER              0x08  // Master mode
#define INX_LIN_BOARD_SLAVE               0x10  // Slave mode
```

## Examples

See the `examples/` folder for complete working examples:
- `lin_master_example.ino` - Master mode operation
- `lin_slave_example.ino` - Slave mode operation

## Technical Details

### LIN Frame Format

```
[BREAK (13 bits)] [DELIMITER] [SYNC (0x55)] [PID] [DATA (0-8 bytes)] [CHECKSUM]
```

### Protected ID (PID)

The LIN protocol adds parity bits to the 6-bit frame ID:
- P0: even parity of ID bits 0, 1, 2, 4
- P1: odd parity of ID bits 1, 3, 4, 5
- PID = ID | (P0 << 6) | (P1 << 7)

### Checksum Calculation

**Classic (LIN 1.x):**
```
checksum = ~(sum of all data bytes with carry propagation)
```

**Enhanced (LIN 2.0):**
```
checksum = ~(sum of ID + all data bytes with carry propagation)
```

## Compatibility

- **Arduino IDE**: 1.8.0 or later
- **Boards**: AVR (Uno, Nano, Mega), ARM (Zero, Due, MKR series)
- **Serial Support**: Any hardware or software serial

## Notes

- Serial library provides UART communication
- Timing is calculated at runtime based on baud rate
- Frame IDs are limited to 0-63 (6 bits)
- Data length per frame: 0-8 bytes

## License

This library is part of the IKAI Annex SDK.
