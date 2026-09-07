#include "inx_hal_uart.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <string.h>

struct inx_hal_uart_dev {
    UART_HandleTypeDef hal_handle;
    inx_hal_uart_config_t cfg;
    uint32_t irq_mask;
    uint32_t irq_status;
    uint8_t in_use;
    uint8_t lin_mode;
};

#define INX_HAL_UART_MAX_DEVS 4U

static struct inx_hal_uart_dev s_dev_pool[INX_HAL_UART_MAX_DEVS];

static uint32_t inx_hal_uart_map_parity(uint8_t parity)
{
    switch (parity) {
    case 1:
        return UART_PARITY_ODD;
    case 2:
        return UART_PARITY_EVEN;
    default:
        return UART_PARITY_NONE;
    }
}

static uint32_t inx_hal_uart_map_stop_bits(uint8_t stop_bits)
{
    return (stop_bits == 2U) ? UART_STOPBITS_2 : UART_STOPBITS_1;
}

static uint32_t inx_hal_uart_map_word_length(uint8_t data_bits)
{
    return (data_bits == 9U) ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;
}

inx_hal_uart_dev_t *inx_hal_uart_open(const inx_hal_uart_hw_desc_t *hw)
{
    if (hw == NULL || hw->base_addr == 0U) {
        return NULL;
    }

    for (size_t i = 0; i < INX_HAL_UART_MAX_DEVS; i++) {
        if (!s_dev_pool[i].in_use) {
            memset(&s_dev_pool[i], 0, sizeof(s_dev_pool[i]));
            s_dev_pool[i].hal_handle.Instance = (USART_TypeDef *)hw->base_addr;
            s_dev_pool[i].cfg.baud_rate = 9600U;
            s_dev_pool[i].cfg.data_bits = 8U;
            s_dev_pool[i].cfg.stop_bits = 1U;
            s_dev_pool[i].cfg.parity = 0U;
            s_dev_pool[i].cfg.flow_control = 0U;
            s_dev_pool[i].in_use = 1U;
            return &s_dev_pool[i];
        }
    }

    return NULL;
}

void inx_hal_uart_close(inx_hal_uart_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }

    HAL_UART_DeInit(&dev->hal_handle);
    dev->in_use = 0U;
}

int inx_hal_uart_config(inx_hal_uart_dev_t *dev, const inx_hal_uart_config_t *cfg)
{
    if (dev == NULL || cfg == NULL) {
        return -1;
    }

    memcpy(&dev->cfg, cfg, sizeof(*cfg));

    dev->hal_handle.Init.BaudRate = cfg->baud_rate;
    dev->hal_handle.Init.WordLength = inx_hal_uart_map_word_length(cfg->data_bits);
    dev->hal_handle.Init.StopBits = inx_hal_uart_map_stop_bits(cfg->stop_bits);
    dev->hal_handle.Init.Parity = inx_hal_uart_map_parity(cfg->parity);
    dev->hal_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    dev->hal_handle.Init.Mode = UART_MODE_TX_RX;
    dev->hal_handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&dev->hal_handle) != HAL_OK) {
        return -1;
    }

    return 0;
}

int inx_hal_uart_write_fifo_u8(inx_hal_uart_dev_t *dev, const uint8_t *data, uint32_t len, uint32_t timeout)
{
    if (dev == NULL || data == NULL || len == 0U) {
        return -1;
    }

    HAL_StatusTypeDef hal_status = HAL_UART_Transmit(&dev->hal_handle, data, len, timeout);

    if (hal_status != HAL_OK) {
        return -1;
    }

    return 0;

}

int inx_hal_uart_write_fifo_u16(inx_hal_uart_dev_t *dev, const uint16_t *data, uint32_t len, uint32_t timeout)
{
    (void)dev;
    (void)data;
    (void)len;
    return -1;
}

int inx_hal_uart_read_fifo_u8(inx_hal_uart_dev_t *dev, uint8_t *data, uint32_t len, uint32_t timeout)
{
    if (dev == NULL || data == NULL || len == 0U) {
        return -1;
    }

    HAL_StatusTypeDef hal_status =  HAL_UART_Receive(&dev->hal_handle, data, len, timeout);

    if (hal_status != HAL_OK) {
        return -1;
    }

    return 0;
}

int inx_hal_uart_read_fifo_u16(inx_hal_uart_dev_t *dev, uint16_t *data, uint32_t len, uint32_t timeout)
{
    (void)dev;
    (void)data;
    (void)len;
    return -1;
}

int inx_hal_uart_start(inx_hal_uart_dev_t *dev, bool tx_enable, bool rx_enable)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    USART_TypeDef *usart = dev->hal_handle.Instance;

    CLEAR_BIT(usart->CR1, USART_CR1_TE | USART_CR1_RE);
    if (tx_enable) {
        SET_BIT(usart->CR1, USART_CR1_TE);
    }
    if (rx_enable) {
        SET_BIT(usart->CR1, USART_CR1_RE);
    }

    __HAL_UART_ENABLE(&dev->hal_handle);
    return 0;
}

int inx_hal_uart_stop(inx_hal_uart_dev_t *dev)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    USART_TypeDef *usart = dev->hal_handle.Instance;
    CLEAR_BIT(usart->CR1, USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
    return 0;
}

/*
 * inx_hal_uart_lin_init() is the proper entry point into LIN mode on
 * STM32. It goes through the ST HAL's dedicated HAL_LIN_Init(), which:
 *   - re-applies the UART_InitTypeDef fields already populated by
 *     inx_hal_uart_config() (baud rate, word length, etc.)
 *   - sets USART_CR2_LINEN for us
 *   - programs the break-detection length (LBDL): 10-bit or 11-bit,
 *     which is the one piece of "how long is a break" that STM32 UART
 *     actually exposes a register for.
 *
 * This replaces the previous approach of hand-toggling CR2_LINEN, which
 * bypassed the HAL entirely and gave the caller no way to select a
 * break-detection length.
 *
 * break_length_bits: pass 11 for an 11-bit break detection length
 * (LIN 2.x / most masters), anything else selects 10-bit (LIN 1.x).
 */
int inx_hal_uart_lin_init(inx_hal_uart_dev_t *dev, uint8_t break_length_bits)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    uint8_t break_len_bits = (break_length_bits >= 11U) ? UART_LINBREAKDETECTLENGTH_11B : UART_LINBREAKDETECTLENGTH_10B;

    if (HAL_LIN_Init(&dev->hal_handle, break_len_bits) != HAL_OK) {
        return -1;
    }

    dev->lin_mode = 1U;
    return 0;
}

/*
 * Kept for backward compatibility with callers that only want to flip
 * LIN mode without picking a break-detection length. Prefer
 * inx_hal_uart_lin_init() for actually entering LIN mode; passing
 * enable=false here just clears LIN mode back to plain UART.
 */
int inx_hal_uart_set_lin_mode(inx_hal_uart_dev_t *dev, bool enable)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    if (enable) {
        return inx_hal_uart_lin_init(dev, 11U);
    }

    CLEAR_BIT(dev->hal_handle.Instance->CR2, USART_CR2_LINEN);
    dev->lin_mode = 0U;
    return 0;
}

/*
 * Sends a LIN break field via the HAL's own HAL_LIN_SendBreak(), rather
 * than setting USART_CR1_SBK by hand. HAL_LIN_SendBreak() sets SBK and
 * returns without waiting for it to clear, so we still poll
 * Transmission Complete (TC) here to give callers a synchronous
 * "break is on the wire" guarantee, matching the previous behavior.
 *
 * Note this no longer silently forces LIN mode on if it isn't already
 * configured -- inx_hal_uart_lin_init() must have been called once,
 * at LIN configuration time, not on every break.
 */
int inx_hal_uart_send_break(inx_hal_uart_dev_t *dev)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    if (!dev->lin_mode) {
        return -1;
    }

    if (HAL_LIN_SendBreak(&dev->hal_handle) != HAL_OK) {
        return -1;
    }

    USART_TypeDef *usart = dev->hal_handle.Instance;
    uint32_t timeout = 100000U;
    while ((READ_BIT(usart->SR, USART_SR_TC) == 0U) && timeout--) {
        ;
    }

    return (timeout != 0U) ? 0 : -1;
}

int inx_hal_uart_wait_for_break(inx_hal_uart_dev_t *dev, uint32_t timeout_ms)
{
    if (dev == NULL || dev->hal_handle.Instance == NULL) {
        return -1;
    }

    /* Must be in LIN mode for LBD to be meaningful */
    if (!dev->lin_mode) {
        return -1;
    }

    uint32_t start = HAL_GetTick();

    /* Wait until the LBD flag is set or timeout */
    while (!(__HAL_UART_GET_FLAG(&dev->hal_handle, UART_FLAG_LBD))) {
        if (timeout_ms != HAL_MAX_DELAY) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                return -1; /* timeout */
            }
        }
        /* small NOP to avoid tight busy-wait collapsing optimizations */
        __asm volatile ("nop");
    }

    /* Clear the LBD flag before returning to caller */
    __HAL_UART_CLEAR_FLAG(&dev->hal_handle, UART_FLAG_LBD);

    return 0;
}

void inx_hal_uart_enable_irq(inx_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    dev->irq_mask |= irqs;

    if (irqs & INX_HAL_UART_IRQ_RX_READY) {
        __HAL_UART_ENABLE_IT(&dev->hal_handle, UART_IT_RXNE);
    }
    if (irqs & INX_HAL_UART_IRQ_TX_EMPTY) {
        __HAL_UART_ENABLE_IT(&dev->hal_handle, UART_IT_TXE);
    }
    if (irqs & (INX_HAL_UART_IRQ_RX_OVERRUN | INX_HAL_UART_IRQ_FRAME_ERR)) {
        __HAL_UART_ENABLE_IT(&dev->hal_handle, UART_IT_ERR);
    }
    if (irqs & INX_HAL_UART_IRQ_BREAK_DET) {
        __HAL_UART_ENABLE_IT(&dev->hal_handle, UART_IT_LBD);
    }
}

void inx_hal_uart_disable_irq(inx_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    dev->irq_mask &= ~irqs;

    if ((irqs & INX_HAL_UART_IRQ_RX_READY) && !(dev->irq_mask & INX_HAL_UART_IRQ_RX_READY)) {
        __HAL_UART_DISABLE_IT(&dev->hal_handle, UART_IT_RXNE);
    }
    if ((irqs & INX_HAL_UART_IRQ_TX_EMPTY) && !(dev->irq_mask & INX_HAL_UART_IRQ_TX_EMPTY)) {
        __HAL_UART_DISABLE_IT(&dev->hal_handle, UART_IT_TXE);
    }
    if ((irqs & (INX_HAL_UART_IRQ_RX_OVERRUN | INX_HAL_UART_IRQ_FRAME_ERR)) &&
        !(dev->irq_mask & (INX_HAL_UART_IRQ_RX_OVERRUN | INX_HAL_UART_IRQ_FRAME_ERR))) {
        __HAL_UART_DISABLE_IT(&dev->hal_handle, UART_IT_ERR);
    }
    if ((irqs & INX_HAL_UART_IRQ_BREAK_DET) && !(dev->irq_mask & INX_HAL_UART_IRQ_BREAK_DET)) {
        __HAL_UART_DISABLE_IT(&dev->hal_handle, UART_IT_LBD);
    }
}

uint32_t inx_hal_uart_get_irq_status(inx_hal_uart_dev_t *dev)
{
    if (dev == NULL) {
        return 0U;
    }

    USART_TypeDef *usart = dev->hal_handle.Instance;
    uint32_t status = 0U;

    if ((READ_BIT(usart->SR, USART_SR_RXNE) != 0U)) {
        status |= INX_HAL_UART_IRQ_RX_READY;
    }
    if ((READ_BIT(usart->SR, USART_SR_TXE) != 0U)) {
        status |= INX_HAL_UART_IRQ_TX_EMPTY;
    }
    if ((READ_BIT(usart->SR, USART_SR_ORE) != 0U)) {
        status |= INX_HAL_UART_IRQ_RX_OVERRUN;
    }
    if ((READ_BIT(usart->SR, USART_SR_FE) != 0U)) {
        status |= INX_HAL_UART_IRQ_FRAME_ERR;
    }
    if ((READ_BIT(usart->SR, USART_SR_LBD) != 0U)) {
        status |= INX_HAL_UART_IRQ_BREAK_DET;
    }

    dev->irq_status = status;
    return status & dev->irq_mask;
}

void inx_hal_uart_clear_irq(inx_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (dev == NULL) {
        return;
    }

    dev->irq_status &= ~irqs;
    if (irqs & INX_HAL_UART_IRQ_BREAK_DET) {
        __HAL_UART_CLEAR_FLAG(&dev->hal_handle, UART_FLAG_LBD);
    }
}
