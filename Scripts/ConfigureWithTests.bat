cd ..
if not exist "Build" mkdir Build

set miniconda_activate=C:\Users\%USERNAME%\miniconda3\Scripts\activate.bat
call %miniconda_activate% base

cmake -B Build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON

pause> nul