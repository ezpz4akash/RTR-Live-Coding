cl.exe /c /EHsc 5.c
rc.exe window.rc
link.exe 5.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS