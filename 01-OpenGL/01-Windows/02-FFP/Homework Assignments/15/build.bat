cl.exe /c /EHsc 15.c
rc.exe OGL.rc
link.exe 15.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS