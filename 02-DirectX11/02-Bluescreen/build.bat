cl.exe /c /EHsc D3D11.cpp
rc.exe D3D11.rc
link.exe D3D11.obj D3D11.res gdi32.lib user32.lib /SUBSYSTEM:WINDOWS