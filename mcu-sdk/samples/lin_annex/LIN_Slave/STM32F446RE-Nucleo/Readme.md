# LIN Annex Board LIN Slave Sample Application on STM32F446RE-Nucleo


## LIN Annex Connections
This section describes the hardware connections required between the **STM32F446RE** development board and the **LIN Annex** board.

STM32F446RE uses USART2.

## LIN Annex Connections

| **STM32F446RE** |                            | **LIN Annex** |                       |
| --------------- | -------------------------- | ------------- | --------------------- |
| **STM Pin**     | **Arduino UNO Header Pin** | **Annex Pin** | **Description**       |
| PA2             | D1                         | RX            | MCU TX → LIN Annex RX |
| PA3             | D0                         | TX            | LIN Annex TX → MCU RX |

## `printf` Connection

`printf` output uses **USART3**.

A **USB-to-TTL Serial Converter** (for example, an **FT232RL FTDI USB-to-TTL UART Adapter Module**) is required to view the serial output.

| **STM32F446RE** |                            | **USB-to-TTL Converter** |                                          |
| --------------- | -------------------------- | ------------------------ | ---------------------------------------- |
| **STM Pin**     | **Arduino UNO Header Pin** | **Converter Pin**        | **Description**                          |
| PB10            | D6                         | RX                       | `printf` output (MCU TX → USB-to-TTL RX) |
| GND             | GND                        | GND                      | Common ground                            |


## Notes

- SB62, SB63 Bridges should be closed. (By default these are Open). Refer to mb1136-default-c03_schematic. 