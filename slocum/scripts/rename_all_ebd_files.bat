@echo off
for /r %%f in (*.ebd) do (
    rename_dbd_files.exe "%%f"
)
pause
