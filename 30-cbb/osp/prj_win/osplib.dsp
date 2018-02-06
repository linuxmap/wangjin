# Microsoft Developer Studio Project File - Name="OspLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=OspLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osplib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osplib.mak" CFG="OspLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OspLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "OspLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "OspLib"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OspLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OspLib___Win32_Release"
# PROP BASE Intermediate_Dir "OspLib___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\..\..\10-Common\include\cbb" /I "..\..\..\10-Common\include\cbbext\container" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D FD_SETSIZE=512 /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../10-common/lib/release/win_vc6_x86/OspLib.lib"

!ELSEIF  "$(CFG)" == "OspLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "OspLib___Win32_Debug"
# PROP BASE Intermediate_Dir "OspLib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\include" /I "..\..\..\10-Common\include\cbb" /I "..\..\..\10-Common\include\cbbext\container" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D FD_SETSIZE=512 /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../10-common/lib/debug/win_vc6_x86/OspLib.lib"

!ENDIF 

# Begin Target

# Name "OspLib - Win32 Release"
# Name "OspLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
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

# PROP Default_Filter ""
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

SOURCE=..\include\ospTest.h
# End Source File
# Begin Source File

SOURCE=..\include\OspTestAgent.h
# End Source File
# Begin Source File

SOURCE=..\include\ospTimer.h
# End Source File
# Begin Source File

SOURCE=..\include\ospVos.h
# End Source File
# Begin Source File

SOURCE=..\include\zconf.h
# End Source File
# Begin Source File

SOURCE=..\include\zlib.h
# End Source File
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

SOURCE=..\src\zlib\zconf.in.h
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\src\zlib\zutil.h
# End Source File
# End Group
# End Target
# End Project
