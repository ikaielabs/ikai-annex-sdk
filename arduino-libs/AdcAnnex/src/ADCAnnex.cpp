#include "ADCAnnex.h"

// ADC SPI Protocol Constants
#define ADC_START_BIT      0x04
#define ADC_SINGLE_ENDED   0x02

/**
 * @brief Constructor - Initialize ADCAnnex object
 * @param cs_pin   Chip Select pin number
 * @param vref_mv  Reference voltage in millivolts (default: 5000mV)
 * @param spi_freq SPI frequency in Hz (default: 2MHz)
 */
ADCAnnex::ADCAnnex(uint8_t cs_pin, uint16_t vref_mv, uint32_t spi_freq)
    : _cs_pin(cs_pin),
      _vref_mv(vref_mv),
      _spi_freq(spi_freq),
      _initialized(false) {
}

/**
 * @brief Initialize the ADC driver
 * Sets up SPI bus and configures the CS pin
 * @return ADC_OK on success, ADC_ERROR otherwise
 */
ADC_Status ADCAnnex::begin() {
    if (_initialized) {
        return ADC_OK;
    }

    // Configure CS pin as output
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);

    // Initialize SPI with specified frequency
    SPI.begin();
    setSPIFrequency(_spi_freq);

    _initialized = true;
    return ADC_OK;
}

/**
 * @brief Deinitialize the ADC driver
 */
void ADCAnnex::end() {
    if (_initialized) {
        SPI.end();
        _initialized = false;
    }
}

/**
 * @brief Set SPI frequency
 * @param freq Frequency in Hz
 */
void ADCAnnex::setSPIFrequency(uint32_t freq) {
    _spi_freq = freq;
    if (_initialized) {
        SPI.setClockDivider(SPI_CLOCK_DIV2);  // Adjust based on desired frequency
    }
}

/**
 * @brief Get reference voltage
 * @return Reference voltage in millivolts
 */
uint16_t ADCAnnex::getVref() const {
    return _vref_mv;
}

/**
 * @brief Internal SPI transfer - performs 3-byte SPI transaction
 * @param tx_data Pointer to 3-byte transmit buffer
 * @param rx_data Pointer to 3-byte receive buffer
 * @return Number of bytes transferred (3 on success)
 */
int ADCAnnex::spiTransfer(uint8_t *tx_data, uint8_t *rx_data) {
    if (!_initialized || !tx_data || !rx_data) {
        return 0;
    }

    // Assert CS (pull LOW)
    digitalWrite(_cs_pin, LOW);

    // Transfer 3 bytes
    for (int i = 0; i < 3; i++) {
        rx_data[i] = SPI.transfer(tx_data[i]);
    }

    // Deassert CS (pull HIGH)
    digitalWrite(_cs_pin, HIGH);

    return 3;
}

/**
 * @brief Extract 12-bit ADC value from SPI response
 * @param rx_data SPI response buffer (3 bytes)
 * @return 12-bit ADC value (bits 11-0)
 */
uint16_t ADCAnnex::extractADCValue(uint8_t *rx_data) {
    // ADC value is in bits 11-0 of rx_data[1] and rx_data[2]
    // rx_data[1] contains bits 11-8 (in lower 4 bits)
    // rx_data[2] contains bits 7-0
    return ((rx_data[1] & 0x0F) << 8) | rx_data[2];
}

/**
 * @brief Read raw ADC value from a single-ended channel
 * @param channel Channel to read (ADC_CH0 to ADC_CH3)
 * @param raw_value Pointer to store the raw ADC value (0-4095)
 * @return ADC_OK on success, ADC_ERROR otherwise
 */
ADC_Status ADCAnnex::readRaw(ADC_Channel channel, uint16_t *raw_value) {
    if (!_initialized || !raw_value || channel > ADC_CH3) {
        return ADC_INVALID_PARAM;
    }

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};

    // Prepare SPI packet for single-ended read
    tx_data[0] = ADC_START_BIT | ADC_SINGLE_ENDED;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Perform SPI transfer
    int transfer_length = spiTransfer(tx_data, rx_data);

    if (transfer_length != 3) {
        return ADC_ERROR;
    }

    // Extract and store the 12-bit ADC value
    *raw_value = extractADCValue(rx_data);

    return ADC_OK;
}

/**
 * @brief Read ADC value in millivolts from a single-ended channel
 * @param channel Channel to read (ADC_CH0 to ADC_CH3)
 * @param voltage_mv Pointer to store the voltage in millivolts
 * @return ADC_OK on success, ADC_ERROR otherwise
 */
ADC_Status ADCAnnex::readVoltage(ADC_Channel channel, uint16_t *voltage_mv) {
    if (!_initialized || !voltage_mv || channel > ADC_CH3) {
        return ADC_INVALID_PARAM;
    }

    uint16_t adc_raw = 0;

    // Read raw ADC value
    ADC_Status status = readRaw(channel, &adc_raw);
    if (status != ADC_OK) {
        return status;
    }

    // Convert raw value to millivolts using reference voltage
    *voltage_mv = (uint32_t)adc_raw * _vref_mv / ADC_RESOLUTION;

    return ADC_OK;
}

/**
 * @brief Read differential ADC value in millivolts
 * @param channel Differential channel pair to read
 * @param voltage_mv Pointer to store the differential voltage in millivolts
 * @return ADC_OK on success, ADC_ERROR otherwise
 */
ADC_Status ADCAnnex::readDifferentialVoltage(ADC_DiffChannel channel, uint16_t *voltage_mv) {
    if (!_initialized || !voltage_mv || channel > ADC_DIFF_CH32) {
        return ADC_INVALID_PARAM;
    }

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};

    // Prepare SPI packet for differential read (no ADC_SINGLE_ENDED flag)
    tx_data[0] = ADC_START_BIT;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Perform SPI transfer
    int transfer_length = spiTransfer(tx_data, rx_data);

    if (transfer_length != 3) {
        return ADC_ERROR;
    }

    // Extract 12-bit ADC value
    uint16_t adc_raw = extractADCValue(rx_data);

    // Convert raw value to millivolts using reference voltage
    *voltage_mv = (uint32_t)adc_raw * _vref_mv / ADC_RESOLUTION;

    return ADC_OK;
}
