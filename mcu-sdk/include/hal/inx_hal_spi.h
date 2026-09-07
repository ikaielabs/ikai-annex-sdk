/**
 * @file inx_hal_spi.h
 * @brief SPI HAL interface.
 *
 * Each supported MCU (e.g. ATmega328p, STM32) implements this HAL
 * in its own inx_hal_spi_xxx.c, defining the real layout of
 * `struct inx_hal_spi_dev`. 
 *
 */

#ifndef INX_HAL_SPI_H
#define INX_HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IRQ flags for SPI events */
#define INX_HAL_SPI_IRQ_RX_READY   (1U << 0)
#define INX_HAL_SPI_IRQ_TX_EMPTY   (1U << 1)
#define INX_HAL_SPI_IRQ_TRANSFER_DONE (1U << 2)
#define INX_HAL_SPI_IRQ_ERROR      (1U << 3)

/* SPI Mode: CPOL (bit 1) and CPHA (bit 0) */
typedef enum {
    INX_HAL_SPI_MODE_0 = 0x00,  /* CPOL=0, CPHA=0 */
    INX_HAL_SPI_MODE_1 = 0x01,  /* CPOL=0, CPHA=1 */
    INX_HAL_SPI_MODE_2 = 0x02,  /* CPOL=1, CPHA=0 */
    INX_HAL_SPI_MODE_3 = 0x03,  /* CPOL=1, CPHA=1 */
} inx_hal_spi_mode_t;

/* Clock polarity */
typedef enum {
    INX_HAL_SPI_CPOL_IDLE_LOW  = 0,
    INX_HAL_SPI_CPOL_IDLE_HIGH = 1,
} inx_hal_spi_cpol_t;

/* Clock phase */
typedef enum {
    INX_HAL_SPI_CPHA_LEADING_EDGE  = 0,
    INX_HAL_SPI_CPHA_TRAILING_EDGE = 1,
} inx_hal_spi_cpha_t;

/* Master/Slave mode */
typedef enum {
    INX_HAL_SPI_MASTER = 0,
    INX_HAL_SPI_SLAVE  = 1,
} inx_hal_spi_role_t;

/* Bit order */
typedef enum {
    INX_HAL_SPI_LSB_FIRST = 0,
    INX_HAL_SPI_MSB_FIRST = 1,
} inx_hal_spi_bit_order_t;

/* Return codes */
typedef enum {
    INX_HAL_SPI_OK = 0,
    INX_HAL_SPI_ERR_NULL_PTR,
    INX_HAL_SPI_ERR_INVALID_PARAM,
    INX_HAL_SPI_ERR_BUSY,
    INX_HAL_SPI_ERR_TIMEOUT,
    INX_HAL_SPI_ERR_HW,
    INX_HAL_SPI_ERR_NOT_SUPPORTED,
} inx_hal_spi_status_t;

/* Opaque SPI device handle */
typedef struct inx_hal_spi_dev inx_hal_spi_dev_t;

/* SPI Configuration */
typedef struct {
    uint32_t         baud_rate_hz;  /* Clock frequency in Hz */
    inx_hal_spi_mode_t mode;      /* SPI mode (0-3) */
    inx_hal_spi_role_t role;      /* Master or Slave */
    inx_hal_spi_bit_order_t bit_order; /* LSB or MSB first */
    uint8_t          bits_per_word;  /* 8, 16, etc. */
    bool             use_dma;        /* DMA transfer support */
} inx_hal_spi_config_t;

/* Hardware descriptor for SPI peripheral */
typedef struct {
    uintptr_t  base_addr;      /* MMIO base address of SPI controller */
    uint32_t   input_clk_hz;   /* Input clock frequency for baud calculation */
    int8_t     mosi_pin;       /* MOSI pin number (-1 = unused) */
    int8_t     miso_pin;       /* MISO pin number (-1 = unused) */
    int8_t     sck_pin;        /* SCK pin number (-1 = unused) */
    int8_t     cs_pin;         /* Chip Select pin number (-1 = unused/GPIO) */
    void      *platform_data;  /* Platform-specific extras (optional) */
} inx_hal_spi_hw_desc_t;

/* --- Lifecycle ------------------------------------------------------ */

/**
 * Open and initialize an SPI peripheral.
 * @param hw Hardware descriptor with peripheral details.
 * @return SPI device handle on success, NULL on failure.
 */
inx_hal_spi_dev_t *inx_hal_spi_open(const inx_hal_spi_hw_desc_t *hw);

/**
 * Close and disable an SPI peripheral.
 * @param dev SPI device handle obtained from inx_hal_spi_open().
 */
void inx_hal_spi_close(inx_hal_spi_dev_t *dev);

/* --- Configuration ---------------------------------------------------- */

/**
 * Configure SPI settings (mode, baud rate, etc).
 * @param dev SPI device handle.
 * @param cfg SPI configuration structure.
 * @return Status code.
 */
int inx_hal_spi_config(inx_hal_spi_dev_t *dev, const inx_hal_spi_config_t *cfg);

/**
 * Enable SPI peripheral.
 * @param dev SPI device handle.
 * @return Status code.
 */
int inx_hal_spi_enable(inx_hal_spi_dev_t *dev);

/**
 * Disable SPI peripheral.
 * @param dev SPI device handle.
 * @return Status code.
 */
int inx_hal_spi_disable(inx_hal_spi_dev_t *dev);

/* --- Data Transfer ------------------------------------------------------ */

/**
 * Perform a full-duplex SPI transfer.
 * @param dev SPI device handle.
 * @param tx_data Bytes to transmit (NULL = send zeros).
 * @param rx_data Buffer for received bytes (NULL = discard).
 * @param len Number of bytes to transfer.
 * @return Number of bytes transferred, or negative on error.
 */
int inx_hal_spi_transfer(inx_hal_spi_dev_t *dev,
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
int inx_hal_spi_write(inx_hal_spi_dev_t *dev, const uint8_t *data, uint32_t len);

/**
 * Read (receive only) data from SPI bus.
 * @param dev SPI device handle.
 * @param data Buffer for received bytes.
 * @param len Number of bytes to read.
 * @return Number of bytes read, or negative on error.
 */
int inx_hal_spi_read(inx_hal_spi_dev_t *dev, uint8_t *data, uint32_t len);

/* --- Status and IRQ ---------------------------------------------------- */

/**
 * Check if SPI is busy (transfer in progress).
 * @param dev SPI device handle.
 * @return true if busy, false if idle.
 */
bool inx_hal_spi_is_busy(inx_hal_spi_dev_t *dev);

/**
 * Enable SPI interrupts.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to enable (INX_HAL_SPI_IRQ_*).
 */
void inx_hal_spi_enable_irq(inx_hal_spi_dev_t *dev, uint32_t irqs);

/**
 * Disable SPI interrupts.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to disable (INX_HAL_SPI_IRQ_*).
 */
void inx_hal_spi_disable_irq(inx_hal_spi_dev_t *dev, uint32_t irqs);

/**
 * Get current IRQ status flags.
 * @param dev SPI device handle.
 * @return Bitmask of active IRQs.
 */
uint32_t inx_hal_spi_get_irq_status(inx_hal_spi_dev_t *dev);

/**
 * Clear pending IRQ flags.
 * @param dev SPI device handle.
 * @param irqs IRQ flags to clear.
 */
void inx_hal_spi_clear_irq(inx_hal_spi_dev_t *dev, uint32_t irqs);

#ifdef __cplusplus
}
#endif

#endif /* INX_HAL_SPI_H */
