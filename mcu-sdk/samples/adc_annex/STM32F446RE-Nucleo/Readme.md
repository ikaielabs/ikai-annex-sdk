# ADC Annex Board Sample Application on STM32F446RE-Nucleo


## SPI Connections
This section describes the hardware connections required between the **STM32F446RE** development board and the **ADC Annex** board.

STM32F446RE uses SPI2.

| STM32F446RE Pin | ADC Annex Pin | Description |
|-----------------|---------------|-------------|
| PB9             | CS            | SPI Chip Select |
| PB10            | SCK           | SPI Clock |
| PC1             | SDI           | SPI Master Out, Slave In (MOSI) |
| PC2             | SDO           | SPI Master In, Slave Out (MISO) |

## Connection Diagram

```text
    STM32F446RE                 ADC Annex
    ------------                ----------
    PB9     ----------------->  CS
    PB10    ----------------->  SCK
    PC1     ----------------->  SDI
    PC2     <-----------------  SDO
```

## Notes

- Ensure both boards share a **common GND**.
- Connect 5V and 3.3V.