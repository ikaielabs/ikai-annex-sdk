# arduino-libs

Arduino libraries for Ikainex Add-on Boards (e.g. `lin_board`, `adc_board`, ...), plus a
build/flash helper script so you don't need the Arduino IDE to compile and upload
examples.

```
arduino-libs/
├── adc_board
│   ├── examples
│   │     └── adc_example
│   │           └── adc_example.ino
│   ├── src
│   │     ├── AdcBoard.cpp
│   │     └── AdcBoard.h
│   ├── keyword.txt
│   ├── library.properties
│   └── Readme.md
├── <module>_board
│   ├── examples
│   ├── src
│   ├── keyword.txt
│   ├── library.properties
│   └── Readme.md
├── ...
├── scripts
│   └── run_ikainex_aob_examples.bat 
└── Readme.md          <- this file
```

Each `<module>_board` folder (`adc_board`, `lin_board`, ...) is a **self-contained
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
`arduino-cli` at the `arduino-libs` folder so any Ikainex add-on board library can `#include` any
other one if required.

---

## 2. Quick Start

From anywhere, run the build/flash script and point it at the board + example you
want:

```bat
scripts/run_ikainex_aob_examples.bat --aob lin_board --example lin_master_example --com COM3
```

This will **build and flash by default**. That's it — no path juggling required.

---

## 3. Repository Layout Conventions

Every Annex board library must follow this layout so the build script and Arduino
IDE can both find things automatically:

| Path                                                            | Purpose                                   |
|-------------------------------------------------------------------|--------------------------------------------|
| `<module>_board/src/`                                              | Library source (`.cpp` / `.h`)            |
| `<module>_board/examples/<example>/<example>.ino`                    | One folder per example, folder name == `.ino` name |
| `<module>_board/library.properties`                                | Arduino library metadata                  |
| `<module>_board/keyword.txt`                                       | Keyword highlighting for the Arduino IDE  |
| `<module>_board/Readme.md`                                         | Board-specific docs (wiring, API, examples) |


### Adding a new Annex board library

1. Create `<module>_board/` with `src/`, `examples/`, `library.properties`,
   `keyword.txt`, and `Readme.md` following the layout above.
2. In `src/*.cpp`, include `Arduino.h` (see [Troubleshooting](#5-troubleshooting)
   below) — library source files don't get it automatically the way `.ino`
   examples do.
3. Add at least one working example under `examples/<example>/<example>.ino`.
4. Document the board in its own `Readme.md` (pinout, constructor/API, example
   usage).
5. Test it with the build script:
   ```bat
   scripts\run_ikainex_aob_examples.bat --aob <NewAnnexBoard> --example <example> --com <COMx>
   ```

---

## 4. Build / Flash Script Reference

Script location: `scripts\run_ikainex_aob_examples.bat`

By default it **builds and flashes**. Add `--build-only` to just compile.

```bat
run_ikainex_aob_examples.bat [clean] --aob <name> --example <name> ^
                 [--arduino-board <board>] [--com <port>] [--build-only]
```

| Argument            | Required | Default | Description                                                                 |
|---------------------|----------|---------|-------------------------------------------------------------------------------|
| `--aob`     | Yes      | `lin_board` | Annex library folder name, e.g. `lin_board`, `adc_board`                    |
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
:: Build + flash lin_board's lin_master_example to an Uno on COM3
run_ikainex_aob_examples.bat --aob lin_board --example lin_master_example --com COM3

:: Build + flash adc_board's adc_example to a Mega on COM5
run_ikainex_aob_examples.bat --aob adc_board --example adc_annex --arduino-board mega --com COM5

:: Just compile, don't flash (e.g. CI / sanity check)
run_ikainex_aob_examples.bat --aob lin_board --example lin_master_example --build-only

:: Clean build output for a given board/example combo
run_ikainex_aob_examples.bat clean --aob lin_board --example lin_master_example
```

Build output for each board/FQBN combination goes to:
```
scripts\build-flash\build\<fqbn-with-colons-replaced-by-underscores>\
```

---

## 5. Per-Board Documentation

For wiring diagrams, API reference, and board-specific example walkthroughs, see
the `Readme.md` inside each annex board's folder:

- [`adc_board/Readme.md`](./adc_board/Readme.md)
- [`lin_board/Readme.md`](./lin_board/Readme.md)
- ...