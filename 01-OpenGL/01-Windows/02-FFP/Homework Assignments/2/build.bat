cl.exe /c /EHsc OGL.c
rc.exe OGL.rc
link.exe OGL.obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS