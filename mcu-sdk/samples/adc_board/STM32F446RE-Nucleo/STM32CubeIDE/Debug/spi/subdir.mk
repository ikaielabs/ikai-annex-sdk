################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/hal/spi/hal_spi_stm32.c 

OBJS += \
./spi/hal_spi_stm32.o 

C_DEPS += \
./spi/hal_spi_stm32.d 


# Each subdirectory must supply rules for building sources it contributes
spi/hal_spi_stm32.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/hal/spi/hal_spi_stm32.c spi/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-spi

clean-spi:
	-$(RM) ./spi/hal_spi_stm32.cyclo ./spi/hal_spi_stm32.d ./spi/hal_spi_stm32.o ./spi/hal_spi_stm32.su

.PHONY: clean-spi

