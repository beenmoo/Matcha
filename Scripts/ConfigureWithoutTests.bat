cd ..
if not exist "Build" mkdir Build

set miniconda_activate=C:\Users\%USERNAME%\miniconda3\Scripts\activate.bat
call %miniconda_activate% base

cmake -B Build --preset All-Windows

pause> nul