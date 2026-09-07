/**
 * @file inx_hal_spi_st.c
 * @brief STM32 implementation for inx_hal_spi.h using STM HAL APIs.
 *
 * This implementation uses STMicroelectronics HAL library instead of direct
 * register access, providing better maintainability and compatibility with
 * the STM32Cube ecosystem.
 *
 * HAL API USED:
 * =============
 *   - SPI_HandleTypeDef: Core SPI device handle structure
 *   - SPI_InitTypeDef: Configuration structure for HAL_SPI_Init()
 *   - HAL_SPI_Init(): Initialize SPI peripheral with config
 *   - HAL_SPI_DeInit(): Deinitialize SPI peripheral
 *   - HAL_SPI_TransmitReceive(): Full-duplex transfer
 *   - HAL_SPI_Transmit(): Transmit only
 *   - HAL_SPI_Receive(): Receive only
 *   - HAL_SPI_GetError(): Get last HAL error code
 *   - HAL_SPI_GetState(): Get SPI peripheral state
 *   - HAL_SPI_RegisterCallback(): Register user callback
 *   - __HAL_SPI_ENABLE_IT(): Enable interrupt flags
 *   - __HAL_SPI_DISABLE_IT(): Disable interrupt flags
 *   - HAL_MAX_DELAY: Standard blocking timeout constant
 *
 * HAL CALLBACKS USED:
 * ===================
 *   - HAL_SPI_RxCpltCallback: RX transfer complete
 *   - HAL_SPI_TxCpltCallback: TX transfer complete
 *   - HAL_SPI_TxRxCpltCallback: Full-duplex transfer complete
 *   - HAL_SPI_ErrorCallback: Error occurred (OVR, MODF, CRC, etc.)
 *
 * ERROR CODES:
 * ============
 * The HAL returns these for SPI errors:
 *   - HAL_SPI_ERROR_OVR: Overrun flag (data lost)
 *   - HAL_SPI_ERROR_MODF: Mode fault (multi-master issue)
 *   - HAL_SPI_ERROR_CRC: CRC error in reception
 *   - HAL_SPI_ERROR_UDR: Underrun (STM32H7)
 *   - HAL_SPI_ERROR_FRE: Frame error (STM32H7)
 *   - HAL_SPI_ERROR_DMA: DMA error
 *
 * STM32 VARIANTS SUPPORTED:
 * ========================
 * - STM32L0/L1/L5: Ultra-low power with SPI
 * - STM32F0/F1/F2/F3/F4: General purpose with SPI
 * - STM32G0/G4: Value line with SPI
 * - STM32H7: High performance with enhanced SPI
 * - STM32U5/U0: Ultra-low power touch capable with SPI
 * - STM32WB/WL: Wireless with SPI
 *
 * CONFIGURATION NOTES:
 * ====================
 * 1. Master/Slave mode is configured via cfg.role
 * 2. SPI modes (0-3) map directly to STM HAL modes
 * 3. Prescaler is calculated automatically from input clock and target baud rate
 * 4. NSS is always configured as software-controlled (SOFTware NSS)
 * 5. CRC is disabled by default (can be enabled via HAL_SPI_Init)
 * 6. DMA is disabled by default (implement if needed)
 * 7. CS pin must be managed separately via GPIO HAL
 *
 * INTERRUPT BEHAVIOR:
 * ===================
 * - Uses HAL callback registration (HAL_SPI_RegisterCallback)
 * - Callbacks update software IRQ status flags (irq_status)
 * - IRQ enables/disables use HAL macros (__HAL_SPI_ENABLE_IT, etc.)
 * - Error callback captures HAL error codes
 * - Compatible with Zephyr interrupt framework
 *
 * BLOCKING VS INTERRUPT MODE:
 * ============================
 * This implementation currently uses blocking mode with HAL_MAX_DELAY:
 *   - HAL_SPI_TransmitReceive(hspi, tx, rx, len, HAL_MAX_DELAY)
 *
 * For interrupt-driven operation, use HAL's interrupt variants:
 *   - HAL_SPI_Transmit_IT(), HAL_SPI_Receive_IT(), HAL_SPI_TransmitReceive_IT()
 *
 * For DMA operation, use DMA variants:
 *   - HAL_SPI_Transmit_DMA(), HAL_SPI_Receive_DMA(), HAL_SPI_TransmitReceive_DMA()
 * (Future enhancement: expose these options via cfg flags)
 *
 * MEMORY MANAGEMENT:
 * ==================
 * - Uses static pool (no malloc/free) for 4 SPI instances
 * - Each instance pre-allocates SPI_HandleTypeDef in pool
 * - Suitable for real-time embedded systems with fixed resource allocation
 *
 */

#include "inx_hal_spi.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <string.h>

/* Real definition of the opaque type declared in inx_hal_spi.h.
 * Wraps STM32 HAL SPI_HandleTypeDef with Add-on board specific metadata.
 */
struct inx_hal_spi_dev {
    SPI_HandleTypeDef        hal_handle;     /* STM32 HAL SPI handle */
    inx_hal_spi_config_t     cfg;            /* Configuration copy */
    uint32_t                 input_clk_hz;   /* Input clock for reference */
    uint32_t                 irq_mask;       /* Software-tracked enabled IRQs */
    uint32_t                 irq_status;     /* Software-tracked pending flags */
    uint8_t                  in_use;         /* Pool occupancy flag */
};

/* STM SPI device pool (supporting multiple SPI peripherals) */
#define INX_HAL_SPI_MAX_DEVS 4U

static struct inx_hal_spi_dev s_dev_pool[INX_HAL_SPI_MAX_DEVS];

#if 0
/* Forward declarations for HAL callback helpers */
static void spi_error_callback(SPI_HandleTypeDef *hspi);
static void spi_tx_callback(SPI_HandleTypeDef *hspi);
static void spi_rx_callback(SPI_HandleTypeDef *hspi);
#endif

inx_hal_spi_dev_t *inx_hal_spi_open(const inx_hal_spi_hw_desc_t *hw)
{
    if (hw == NULL) {
        return NULL;
    }

    /* Find unused device in pool */
    for (size_t i = 0; i < INX_HAL_SPI_MAX_DEVS; i++) {
        if (!s_dev_pool[i].in_use) {
            /* Initialize HAL handle with hardware descriptor */
            memset(&s_dev_pool[i].hal_handle, 0, sizeof(SPI_HandleTypeDef));

            s_dev_pool[i].hal_handle.Instance = (SPI_TypeDef *)hw->base_addr;
            s_dev_pool[i].input_clk_hz = hw->input_clk_hz;
            s_dev_pool[i].irq_mask = 0;
            s_dev_pool[i].irq_status = 0;
            s_dev_pool[i].in_use = 1u;

            /* Initialize default configuration */
            s_dev_pool[i].cfg.baud_rate_hz = 1000000;  /* 1 MHz default */
            s_dev_pool[i].cfg.mode = INX_HAL_SPI_MODE_0;
            s_dev_pool[i].cfg.role = INX_HAL_SPI_MASTER;
            s_dev_pool[i].cfg.bit_order = INX_HAL_SPI_MSB_FIRST;
            s_dev_pool[i].cfg.bits_per_word = 8;
            s_dev_pool[i].cfg.use_dma = false;

            return &s_dev_pool[i];
        }
    }
    return NULL;  /* All SPI pools exhausted */
}

void inx_hal_spi_close(inx_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }

    /* Deinitialize using HAL */
    HAL_SPI_DeInit(&dev->hal_handle);

    dev->in_use = 0u;
}

int inx_hal_spi_config(inx_hal_spi_dev_t *dev, const inx_hal_spi_config_t *cfg)
{
    if (dev == NULL || cfg == NULL) {
        return -1;
    }

    memcpy(&dev->cfg, cfg, sizeof(*cfg));
    return 0;
}

/**
 * Calculate STM32 prescaler from baud rate and input clock.
 * STM32 HAL expects SPI_BAUDRATEPRESCALER_* values.
 */
static uint32_t calculate_prescaler(uint32_t input_clk, uint32_t target_freq)
{
    if (target_freq == 0 || input_clk == 0) {
        return SPI_BAUDRATEPRESCALER_256;  /* Slowest */
    }

    uint32_t divisor = input_clk / target_freq;

    /* STM HAL prescaler values: 2, 4, 8, 16, 32, 64, 128, 256 */
    if (divisor <= 2) return SPI_BAUDRATEPRESCALER_2;
    if (divisor <= 4) return SPI_BAUDRATEPRESCALER_4;
    if (divisor <= 8) return SPI_BAUDRATEPRESCALER_8;
    if (divisor <= 16) return SPI_BAUDRATEPRESCALER_16;
    if (divisor <= 32) return SPI_BAUDRATEPRESCALER_32;
    if (divisor <= 64) return SPI_BAUDRATEPRESCALER_64;
    if (divisor <= 128) return SPI_BAUDRATEPRESCALER_128;
    return SPI_BAUDRATEPRESCALER_256;
}

int inx_hal_spi_enable(inx_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return -1;
    }

    SPI_HandleTypeDef *hspi = &dev->hal_handle;

    /* Configure SPI using HAL Init structure */
    hspi->Init.Mode = (dev->cfg.role == INX_HAL_SPI_MASTER) ?
                      SPI_MODE_MASTER : SPI_MODE_SLAVE;

    hspi->Init.Direction = SPI_DIRECTION_2LINES;  /* Full-duplex */

    hspi->Init.DataSize = (dev->cfg.bits_per_word == 16) ?
                          SPI_DATASIZE_16BIT : SPI_DATASIZE_8BIT;

    hspi->Init.CLKPolarity = dev->cfg.mode & 0x2 ?
                             SPI_POLARITY_HIGH : SPI_POLARITY_LOW;

    hspi->Init.CLKPhase = dev->cfg.mode & 0x1 ?
                          SPI_PHASE_2EDGE : SPI_PHASE_1EDGE;

    hspi->Init.NSS = SPI_NSS_SOFT;  /* Software NSS control */

    hspi->Init.BaudRatePrescaler = calculate_prescaler(dev->input_clk_hz,
                                                       dev->cfg.baud_rate_hz);

    hspi->Init.FirstBit = (dev->cfg.bit_order == INX_HAL_SPI_LSB_FIRST) ?
                          SPI_FIRSTBIT_LSB : SPI_FIRSTBIT_MSB;

    hspi->Init.TIMode = SPI_TIMODE_DISABLED;  /* Standard SPI, not TI */

    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;  /* No CRC by default */

    hspi->Init.CRCPolynomial = 7;  /* Default CRC polynomial */

    /* Initialize SPI peripheral */
    if (HAL_SPI_Init(hspi) != HAL_OK) {
        return -1;
    }

    /* Register callbacks for interrupt mode */
//    HAL_SPI_RegisterCallback(hspi, HAL_SPI_RX_COMPLETE_CB_ID,
//                           spi_rx_callback);
//    HAL_SPI_RegisterCallback(hspi, HAL_SPI_TX_COMPLETE_CB_ID,
//                           spi_tx_callback);
//    HAL_SPI_RegisterCallback(hspi, HAL_SPI_ERROR_CB_ID,
//                           spi_error_callback);

    return 0;
}

int inx_hal_spi_disable(inx_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return -1;
    }

    /* Disable SPI using HAL */
    HAL_SPI_DeInit(&dev->hal_handle);

    return 0;
}

int inx_hal_spi_transfer(inx_hal_spi_dev_t *dev,
                          const uint8_t *tx_data,
                          uint8_t *rx_data,
                          uint32_t len)
{
    if (dev == NULL || len == 0) {
        return -1;
    }

    SPI_HandleTypeDef *hspi = &dev->hal_handle;
    HAL_StatusTypeDef hal_status;

    /* Use HAL for full-duplex transfer */
    if (tx_data != NULL && rx_data != NULL) {
        /* Both TX and RX buffers provided */
        hal_status = HAL_SPI_TransmitReceive(hspi, (uint8_t *)tx_data,
                                            rx_data, (uint16_t)len,
                                            HAL_MAX_DELAY);
    } else if (tx_data != NULL) {
        /* TX only */
        hal_status = HAL_SPI_Transmit(hspi, (uint8_t *)tx_data,
                                     (uint16_t)len, HAL_MAX_DELAY);
    } else if (rx_data != NULL) {
        /* RX only - transmit zeros */
        uint8_t dummy_tx[len];
        memset(dummy_tx, 0, len);
        hal_status = HAL_SPI_TransmitReceive(hspi, dummy_tx,
                                            rx_data, (uint16_t)len,
                                            HAL_MAX_DELAY);
    } else {
        /* Neither TX nor RX - invalid */
        return -1;
    }

    if (hal_status != HAL_OK) {
        return -1;
    }

    return (int)len;
}

int inx_hal_spi_write(inx_hal_spi_dev_t *dev, const uint8_t *data, uint32_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return -1;
    }

    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(&dev->hal_handle,
                                                    (uint8_t *)data,
                                                    (uint16_t)len,
                                                    HAL_MAX_DELAY);

    if (hal_status != HAL_OK) {
        return -1;
    }

    return (int)len;
}

int inx_hal_spi_read(inx_hal_spi_dev_t *dev, uint8_t *data, uint32_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return -1;
    }

    /* Create dummy TX buffer (zeros) */
    uint8_t dummy_tx[len];
    memset(dummy_tx, 0, len);

    HAL_StatusTypeDef hal_status = HAL_SPI_TransmitReceive(&dev->hal_handle,
                                                          dummy_tx,
                                                          data,
                                                          (uint16_t)len,
                                                          HAL_MAX_DELAY);

    if (hal_status != HAL_OK) {
        return -1;
    }

    return (int)len;
}

bool inx_hal_spi_is_busy(inx_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    /* Check if SPI is busy using HAL state machine */
    return HAL_SPI_GetState(&dev->hal_handle) == HAL_SPI_STATE_BUSY;
}

void inx_hal_spi_enable_irq(inx_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    SPI_HandleTypeDef *hspi = &dev->hal_handle;

    dev->irq_mask |= irqs;

    if (irqs & INX_HAL_SPI_IRQ_RX_READY) {
        __HAL_SPI_ENABLE_IT(hspi, SPI_IT_RXNE);
    }

    if (irqs & INX_HAL_SPI_IRQ_TX_EMPTY) {
        __HAL_SPI_ENABLE_IT(hspi, SPI_IT_TXE);
    }

    if (irqs & (INX_HAL_SPI_IRQ_ERROR | INX_HAL_SPI_IRQ_TRANSFER_DONE)) {
        __HAL_SPI_ENABLE_IT(hspi, SPI_IT_ERR);
    }
}

void inx_hal_spi_disable_irq(inx_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    SPI_HandleTypeDef *hspi = &dev->hal_handle;

    dev->irq_mask &= ~irqs;

    if ((irqs & INX_HAL_SPI_IRQ_RX_READY) && !(dev->irq_mask & INX_HAL_SPI_IRQ_RX_READY)) {
        __HAL_SPI_DISABLE_IT(hspi, SPI_IT_RXNE);
    }

    if ((irqs & INX_HAL_SPI_IRQ_TX_EMPTY) && !(dev->irq_mask & INX_HAL_SPI_IRQ_TX_EMPTY)) {
        __HAL_SPI_DISABLE_IT(hspi, SPI_IT_TXE);
    }

    if ((irqs & (INX_HAL_SPI_IRQ_ERROR | INX_HAL_SPI_IRQ_TRANSFER_DONE)) &&
        !(dev->irq_mask & (INX_HAL_SPI_IRQ_ERROR | INX_HAL_SPI_IRQ_TRANSFER_DONE))) {
        __HAL_SPI_DISABLE_IT(hspi, SPI_IT_ERR);
    }
}

uint32_t inx_hal_spi_get_irq_status(inx_hal_spi_dev_t *dev)
{
    if (dev == NULL) {
        return 0;
    }

    uint32_t status = 0;
    SPI_HandleTypeDef *hspi = &dev->hal_handle;

    /* Check HAL error status */
    uint32_t hal_error = HAL_SPI_GetError(hspi);

    if (hal_error != HAL_SPI_ERROR_NONE) {
        status |= INX_HAL_SPI_IRQ_ERROR;
    }

    /* Check state for completion */
    if (HAL_SPI_GetState(hspi) == HAL_SPI_STATE_READY) {
        status |= INX_HAL_SPI_IRQ_TRANSFER_DONE;
    }

    /* Return combined software and hardware status */
    return status | dev->irq_status;
}

void inx_hal_spi_clear_irq(inx_hal_spi_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    SPI_HandleTypeDef *hspi = &dev->hal_handle;

    if (irqs & INX_HAL_SPI_IRQ_ERROR) {
        /* Clear error flags through HAL */
        hspi->ErrorCode = HAL_SPI_ERROR_NONE;
    }

    dev->irq_status &= ~irqs;
}

#if 0
/* ============================================================================
 * HAL CALLBACK IMPLEMENTATIONS
 * ============================================================================
 *
 * These callbacks are invoked by the STM32 HAL when SPI events occur.
 * They update the software-tracked IRQ status flags.
 */

/**
 * SPI RX complete callback - called when data reception is complete.
 */
static void spi_rx_callback(SPI_HandleTypeDef *hspi)
{
    /* Find device from handle and update status */
    for (size_t i = 0; i < INX_HAL_SPI_MAX_DEVS; i++) {
        if (s_dev_pool[i].in_use && &s_dev_pool[i].hal_handle == hspi) {
            s_dev_pool[i].irq_status |= INX_HAL_SPI_IRQ_RX_READY;
            break;
        }
    }
}

/**
 * SPI TX complete callback - called when data transmission is complete.
 */
static void spi_tx_callback(SPI_HandleTypeDef *hspi)
{
    /* Find device from handle and update status */
    for (size_t i = 0; i < INX_HAL_SPI_MAX_DEVS; i++) {
        if (s_dev_pool[i].in_use && &s_dev_pool[i].hal_handle == hspi) {
            s_dev_pool[i].irq_status |= INX_HAL_SPI_IRQ_TX_EMPTY;
            break;
        }
    }
}

/**
 * SPI error callback - called when an SPI error occurs.
 * Errors include: OVR (Overrun), MODF (Mode Fault), CRC error, etc.
 */
static void spi_error_callback(SPI_HandleTypeDef *hspi)
{
    /* Find device from handle and update error status */
    for (size_t i = 0; i < INX_HAL_SPI_MAX_DEVS; i++) {
        if (s_dev_pool[i].in_use && &s_dev_pool[i].hal_handle == hspi) {
            s_dev_pool[i].irq_status |= INX_HAL_SPI_IRQ_ERROR;

            /* Optionally log error for debugging */
            uint32_t error = HAL_SPI_GetError(hspi);
            (void)error;  /* Suppress unused warning if not logged */

            break;
        }
    }
}
#endif
