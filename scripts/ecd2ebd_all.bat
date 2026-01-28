@echo off
for /r %%f in (*.ecd) do (
    compexp.exe x "%%f" "%%~dpnf.ebd"
)
pause
