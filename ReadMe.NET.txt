Compilation instructions for Visual Studio .NET 2003
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Unpack the DScaler source code into any subdirectory

The DDK is required to compile the drivers. In their wisdom Mircosoft no longer have the 
DDK available for download.  It is possible to compile the rest of DScaler without the DDK.

If you can get hold of the latest DDK follow those instructions otherwise follow the No DDK instructions.


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

8. Open DScaler\DScaler.sln in Visual Studio and build

No DDK
~~~~~~

5. Copy the DSDrv4.vxd and DSDrv4.sys from a recent DScaler installation to 
   the Release and Debug directories


6. Open DScaler\DScaler.sln in Visual Studio and build



