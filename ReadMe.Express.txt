Compilation instructions for Visual Studio 2005 C++ Express
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Unpack the DScaler source code into any subdirectory

2. Install Sevice Pack 1 for visual studio
   http://www.microsoft.com/downloads/details.aspx?FamilyId=7B0B0339-613A-46E6-AB4D-080D4D4A8C4E&displaylang=en

3. Install the windows SDK from
   http://www.microsoft.com/downloads/details.aspx?FamilyID=e6e1c3df-a74f-4207-8586-711ebe331cdc&displaylang=en (Websetup)
   http://www.microsoft.com/downloads/details.aspx?FamilyId=F26B1AA4-741A-433A-9BE5-FA919850BDBF&displaylang=en (iso)
   This will install to a root of c:\program files\Microsoft SDKs\Windows\v6.1\ by default, this is the root we will use in the examples later

4. Get MASM from
   http://www.microsoft.com/downloads/details.aspx?FamilyID=7a1c9da0-0510-44a2-b042-7ef370530c64&displaylang=en
   Run MASMsetup.exe

5. Get Nasm From:
   http://www.nasm.us/pub/nasm/releasebuilds/2.06/win32/
   Extract nasm.exe and put this file into a tools directory. I use c:\tools

6. Get the Windows Server 2003 DDK from
   http://www.microsoft.com/whdc/DevTools/ddk/default.mspx

7. Patch up the ATL files
   Some files need to be changed, see here:
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   http://www.codeproject.com/wtl/WTLExpress.asp
   http://forums.microsoft.com/msdn/showpost.aspx?postid=66102&siteid=1


   Find this files
   ~~~~~~~~~~~~~~~
   afx.inl
   afxmsg_.h
   afxwin1.inl
   atlwin.h

   and patch them
   ~~~~~~~~~~~~~~

--- afx.inl.old	Fri Mar  3 22:20:28 2006
+++ afx.inl	Tue Dec 12 23:54:02 2006
@@ -249,17 +249,17 @@
 _AFX_INLINE const CTimeSpan& CTimeSpan::operator=(const CTimeSpan& timeSpanSrc)
 	{ m_timeSpan = timeSpanSrc.m_timeSpan; return *this; }
 _AFX_INLINE LONG_PTR CTimeSpan::GetDays() const
-	{ return m_timeSpan / (24*3600L); }
+	{ return (LONG_PTR)(m_timeSpan / (24*3600L)); }
 _AFX_INLINE LONG_PTR CTimeSpan::GetTotalHours() const
-	{ return m_timeSpan/3600; }
+	{ return (LONG_PTR)(m_timeSpan/3600); }
 _AFX_INLINE int CTimeSpan::GetHours() const
 	{ return (int)(GetTotalHours() - GetDays()*24); }
 _AFX_INLINE LONG_PTR CTimeSpan::GetTotalMinutes() const
-	{ return m_timeSpan/60; }
+	{ return (LONG_PTR)(m_timeSpan/60); }
 _AFX_INLINE int CTimeSpan::GetMinutes() const
 	{ return (int)(GetTotalMinutes() - GetTotalHours()*60); }
 _AFX_INLINE LONG_PTR CTimeSpan::GetTotalSeconds() const
-	{ return m_timeSpan; }
+	{ return (LONG_PTR)m_timeSpan; }
 _AFX_INLINE int CTimeSpan::GetSeconds() const
 	{ return (int)(GetTotalSeconds() - GetTotalMinutes()*60); }
 _AFX_INLINE CTimeSpan CTimeSpan::operator-(CTimeSpan timeSpan) const


--- afxmsg_.h.old	Fri Mar  3 22:19:24 2006
+++ afxmsg_.h	Tue Dec 12 18:59:18 2006
@@ -325,7 +325,7 @@
 		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(int, LPMEASUREITEMSTRUCT))&OnMeasureItem },
 #define ON_WM_MEASUREITEM_REFLECT() \
 	{ WM_MEASUREITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vs, \
-		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(LPMEASUREITEMSTRUCT))MeasureItem },
+		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(LPMEASUREITEMSTRUCT))&MeasureItem },
 #define ON_WM_DELETEITEM() \
 	{ WM_DELETEITEM, 0, 0, 0, AfxSig_vOWNER, \
 		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(int, LPDELETEITEMSTRUCT))&OnDeleteItem },


--- afxwin1.inl.old	Fri Mar  3 22:20:28 2006
+++ afxwin1.inl	Tue Dec 12 22:20:28 2006
@@ -1030,9 +1030,9 @@
 _AFXWIN_INLINE CMenu::operator HMENU() const
     { ASSERT(this == NULL || m_hMenu == NULL || ::IsMenu(m_hMenu));
         return this == NULL ? NULL : m_hMenu; }
-_AFXWIN_INLINE CMenu::operator==(const CMenu& menu) const
+_AFXWIN_INLINE int CMenu::operator==(const CMenu& menu) const
     { return ((HMENU) menu) == m_hMenu; }
-_AFXWIN_INLINE CMenu::operator!=(const CMenu& menu) const
+_AFXWIN_INLINE int CMenu::operator!=(const CMenu& menu) const
     { return ((HMENU) menu) != m_hMenu; }
 _AFXWIN_INLINE HMENU CMenu::GetSafeHmenu() const
     { ASSERT(this == NULL || m_hMenu == NULL || ::IsMenu(m_hMenu));


--- atlwin.h.old	Fri Mar  3 22:19:14 2006
+++ atlwin.h	Tue Dec 12 22:11:28 2006
@@ -1750,7 +1750,7 @@
 
         // search for an empty one
 
-                for(i = 0; i < m_aChainEntry.GetSize(); i++)
+                for(int i = 0; i < m_aChainEntry.GetSize(); i++)
                 {
                         if(m_aChainEntry[i] == NULL)
                         {



8. In Visual Studio set up the build directories
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
      C:\WINDDK\3790.1830\inc\atl30
      {the various MSVC directories}

   Visual Studio->Tools->Options->Directories->Library Files for Win32
      c:\program files\Microsoft SDKs\Windows\v6.1\Lib
      C:\WINDDK\3790.1830\lib\atl\i386
      {the various MSVC directories}

9. Open DScaler\DScaler2005.sln in Visual Studio. Go to Solution Explorer, right click on 'DScaler' in tree view, select 'Set as StartUp Project'.
   In Build->Configuration Manager->Active solution configuration select 'Release' or 'Debug'. Build with F7.





