@echo off
for /r %%f in (*.dbd) do (
    dbd2asc.exe "%%f" > "%%~dpnf.dba"
)
pause
