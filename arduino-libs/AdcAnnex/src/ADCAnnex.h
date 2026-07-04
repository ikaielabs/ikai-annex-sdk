#ifndef ADC_ANNEX_H
#define ADC_ANNEX_H

#include <stdint.h>
#include <SPI.h>

/**
 * @brief ADC Reference voltage definitions (in millivolts)
 */
#define ADC_VREF_3300MV     3300
#define ADC_VREF_4096MV     4096
#define ADC_VREF_5000MV     5000

/**
 * @brief ADC Resolution (12-bit)
 */
#define ADC_RESOLUTION      4095

/**
 * @brief ADC Status codes
 */
typedef enum {
    ADC_OK       = 0x00,
    ADC_ERROR    = 0x01,
    ADC_INVALID_CHANNEL = 0x02,
    ADC_INVALID_PARAM = 0x03
} ADC_Status;

/**
 * @brief Single-ended channel selection
 */
typedef enum {
    ADC_CH0 = 0,
    ADC_CH1 = 1,
    ADC_CH2 = 2,
    ADC_CH3 = 3
} ADC_Channel;

/**
 * @brief Differential channel selection
 */
typedef enum {
    ADC_DIFF_CH01 = 0,  // CH0 - CH1
    ADC_DIFF_CH10 = 1,  // CH1 - CH0
    ADC_DIFF_CH23 = 2,  // CH2 - CH3
    ADC_DIFF_CH32 = 3   // CH3 - CH2
} ADC_DiffChannel;

/**
 * @class ADCAnnex
 * @brief Arduino-compatible ADC Annex driver using SPI communication
 * 
 * This class provides interface to read analog values from an ADC chip
 * connected via SPI bus. It supports both single-ended and differential readings.
 */
class ADCAnnex {
public:
    /**
     * @brief Constructor for ADCAnnex
     * @param cs_pin   Chip Select pin number
     * @param vref_mv  Reference voltage in millivolts
     * @param spi_freq SPI frequency in Hz (default: 2MHz)
     */
    ADCAnnex(uint8_t cs_pin, uint16_t vref_mv = ADC_VREF_5000MV, uint32_t spi_freq = 2000000);

    /**
     * @brief Initialize the ADC driver
     * @return ADC_OK on success, ADC_ERROR otherwise
     */
    ADC_Status begin();

    /**
     * @brief Close/deinitialize the ADC driver
     */
    void end();

    /**
     * @brief Read raw ADC value from a single-ended channel
     * @param channel Channel to read (ADC_CH0 to ADC_CH3)
     * @param raw_value Pointer to store the raw ADC value (0-4095)
     * @return ADC_OK on success, ADC_ERROR otherwise
     */
    ADC_Status readRaw(ADC_Channel channel, uint16_t *raw_value);

    /**
     * @brief Read ADC value in millivolts from a single-ended channel
     * @param channel Channel to read (ADC_CH0 to ADC_CH3)
     * @param voltage_mv Pointer to store the voltage in millivolts
     * @return ADC_OK on success, ADC_ERROR otherwise
     */
    ADC_Status readVoltage(ADC_Channel channel, uint16_t *voltage_mv);

    /**
     * @brief Read differential ADC value in millivolts
     * @param channel Differential channel pair to read
     * @param voltage_mv Pointer to store the differential voltage in millivolts
     * @return ADC_OK on success, ADC_ERROR otherwise
     */
    ADC_Status readDifferentialVoltage(ADC_DiffChannel channel, uint16_t *voltage_mv);

    /**
     * @brief Set SPI frequency
     * @param freq SPI frequency in Hz
     */
    void setSPIFrequency(uint32_t freq);

    /**
     * @brief Get reference voltage
     * @return Reference voltage in millivolts
     */
    uint16_t getVref() const;

private:
    uint8_t _cs_pin;
    uint16_t _vref_mv;
    uint32_t _spi_freq;
    bool _initialized;

    /**
     * @brief Internal method to perform SPI transfer for ADC read
     * @param tx_data Pointer to transmit data (3 bytes)
     * @param rx_data Pointer to receive buffer (3 bytes)
     * @return Number of bytes transferred (3 on success)
     */
    int spiTransfer(uint8_t *tx_data, uint8_t *rx_data);

    /**
     * @brief Extract 12-bit ADC value from SPI response
     * @param rx_data SPI response buffer
     * @return 12-bit ADC value (0-4095)
     */
    uint16_t extractADCValue(uint8_t *rx_data);
};

#endif /* ADC_ANNEX_H */
