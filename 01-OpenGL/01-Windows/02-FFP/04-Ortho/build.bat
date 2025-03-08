cl.exe /c /EHsc OGL.c
rc.exe OGL.rc
link.exe OGL.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS