@echo off
REM ArborOS Windows Build Script
REM Double-click to build ISO using Docker

echo ========================================
echo ArborOS Docker Build
echo ========================================
echo.
echo Requirements:
echo - Docker Desktop running
echo - WSL2 enabled
echo.
pause

cd /d "%~dp0"
cd ..\..

echo Starting build in Docker...
echo This takes 15-30 minutes
echo.

docker run --rm -it --privileged -v "%CD%:/workspace" fedora:39 bash /workspace/Code/build/create_iso.sh

echo.
echo ========================================
if exist Code\build\output\ArborOS-0.5.iso (
    echo BUILD SUCCESS!
    echo ISO: Code\build\output\ArborOS-0.5.iso
    for %%A in (Code\build\output\ArborOS-0.5.iso) do echo Size: %%~zA bytes
) else (
    echo BUILD FAILED - Check errors above
)
echo ========================================
echo.
pause
