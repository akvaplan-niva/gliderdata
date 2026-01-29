@echo off
for /r %%f in (*.sbd *.tbd) do (
    rename_dbd_files.exe "%%f"
)
pause
