@echo off
echo DScaler make a sourceforge build batch file
echo (c) John Adcock 2009
set buildnum=%1
if "%buildnum%" == "" set /p buildnum= BuildNum to use?
cd ..\..
md DScalerBuild%1
cd DScalerBuild%1
TortoiseProc.exe /command:checkout /path:"./DScaler" /url:https://deinterlace.svn.sourceforge.net/svnroot/deinterlace/trunk/DScaler/ /closeonend:0
cd DScaler\Dscaler
subwcrev ..\.. BuildNum.tmpl BuildNum.cpp
cd ..
pause
cd ..\DScaler
7z a -tzip ..\DScaler%1src.zip *.* -r
cd Help
"c:\Program Files\HTML Help Workshop\hhc.exe" DScaler.hhp
cd ..\DScaler
vcbuild DScaler2008.sln "Release|Win32"
rem cd ..\Driver\DSDrvNT
rem cmd /c ..\..\Install\makeSfBuild2.bat 64
rem cmd /c ..\..\Install\makeSfBuild2.bat AMD64
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
ftp -s:ftp.txt -A upload.sourceforge.net
del ftp.txt
