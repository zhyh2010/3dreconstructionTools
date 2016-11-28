@REM @Author: anchen
@REM @Date:   2016-11-22 11:33:37
@REM @Last Modified by:   anchen
@REM Modified time: 2016-11-22 11:36:43

@echo off
for %%i in (*.ilk, *.pdb) do (
    del /F /Q %%i
    echo "del " %%i
)
pause