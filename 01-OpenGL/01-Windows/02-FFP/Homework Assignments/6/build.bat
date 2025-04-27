cl.exe /c /EHsc 6.c
rc.exe OGL.rc
link.exe 6.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS