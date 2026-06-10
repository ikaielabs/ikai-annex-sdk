@echo off
setlocal EnableDelayedExpansion

:: ============================================================
::  Arduino HAL UART - Build Script for Windows
::  Usage: build_arduino.bat [clean] [upload] [all]
::  Examples:
::    build_arduino.bat            -- build only
::    build_arduino.bat clean      -- clean build output
::    build_arduino.bat upload     -- build and upload
::    build_arduino.bat all        -- clean + build + upload
:: ============================================================

:: ── Anchor everything to the script location ─────────────────
set SDK_ROOT=D:\ikaielabs\driver-development\ikai-annex-sdk\annex

:: ── OR use %~dp0 so it works on any machine ──────────────────
:: set SDK_ROOT=%~dp0..\..

:: ── Then define includes relative to SDK_ROOT ────────────────


:: ------------------------------------------------------------
::  SECTION 1: arduino-cli path
:: ------------------------------------------------------------

:: Option A: If arduino-cli is on your system PATH, just use the name
set ARDUINO_CLI=arduino-cli

:: Option B: Full path if not on PATH (uncomment and edit)
:: set ARDUINO_CLI=C:\Users\YourName\AppData\Local\Programs\arduino-cli\arduino-cli.exe


:: ------------------------------------------------------------
::  SECTION 2: Board selection
::  Find your FQBN by running: arduino-cli board listall
:: ------------------------------------------------------------

:: Arduino Uno
set FQBN=arduino:avr:uno

:: Other common boards (uncomment one to switch):
:: set FQBN=arduino:avr:mega
:: set FQBN=arduino:avr:nano
:: set FQBN=arduino:samd:mkr1000
:: set FQBN=arduino:samd:nano_33_iot
:: set FQBN=esp32:esp32:esp32
:: set FQBN=rp2040:rp2040:rpipico


:: ------------------------------------------------------------
::  SECTION 3: Sketch to compile
::  Path to the .ino file (relative to this script)
:: ------------------------------------------------------------

set SKETCH=..\..\samples\uart\uart_echo\uart_echo.ino


:: ------------------------------------------------------------
::  SECTION 4: Library paths
::  Folders that contain Arduino libraries (with library.properties)
::  Add more lines for additional library roots
:: ------------------------------------------------------------

:: set LIB_FLAGS=
:: set LIB_FLAGS=%LIB_FLAGS% --libraries .
:: set LIB_FLAGS=%LIB_FLAGS% --libraries .\third_party
set LIB_FLAGS=--libraries %SDK_ROOT%\hal\arduino


:: ------------------------------------------------------------
::  SECTION 5: Extra include paths
::  Raw -I paths for loose headers (no library.properties)
::  Space-separate multiple paths inside the quotes
:: ------------------------------------------------------------

set EXTRA_C_FLAGS=-I%SDK_ROOT%\include\annex\hal
set EXTRA_CPP_FLAGS=-I%SDK_ROOT%\include\annex\hal

:: Add more includes as needed:
:: set EXTRA_C_FLAGS=%EXTRA_C_FLAGS% -I.\platform\avr
:: set EXTRA_CPP_FLAGS=%EXTRA_CPP_FLAGS% -I.\platform\avr


:: ------------------------------------------------------------
::  SECTION 6: Extra compiler defines
::  Add -D flags here
:: ------------------------------------------------------------

set EXTRA_DEFINES=
:: set EXTRA_DEFINES=-DDEBUG_UART=1 -DHAL_VERSION=2


:: ------------------------------------------------------------
::  SECTION 7: Build output directory
:: ------------------------------------------------------------

set BUILD_DIR=build\%FQBN::=_%


:: ------------------------------------------------------------
::  SECTION 8: Upload port (only used with 'upload' argument)
::  Find your port: arduino-cli board list
:: ------------------------------------------------------------

set PORT=COM15
:: set PORT=COM4


:: ============================================================
::  INTERNAL — do not edit below this line
:: ============================================================

set SCRIPT_DIR=%~dp0
set BUILD_PROPERTY_C=compiler.c.extra_flags=%EXTRA_C_FLAGS% %EXTRA_DEFINES%
set BUILD_PROPERTY_CPP=compiler.cpp.extra_flags=%EXTRA_CPP_FLAGS% %EXTRA_DEFINES%

:: Parse arguments
set DO_CLEAN=0
set DO_BUILD=1
set DO_UPLOAD=0

if "%1"=="clean"  set DO_CLEAN=1 & set DO_BUILD=0
if "%1"=="upload" set DO_UPLOAD=1
if "%1"=="all"    set DO_CLEAN=1 & set DO_UPLOAD=1

:: ── Header ──────────────────────────────────────────────────
echo.
echo ============================================================
echo   Arduino HAL UART Build Script
echo ============================================================
echo   Board   : %FQBN%
echo   Sketch  : %SKETCH%
echo   Output  : %BUILD_DIR%
echo   Port    : %PORT%
echo ============================================================
echo.

:: ── Check arduino-cli exists ─────────────────────────────────
%ARDUINO_CLI% version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] arduino-cli not found.
    echo         Install from: https://arduino.github.io/arduino-cli/latest/installation/
    echo         Or set ARDUINO_CLI to the full path in this script.
    goto :fail
)

for /f "tokens=*" %%v in ('%ARDUINO_CLI% version 2^>^&1') do echo   CLI     : %%v
echo.

:: ── Clean ────────────────────────────────────────────────────
if "%DO_CLEAN%"=="1" (
    echo [STEP] Cleaning build directory...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
        echo        Removed: %BUILD_DIR%
    ) else (
        echo        Nothing to clean.
    )
    echo.
    if "%DO_BUILD%"=="0" goto :done
)

:: ── Check sketch exists ──────────────────────────────────────
if not exist "%SKETCH%" (
    echo [ERROR] Sketch not found: %SKETCH%
    echo         Edit SKETCH= in this script to point to your .ino file.
    goto :fail
)

:: ── Create build dir ─────────────────────────────────────────
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: ── Compile ──────────────────────────────────────────────────
echo [STEP] Compiling...
echo        Sketch : %SKETCH%
echo        Board  : %FQBN%
echo.

%ARDUINO_CLI% compile ^
    --fqbn %FQBN% ^
    %LIB_FLAGS% ^
    --build-property "%BUILD_PROPERTY_C%" ^
    --build-property "%BUILD_PROPERTY_CPP%" ^
    --build-path "%BUILD_DIR%" ^
    --verbose ^
    "%SKETCH%"

if errorlevel 1 (
    echo.
    echo [FAILED] Compilation failed. Check errors above.
    goto :fail
)

echo.
echo [OK] Compilation successful.
echo      Output files in: %BUILD_DIR%
echo.

:: ── Show binary size ─────────────────────────────────────────
echo [INFO] Build artifacts:
for %%f in ("%BUILD_DIR%\*.hex" "%BUILD_DIR%\*.elf" "%BUILD_DIR%\*.bin") do (
    if exist "%%f" (
        for %%s in ("%%f") do echo        %%~nxf  ^(%%~zs bytes^)
    )
)
echo.

:: ── Upload ───────────────────────────────────────────────────
if "%DO_UPLOAD%"=="1" (
    echo [STEP] Uploading to %PORT%...
    echo.

    %ARDUINO_CLI% upload ^
        --fqbn %FQBN% ^
        --port %PORT% ^
        --input-dir "%BUILD_DIR%" ^
        --verbose ^
        "%SKETCH%"

    if errorlevel 1 (
        echo.
        echo [FAILED] Upload failed.
        echo          Check PORT=%PORT% is correct: arduino-cli board list
        goto :fail
    )

    echo.
    echo [OK] Upload successful.
    echo.
)

:done
echo ============================================================
echo   Done.
echo ============================================================
echo.
exit /b 0

:fail
echo ============================================================
echo   Build FAILED.
echo ============================================================
echo.
exit /b 1