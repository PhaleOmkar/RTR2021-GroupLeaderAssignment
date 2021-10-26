cls
del RTR2021Project.exe CodeExecutionLog.txt

cl.exe "source/RTR2021Project.cpp" /c /EHsc /I "C:\glew-2.1.0\include"
rc.exe Resource.rc
link.exe RTR2021Project.obj Resource.res /LIBPATH:"C:\glew-2.1.0\lib\Release\x64" /LIBPATH:"D:\AstroMediComp\RTR\_GLAssignmentSolarSystemProject\Project\FirstVersion\lib" gdi32.lib user32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
RTR2021Project.exe
del RTR2021Project.obj Resource.res
