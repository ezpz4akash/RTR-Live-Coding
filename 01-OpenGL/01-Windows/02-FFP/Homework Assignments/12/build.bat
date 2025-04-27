cl.exe /c /EHsc 12.c
rc.exe OGL.rc
link.exe 12.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS