@echo off
for /r %%f in (*.tbd) do (
    rename_dbd_files.exe "%%f"
)
pause
