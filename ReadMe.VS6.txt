Compilation instructions for Visual Studio 6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Unpack the DScaler source code into any subdirectory

2. Upgrade your Visual Studio to SP5:
   http://msdn.microsoft.com/vstudio/downloads/updates/sp/vs6/sp5/default.aspx
   Note that SP6 is NOT supported at present since there is no processor pack

3. Get the Visual C++ Processor Pack from:
   http://msdn.microsoft.com/en-us/vstudio/aa718349.aspx
   Install.  It will automatically upgrade Visual Studio.
   This software adds SSE and SSE2 support for DScaler's assembly code.

4. Get the Windows Platform SDK from:
   http://www.microsoft.com/msdownload/platformsdk/sdkupdate/psdk-full.htm
   You will need to install at least Core SDK and the Internet Development SDK
   Install as c:\program files\Microsoft SDK or another drive of your choice.
   We'll use c:\program files\Microsoft SDK as an example here .

5. Get the DirectX SDK (Oct 2004) from:
   http://www.microsoft.com/downloads/details.aspx?FamilyId=B7BC31FA-2DF1-44FD-95A4-C2555446AED4&displaylang=en
   Install as c:\program files\Microsoft DirectX 9.0 SDK (October 2004) or another drive of your choice.
   We'll use c:\program files\Microsoft DirectX 9.0 SDK (October 2004) as an example here.

6. Get Nasm From:
   http://www.nasm.us/pub/nasm/releasebuilds/2.06/win32/
   Extract nasm.exe and put this file into a tools directory. I use c:\tools
   

7. In Visual Studio set up the build directories
   EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
   compile properly.  You will need to substitute the directories
   with the ones on your system if you have installed the sdks to 
   different directories

   Visual Studio->Tools->Options->Directories->Executable Files
      {the various MSVC directories}
      C:\Tools

   Visual Studio->Tools->Options->Directories->Include Files
      C:\Program Files\Microsoft SDK\include
      c:\program files\Microsoft DirectX 9.0 SDK (October 2004)\include
      c:\program files\Microsoft DirectX 9.0 SDK (October 2004)\include\DShowIDL
      {the various MSVC directories}

   Visual Studio->Tools->Options->Directories->Library Files
      C:\Program Files\Microsoft SDK\Lib
      c:\program files\Microsoft DirectX 9.0 SDK (October 2004)\Lib
      {the various MSVC directories}

8. Open DScaler\DScaler.dsw in Visual Studio, set DScaler to be the active project and build



