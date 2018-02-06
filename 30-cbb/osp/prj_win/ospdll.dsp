# Microsoft Developer Studio Project File - Name="OspDll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=OspDll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ospdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ospdll.mak" CFG="OspDll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OspDll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "OspDll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/software/implement/osp/prj", ZLTAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OspDll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../10-common/lib/release/win_vc6_x86"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSPDLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "../include" /I "..\..\..\10-Common\include\cbb" /I "..\..\..\10-Common\include\cbbext\container" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSPDLL_EXPORTS" /D FD_SETSIZE=512 /D "_WINDLL" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 Ws2_32.lib user32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "OspDll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../10-common/lib/debug/win_vc6_x86"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSPDLL_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /I "../include" /I "..\..\..\10-Common\include\cbb" /I "..\..\..\10-Common\include\cbbext\container" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSPDLL_EXPORTS" /D FD_SETSIZE=512 /D "_WINDLL" /D "_AFXDLL" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib user32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "OspDll - Win32 Release"
# Name "OspDll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\ospLog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospNodeMan.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospPost.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospSch.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OspTeleServer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospTest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OspTestAgent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ospVos.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\..\10-Common\include\cbb\osp.h"
# End Source File
# Begin Source File

SOURCE=..\include\ospLog.h
# End Source File
# Begin Source File

SOURCE=..\include\ospNodeMan.h
# End Source File
# Begin Source File

SOURCE=..\include\ospPost.h
# End Source File
# Begin Source File

SOURCE=..\include\ospSch.h
# End Source File
# Begin Source File

SOURCE=..\include\OspTeleServer.h
# End Source File
# Begin Source File

SOURCE=..\include\ospTimer.h
# End Source File
# Begin Source File

SOURCE=..\include\ospVos.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zconf.in.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
