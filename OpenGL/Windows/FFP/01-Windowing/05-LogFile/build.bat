cl.exe /c /EHsc logfile.c
rc.exe window.rc
link.exe logfile.obj window.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS