cls

del *.obj
del *.res
del *.exe

cl.exe /c /EHsc printDXInfo.cpp 

rc.exe Window.rc

link.exe printDXInfo.obj Window.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS