cd ..
if not exist "Build" mkdir Build

set miniconda_activate=C:\Users\%USERNAME%\miniconda3\Scripts\activate.bat
call %miniconda_activate% base
call conda install --yes --file requirements.txt --satisfied-skip-solve

set VCPKG_DIR=%CD%\vcpkg
set VCPKG_CMD=%VCPKG_DIR%\vcpkg.exe
call %VCPKG_CMD% install

cmake --preset "Windows Visual Studio" -DBUILD_TESTS=ON

pause> nul