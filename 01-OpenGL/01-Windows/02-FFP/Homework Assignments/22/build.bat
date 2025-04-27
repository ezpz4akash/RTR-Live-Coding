cl.exe /c /EHsc 22.c
rc.exe OGL.rc
link.exe 22.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS