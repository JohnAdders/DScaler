@echo off
echo Dscaler make a sourceforge build batch file
echo (c) John Adcock 2002
if "%1" == "" goto usage
cd ..\..
md DScalerBuild%1
cd DScalerBuild%1
cvs -z3 -d:ext:adcockj@cvs.sf.net:/cvsroot/deinterlace co DScaler
cd DScaler\Dscaler
..\..\..\Dscaler\Release\verinc
cvs commit -m "Update Verion for release %1"
cd ..
pause
7z a -tzip ..\DScaler%1src.zip *.* -r
cd DScaler
call d:\PROGRA~1\MICROS~2\VC98\bin\vcvars32.bat
msdev Dscaler.dsw /MAKE "All"
cd ..\Help
"d:\Program Files\HTML Help Workshop\hhc.exe" Dscaler.hhp
cd ..\Install
"e:\Program Files\Inno Setup 2\Compil32.exe" /cc DScaler.iss
copy Output\Setup.exe ..\..\DScaler%1.exe
cd ..\..
del /f /q /s DScaler  
rd /s /q Dscaler
goto endofbatch
:usage
echo To use this program enter the build number as a parameter
:endofbatch