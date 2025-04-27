cl.exe /c /EHsc 21.c
rc.exe OGL.rc
link.exe 21.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS