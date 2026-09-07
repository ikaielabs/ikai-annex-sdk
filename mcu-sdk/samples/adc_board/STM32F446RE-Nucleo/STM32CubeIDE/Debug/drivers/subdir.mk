################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/drivers/adc_board.c \
C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/drivers/lin_board.c 

OBJS += \
./drivers/adc_board.o \
./drivers/lin_board.o 

C_DEPS += \
./drivers/adc_board.d \
./drivers/lin_board.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/adc_board.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/drivers/adc_board.c drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
drivers/lin_board.o: C:/Users/kunch/kc/ikainex/working/ikainex-aob-sdk/mcu-sdk/drivers/lin_board.c drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"/samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/Core/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc" -I"/modules/hal/stm32/stm32f4xx/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Include" -I"/modules/hal/stm32/stm32f4xx/CMSIS/Device/ST/STM32F4xx/Include" -I"/include/hal" -I"/hal/spi/inc" -I"/utility/serial_print/inc" -I"/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/adc_board.cyclo ./drivers/adc_board.d ./drivers/adc_board.o ./drivers/adc_board.su ./drivers/lin_board.cyclo ./drivers/lin_board.d ./drivers/lin_board.o ./drivers/lin_board.su

.PHONY: clean-drivers

