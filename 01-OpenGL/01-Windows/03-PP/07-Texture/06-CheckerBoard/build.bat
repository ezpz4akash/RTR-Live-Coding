cl.exe /c /EHsc /I ".\..\..\GLEW\include" "OGL.cpp"
rc.exe OGL.rc
link.exe OGL.obj OGL.res gdi32.lib user32.lib ".\..\..\GLEW\lib\Release\x64\glew32.lib" /SUBSYSTEM:WINDOWS