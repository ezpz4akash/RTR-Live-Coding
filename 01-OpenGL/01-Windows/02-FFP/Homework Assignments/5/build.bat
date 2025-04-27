cl.exe /c /EHsc 5.c
rc.exe OGL.rc
link.exe 5.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS