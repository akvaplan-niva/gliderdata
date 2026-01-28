@echo off
for /r %%f in (*.tbd) do (
    dbd2asc.exe "%%f" > "%%~dpnf.tba"
)
pause
