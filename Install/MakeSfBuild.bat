@echo off
echo Dscaler make a sourceforge build batch file
echo (c) John Adcock 2002
if "%1" == "" goto usage
if "%2" == "" goto usage
cd ..\..
md DScalerBuild%1
cd DScalerBuild%1
cvs -z3 -d:ext:%2@cvs.sf.net:/cvsroot/deinterlace co DScaler
cvs -z3 -d:ext:%2@cvs.sf.net:/cvsroot/deinterlace co DSRend
cd DScaler\Dscaler
..\..\..\Dscaler\Release\verinc
cvs commit -m "Update Verion for release %1"
cd ..
pause
cd ..\DSRend
7z a -tzip ..\DScaler\DSRend%1src.zip *.* -r
cd ..\DScaler
7z a -tzip ..\DScaler%1src.zip *.* -r
cd Help
"c:\Program Files\HTML Help Workshop\hhc.exe" Dscaler.hhp
cd ..\DScaler
call c:\PROGRA~1\MICROS~4\VC98\bin\vcvars32.bat
msdev Dscaler.dsw /MAKE "All"
cd ..\Install
"c:\Program Files\Inno Setup 5\Compil32.exe" /cc DScaler.iss
copy Output\Setup.exe ..\..\DScaler%1.exe
cd ..\..
del /f /q /s DSRend
rd /s /q DSRend
del /f /q /s DScaler  
rd /s /q DScaler
echo Break if there was a problem with the above build
echo Otherwise pressing enter will send the files to the
echo incoming directory on sourceforge ready to be released
pause
echo cd incoming > ftp.txt
echo bin >> ftp.txt
echo put DScaler%1.exe >> ftp.txt
echo put DScaler%1src.zip >> ftp.txt
echo bye >> ftp.txt
ftp -s:ftp.txt -A upload.sf.net
del ftp.txt
goto endofbatch
:usage
echo To use this program enter the build number as parameter 1
echo and a valid sourceforge user for parameter 2
:endofbatch