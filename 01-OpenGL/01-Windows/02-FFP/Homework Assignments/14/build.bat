cl.exe /c /EHsc 14.c
rc.exe OGL.rc
link.exe 14.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS