cl.exe /c /EHsc 20.c
rc.exe OGL.rc
link.exe 20.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS