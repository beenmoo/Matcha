cd ..
if not exist "Build" mkdir Build

set miniconda_activate=C:\Users\%USERNAME%\miniconda3\Scripts\activate.bat
call %miniconda_activate% base
call conda install --yes --file requirements.txt --satisfied-skip-solve

set VCPKG_DIR=%CD%\vcpkg
set VCPKG_CMD=%VCPKG_DIR%\vcpkg.exe
call %VCPKG_CMD% install

cmake -B Build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\Users\benja\source\repos\Matcha\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

pause> nul