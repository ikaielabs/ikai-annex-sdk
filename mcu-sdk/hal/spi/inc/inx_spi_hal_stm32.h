/**
 * @file inx_hal_spi_st.h
 * @brief STM Compatibility header
 *
 */

#ifndef STM32_HAL_SPI_H
#define STM32_HAL_SPI_H

// Include the correct STM variant
#if defined(STM32L1)
  #include "stm32l1xx_hal_spi.h"
  #include "stm32l1xx_hal.h"
#elif defined(STM32H7)
  #include "stm32h7xx_hal_spi.h"
  #include "stm32h7xx_hal.h"
#elif defined(STM32F4)
  #include "stm32f4xx_hal_spi.h"
  #include "stm32f4xx_hal.h"
#else
  #error "STM32 variant not defined"
#endif

#endif // STM32_HAL_SPI_H