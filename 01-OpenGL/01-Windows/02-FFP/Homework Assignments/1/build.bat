cl.exe /c /EHsc 1.c
rc.exe OGL.rc
link.exe 1.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS