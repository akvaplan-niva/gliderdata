@echo off
for /r %%f in (*.tcd) do (
    compexp.exe x "%%f" "%%~dpnf.tbd"
)
pause
