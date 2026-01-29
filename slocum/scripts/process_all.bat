@echo off
REM ============================================================
REM  Unified processor for Slocum glider files
REM  - Decompress: tcd, scd, dcd, ecd, mcd, ncd, ccc
REM  - Rename:     tbd, sbd, dbd, ebd, mbd, nbd
REM  - To ASCII:   tba, sba, dba, eba, mba, nba
REM  Requires: compexp.exe, rename_dbd_files.exe, dbd2asc.exe
REM  Run this in the top directory containing your data.
REM ============================================================

echo Starting glider file processing...
echo.

REM ------------------------------------------------------------
REM STEP 1: Decompress compressed files to their binary versions
REM ------------------------------------------------------------

call :decompress tcd tbd
call :decompress scd sbd
call :decompress dcd dbd
call :decompress ecd ebd
call :decompress mcd mbd
call :decompress ncd nbd
call :decompress ccc cac

REM ------------------------------------------------------------
REM STEP 2: Rename binary files (except cac)
REM ------------------------------------------------------------

call :rename_bd tbd
call :rename_bd sbd
call :rename_bd dbd
call :rename_bd ebd
call :rename_bd mbd
call :rename_bd nbd

REM ------------------------------------------------------------
REM STEP 3: Convert binary data files to ASCII
REM ------------------------------------------------------------

call :bd2asc tbd tba
call :bd2asc sbd sba
call :bd2asc dbd dba
call :bd2asc ebd eba
call :bd2asc mbd mba
call :bd2asc nbd nba

echo.
echo All processing finished.
pause
goto :eof


:decompress
REM Usage:  call :decompress INEXT OUTEXT
REM Example: call :decompress dcd dbd
set "INEXT=%1"
set "OUTEXT=%2"

echo ----------------------------------------------
echo Decompressing *.%INEXT% to *.%OUTEXT% ...
for /r %%F in (*.%INEXT%) do (
    echo   [decompress] %%F
    compexp.exe x "%%F" "%%~dpnF.%OUTEXT%"
)
echo.
goto :eof


:rename_bd
REM Usage:  call :rename_bd EXT
REM Example: call :rename_bd dbd
set "EXT=%1"

echo ----------------------------------------------
echo Renaming *.%EXT% with rename_dbd_files.exe ...
for /r %%F in (*.%EXT%) do (
    echo   [rename] %%F
    rename_dbd_files.exe "%%F"
)
echo.
goto :eof


:bd2asc
REM Usage:  call :bd2asc INEXT OUTEXT
REM Example: call :bd2asc dbd dba
set "INEXT=%1"
set "OUTEXT=%2"

echo ----------------------------------------------
echo Converting *.%INEXT% to *.%OUTEXT% (ASCII) ...
for /r %%F in (*.%INEXT%) do (
    echo   [ASCII] %%F
    dbd2asc.exe "%%F" > "%%~dpnF.%OUTEXT%"
)
echo.
goto :eof
