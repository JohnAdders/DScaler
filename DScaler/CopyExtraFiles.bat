rem This batch file is run as a post-build step. 
rem if "%2" == "2005" goto skipDisasm
..\%1\disasm.exe ..\Disasm\ia32.txt ..\%1\ia32.bin
..\%1\mapconv.exe ..\%1\DScaler.map ..\%1\DScaler.vdi ..\%1\ia32.bin
:skipDisasm
xcopy ..\ThirdParty\LibJpeg\*.dll ..\%1\ /Y /D
xcopy ..\ThirdParty\LibTiff\*.dll ..\%1\ /Y /D
xcopy ..\ThirdParty\zlib\*.dll ..\%1\ /Y /D
rem It copies the extra files from the Release folder to the Debug folder.
if "%1" == "Release" goto endbatch
if "%2" == "" xcopy ..\release\DSDrv4.sys ..\%1\ /Y /D
if "%2" == "" xcopy ..\release\DSDrv4.vxd ..\%1\ /Y /D
xcopy ..\release\channel.txt ..\%1\ /Y /D
xcopy ..\release\channel.txt ..\%1\ /Y /D
xcopy ..\release\dscaler.d3u ..\%1\ /Y /D
xcopy ..\release\dscaler_intro.tif ..\%1\ /Y /D
xcopy ..\release\skins\*.* ..\%1\skins\ /E /Y /D
xcopy ..\release\patterns\*.* ..\%1\patterns\ /E /Y /D
if exist ..\release\DScaler.chm xcopy ..\release\DScaler.chm ..\%1\ /Y /D
xcopy ..\release\SAA713xCards.ini ..\%1\ /Y /D
xcopy ..\release\CX2388xCards.ini ..\%1\ /Y /D
:endbatch
