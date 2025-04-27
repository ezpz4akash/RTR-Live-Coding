cl.exe /c /EHsc 2.c
rc.exe OGL.rc
link.exe 2.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS