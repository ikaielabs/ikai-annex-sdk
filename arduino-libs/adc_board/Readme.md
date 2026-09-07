# ADC Annex - Arduino Library

An Arduino-compatible driver for reading analog values from an ADC chip over SPI bus. This library provides a simple, class-based interface using the native Arduino SPI library.

## Features

- **Simple Arduino API**: Object-oriented C++ interface
- **Native SPI Integration**: Uses Arduino's built-in SPI library (no HAL layer needed)
- **Single-ended Reads**: Read individual ADC channels (CH0-CH3)
- **Differential Reads**: Support for differential channel pair readings
- **Voltage Conversion**: Automatic conversion from raw ADC values to millivolts
- **Easy Initialization**: Single-line setup with sensible defaults

## Hardware Requirements

- Arduino board with SPI capability (Uno, Mega, Leonardo, etc.)
- ADC Annex chip connected via SPI:
  - MOSI → Arduino pin 11 (SPI MOSI)
  - MISO → Arduino pin 12 (SPI MISO)
  - SCK  → Arduino pin 13 (SPI SCK)
  - CS   → Arduino pin 10 (configurable)

## Installation

1. Copy the `AdcAnnex` folder to your Arduino `libraries` folder
2. Restart Arduino IDE
3. Library will appear under **Sketch → Include Library → ADC Annex**

## Quick Start

```cpp
#include <ADCAnnex.h>

// Create ADC instance (CS pin 10, 5V reference, 2MHz SPI)
ADCAnnex adc(10, INX_ADC_BOARD_VREF_5000MV, 2000000);

void setup() {
    Serial.begin(115200);
    
    if (adc.begin() != INX_STATUS_OK) {
        Serial.println("ADC initialization failed!");
        return;
    }
}

void loop() {
    uint16_t raw_value, voltage_mv;
    
    // Read channel 0 raw ADC value
    if (adc.readRaw(INX_ADC_BOARD_CH0, &raw_value) == INX_STATUS_OK) {
        Serial.print("Raw: ");
        Serial.println(raw_value);
    }
    
    // Read channel 0 voltage
    if (adc.readVoltage(INX_ADC_BOARD_CH0, &voltage_mv) == INX_STATUS_OK) {
        Serial.print("Voltage: ");
        Serial.print(voltage_mv);
        Serial.println(" mV");
    }
    
    delay(1000);
}
```

## API Reference

### Constructor

```cpp
ADCAnnex(uint8_t cs_pin, uint16_t vref_mv = INX_ADC_BOARD_VREF_5000MV, uint32_t spi_freq = 2000000)
```

- `cs_pin`: Chip Select pin number
- `vref_mv`: Reference voltage in millivolts (default: 5000mV)
- `spi_freq`: SPI frequency in Hz (default: 2MHz)

### Initialization

```cpp
Inx_Adc_Board_Status begin()
```

Initialize the ADC driver. Must be called in `setup()`. Returns `INX_STATUS_OK` on success.

```cpp
void end()
```

Deinitialize the ADC driver and release SPI resources.

### Reading Functions

#### Single-Ended Reads

```cpp
Inx_Adc_Board_Status readRaw(Inx_Adc_Board_Ch channel, uint16_t *raw_value)
```

Read raw 12-bit ADC value (0-4095) from a single-ended channel.

```cpp
Inx_Adc_Board_Status readVoltage(Inx_Adc_Board_Ch channel, uint16_t *voltage_mv)
```

Read converted voltage (in millivolts) from a single-ended channel.

#### Differential Reads

```cpp
Inx_Adc_Board_Status readDifferentialVoltage(Inx_Adc_Board_DiffCh channel, uint16_t *voltage_mv)
```

Read differential voltage between channel pairs.

### Channel Selection

**Single-ended channels:**
- `INX_ADC_BOARD_CH0` - Channel 0
- `INX_ADC_BOARD_CH1` - Channel 1
- `INX_ADC_BOARD_CH2` - Channel 2
- `INX_ADC_BOARD_CH3` - Channel 3

**Differential channels:**
- `INX_ADC_BOARD_DIFF_CH01` - CH0 - CH1
- `INX_ADC_BOARD_DIFF_CH10` - CH1 - CH0
- `INX_ADC_BOARD_DIFF_CH23` - CH2 - CH3
- `INX_ADC_BOARD_DIFF_CH32` - CH3 - CH2

### Status Codes

- `INX_STATUS_OK` - Operation successful
- `INX_STATUS_ERR` - Operation failed
- `INX_STATUS_INVALID_CH` - Invalid channel selected
- `INX_STATUS_INVALID_PARAM` - Invalid parameter

### Reference Voltage Constants

- `INX_ADC_BOARD_VREF_3300MV` - 3.3V reference
- `INX_ADC_BOARD_VREF_4096MV` - 4.096V reference
- `INX_ADC_BOARD_VREF_5000MV` - 5.0V reference

### Utility Functions

```cpp
void setSPIFrequency(uint32_t freq)
```

Change SPI frequency at runtime.

```cpp
uint16_t getVref() const
```

Get the current reference voltage in millivolts.

## Migration from C API (v1.x)

If you're upgrading from the C-based driver (v1.x), here's how the APIs map:

### Old C API → New C++ API

| Old Function | New Method | Notes |
|---|---|---|
| `inx_adc_board_open()` | Constructor | Simpler initialization |
| `inx_adc_board_close()` | `end()` | Cleanup method |
| `inx_adc_board_read_raw()` | `readRaw()` | Same functionality, simpler API |
| `inx_adc_board_read_mv()` | `readVoltage()` | Renamed for clarity |
| `inx_adc_board_read_differential_mv()` | `readDifferentialVoltage()` | Renamed for clarity |

### Example Migration

**Old C Code:**
```c
inx_adc_board_hw_desc_t adc_hw = {
    .spi_handle = spi0,
    .spi_baud_rate_hz = 2000000UL,
    .cs_port_handle = gpio_portb,
    .cs_pin = 10,
    .vref_mv = INX_ADC_BOARD_VREF_5000MV
};
inx_adc_board_dev_t *adc = inx_adc_board_open(&adc_hw);
inx_adc_board_read_raw(adc, ANNEX_ADC_CH0, &raw_value);
```

**New C++ Code:**
```cpp
ADCAnnex adc(10, INX_ADC_BOARD_VREF_5000MV, 2000000);
adc.begin();
adc.readRaw(INX_ADC_BOARD_CH0, &raw_value);
```

## Key Improvements

1. **Simplified API** - No need to manage HAL handles or configuration structures
2. **Native Arduino** - Uses built-in SPI library instead of register access
3. **Object-Oriented** - Clean C++ class interface
4. **Automatic Resource Management** - Constructor/destructor pattern
5. **Better Error Handling** - Consistent status codes
6. **Less Code** - Reduced setup complexity

## Examples

See the `examples/` folder for complete working examples:
- `inx_adc_board_example.ino` - Basic single-ended and differential readings

## Compatibility

- **Arduino IDE**: 1.8.0 or later
- **Boards**: AVR (Uno, Nano, Mega), ARM (Zero, Due, MKR series)
- **SPI Support**: Any board with hardware SPI

## Notes

- The ADC chip expects 12-bit values (resolution: 0-4095)
- SPI transfers are 3 bytes (command, config, result)
- CS pin is automatically managed by the library
- Reference voltage should match your actual hardware setup for accurate conversions

## License

This library is part of the IKAI Annex SDK.


If memory footprint is critical, floating point, exponential and 'long long' support and can be turned off via the `PRINTF_DISABLE_SUPPORT_FLOAT`, `PRINTF_DISABLE_SUPPORT_EXPONENTIAL` and `PRINTF_DISABLE_SUPPORT_LONG_LONG` compiler switches. You must define these symbols in the build system.

## Examples

Multiple examples are provided with this library in the [examples/](examples/) folder.

* [Default Usage](examples/default_to_serial/default_to_serial.ino)
    - Without any initialization, `Serial` will be the default output for `printf()`
    - This example initializes the `Serial` class and prints in a loop
    - Open the Arduino serial monitor to see the output
* [Specify Print Class](examples/specify_print_class/specify_print_class.ino)
    - Any class derived from the `Print` base class can be used with the `printf` library
    - This example initializes `printf` with `Serial1` instead of `Serial`
* [Override Putchar](examples/override_putchar/override_putchar.ino)
    - Because the `_putchar` function is weakly linked, you can override the default implementation to supply your own in the sketch
    - This example overrides `_putchar` and adds a space in between every letter
    - You can implement any kind of logic within `_putchar` that you like, such as outputting information to multiple ports
