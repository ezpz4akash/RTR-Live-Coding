mkdir -p OGL.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o OGL.app/Contents/MacOS/OGL OGL.mm -framework Foundation -framework Cocoa -framework QuartzCore -framework OpenGL



