@echo off
for /r %%f in (*.scd) do (
    compexp.exe x "%%f" "%%~dpnf.sbd"
)
pause
