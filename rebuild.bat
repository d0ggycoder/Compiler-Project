@echo off

setlocal EnableDelayedExpansion
if not exist "build_cache" (
    mkdir build_cache
)

set "changed=no"

for %%i in (*) do (
    if "%%~xi"==".c" (
        set "rebuild=no"
        if not exist "build_cache/%%i" (
            copy /Y "%%i" "build_cache/%%i"
            set "rebuild=yes"
        )
        if not exist "build_cache/%%~ni.o" (
            set "rebuild=yes"
        )
        fc.exe %%i "build_cache/%%i" /B > nul
        if errorlevel 1 (
            set "rebuild=yes"
        )
        if "!rebuild!" == "yes" (
            echo "Rebuilding file %%i"
            set "changed=yes"
            copy /Y "%%i" "build_cache/%%i"
            gcc -c %%i -o "build_cache/%%~ni.o"
        )
    )
)

if "!changed!" == "yes" (
    echo Rebuilding executable...
    gcc build_cache\*.o -o test.exe
)