@echo off
for /r %%f in (*.sbd) do (
    dbd2asc.exe "%%f" > "%%~dpnf.sba"
)
pause
