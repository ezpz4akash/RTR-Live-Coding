cl.exe /c /EHsc 9.c
rc.exe OGL.rc
link.exe 9.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS