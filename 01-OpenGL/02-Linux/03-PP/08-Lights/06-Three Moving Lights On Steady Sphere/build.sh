g++ -c -o OGL.o OGL.cpp
g++ OGL.o -o OGL -lX11 -lGL -lGLEW -lSphere
# GLEW Include Path : /usr/include/GL
# GLEW Library Path : /usr/lib64/x86_64-linux-gnu/libGLEW.so OR /usr/lib64/libGLEW.so
# /usr/lib64/ is not a standard linker path, so you may need to specify it explicitly
    # echo "/usr/lib64" | sudo tee /etc/ld.so.conf.d/usr-lib64.conf
    # sudo ldconfig
#OR set LD_LIBRARY_PATH in this script
# export LD_LIBRARY_PATH=/usr/lib64/x86_64-linux-gnu:$LD_LIBRARY OR
# export LD_LIBRARY_PATH=/usr/lib64:$LD_LIBRARY_PATH
