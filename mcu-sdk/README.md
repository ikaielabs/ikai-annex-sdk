# mcu-sdk

Portable C SDK for **Ikainex Add-on Board** firmware, targeting popular Cortex-M
development boards. Primary reference target is the **STM32 Nucleo-F446RE**;
the HAL layer is structured so additional vendors (NXP, etc.) can be added
without touching driver or application code.

## Design overview

The SDK is split into three layers so that Inx driver logic never depends
directly on a specific MCU vendor:

```
samples  →  drivers ──depends on ──▶ include/hal/*.h
                                            ▲
                                            │ implemented by
                                            │
                                  hal/<peripheral>/hal_<peripheral>_<vendor>.c
                                  Example: hal/gpio/hal_gpio_stm32.c
```

- **`include/`** — public API. `hal/` contains the vendor-neutral HAL
  interface (`hal_gpio.h`, `hal_uart.h`, `hal_spi.h`, `hal_i2c.h`, `hal_adc.h`); the
  top-level headers (`lin_board.h`, `canfd_board.h`, `adc_board.h`) are the Ikainex Add-on Board
  APIs that applications and samples include.
- **`drivers/`** — Ikainex Add-on board driver implementations (protocol/module logic),
  written only against `include/hal/*.h`, never against vendor SDKs
  directly.
- **`hal/`** — per-vendor implementations of the HAL interface
  (`stm32/`, `nxp/`). `hal/template/` is a skeleton to copy when porting to a
  new MCU family.
- **`modules/`** — contains per vendor hal (includes cmsis) and additional modules required.  
- **`samples/`** — end-to-end example applications, with vendor-specific build
  configuration showing how `main.c` wires the driver to a specific vendor HAL.
- **`scripts/`** — release/build tooling, including per-module version and
  config files.
- **`doc/`** — toolchain integration guides (Keil, IAR, CMake).

## Directory structure

```
mcu-sdk
├── doc/                          Integration guides (Keil / IAR / CMake)
├── scripts/
│   ├── make_dist.sh              Master distribution builder
│   ├── boards/                   Per-module conf + version files
│   └── make
├── include/
│   ├── hal/                      Vendor-neutral HAL interface
│   ├── adc_board.h
│   ├── lin_board.h
│   ├── canfd_board.h
│   └── <module>_board.h
├── drivers/                      Inx module driver implementations
│   ├── adc_board.c
│   ├── lin_board.c
│   └── <module>_board.c
├── hal/                          Vendor HAL implementations
│   ├── gpio/
│   ├── spi/
│   ├── ...
│   └── template/                 Copy this to port a new MCU family
├── modules/                      Vendor specific files
│   └── hal/                      Vendor specific HAL
│       ├── stm32/
│       └── ...
├── samples/
│   ├── adc_board/
│   │   └── STM32F446RE-Nucleo
│   │       ├── STM32CubeIDE      STM32CubeIDE project for the sample
│   │       └── README.md
│   ├── lin_board/
│   │   ├── LIN_Master/
│   │   └── LIN_Slave/
│   └── <module>_board/
│       └── <board>/
└── utility/                      printf/timer and other shared helpers
```

## Supported targets

| Vendor | Family / Board             | Status           |
|--------|----------------------------|------------------|
| STM32  | Nucleo-F446RE              | Primary/tested   |

## Supported Ikainex Add-on Boards

| Module       | Header                      | Driver                  | Status    |
|--------------|-----------------------------|-----------------------|-----------|
| ADC          | `include/adc_board.h`       | `drivers/adc_board.c`   | Planned   |
| LIN          | `include/lin_board.h`       | `drivers/lin_board.c`   | Planned   |
| CANFD        | `include/canfd_board.h`     | `drivers/canfd_board.c` | Planned   |
| Motor Driver | `include/motordriver_board.h` | `drivers/motordriver_board.c` | Planned   |

## Getting started

1. Pick your toolchain and follow the relevant guide in `doc/`:
   `integration-guide-keil.rst`, `integration-guide-iar.rst`, or `integration-guide-cmake.rst`.
2. Build the sample for your board, e.g., `samples/adc_board/STM32F446RE-Nucleo/STM32CubeIDE/`.
3. Link against `hal/stm32/*.c` and `drivers/<module>_board.c`.

## Porting to a new MCU vendor

1. Add hal_<peripheral>_<vendor>.c in hal/<peripheral>/ folder
2. Implement each function declared in `include/hal/hal_*.h`.
3. No changes are needed in `drivers/` — that layer is vendor-agnostic.

## Versioning

Each Inx module has an independent version tracked under
`scripts/boards/<module>.version`, alongside its build config
(`<module>.conf`).   
Run `scripts/make_dist.sh` to produce a distributable package.

## Contributing

Please open an issue or PR. When adding a new Ikainex Add-on Boards, add:
- a header under `include/` named `<module>_board.h`
- a driver under `drivers/` named `<module>_board.c`
- a config + version file under `scripts/boards/`
- a sample under `samples/` with board-specific implementations

## License

_TODO: Add License information_