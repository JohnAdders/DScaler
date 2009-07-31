@echo off
echo DScaler make a sourceforge build batch file
echo (c) John Adcock 2009
set buildnum=%1
if "%buildnum%" == "" set /p buildnum= BuildNum to use?
cd ..\..
md DScalerBuild%buildnum%
cd DScalerBuild%buildnum%
TortoiseProc.exe /command:checkout /path:"./DScaler" /url:https://deinterlace.svn.sourceforge.net/svnroot/deinterlace/trunk/DScaler/ /closeonend:0
cd DScaler\Dscaler
subwcrev .. BuildNum.tmpl BuildNum.cpp
cd ..
pause
cd ..\DScaler
7z a -tzip ..\DScaler%buildnum%src.zip *.* -r
cd Help
"c:\Program Files\HTML Help Workshop\hhc.exe" DScaler.hhp
cd ..\DScaler
call "%VS90COMNTOOLS%vsvars32.bat"
vcbuild DScaler2008.sln "Release|Win32"
rem cd ..\Driver\DSDrvNT
rem cmd /c ..\..\Install\makeSfBuild2.bat 64
rem cmd /c ..\..\Install\makeSfBuild2.bat AMD64
cd ..\Release
7z a -tzip pdb.zip *.pdb -r
copy pdb.zip ..\..\DScaler%buildnum%pdb.zip
cd ..\Install
"c:\Program Files\Inno Setup 5\Compil32.exe" /cc DScaler.iss
copy Output\Setup.exe ..\..\DScaler%buildnum%.exe
cd ..\..
del /f /q /s DScaler  
rd /s /q DScaler
echo The files are now ready to be uploaded to sourceforge
pause
