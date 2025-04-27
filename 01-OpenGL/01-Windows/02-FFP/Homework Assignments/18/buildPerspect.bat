cl.exe /c /EHsc 18(b).c
rc.exe OGL.rc
link.exe 18(b).obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS