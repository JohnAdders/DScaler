Compilation instructions for Visual Studio 6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Unpack the DScaler source code into any subdirectory

2. Upgrade your Visual Studio to SP5:
   http://msdn.microsoft.com/vstudio/downloads/updates/sp/vs6/sp5/default.aspx
   Note that SP6 is NOT supported at present since there is no processor pack

3. Get the Visual C++ Processor Pack from:
   http://msdn.microsoft.com/vstudio/downloads/tools/ppack/download.aspx
   Install.  It will automatically upgrade Visual Studio.
   This software adds SSE and SSE2 support for DScaler's assembly code.

4. Get the Windows Platform SDK (Core SDK) from:
   http://www.microsoft.com/msdownload/platformsdk/sdkupdate/
   Install as c:\program files\Microsoft SDK or another drive of your choice.
   We'll use c:\program files\Microsoft SDK as an example here .

The DDK is required to compile the drivers. In their wisdom Mircosoft no longer have the 
DDK available for download.  It is possible to compile the rest of DScaler without the DDK.

Also there are two version of the DDK that we support.  Follow the correct instructions  depending 
on the DDK that you have

Windows DDK (with Windows 2000 and ME support):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5. Install as C:\NTDDK or another location of your choice.
   We'll use drive C:\NTDDK as an example here.

6. In Visual Studio set up the build directories
   EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
   compile properly.  You will need to substitute the directories
   with the ones on your system if you have installed thsdk or ddk to 
   different directories

   Visual Studio->Tools->Options->Directories->Include Files
      C:\NTDDK\inc
      C:\NTDDK\inc\ddk
	  C:\Program Files\Microsoft SDK\include
      {the various MSVC directories}
      C:\NTDDK\inc\win_me

   Visual Studio->Tools->Options->Directories->Library Files
      C:\Program Files\Microsoft SDK\lib
	  {the various MSVC directories}
      C:\NTDDK\libfre\i386
      C:\NTDDK\lib\win_me

   Visual Studio->Tools->Options->Directories->Executable Files
      {the various MSVC directories}
      C:\NTDDK\bin

7. Open and build DScaler\DScaler.dsw 
   This should build all of DScaler's components


Windows XP SP1 DDK
~~~~~~~~~~~~~~~~~~

5. Install the DDK to your machine (IMPORTANT you must include ME stuff).
   You must NOT accept the default setup options.
   Expand the "Build Environment" component group and select all the entries,
   especially "VxD and 16 Bit Devleopement Enviroment"
   And install.

6. Build the Win95 VxD
   Start "Win Me Free Build Environment" from the DDK menu.
   Go to the Driver\DSDrv95 directory
   Build the driver using the command "nmake"
   You should then copy DSDrv4.vxd manually from Release to Debug

7. Build the NT driver
   Start "Win XP Free Build Environment" from the DDK menu.
   Go to the Driver\DSDrvNT directory
   Build the driver using the command "build"
   You should then copy DSDrv4.sys manually from Release to Debug

8. In Visual Studio set up the build directories
   EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
   compile properly.  You will need to substitute the directories
   with the ones on your system if you have installed thsdk or ddk to 
   different directories

   Visual Studio->Tools->Options->Directories->Include Files
      C:\Program Files\Microsoft SDK\include
      {the various MSVC directories}

   Visual Studio->Tools->Options->Directories->Library Files
      C:\Program Files\Microsoft SDK\lib
     {the various MSVC directories}

9. Open DScaler\DScaler.dsw in Visual Studio remove the DSDrvNT and DSDrv95 
   projects from the workspace and build

No DDK
~~~~~~

5. Copy the DSDrv4.vxd and DSDrv4.sys from a recent DScaler installation to 
   the Release and Debug directories

6. In Visual Studio set up the build directories
   EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
   compile properly.  You will need to substitute the directories
   with the ones on your system if you have installed thsdk or ddk to 
   different directories

   Visual Studio->Tools->Options->Directories->Include Files
      C:\Program Files\Microsoft SDK\include
      {the various MSVC directories}

   Visual Studio->Tools->Options->Directories->Library Files
      C:\Program Files\Microsoft SDK\lib
     {the various MSVC directories}

7. Open DScaler\DScaler.dsw in Visual Studio remove the DSDrvNT and DSDrv95 
   projects from the workspace and build



