@echo off
for /r %%f in (*.ek8) do (
    rename_dbd_files.exe "%%f"
)
pause
