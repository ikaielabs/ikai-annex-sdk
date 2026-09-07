################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c 

OBJS += \
./Src/stm32f4xx_hal.o \
./Src/stm32f4xx_hal_cortex.o \
./Src/stm32f4xx_hal_dma.o \
./Src/stm32f4xx_hal_dma_ex.o \
./Src/stm32f4xx_hal_exti.o \
./Src/stm32f4xx_hal_flash.o \
./Src/stm32f4xx_hal_flash_ex.o \
./Src/stm32f4xx_hal_flash_ramfunc.o \
./Src/stm32f4xx_hal_gpio.o \
./Src/stm32f4xx_hal_pwr.o \
./Src/stm32f4xx_hal_pwr_ex.o \
./Src/stm32f4xx_hal_rcc.o \
./Src/stm32f4xx_hal_rcc_ex.o \
./Src/stm32f4xx_hal_spi.o \
./Src/stm32f4xx_hal_uart.o 

C_DEPS += \
./Src/stm32f4xx_hal.d \
./Src/stm32f4xx_hal_cortex.d \
./Src/stm32f4xx_hal_dma.d \
./Src/stm32f4xx_hal_dma_ex.d \
./Src/stm32f4xx_hal_exti.d \
./Src/stm32f4xx_hal_flash.d \
./Src/stm32f4xx_hal_flash_ex.d \
./Src/stm32f4xx_hal_flash_ramfunc.d \
./Src/stm32f4xx_hal_gpio.d \
./Src/stm32f4xx_hal_pwr.d \
./Src/stm32f4xx_hal_pwr_ex.d \
./Src/stm32f4xx_hal_rcc.d \
./Src/stm32f4xx_hal_rcc_ex.d \
./Src/stm32f4xx_hal_spi.d \
./Src/stm32f4xx_hal_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/stm32f4xx_hal.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_cortex.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_dma.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_dma_ex.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_exti.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_flash.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_flash_ex.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_flash_ramfunc.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_gpio.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_pwr.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_pwr_ex.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_rcc.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_rcc_ex.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_spi.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/stm32f4xx_hal_uart.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/stm32f4xx_hal.cyclo ./Src/stm32f4xx_hal.d ./Src/stm32f4xx_hal.o ./Src/stm32f4xx_hal.su ./Src/stm32f4xx_hal_cortex.cyclo ./Src/stm32f4xx_hal_cortex.d ./Src/stm32f4xx_hal_cortex.o ./Src/stm32f4xx_hal_cortex.su ./Src/stm32f4xx_hal_dma.cyclo ./Src/stm32f4xx_hal_dma.d ./Src/stm32f4xx_hal_dma.o ./Src/stm32f4xx_hal_dma.su ./Src/stm32f4xx_hal_dma_ex.cyclo ./Src/stm32f4xx_hal_dma_ex.d ./Src/stm32f4xx_hal_dma_ex.o ./Src/stm32f4xx_hal_dma_ex.su ./Src/stm32f4xx_hal_exti.cyclo ./Src/stm32f4xx_hal_exti.d ./Src/stm32f4xx_hal_exti.o ./Src/stm32f4xx_hal_exti.su ./Src/stm32f4xx_hal_flash.cyclo ./Src/stm32f4xx_hal_flash.d ./Src/stm32f4xx_hal_flash.o ./Src/stm32f4xx_hal_flash.su ./Src/stm32f4xx_hal_flash_ex.cyclo ./Src/stm32f4xx_hal_flash_ex.d ./Src/stm32f4xx_hal_flash_ex.o ./Src/stm32f4xx_hal_flash_ex.su ./Src/stm32f4xx_hal_flash_ramfunc.cyclo ./Src/stm32f4xx_hal_flash_ramfunc.d ./Src/stm32f4xx_hal_flash_ramfunc.o ./Src/stm32f4xx_hal_flash_ramfunc.su ./Src/stm32f4xx_hal_gpio.cyclo ./Src/stm32f4xx_hal_gpio.d ./Src/stm32f4xx_hal_gpio.o ./Src/stm32f4xx_hal_gpio.su ./Src/stm32f4xx_hal_pwr.cyclo ./Src/stm32f4xx_hal_pwr.d ./Src/stm32f4xx_hal_pwr.o ./Src/stm32f4xx_hal_pwr.su ./Src/stm32f4xx_hal_pwr_ex.cyclo ./Src/stm32f4xx_hal_pwr_ex.d ./Src/stm32f4xx_hal_pwr_ex.o ./Src/stm32f4xx_hal_pwr_ex.su ./Src/stm32f4xx_hal_rcc.cyclo ./Src/stm32f4xx_hal_rcc.d ./Src/stm32f4xx_hal_rcc.o ./Src/stm32f4xx_hal_rcc.su ./Src/stm32f4xx_hal_rcc_ex.cyclo ./Src/stm32f4xx_hal_rcc_ex.d ./Src/stm32f4xx_hal_rcc_ex.o ./Src/stm32f4xx_hal_rcc_ex.su ./Src/stm32f4xx_hal_spi.cyclo ./Src/stm32f4xx_hal_spi.d ./Src/stm32f4xx_hal_spi.o ./Src/stm32f4xx_hal_spi.su ./Src/stm32f4xx_hal_uart.cyclo ./Src/stm32f4xx_hal_uart.d ./Src/stm32f4xx_hal_uart.o ./Src/stm32f4xx_hal_uart.su

.PHONY: clean-Src

