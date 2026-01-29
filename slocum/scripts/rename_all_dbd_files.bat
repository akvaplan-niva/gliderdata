@echo off
for /r %%f in (*.dbd) do (
    rename_dbd_files.exe "%%f"
)
pause
