Last Update: 2007/03/04

Required Tools
~~~~~~~~~~~~~~
- Visual Studio C++ 2005 Express Edition
- VS2005 Express SP1 update
- Microsoft Platform SDK for Windows Server 2003 R2
- Microsoft DirectX SDK (October 2006)
- Windows Server 2003 Service Pack 1 (SP1) Driver Development Kit (DDK)

Unpack the DScaler source code into any subdirectory.

In Visual Studio set up the build directories
EXACTLY as ordered.  This is VERY IMPORTANT, or the project won't
compile properly.  You will need to substitute the directories
with the ones on your system if you have installed the sdk or ddk to 
different directories.

Visual Studio->Tools->Options->Projects and Solutions->VC++ Directories->Include Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include
C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\mfc
C:\WINDDK\3790.1830\inc\ddk\wxp
C:\WINDDK\3790.1830\inc\wxp
C:\WINDDK\3790.1830\inc\atl30
C:\WINDDK\3790.1830\inc\mfc42
C:\Programme\Microsoft DirectX SDK (October 2006)\Include
$(VCInstallDir)include

Visual Studio->Tools->Options->Projects and Solutions->VC++ Directories->Library files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Lib
C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Lib\Win95
C:\WINDDK\3790.1830\lib\atl\i386
C:\WINDDK\3790.1830\lib\wxp\i386
C:\WINDDK\3790.1830\lib\mfc\i386
C:\Program Files\Microsoft DirectX SDK (October 2006)\Lib\x86
$(VSInstallDir)
$(VCInstallDir)lib
$(VSInstallDir)lib


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


Open DScaler\DScaler2005.sln in Visual Studio. Go to Solution Explorer, right click on 'DScaler' in tree view, select 'Set as StartUp Project'. In Build->Configuration Manager->Active solution configuration select 'Release_Express' or 'Debug_Express'. Build with F7.
