@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul
echo ========================================
echo  Compilando Nico v2.0.0 para Windows
echo ========================================
echo.

:: Verificar gcc
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] gcc no encontrado.
    echo Instalá MinGW-w64 y agregá la carpeta bin al PATH.
    echo https://github.com/niXman/mingw-builds-binaries/releases
    pause
    exit /b 1
)

echo [1/2] Compilando fuentes...

set CFILES=
for %%f in (src\*.c) do (
    set "FNAME=%%~nxf"
    if /I "!FNAME!"=="nico_gpio.c" (
        echo [SKIP] %%f
    ) else if /I "!FNAME!"=="nico_pwm.c" (
        echo [SKIP] %%f
    ) else if /I "!FNAME:~0,5!"=="test_" (
        echo [SKIP] %%f
    ) else (
        set CFILES=!CFILES! %%f
        echo [OK] %%f
    )
)

gcc -o nico.exe %CFILES% ^
    -std=c11 -O2 -Wall -Wextra -Wno-stringop-truncation -Wno-unused-parameter ^
    -lm -lsqlite3 -lwinpthread -lws2_32 ^
    -I"C:/msys64/mingw64/include" ^
    -L"C:/msys64/mingw64/lib" ^
    -DWIN32_LEAN_AND_MEAN ^
    -include src/win_compat.h ^
    -I. ^
    -Wl,--stack,67108864 2>compile_errors.txt
    
if %errorlevel% neq 0 (
    echo [ERROR] Compilacion fallida. Detalles:
    type compile_errors.txt
    pause
    exit /b 1
)
del compile_errors.txt >nul 2>&1

echo.
echo [2/2] Verificando binario...
if exist nico.exe (
    echo [OK] nico.exe generado correctamente.
    echo [INFO] Stack size: 64MB (soporta recursión profunda)
    exit /b 0
) else (
    echo [ERROR] Error desconocido.
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Listo. Usá: nico.exe archivo.nico
echo ========================================
endlocal
pause