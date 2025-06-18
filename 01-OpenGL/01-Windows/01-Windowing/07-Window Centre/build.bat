cl.exe /c /EHsc window.c
rc.exe window.rc
link.exe window.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS