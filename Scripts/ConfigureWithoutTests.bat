cd ..
if not exist "Build" mkdir Build

set miniconda_activate=C:\Users\%USERNAME%\miniconda3\Scripts\activate.bat
call %miniconda_activate% base

cmake --preset "Windows Visual Studio" -DBUILD_TESTS=OFF

pause> nul