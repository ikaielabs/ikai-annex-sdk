# ikainex-aob-sdk — Ikainex Add-on Board SDK

Comprehensive cross-platform SDK for **Ikainex Add-on Boards**. This repository contains:
- **MCU SDK** — Portable C firmware for embedded systems (STM32, NXP, etc.)
- **Arduino Libraries** — Ready-to-use Arduino libraries with examples
- **MicroPython Support** — Python integration for Inx boards

## Quick Links

- 📖 **[MCU SDK Documentation](mcu-sdk/README.md)** — C/C++ SDK details
- 🔧 **[Arduino Libraries](arduino-libs/README.md)** — Arduino integration guide
- 🐍 **[MicroPython Support](micropython/Readme.md)** — MicroPython module guide

## Project Structure

```
ikainex-aob-sdk/
├── mcu-sdk/              # Portable C SDK for Ikainex Add-on boards
│   ├── include/          # Public APIs (board-level and HAL interfaces)
│   ├── drivers/          # Board driver implementations (vendor-agnostic)
│   ├── hal/              # Hardware Abstraction Layer (vendor-specific)
│   ├── modules/          # vendor-specific Modules
│   ├── samples/          # Example applications
│   ├── scripts/          # Build and distribution scripts
│   └── utility/          # Shared utilities (printf, logging, etc.)
├── arduino-libs/         # Arduino libraries for Inx boards
│   ├── adc_board/        # ADC Add-on Board library
│   ├── lin_board/        # LIN Add-on Board library
│   └── scripts/          # Build/flash helper scripts
├── micropython/          # MicroPython module support
└── Readme                # This file
```

## Supported Platforms

### MCU SDK
| Vendor | Target Board | Status |
|--------|--------------|--------|
| STM32  | Nucleo-F446RE | ✅ Primary / Tested |
| NXP    | (Template)   | 🔄 Porting skeleton |

### Arduino
- AVR (Uno, Nano, Mega)
- ARM (SAMD, STM32, etc.)

### MicroPython
- Planned (under development)

## Supported Modules

| Module | API | Driver | Arduino | MicroPython | Status |
|--------|-----|--------|---------|-------------|--------|
| **ADC** | `adc_board.h` | `adc_board.c` | ✅ AdcBoard | 🔄 Planned | ✅ Available |
| **LIN** | `lin_board.h` | `lin_board.c` | ✅ InxLinBoard | 🔄 Planned | ✅ Available |
| **CANFD** | `canfd_board.h` | `canfd_board.c` | ❌ Planned | ❌ Planned | ⏳ Planned |
| **Motor Driver** | `motordriver_board.h` | `motordriver_board.c` | ❌ Planned | ❌ Planned | ⏳ Planned |

## Getting Started

### For MCU SDK (C/C++)
1. See [MCU SDK README](mcu-sdk/README.md) for detailed setup
2. Choose your toolchain: Keil, IAR, or CMake
3. Follow the integration guide in `mcu-sdk/doc/`

### For Arduino
```bash
# Install via Arduino Library Manager
# Search for "Ikainex" and install each board library

# Or use arduino-cli
arduino-cli lib search ADC Add-on Board
arduino-cli lib install "ADC Add-on Board"
```

See [Arduino Libraries README](arduino-libs/README.md) for examples.

### For MicroPython
See [MicroPython README](micropython/Readme.md) for setup instructions (TBD).

## Naming Conventions

This project uses **consistent, hierarchical naming** across all implementations:

### C/C++ Code
- **Header Guards**: `INX_<MODULE>_H` (e.g., `INX_ADC_BOARD_H`)
- **Macros**: `INX_<MODULE>_<CONSTANT>` (e.g., `INX_ADC_BOARD_VREF_3300MV`)
- **Functions**: `inx_<module>_<action>()` (e.g., `inx_adc_board_read_channel()`)
- **Types**: `inx_<module>_<name>_t` (e.g., `inx_adc_board_dev_t`)

### Arduino
- **Classes**: PascalCase (e.g., `AdcBoard`, `InxLinBoard`)
- **Methods**: camelCase (e.g., `readSingleEnded()`)

### Documentation
- **Primary README files**: `README.md` (uppercase)
- **Alternative capitalization**: `Readme.md` (used in Arduino libs)

## Key Features

✅ **Vendor-Agnostic Design**
- Hardware Abstraction Layer (HAL) ensures drivers work across MCU vendors
- Add new vendor support by implementing HAL interfaces

✅ **Multiple Platform Support**
- Native C/C++ for embedded systems
- Arduino IDE integration for hobbyists
- MicroPython support planned

✅ **Clean Architecture**
- Public APIs separated from vendor-specific implementations
- Driver logic completely independent of MCU vendor
- Modular, extensible structure

✅ **Examples & Samples**
- Complete working examples for each board
- Both MCU SDK and Arduino implementations
- Per-module documentation

## Contributing

Contributions welcome! When adding new add-on board or features:

2. **Add board-level API** — New header in `include/`
3. **Implement driver** — New source in `drivers/`
4. **Create sample** — Add example in `samples/`
5. **Document changes** — Update relevant README files

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│ Applications & Samples                                      │
│ (MCU SDK samples, Arduino sketches, MicroPython scripts)    │
└────────────────┬────────────────────────────────────────────┘
                 │
┌─────────────────┴────────────────────────────────────────────┐
│ Board-Level Drivers (Vendor-Agnostic)                       │
│ (adc_board.c, lin_board.c, canfd_board.c, etc.)            │
└────────────────┬───────────────────────────────────────────┘
                 │
┌─────────────────┴────────────────────────────────────────────┐
│ Hardware Abstraction Layer (HAL)                            │
│ (hal_gpio.h, hal_uart.h, hal_spi.h, hal_i2c.h, etc.)      │
└────────────────┬───────────────────────────────────────────┘
                 │
     ┌───────────┼───────────┬──────────────┐
     │           │           │              │
     ▼           ▼           ▼              ▼
  STM32       NXP       Template      (Other Vendors)
  (hal/       (hal/     (hal/          (hal/vendor/)
   stm32/)     nxp/)     template/)
```

## License

_See LICENSE file for details_

---

**Ikainex Add-on Boards SDK** — Making embedded systems development accessible across platforms.
