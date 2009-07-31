Compilation instructions for Visual Studio .NET 2005
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Unpack the DScaler source code into any subdirectory

2. Install Sevice Pack 1 for visual studio
   http://www.microsoft.com/downloads/details.aspx?familyid=bb4a75ab-e2d4-4c96-b39d-37baf6b5b1dc&displaylang=en

3. Install the windows SDK from
   http://www.microsoft.com/downloads/details.aspx?FamilyID=e6e1c3df-a74f-4207-8586-711ebe331cdc&displaylang=en (Websetup)
   http://www.microsoft.com/downloads/details.aspx?FamilyId=F26B1AA4-741A-433A-9BE5-FA919850BDBF&displaylang=en (iso)
   This will install to a root of c:\program files\Microsoft SDKs\Windows\v6.1\ by default, this is the root we will use in the examples later

4. Get Nasm From:
   http://www.nasm.us/pub/nasm/releasebuilds/2.06/win32/
   Extract nasm.exe and put this file into a tools directory. I use c:\tools

5. In Visual Studio set up the build directories
   EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
   compile properly.  You will need to substitute the directories
   with the ones on your system if you have installed the sdks to 
   different directories

   Visual Studio->Tools->Options->Directories->Executable Files for Win32
      c:\program files\Microsoft SDKs\Windows\v6.1\bin
      {the various MSVC directories}
      C:\Tools

   Visual Studio->Tools->Options->Directories->Include Files for Win32
      c:\program files\Microsoft SDKs\Windows\v6.1\include
      {the various MSVC directories}

   Visual Studio->Tools->Options->Directories->Library Files for Win32
      c:\program files\Microsoft SDKs\Windows\v6.1\Lib
      {the various MSVC directories}

6. Open DScaler\DScaler2005.sln in Visual Studio, set DScaler to be the active project, 
   ensure that you have selected either the Debug or Release configuration and build



