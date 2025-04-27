cl.exe /c /EHsc 3.c
rc.exe OGL.rc
link.exe 3.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS