/**
 * @file annex_hal_spi_atmega328p.c
 * @brief ATmega328P implementation for annex_hal_spi.h.
 *
 * SPI Register layout on ATmega328P:
 *      base_addr + 0  ->  SPCR   (Control Register)
 *      base_addr + 1  ->  SPSR   (Status Register)
 *      base_addr + 2  ->  SPDR   (Data Register)
 *
 * Control Register (SPCR) bits:
 *      Bit 7: SPIE   (SPI Interrupt Enable)
 *      Bit 6: SPE    (SPI Enable)
 *      Bit 5: DORD   (Data Order: 0=MSB first, 1=LSB first)
 *      Bit 4: MSTR   (Master/Slave Select: 1=Master, 0=Slave)
 *      Bit 3: CPOL   (Clock Polarity)
 *      Bit 2: CPHA   (Clock Phase)
 *      Bit 1-0: SPR1:SPR0 (Clock Rate Select)
 *
 * Status Register (SPSR) bits:
 *      Bit 7: SPIF   (SPI Interrupt Flag)
 *      Bit 6: WCOL   (Write Collision Flag)
 *      Bit 0: SPI2X  (Double SPI Speed Bit)
 *
 */

#include "annex_hal_spi.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* Real definition of the opaque type declared in annex_hal_spi.h. */
struct annex_hal_spi_dev {
    volatile uint8_t *spcr;        /* Control Register */
    volatile uint8_t *spsr;        /* Status Register */
    volatile uint8_t *spdr;        /* Data Register */
    annex_hal_spi_config_t cfg;
    uint32_t   input_clk_hz;       /* Input clock for baud calculation */
    uint32_t   irq_mask;           /* Software-tracked enabled IRQs */
    uint32_t   irq_status;         /* Software-tracked pending flags */
    uint8_t    in_use;
};

/* ATmega328P has one SPI port */
#define ANNEX_HAL_SPI_MAX_DEVS 1U

static struct annex_hal_spi_dev s_dev_pool[ANNEX_HAL_SPI_MAX_DEVS];

/* Register helper functions */
static inline volatile uint8_t *reg_spcr(uintptr_t base)
{
    return (volatile uint8_t *)base;
}

static inline volatile uint8_t *reg_spsr(uintptr_t base)
{
    return (volatile uint8_t *)(base + 1u);
}

static inline volatile uint8_t *reg_spdr(uintptr_t base)
{
    return (volatile uint8_t *)(base + 2u);
}

annex_hal_spi_dev_t *annex_hal_spi_open(const annex_hal_spi_hw_desc_t *hw)
{
    if (hw == NULL) {
        return NULL;
    }

    /* Find unused device in pool */
    for (size_t i = 0; i < ANNEX_HAL_SPI_MAX_DEVS; i++) {
        if (!s_dev_pool[i].in_use) {
            s_dev_pool[i].spcr = reg_spcr(hw->base_addr);
            s_dev_pool[i].spsr = reg_spsr(hw->base_addr);
            s_dev_pool[i].spdr = reg_spdr(hw->base_addr);
            s_dev_pool[i].input_clk_hz = hw->input_clk_hz;
            s_dev_pool[i].irq_mask = 0;
            s_dev_pool[i].irq_status = 0;
            s_dev_pool[i].in_use = 1u;

            /* Initialize default configuration */
            s_dev_pool[i].cfg.baud_rate_hz = 1000000;  /* 1 MHz default */
            s_dev_pool[i].cfg.mode = ANNEX_HAL_SPI_MODE_0;
            s_dev_pool[i].cfg.role = ANNEX_HAL_SPI_MASTER;
            s_dev_pool[i].cfg.bit_order = ANNEX_HAL_SPI_MSB_FIRST;
            s_dev_pool[i].cfg.bits_per_word = 8;
            s_dev_pool[i].cfg.use_dma = false;

            return &s_dev_pool[i];
        }
    }
    return NULL;  /* SPI port already open */
}

void annex_hal_spi_close(annex_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }

    /* Disable SPI */
    *dev->spcr &= ~(1u << SPE);
    *dev->spcr &= ~(1u << SPIE);
    *dev->spsr = 0;

    dev->in_use = 0u;
}

int annex_hal_spi_config(annex_hal_spi_dev_t *dev, const annex_hal_spi_config_t *cfg)
{
    if (dev == NULL || cfg == NULL) {
        return -1;
    }

    memcpy(&dev->cfg, cfg, sizeof(*cfg));
    return 0;
}

/**
 * Calculate and set SPI clock rate divisor.
 * Returns prescaler bits (SPR1:SPR0) and SPI2X bit for SPSR.
 */
static void spi_set_clock_rate(annex_hal_spi_dev_t *dev)
{
    uint32_t target_freq = dev->cfg.baud_rate_hz;
    uint32_t fosc = dev->input_clk_hz;

    /* Clock divisors available: 2, 4, 8, 16, 32, 64, 128
     * SPR1:SPR0 divides by 4, 16, 64, 256 (with SPI2X=0)
     * With SPI2X=1: divide by 2, 8, 32, 128
     */

    uint8_t spr_bits = 0;
    uint8_t spi2x_bit = 0;

    if (target_freq >= fosc / 2) {
        spr_bits = 0;
        spi2x_bit = 1;  /* fosc/2 */
    } else if (target_freq >= fosc / 4) {
        spr_bits = 0;
        spi2x_bit = 0;  /* fosc/4 */
    } else if (target_freq >= fosc / 8) {
        spr_bits = 1;
        spi2x_bit = 1;  /* fosc/8 */
    } else if (target_freq >= fosc / 16) {
        spr_bits = 1;
        spi2x_bit = 0;  /* fosc/16 */
    } else if (target_freq >= fosc / 32) {
        spr_bits = 2;
        spi2x_bit = 1;  /* fosc/32 */
    } else if (target_freq >= fosc / 64) {
        spr_bits = 2;
        spi2x_bit = 0;  /* fosc/64 */
    } else {
        spr_bits = 3;
        spi2x_bit = 0;  /* fosc/128 */
    }

    uint8_t spcr_val = *dev->spcr;
    spcr_val &= ~((1u << SPR1) | (1u << SPR0));
    spcr_val |= (spr_bits << SPR0);

    uint8_t spsr_val = *dev->spsr;
    if (spi2x_bit) {
        spsr_val |= (1u << SPI2X);
    } else {
        spsr_val &= ~(1u << SPI2X);
    }

    *dev->spsr = spsr_val;
    *dev->spcr = spcr_val;
}

int annex_hal_spi_enable(annex_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return -1;
    }

    uint8_t spcr_val = 0;

    /* Set MSTR bit if master mode */
    if (dev->cfg.role == ANNEX_HAL_SPI_MASTER) {
        spcr_val |= (1u << MSTR);
    }

    /* Set DORD bit for bit order */
    if (dev->cfg.bit_order == ANNEX_HAL_SPI_LSB_FIRST) {
        spcr_val |= (1u << DORD);
    }

    /* Set CPOL for clock polarity */
    if (dev->cfg.mode == ANNEX_HAL_SPI_MODE_2 || dev->cfg.mode == ANNEX_HAL_SPI_MODE_3) {
        spcr_val |= (1u << CPOL);
    }

    /* Set CPHA for clock phase */
    if (dev->cfg.mode == ANNEX_HAL_SPI_MODE_1 || dev->cfg.mode == ANNEX_HAL_SPI_MODE_3) {
        spcr_val |= (1u << CPHA);
    }

    /* Set SPE to enable SPI */
    spcr_val |= (1u << SPE);

    *dev->spcr = spcr_val;

    /* Set clock rate */
    spi_set_clock_rate(dev);

    return 0;
}

int annex_hal_spi_disable(annex_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return -1;
    }

    *dev->spcr &= ~(1u << SPE);
    return 0;
}

int annex_hal_spi_transfer(annex_hal_spi_dev_t *dev,
                          const uint8_t *tx_data,
                          uint8_t *rx_data,
                          uint32_t len)
{
    if (dev == NULL || len == 0) {
        return -1;
    }

    uint32_t transferred = 0;

    for (uint32_t i = 0; i < len; i++) {
        /* Send byte */
        uint8_t tx_byte = (tx_data != NULL) ? tx_data[i] : 0x00;
        *dev->spdr = tx_byte;

        /* Wait for transfer complete */
        while (!(*dev->spsr & (1u << SPIF))) {
            /* Spin wait */
        }

        /* Read received byte */
        uint8_t rx_byte = *dev->spdr;
        if (rx_data != NULL) {
            rx_data[i] = rx_byte;
        }

        /* Clear SPIF by reading SPSR then SPDR (already read above) */
        uint8_t dummy = *dev->spsr;
        (void)dummy;

        transferred++;
    }

    return (int)transferred;
}

int annex_hal_spi_write(annex_hal_spi_dev_t *dev, const uint8_t *data, uint32_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return -1;
    }

    return annex_hal_spi_transfer(dev, data, NULL, len);
}

int annex_hal_spi_read(annex_hal_spi_dev_t *dev, uint8_t *data, uint32_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return -1;
    }

    return annex_hal_spi_transfer(dev, NULL, data, len);
}

bool annex_hal_spi_is_busy(annex_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    /* Check SPIF flag - if not set, transfer is still in progress */
    return !(*dev->spsr & (1u << SPIF));
}

void annex_hal_spi_enable_irq(annex_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    dev->irq_mask |= irqs;

    if (irqs & ANNEX_HAL_SPI_IRQ_TRANSFER_DONE) {
        *dev->spcr |= (1u << SPIE);  /* Enable SPI interrupt */
    }
}

void annex_hal_spi_disable_irq(annex_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    dev->irq_mask &= ~irqs;

    if ((irqs & ANNEX_HAL_SPI_IRQ_TRANSFER_DONE) && !(dev->irq_mask & ANNEX_HAL_SPI_IRQ_TRANSFER_DONE)) {
        *dev->spcr &= ~(1u << SPIE);  /* Disable SPI interrupt */
    }
}

uint32_t annex_hal_spi_get_irq_status(annex_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return 0;
    }

    uint32_t status = 0;

    if (*dev->spsr & (1u << SPIF)) {
        status |= ANNEX_HAL_SPI_IRQ_TRANSFER_DONE;
    }

    if (*dev->spsr & (1u << WCOL)) {
        status |= ANNEX_HAL_SPI_IRQ_ERROR;
    }

    return status;
}

void annex_hal_spi_clear_irq(annex_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    if (irqs & ANNEX_HAL_SPI_IRQ_TRANSFER_DONE) {
        /* Clear SPIF by reading SPSR followed by SPDR */
        uint8_t dummy = *dev->spsr;
        dummy = *dev->spdr;
        (void)dummy;
    }

    if (irqs & ANNEX_HAL_SPI_IRQ_ERROR) {
        /* WCOL is cleared by reading SPSR followed by SPDR */
        uint8_t dummy = *dev->spsr;
        dummy = *dev->spdr;
        (void)dummy;
    }

    dev->irq_status &= ~irqs;
}
