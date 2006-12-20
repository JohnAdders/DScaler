@echo off
REM ************************************************************
REM The batch file uses the source file javadoc to 
REM generate a compressed help file.
REM
REM to use you must have both 
REM doxygen.exe, hhc.exe and dot.exe in your path
REM 
REM Get doxygen from http://www.stack.nl/~dimitri/doxygen/
REM Get dot.exe from http://www.graphviz.org/pub/graphviz/windows/
REM        (You will also need cygwin1.dll)
REM Get hhc.exe by installing htmlhelp from the service pack
REM ************************************************************
del /q .\html\*.*
doxygen Dscaler.cfg
cd html
hhc index.hhp
copy index.chm ..\DScalerDev.chm
cd ..