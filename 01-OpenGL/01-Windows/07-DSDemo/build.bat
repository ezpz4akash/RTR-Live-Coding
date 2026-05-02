@echo off
setlocal enabledelayedexpansion

rem Ensure build directory exists
if not exist build mkdir build

rem Compile all .c files in src/
for %%f in (src\*.c) do (
    set "file=%%~nf"
    echo Compiling %%f to build\!file!.obj
    cl.exe /c /I include/ /EHsc /nologo "%%f" /Fo:build\!file!.obj
)

rem Compile all .rc files in res/RC/
rc.exe /I include/ /fo build/OGL.res res/RC/OGL.rc

rem Compile all .obj and .res files in build/
link.exe build\*.obj build\*.res lib\assimp\*.lib gdi32.lib user32.lib Winmm.lib /OUT:OGL.exe /SUBSYSTEM:WINDOWS

OGL.exe