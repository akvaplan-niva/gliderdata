@echo off
for /r %%f in (*.mcg) do (
    compexp.exe x "%%f" "%%~dpnf.mlg"
)
pause
