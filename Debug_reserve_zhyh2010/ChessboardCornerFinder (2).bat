@REM @Author: anchen
@REM @Date:   2016-11-22 08:57:48
@REM @Last Modified by:   anchen
@REM Modified time: 2016-11-23 18:14:59

@echo off
del /F /Q ..\data\images\filenameVec.txt
for /L %%i in (3, 1, 8) do (
    ChessboardCornerFinder.exe ..\data\images\image0_%%i.jpg 9 7
    ChessboardCornerFinder.exe ..\data\images\image1_%%i.jpg 9 7
    echo ..\data\images\image0_%%i.jpg-corners.yml >> ..\data\images\coorfile0.ifl
    echo ..\data\images\image1_%%i.jpg-corners.yml >> ..\data\images\coorfile1.ifl
)
pause