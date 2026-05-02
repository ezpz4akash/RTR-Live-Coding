DEL *.exe
DEL *.obj
cl.exe /c /EHsc /I C:\glew\include OGL.cpp
rc.exe OGL.rc
link.exe OGL.obj OGL.res /LIBPATH:C:\glew\lib\Release\x64\ User32.lib Kernel32.lib GDI32.lib glew32.lib /SUBSYSTEM:WINDOWS