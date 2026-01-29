@echo off
for /r %%f in (*.dcd) do (
    compexp.exe x "%%f" "%%~dpnf.dbd"
)
pause
