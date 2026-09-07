@echo off
setlocal

REM Get the parent directory of the scripts folder
for %%I in ("%~dp0..") do set "IKAINEX_ROOT=%%~fI"

REM Store permanently as a User Environment Variable
setx IKAINEX_ROOT "%IKAINEX_ROOT%"

echo.
echo ==========================================
echo IKAINEX_ROOT configured successfully
echo ==========================================
echo %IKAINEX_ROOT%
echo.
echo Please restart STM32CubeIDE if it is open.
echo.

endlocal