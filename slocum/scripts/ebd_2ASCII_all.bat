@echo off
for /r %%f in (*.ebd) do (
    dbd2asc.exe "%%f" > "%%~dpnf.eba"
)
pause
