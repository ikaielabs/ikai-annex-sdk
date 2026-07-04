#include "adc_annex.h"
#include "annex_hal_spi.h"
#include "annex_hal_gpio.h"
#include "stdlib.h"

#define ADC_START_BIT      0x04
#define ADC_SINGLE_ENDED   0x02
#define ADC_RESOLUTION     4095

struct annex_adc_dev {
    annex_hal_spi_dev_t *spi_handle;
    annex_hal_gpio_dev_t *cs_port_handle;
    annex_gpio_pin_t cs_pin;
    uint16_t vref_mv;
};

annex_adc_dev_t *annex_adc_open(const annex_adc_hw_desc_t *hw)
{
    if (!hw) return NULL;

    annex_adc_dev_t *dev = (annex_adc_dev_t *)calloc(1, sizeof(*dev));
    if (!dev) return NULL;

    annex_hal_spi_config_t cfg = {
        .baud_rate_hz    = hw->spi_baud_rate_hz,
        .mode    = ANNEX_HAL_SPI_MODE_0,
        .role    = ANNEX_HAL_SPI_MASTER,
        .bit_order       = ANNEX_HAL_SPI_MSB_FIRST,
        .bits_per_word = 8,
        .use_dma = false
    };

    /* Get the SPI handle */
    dev->spi_handle = hw->spi_handle;

    /* Get the CS PORT handle */
    dev->cs_port_handle = hw->cs_port_handle;

    (void)annex_hal_spi_config(dev->spi_handle, &cfg);
    
    annex_hal_spi_enable(dev->spi_handle);

    dev->cs_pin = hw->cs_pin;

    /* Set CS Pin Mode and write CS Pin HIGH */
    annex_hal_gpio_status_t status = annex_hal_gpio_set_mode(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_MODE_OUTPUT_PP);
    status = annex_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_HIGH);
    if (status != ANNEX_HAL_GPIO_OK) {   
        annex_hal_spi_close(dev->spi_handle);
        annex_hal_gpio_close(dev->cs_port_handle);
        free(dev);
        return NULL;
    }

    dev->vref_mv= hw->vref_mv;

    return dev;
}

void annex_adc_close(annex_adc_dev_t *dev)
{
    if (!dev) return;

    annex_hal_spi_close(dev->spi_handle);
    annex_hal_gpio_close(dev->cs_port_handle);

    free(dev);
}

annex_adc_status_t adc_annex_read_raw(annex_adc_dev_t *dev, annex_adc_channel_t channel, uint16_t *adc_value) {
	if ((!dev) || (!adc_value))
	{
	    return ANNEX_ADC_ERROR;
	}

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};

    tx_data[0] = ADC_START_BIT | ADC_SINGLE_ENDED;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Write CS LOW
    if (annex_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_LOW) != ANNEX_HAL_GPIO_OK)
       return ANNEX_ADC_ERROR;

    int transfer_length = annex_hal_spi_transfer(dev->spi_handle, tx_data, rx_data, 3U);

    // Write CS HIGH
    if (annex_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_HIGH) != ANNEX_HAL_GPIO_OK)
       return ANNEX_ADC_ERROR;

    if(transfer_length != 3U) {
        return ANNEX_ADC_ERROR;
    }

    *adc_value =
        ((rx_data[1] & 0x0F) << 8) |
        rx_data[2];

    return ANNEX_ADC_OK;
}

annex_adc_status_t adc_annex_read_mv(annex_adc_dev_t *dev, annex_adc_channel_t channel, uint16_t *mv)
{
    if ((!dev) || (!mv)) {
        return ANNEX_ADC_ERROR;
    }
    
    uint16_t adc_raw;
    annex_adc_status_t status = adc_annex_read_raw(dev, channel, &adc_raw);

    if(status != ANNEX_ADC_OK)
    {
        return status;
    }

    *mv = (uint32_t)adc_raw * dev->vref_mv / ADC_RESOLUTION;

    return ANNEX_ADC_OK;
}

annex_adc_status_t adc_annex_read_differential_mv(annex_adc_dev_t *dev, annex_adc_diff_channel_t channel, uint16_t *mv) {
    if ((!dev) || (!mv)) {
        return ANNEX_ADC_ERROR;
    }

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};
    uint16_t adc_raw;

    /* Differential mode */
    tx_data[0] = ADC_START_BIT;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Write CS LOW
    if (annex_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_LOW) != ANNEX_HAL_GPIO_OK)
       return ANNEX_ADC_ERROR;

    int transfer_length = annex_hal_spi_transfer(dev->spi_handle, tx_data, rx_data, 3U);

    // Write CS HIGH
    if (annex_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, ANNEX_GPIO_HIGH) != ANNEX_HAL_GPIO_OK)
       return ANNEX_ADC_ERROR;

    if(transfer_length != 3U) {
        return ANNEX_ADC_ERROR;
    }

    adc_raw = ((uint16_t)(rx_data[1] & 0x0F) << 8) | (uint16_t)rx_data[2];

    *mv = (uint16_t)(((uint32_t)adc_raw * dev->vref_mv) / ADC_RESOLUTION);

    return ANNEX_ADC_OK;
}
