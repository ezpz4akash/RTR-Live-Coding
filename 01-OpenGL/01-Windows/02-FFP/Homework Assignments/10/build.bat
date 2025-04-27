cl.exe /c /EHsc 10.c
rc.exe OGL.rc
link.exe 10.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS