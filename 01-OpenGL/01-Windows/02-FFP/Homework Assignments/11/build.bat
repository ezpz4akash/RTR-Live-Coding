cl.exe /c /EHsc 11.c
rc.exe OGL.rc
link.exe 11.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS