# LIN Annex Board LIN Slave Sample Application on Arduino UNO

## LIN Annex Connections
This section describes the hardware connections required between the **Arduino UNO** development board and the **LIN Annex** board.

| **UNO Header Pin** | **LIN Annex** | **Description**       |
| ------------------ | ------------- | --------------------- |
| D1                 | RX            | MCU TX → LIN Annex RX |
| D0                 | TX            | LIN Annex TX → MCU RX |

## `printf` Connection

A **USB-to-TTL Serial Converter** (for example, an **FT232RL FTDI USB-to-TTL UART Adapter Module**) is required to view the serial output.

| **UNO Header Pin** | **USB-to-TTL Converter**   | **Description**                          |
| ---------------    | -------------------------- | -----------------------------------------| 
| D11                | RX                         | `printf` output (MCU TX → USB-to-TTL RX) |
| GND                | GND                        | Common ground                            |

## Output Result
After successful execution, below output is visible.
(Baudrate for serial output: 115200)
```
Lin Responder Start
Received Data: 1, 2, 3, 4, 5, 6, 7, 8
Received Data Matched: OK
```
## Notes