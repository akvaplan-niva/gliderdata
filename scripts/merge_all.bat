@echo off
set output_file=merged.dba

REM Delete the merged.dba file if it already exists
if exist %output_file% del %output_file%

REM Loop through all .dba files in the current directory
for %%f in (*.dba) do (
    type "%%f" >> %output_file%
)

echo All .dba files have been merged into %output_file%
pause
