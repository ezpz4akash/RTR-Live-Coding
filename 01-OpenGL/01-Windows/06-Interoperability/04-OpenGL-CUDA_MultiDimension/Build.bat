DEL *.exe
DEL *.obj

rc.exe OGL.rc
nvcc.exe -c Sinewave.cu -o Sinewave.obj
cl.exe /c /EHsc /I C:\glew\include  /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\include" OGL.cpp
link.exe OGL.obj Sinewave.obj OGL.res /LIBPATH:C:\glew\lib\Release\x64\ /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\lib\x64"  User32.lib Kernel32.lib GDI32.lib glew32.lib /SUBSYSTEM:WINDOWS
