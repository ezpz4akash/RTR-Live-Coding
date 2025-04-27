cl.exe /c /EHsc 17.c
rc.exe OGL.rc
link.exe 17.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS