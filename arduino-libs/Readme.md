# arduino-libs

Arduino libraries for ikai annex boards (e.g. `LinAnnex`, `AdcAnnex`, ...), plus a
build/flash helper script so you don't need the Arduino IDE to compile and upload
examples.

```
arduino-libs/
├── AdcAnnex
│   ├── examples
│   │     └── adc_annex
│   │           └── adc_annex.ino
│   ├── src
│   │     ├── ADCAnnex.cpp
│   │     └── ADCAnnex.h
│   ├── keyword.txt
│   ├── library.properties
│   └── Readme.md
├── <Module>Annex
│   ├── examples
│   ├── src
│   ├── keyword.txt
│   ├── library.properties
│   └── Readme.md
├── ...
├── scripts
│   └── build-flash
└── Readme.md          <- this file
```

Each `<Module>Annex` folder (`AdcAnnex`, `LinAnnex`, ...) is a **self-contained
Arduino library** in the standard Arduino library layout, so it also works if you
open it directly in the Arduino IDE. See each board's own `Readme.md` for
board-specific wiring, API, and usage notes.

---

## 1. Prerequisites

1. **arduino-cli** installed and available on your `PATH`.  
   Install from: https://arduino.github.io/arduino-cli/latest/installation/   
   Verify with:
   ```
   arduino-cli version
   ```
2. **Board core installed** for whichever Arduino board you're targeting, e.g.:
   ```
   arduino-cli core install arduino:avr        # Uno / Nano / Mega
   ```
3. Know which **COM port** your board is on:
   ```
   arduino-cli board list
   ```

No manual `--libraries` setup is needed — the build script automatically points
`arduino-cli` at the `arduino-libs` folder so any Annex library can `#include` any
other one if required.

---

## 2. Quick Start

From anywhere, run the build/flash script and point it at the board + example you
want:

```bat
scripts/ikai_annex_run.bat --annex-board LinAnnex --example lin_master_example --com COM3
```

This will **build and flash by default**. That's it — no path juggling required.

---

## 3. Repository Layout Conventions

Every Annex board library must follow this layout so the build script and Arduino
IDE can both find things automatically:

| Path                                                            | Purpose                                   |
|-------------------------------------------------------------------|--------------------------------------------|
| `<Module>Annex/src/`                                              | Library source (`.cpp` / `.h`)            |
| `<Module>Annex/examples/<example>/<example>.ino`                    | One folder per example, folder name == `.ino` name |
| `<Module>Annex/library.properties`                                | Arduino library metadata                  |
| `<Module>Annex/keyword.txt`                                       | Keyword highlighting for the Arduino IDE  |
| `<Module>Annex/Readme.md`                                         | Board-specific docs (wiring, API, examples) |


### Adding a new Annex board library

1. Create `<Module>Annex/` with `src/`, `examples/`, `library.properties`,
   `keyword.txt`, and `Readme.md` following the layout above.
2. In `src/*.cpp`, include `Arduino.h` (see [Troubleshooting](#5-troubleshooting)
   below) — library source files don't get it automatically the way `.ino`
   examples do.
3. Add at least one working example under `examples/<example>/<example>.ino`.
4. Document the board in its own `Readme.md` (pinout, constructor/API, example
   usage).
5. Test it with the build script:
   ```bat
   scripts\ikai_annex_run.bat --annex-board <NewAnnexBoard> --example <example> --com <COMx>
   ```

---

## 4. Build / Flash Script Reference

Script location: `scripts\ikai_annex_run.bat`

By default it **builds and flashes**. Add `--build-only` to just compile.

```bat
annex_flash.bat [clean] --annex-board <name> --example <name> ^
                 [--arduino-board <board>] [--com <port>] [--build-only]
```

| Argument            | Required | Default | Description                                                                 |
|---------------------|----------|---------|-------------------------------------------------------------------------------|
| `--annex-board`     | Yes      | `LinAnnex` | Annex library folder name, e.g. `LinAnnex`, `AdcAnnex`                    |
| `--example`          | Yes      | —       | Example name only (no path, no `.ino`) — e.g. `lin_master_example`  |
| `--arduino-board`   | No       | `uno`   | Target board shorthand or full FQBN — see table below                     |
| `--com`             | No       | `COM15` | Serial port to flash, e.g. `COM3`                                          |
| `--build-only`      | No       | off     | Compile only, skip flashing                                                |
| `clean`             | No       | —       | Remove that board's build output folder and exit                          |
| `--help` / `-h`     | No       | —       | Show usage                                                                  |

### `--arduino-board` shorthand → FQBN mapping

| Shorthand       | FQBN                        |
|-----------------|------------------------------|
| `uno`           | `arduino:avr:uno`            |
| `nano`          | `arduino:avr:nano`           |
| `mega`          | `arduino:avr:mega`           |
| `mkr1000`       | `arduino:samd:mkr1000`       |
| `nano_33_iot`   | `arduino:samd:nano_33_iot`   |
| `esp32`         | `esp32:esp32:esp32`          |
| `rpipico`       | `rp2040:rp2040:rpipico`      |

You can also pass a full FQBN directly (anything containing `:`), e.g.
`--arduino-board arduino:avr:leonardo`.

### Examples

```bat
:: Build + flash LinAnnex's lin_master_example to an Uno on COM3
annex_flash.bat --annex-board LinAnnex --example lin_master_example --com COM3

:: Build + flash AdcAnnex's adc_annex example to a Mega on COM5
annex_flash.bat --annex-board AdcAnnex --example adc_annex --arduino-board mega --com COM5

:: Just compile, don't flash (e.g. CI / sanity check)
annex_flash.bat --annex-board LinAnnex --example lin_master_example --build-only

:: Clean build output for a given board/example combo
annex_flash.bat clean --annex-board LinAnnex --example lin_master_example
```

Build output for each board/FQBN combination goes to:
```
scripts\build-flash\build\<fqbn-with-colons-replaced-by-underscores>\
```

---

## 5. Per-Board Documentation

For wiring diagrams, API reference, and board-specific example walkthroughs, see
the `Readme.md` inside each annex board's folder:

- [`AdcAnnex/Readme.md`](./AdcAnnex/Readme.md)
- [`LinAnnex/Readme.md`](./LinAnnex/Readme.md)
- ...