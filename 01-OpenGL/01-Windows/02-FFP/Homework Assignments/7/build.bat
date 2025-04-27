cl.exe /c /EHsc 7.c
rc.exe OGL.rc
link.exe 7.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS