cl.exe /c /EHsc 8.c
rc.exe OGL.rc
link.exe 8.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS