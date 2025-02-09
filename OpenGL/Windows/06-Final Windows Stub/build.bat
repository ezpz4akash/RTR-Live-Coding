cl.exe /c /EHsc finalstub.c
rc.exe window.rc
link.exe finalstub.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS