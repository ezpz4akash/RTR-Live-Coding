cl.exe /c /EHsc 4.c
rc.exe OGL.rc
link.exe 4.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS