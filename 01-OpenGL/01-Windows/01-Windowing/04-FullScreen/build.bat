cl.exe /c /EHsc fullscreen.c
rc.exe window.rc
link.exe fullscreen.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS