cd prj_win

set COMPILE_WIN=../../../10-Common/include/common_compile_win.bat

call %COMPILE_WIN% kdvmedianet_vs2010 VS2010 Debug Win32 %1
call %COMPILE_WIN% kdvmedianet_vs2010 VS2010 Release Win32 %1
call %COMPILE_WIN% kdvmedianet_vs2010 VS2010 Debug x64 %1
call %COMPILE_WIN% kdvmedianet_vs2010 VS2010 Release x64 %1

call %COMPILE_WIN% kdvmedianet_2008 VS2008 Debug Win32 %1
call %COMPILE_WIN% kdvmedianet_2008 VS2008 Release Win32 %1
rem call %COMPILE_WIN% kdvmedianet_2008 VS2008 Debug x64 %1
rem call %COMPILE_WIN% kdvmedianet_2008 VS2008 Release x64 %1

call %COMPILE_WIN% kdvmedianet VC6 Debug Win32 %1
call %COMPILE_WIN% kdvmedianet VC6 Release Win32 %1
rem call %COMPILE_WIN% kdvmedianet VC6 Debug x64 %1
rem call %COMPILE_WIN% kdvmedianet VC6 Release x64 %1

cd ..