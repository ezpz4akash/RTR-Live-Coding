rc.exe OGL.rc
nvcc.exe -I ".\..\..\GLEW\include" ".\..\..\GLEW\lib\Release\x64\glew32.lib" OGL.res OGL.cu gdi32.lib user32.lib -o OGL.exe