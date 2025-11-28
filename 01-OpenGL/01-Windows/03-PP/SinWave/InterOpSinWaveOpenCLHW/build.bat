cl.exe /c /EHsc /I ".\..\..\GLEW\include" /I  "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\include" "OGL.cpp"
rc.exe OGL.rc
link.exe OGL.obj OGL.res gdi32.lib user32.lib ".\..\..\GLEW\lib\Release\x64\glew32.lib" "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\lib\x64\OpenCL.lib" /SUBSYSTEM:WINDOWS