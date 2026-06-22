/**
 * @file annex_hal_spi.h
 * @brief SPI HAL interface.
 *
 * Each supported MCU (e.g. ATmega328p, STM32) implements this HAL
 * in its own annex_hal_spi_xxx.c, defining the real layout of
 * `struct annex_hal_spi_dev`. 
 *
 */

#ifndef ANNEX_HAL_SPI_H
#define ANNEX_HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IRQ flags for SPI events */
#define ANNEX_HAL_SPI_IRQ_RX_READY   (1U << 0)
#define ANNEX_HAL_SPI_IRQ_TX_EMPTY   (1U << 1)
#define ANNEX_HAL_SPI_IRQ_TRANSFER_DONE (1U << 2)
#define ANNEX_HAL_SPI_IRQ_ERROR      (1U << 3)

/* SPI Mode: CPOL (bit 1) and CPHA (bit 0) */
typedef enum {
    ANNEX_HAL_SPI_MODE_0 = 0x00,  /* CPOL=0, CPHA=0 */
    ANNEX_HAL_SPI_MODE_1 = 0x01,  /* CPOL=0, CPHA=1 */
    ANNEX_HAL_SPI_MODE_2 = 0x02,  /* CPOL=1, CPHA=0 */
    ANNEX_HAL_SPI_MODE_3 = 0x03,  /* CPOL=1, CPHA=1 */
} annex_hal_spi_mode_t;

/* Clock polarity */
typedef enum {
    ANNEX_HAL_SPI_CPOL_IDLE_LOW  = 0,
    ANNEX_HAL_SPI_CPOL_IDLE_HIGH = 1,
} annex_hal_spi_cpol_t;

/* Clock phase */
typedef enum {
    ANNEX_HAL_SPI_CPHA_LEADING_EDGE  = 0,
    ANNEX_HAL_SPI_CPHA_TRAILING_EDGE = 1,
} annex_hal_spi_cpha_t;

/* Master/Slave mode */
typedef enum {
    ANNEX_HAL_SPI_MASTER = 0,
    ANNEX_HAL_SPI_SLAVE  = 1,
} annex_hal_spi_role_t;

/* Bit order */
typedef enum {
    ANNEX_HAL_SPI_LSB_FIRST = 0,
    ANNEX_HAL_SPI_MSB_FIRST = 1,
} annex_hal_spi_bit_order_t;

/* Return codes */
typedef enum {
    ANNEX_HAL_SPI_OK = 0,
    ANNEX_HAL_SPI_ERR_NULL_PTR,
    ANNEX_HAL_SPI_ERR_INVALID_PARAM,
    ANNEX_HAL_SPI_ERR_BUSY,
    ANNEX_HAL_SPI_ERR_TIMEOUT,
    ANNEX_HAL_SPI_ERR_HW,
    ANNEX_HAL_SPI_ERR_NOT_SUPPORTED,
} annex_hal_spi_status_t;

/* Opaque SPI device handle */
typedef struct annex_hal_spi_dev annex_hal_spi_dev_t;

/* SPI Configuration */
typedef struct {
    uint32_t         baud_rate_hz;  /* Clock frequency in Hz */
    annex_hal_spi_mode_t mode;      /* SPI mode (0-3) */
    annex_hal_spi_role_t role;      /* Master or Slave */
    annex_hal_spi_bit_order_t bit_order; /* LSB or MSB first */
    uint8_t          bits_per_word;  /* 8, 16, etc. */
    bool             use_dma;        /* DMA transfer support */
} annex_hal_spi_config_t;

/* Hardware descriptor for SPI peripheral */
typedef struct {
    uintptr_t  base_addr;      /* MMIO base address of SPI controller */
    uint32_t   input_clk_hz;   /* Input clock frequency for baud calculation */
    int8_t     mosi_pin;       /* MOSI pin number (-1 = unused) */
    int8_t     miso_pin;       /* MISO pin number (-1 = unused) */
    int8_t     sck_pin;        /* SCK pin number (-1 = unused) */
    int8_t     cs_pin;         /* Chip Select pin number (-1 = unused/GPIO) */
    void      *platform_data;  /* Platform-specific extras (optional) */
} annex_hal_spi_hw_desc_t;

/* --- Lifecycle ------------------------------------------------------ */

/**
 * Open and initialize an SPI peripheral.
 * @param hw Hardware descriptor with peripheral details.
 * @return SPI device handle on success, NULL on failure.
 */
annex_hal_spi_dev_t *annex_hal_spi_open(const annex_hal_spi_hw_desc_t *hw);

/**
 * Close and disable an SPI peripheral.
 * @param dev SPI device handle obtained from annex_hal_spi_open().
 */
void annex_hal_spi_close(annex_hal_spi_dev_t *dev);

/* --- Configuration ---------------------------------------------------- */

/**
 * Configure SPI settings (mode, baud rate, etc).
 * @param dev SPI device handle.
 * @param cfg SPI configuration structure.
 * @return Status code.
 */
int annex_hal_spi_config(annex_hal_spi_dev_t *dev, const annex_hal_spi_config_t *cfg);

/**
 * Enable SPI peripheral.
 * @param dev SPI device handle.
 * @return Status code.
 */
int annex_hal_spi_enable(annex_hal_spi_dev_t *dev);

/**
 * Disable SPI peripheral.
 * @param dev SPI device handle.
 * @return Status code.
 */
int annex_hal_spi_disable(annex_hal_spi_dev_t *dev);

/* --- Data Transfer ------------------------------------------------------ */

/**
 * Perform a full-duplex SPI transfer.
 * @param dev SPI device handle.
 * @param tx_data Bytes to transmit (NULL = send zeros).
 * @param rx_data Buffer for received bytes (NULL = discard).
 * @param len Number of bytes to transfer.
 * @return Number of bytes transferred, or negative on error.
 */
int annex_hal_spi_transfer(annex_hal_spi_dev_t *dev,
                          const uint8_t *tx_data,
                          uint8_t *rx_data,
                          uint32_t len);

/**
 * Write (transmit only) data on SPI bus.
 * @param dev SPI device handle.
 * @param data Bytes to transmit.
 * @param len Number of bytes.
 * @return Number of bytes written, or negative on error.
 */
int annex_hal_spi_write(annex_hal_spi_dev_t *dev, const uint8_t *data, uint32_t len);

/**
 * Read (receive only) data from SPI bus.
 * @param dev SPI device handle.
 * @param data Buffer for received bytes.
 * @param len Number of bytes to read.
 * @return Number of bytes read, or negative on error.
 */
int annex_hal_spi_read(annex_hal_spi_dev_t *dev, uint8_t *data, uint32_t len);

/* --- Status and IRQ ---------------------------------------------------- */

/**
 * Check if SPI is busy (transfer in progress).
 * @param dev SPI device handle.
 * @return true if busy, false if idle.
 */
bool annex_hal_spi_is_busy(annex_hal_spi_dev_t *dev);

/**
 * Enable SPI interrupts.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to enable (ANNEX_HAL_SPI_IRQ_*).
 */
void annex_hal_spi_enable_irq(annex_hal_spi_dev_t *dev, uint32_t irqs);

/**
 * Disable SPI interrupts.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to disable (ANNEX_HAL_SPI_IRQ_*).
 */
void annex_hal_spi_disable_irq(annex_hal_spi_dev_t *dev, uint32_t irqs);

/**
 * Get current IRQ status flags.
 * @param dev SPI device handle.
 * @return Bitmask of active IRQs.
 */
uint32_t annex_hal_spi_get_irq_status(annex_hal_spi_dev_t *dev);

/**
 * Clear pending IRQ flags.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to clear.
 */
void annex_hal_spi_clear_irq(annex_hal_spi_dev_t *dev, uint32_t irqs);

#ifdef __cplusplus
}
#endif

#endif /* ANNEX_HAL_SPI_H */
