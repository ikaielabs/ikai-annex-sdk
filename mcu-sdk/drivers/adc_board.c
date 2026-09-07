#include "adc_board.h"
#include "inx_hal_spi.h"
#include "inx_hal_gpio.h"
#include "stdlib.h"

#define INX_ADC_BOARD_START_BIT      0x04
#define INX_ADC_BOARD_SINGLE_ENDED   0x02
#define INX_ADC_BOARD_RESOLUTION     4095

struct inx_adc_board_dev {
    inx_hal_spi_dev_t *spi_handle;
    inx_hal_gpio_dev_t *cs_port_handle;
    inx_hal_gpio_pin_t cs_pin;
    uint16_t vref_mv;
};

inx_adc_board_dev_t *inx_adc_board_open(const inx_adc_board_hw_desc_t *hw)
{
    if (!hw) return NULL;

    inx_adc_board_dev_t *dev = (inx_adc_board_dev_t *)calloc(1, sizeof(*dev));
    if (!dev) return NULL;

    inx_hal_spi_config_t cfg = {
        .baud_rate_hz    = hw->spi_baud_rate_hz,
        .mode    = INX_HAL_SPI_MODE_0,
        .role    = INX_HAL_SPI_MASTER,
        .bit_order       = INX_HAL_SPI_MSB_FIRST,
        .bits_per_word = 8,
        .use_dma = false
    };

    /* Get the SPI handle */
    dev->spi_handle = hw->spi_handle;

    /* Get the CS PORT handle */
    dev->cs_port_handle = hw->cs_port_handle;

    (void)inx_hal_spi_config(dev->spi_handle, &cfg);
    
    inx_hal_spi_enable(dev->spi_handle);

    dev->cs_pin = hw->cs_pin;

    /* Set CS Pin Mode and write CS Pin HIGH */
    inx_hal_gpio_status_t status = inx_hal_gpio_set_mode(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_MODE_OUTPUT_PP);
    status = inx_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_HIGH);
    if (status != INX_HAL_GPIO_OK) {   
        inx_hal_spi_close(dev->spi_handle);
        inx_hal_gpio_close(dev->cs_port_handle);
        free(dev);
        return NULL;
    }

    dev->vref_mv= hw->vref_mv;

    return dev;
}

void inx_adc_board_close(inx_adc_board_dev_t *dev)
{
    if (!dev) return;

    inx_hal_spi_close(dev->spi_handle);
    inx_hal_gpio_close(dev->cs_port_handle);

    free(dev);
}

inx_adc_board_status_t inx_adc_board_read_raw(inx_adc_board_dev_t *dev, inx_adc_board_ch_t channel, uint16_t *adc_value) {
	if ((!dev) || (!adc_value))
	{
	    return INX_STATUS_ERR;
	}

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};

    tx_data[0] = INX_ADC_BOARD_START_BIT | INX_ADC_BOARD_SINGLE_ENDED;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Write CS LOW
    if (inx_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_LOW) != INX_HAL_GPIO_OK)
       return INX_STATUS_ERR;

    int transfer_length = inx_hal_spi_transfer(dev->spi_handle, tx_data, rx_data, 3U);

    // Write CS HIGH
    if (inx_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_HIGH) != INX_HAL_GPIO_OK)
       return INX_STATUS_ERR;

    if(transfer_length != 3U) {
        return INX_STATUS_ERR;
    }

    *adc_value =
        ((rx_data[1] & 0x0F) << 8) |
        rx_data[2];

    return INX_STATUS_OK;
}

inx_adc_board_status_t inx_adc_board_read_mv(inx_adc_board_dev_t *dev, inx_adc_board_ch_t channel, uint16_t *mv)
{
    if ((!dev) || (!mv)) {
        return INX_STATUS_ERR;
    }
    
    uint16_t adc_raw;
    inx_adc_board_status_t status = inx_adc_board_read_raw(dev, channel, &adc_raw);

    if(status != INX_STATUS_OK)
    {
        return status;
    }

    *mv = (uint32_t)adc_raw * dev->vref_mv / INX_ADC_BOARD_RESOLUTION;

    return INX_STATUS_OK;
}

inx_adc_board_status_t inx_adc_board_read_differential_mv(inx_adc_board_dev_t *dev, inx_adc_board_diff_ch_t channel, uint16_t *mv) {
    if ((!dev) || (!mv)) {
        return INX_STATUS_ERR;
    }

    uint8_t tx_data[3] = {0};
    uint8_t rx_data[3] = {0};
    uint16_t adc_raw;

    /* Differential mode */
    tx_data[0] = INX_ADC_BOARD_START_BIT;
    tx_data[1] = ((uint8_t)channel << 6);
    tx_data[2] = 0x00;

    // Write CS LOW
    if (inx_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_LOW) != INX_HAL_GPIO_OK)
       return INX_STATUS_ERR;

    int transfer_length = inx_hal_spi_transfer(dev->spi_handle, tx_data, rx_data, 3U);

    // Write CS HIGH
    if (inx_hal_gpio_write(dev->cs_port_handle, dev->cs_pin, INX_HAL_GPIO_HIGH) != INX_HAL_GPIO_OK)
       return INX_STATUS_ERR;

    if(transfer_length != 3U) {
        return INX_STATUS_ERR;
    }

    adc_raw = ((uint16_t)(rx_data[1] & 0x0F) << 8) | (uint16_t)rx_data[2];

    *mv = (uint16_t)(((uint32_t)adc_raw * dev->vref_mv) / INX_ADC_BOARD_RESOLUTION);

    return INX_STATUS_OK;
}
