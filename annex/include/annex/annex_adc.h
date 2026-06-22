#ifndef ANNEX_ADC_H
#define ANNEX_ADC_H

#include <stdint.h>
#include "annex_hal_gpio.h"
#include "annex_hal_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Annex reference voltage definition.
 */
#define ADC_VREF_3300MV     3300
#define ADC_VREF_4096MV     4096
#define ADC_VREF_5000MV     5000

/**
  * @brief  Annex Status structures definition
  */
typedef enum {
  ANNEX_ADC_OK       = 0x00U,
  ANNEX_ADC_ERROR    = 0x01U,
  ANNEX_ADC_SPI_BUSY     = 0x02U,
  ANNEX_ADC_SPI_TIMEOUT  = 0x03U
} annex_adc_status_t;

typedef struct {
    annex_hal_spi_dev_t *spi_handle;  /* SPI handle */
    uint32_t spi_baud_rate_hz; /* SPI baud rate */
    annex_hal_gpio_dev_t *cs_port_handle; /* CS PORT handle */
    annex_gpio_pin_t    cs_pin;     /* CS pin number                   */
    uint16_t   vref_mv;             /* Voltage reference in mv         */
    void      *platform_data;       /* Anything else (optional)        */
} annex_adc_hw_desc_t;

typedef enum {
    ANNEX_ADC_CH0 = 0,
    ANNEX_ADC_CH1 = 1,
    ANNEX_ADC_CH2 = 2,
    ANNEX_ADC_CH3 = 3
} annex_adc_channel_t;

typedef enum {
    ANNEX_ADC_DIFF_CH01 = 0,
    ANNEX_ADC_DIFF_CH10 = 1,
    ANNEX_ADC_DIFF_CH23 = 2,
    ANNEX_ADC_DIFF_CH32 = 3
} annex_adc_diff_channel_t;

typedef struct annex_adc_dev annex_adc_dev_t;

annex_adc_dev_t *annex_adc_open(const annex_adc_hw_desc_t *hw);
void annex_adc_close(annex_adc_dev_t *dev);
annex_adc_status_t adc_annex_init(annex_adc_dev_t *dev);
annex_adc_status_t adc_annex_read_raw(annex_adc_dev_t *dev, annex_adc_channel_t channel, uint16_t *adc_value);
annex_adc_status_t adc_annex_read_mv(annex_adc_dev_t *dev, annex_adc_channel_t channel, uint16_t *mv);
annex_adc_status_t adc_annex_read_differential_mv(annex_adc_dev_t *dev, annex_adc_diff_channel_t channel, uint16_t *mv);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ANNEX_ADC_H */
