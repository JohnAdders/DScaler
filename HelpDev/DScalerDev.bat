@echo off
REM ************************************************************
REM The batch file uses the source file javadoc to 
REM generate a compressed help file.
REM
REM to use you must have both 
REM doxygen.exe and hhc.exe in your path
REM ************************************************************
del /q .\html\*.*
doxygen Dscaler.cfg
cd html
hhc index.hhp
copy index.chm ..\DScalerDev.chm
cd ..