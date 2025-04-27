cl.exe /c /EHsc 19.c
rc.exe OGL.rc
link.exe 19.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS