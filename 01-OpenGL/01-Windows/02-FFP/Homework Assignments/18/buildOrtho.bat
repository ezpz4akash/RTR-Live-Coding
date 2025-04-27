cl.exe /c /EHsc 18(a).c
rc.exe OGL.rc
link.exe 18(a).obj OGL.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS