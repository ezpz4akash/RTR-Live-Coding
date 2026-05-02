DEL *.exe
DEL *.obj

rc.exe OGL.rc
nvcc.exe -I C:\glew\include -L C:\glew\lib\Release\x64\ -o OGL.exe  OGL.res User32.lib GDI32.lib OGL.cu
