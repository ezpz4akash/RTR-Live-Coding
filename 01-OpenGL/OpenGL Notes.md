# Column Major Matrix : 
    M[16] : 

    [ M[0]   M[4]   M[8]    M[12] ]
    [ M[1]   M[5]   M[9]    M[13] ]
    [ M[2]   M[6]   M[10]   M[14] ]
    [ M[3]   M[7]   M[11]   M[15] ]


# 1) Identity Matrix
    [ 1   0   0   0 ]
    [ 0   1   0   0 ]
    [ 0   0   1   0 ]
    [ 0   0   0   1 ]

# 2) Translation
    [ 1   0   0   tx ]
    [ 0   1   0   ty ]
    [ 0   0   1   tz ]
    [ 0   0   0   1 ]

# 3) Rotation X (t=theta)
    [ 1         0       0       0 ]
    [ 0     cos(t)   -sin(t)    0 ]
    [ 0     sin(t)   cos(t)     0 ]
    [ 0         0       0       1 ]

# 4) Rotation Y (t=theta)
    [ cos(t)    0   sin(t)  0 ]
    [ 0         1       0   0 ]
    [ -sin(t)   0   cos(t)  0 ]
    [ 0         0       0   1 ]

# 5) Rotation Z (t=theta)
    [ cos(t)    -sin(t)  0   0 ]
    [ sin(t)    cos(t)   0   0 ]
    [ 0             0    1   0 ]
    [ 0             0    0   1 ]

# 6)  Scaling Matrix
    [ sx    0    0    0 ]
    [ 0     sy   0    0 ]
    [ 0     0    sz   0 ]
    [ 0     0    0    1 ]

# 7) Orthographic Projection Matrix (l=left, r=right, b=bottom, t=top, n=near, f=far)
    [ 2/(r-l)   0               0                   -((r + l) / (r - l)) ]
    [ 0         2/(t-b)         0                   -((t + b)/ (t - b))  ]
    [ 0         0               -(2 / (f - n))      -((f + n) / (f - n)) ]
    [ 0         0               0                   1                    ]

# 8) Perspective Projection Matrix (l=left, r=right, b=bottom, t=top, n=near, f=far)
    [ 2n / (r - l)      0           (r + l) / (r - l)         0                  ]
    [ 0                 2n/(t-b)    (t + b) / (t - b)         0                  ]
    [ 0                 0           -((f + n) / (f - n))      -(2*f*n / (f - n)) ]
    [ 0                 0           -1                        0                  ]

------------------------------------------------------------------------------------------------
# Transformations : 
    Object -> (ModeView) -> Eye -> (Projection) -> Clipping -> (Perspective Division) -> NDC -> (Viewport) -> Window

------------------------------------------------------------------------------------------------

# Books List
    - FFP : 
        1) OpenGL Programming Guide: The Official Guide to Learning OpenGL By  Dave Shreiner (Red Book, 3rd Edtion)
        2) OpenGL SuperBible By Richard S. Wright Jr (Blue Book, 4Th Edition)
        3) OpenGL Programming For X Windows By Mark Kilgard(Green Book)
        4) Advanced Graphics Programming Using OpenGL By David Blythe

        (Extras)
        5) Interactive Computer Graphics: A Top-Down Approach with Shader-Based OpenGL By Edward Angel 
        6) OpenGL Game Programming (Game development series) By Dave Astle 

------------------------------------------------------------------------------------------------

# Most commonly used Image formarts as texture
Average Quality : 
1. BMP : Bitmap
2. JPG/JPEG : Joint Picture Expert Group
3. PNG : Portable Network Graphics
4. GIF : Graphics Interchangeable Format
5. TGA : TrueVision Graphics Adaptor
6. TARGA : True Vision Raster Graphics Adaptor

Best Quality
1. DDS : Direct Draw Surface
2. HDR : High Dynamic Range
3. OpenEXR : Open EXtended Range
4. ASTC : Adaptive Scalable Texture Compression
5. KTX : Khronos TeXture

Better than average but less than Best Quality
1. TIFF : Tagged Image File Format
2. HEIF : High Efficiency Image Format

# General
    - Linux Cleanup : 
        find . -type f \( -name 'OGL' -o -name 'OGL.o' -o -name 'Log.txt' \) -exec rm -v {} +
