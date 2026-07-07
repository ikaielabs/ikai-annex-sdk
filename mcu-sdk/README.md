# mcu-sdk

Portable C SDK for **annex board** firmware, targeting popular Cortex-M
development boards. Primary reference target is the **STM32 Nucleo-F446RE**;
the HAL layer is structured so additional vendors (NXP, etc.) can be added
without touching driver or application code.

## Design overview

The SDK is split into three layers so that annex driver logic never depends
directly on a specific MCU vendor:

```
samples  →  drivers (annex/) ──depends on──▶ include/hal/*.h
                                                  ▲
                                                  │ implemented by
                                                  │
                                          hal/<vendor>/*.c  (stm32, nxp, template)
```

- **`include/`** — public API. `hal/` contains the vendor-neutral HAL
  interface (`gpio.h`, `uart.h`, `spi.h`, `i2c.h`, `pwm.h`, `adc.h`); the
  top-level headers (`lin.h`, `can.h`, `temp_sensor.h`) are the annex module
  APIs that applications and samples include.
- **`drivers/`** — annex board driver implementations (protocol/module logic),
  written only against `include/hal/*.h`, never against vendor SDKs
  directly.
- **`hal/`** — per-vendor implementations of the HAL interface
  (`stm32/`, `nxp/`). `hal/template/` is a skeleton to copy when porting to a
  new MCU family.
- **`samples/`** — end-to-end example applications, with a `hal_impls/`
  subfolder per target board showing how `main.c` wires the driver to a
  specific vendor HAL.
- **`scripts/`** — release/build tooling, including per-module version and
  config files.
- **`docs/`** — toolchain integration guides (Keil, IAR, CMake).

## Directory structure

```
mcu-sdk
├── docs/                         Integration guides (Keil / IAR / CMake)
├── scripts/
│   ├── make_dist.sh              Master distribution builder
│   ├── boards/                   Per-module conf + version files
│   └── make
├── include/
│   ├── hal/                      Vendor-neutral HAL interface
│   ├── lin_annex.h
│   └── <module>_annex.h
├── drivers/                      Annex module driver implementations
│   ├── lin_annex.c
│   └── <module>_annex.c
├── hal/                          Vendor HAL implementations
│   ├── stm32/
│   ├── nxp/
│   └── template/                 Copy this to port a new MCU family
├── samples/
│   ├── lin_annex/
│   │   └── STM32F446RE-Nucleo
│   │       ├── STM32CubeIDE      This is STM32CubeIDE project for the sample application
│   │       └── Readme.md
│   └── <module>_annex/
│       └── main.c
└── utility/                      printf/timer and other shared helpers
```

## Supported targets

| Vendor | Family / Board             | Status           |
|--------|----------------------------|------------------|
| STM32  | Nucleo-F446RE              | Primary/tested   |
| —      | `hal/template/`            | Porting skeleton |

## Supported annex modules

| Module       | Header                        | Driver                        | Status    |
|--------------|-------------------------------|-------------------------------|-----------|
| LIN          | `include/lin_annex.h`         | `drivers/lin_annex.c`         | Planned   |
| ADC          | `include/adc_annex.h`         | `drivers/adc_annex.c`         | Planned   |
| CANFD        | `include/canfd_annex.h`       | `drivers/canfd_annex.c`       | Planned   |
| Motor Driver | `include/motordriver_annex.h` | `drivers/motordriver_annex.c` | Planned   |

## Getting started

1. Pick your toolchain and follow the relevant guide in `docs/`:
   `integration_keil.md`, `integration_iar.md`, or `integration_cmake.md`.
2. Build the sample for your board, e.g. `samples/lin_commander/hal_impls/stm32/main_stm32.c`.
3. Link against `hal/stm32/*.c` and `drivers/lin_annex/lin_annex.c`.

## Porting to a new MCU vendor

1. Copy `hal/template/` to `hal/<vendor>/`.
2. Implement each function declared in `include/hal/*.h`.
4. No changes are needed in `drivers/` — that layer is vendor-agnostic.

## Versioning

Each annex module has an independent version tracked under
`scripts/boards/<module>.version`, alongside its build config
(`<module>.conf`).   
Run `scripts/make_dist.sh` to produce a distributable
package.

## Contributing

Please open an issue or PR. When adding a new annex module, add:
- a header under `include/`
- a driver under `drivers/` with the driver name as `<module>_annex.c`
- a config + version file under `scripts/boards/`
- a sample under `samples/`

## License

_TODO: Add License information_