@echo off
set OUTPUT_EXE=agri_manager.exe
set SRC_FILES=src\main.c src\utils.c src\auth.c src\producers.c src\categories.c src\products.c src\orders.c src\deliveries.c src\reservations.c src\losses.c src\reports.c

echo =======================================================
echo     AGRI-MANAGER - Script de Compilation C
echo =======================================================

where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Compilateur trouve : MinGW / GCC
    gcc -Wall -Wextra -std=c99 -D_CRT_SECURE_NO_WARNINGS -Iinclude %SRC_FILES% -o %OUTPUT_EXE%
    goto :end
)

set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist %VCVARS% (
    echo Chargement de l'environnement MSVC...
    call %VCVARS% x64
    cl.exe /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /Iinclude %SRC_FILES% /Fe:%OUTPUT_EXE%
    if exist main.obj del *.obj 2>nul
    goto :end
)

echo [ERREUR] Aucun compilateur C (gcc ou cl.exe) trouve.
exit /b 1

:end
if exist %OUTPUT_EXE% (
    echo =======================================================
    echo [SUCCES] Compilation terminee avec succes : %OUTPUT_EXE%
    echo =======================================================
    exit /b 0
) else (
    echo [ERREUR] La compilation a echoue.
    exit /b 1
)
