rem This batch file is run as a post-build step. It copies the 
rem extra files from the Release folder to the Debug folder.

xcopy ..\release\channel.txt ..\debug\ /Y /D
xcopy ..\release\dscaler.d3u ..\debug\ /Y /D
xcopy ..\release\dscaler_intro.tif ..\debug\ /Y /D
xcopy ..\release\skins\*.* ..\debug\skins\ /E /Y /D
xcopy ..\release\patterns\*.* ..\debug\patterns\ /E /Y /D