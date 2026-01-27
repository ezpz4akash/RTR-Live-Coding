#pragma once

struct sphere_material
{
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess[1];
};

struct sphere_material sphereMaterials[24] =
{
    {
    // ***** 1st sphere on 1st column, emerald *****
// ambient material
    {0.0215 ,0.1745 ,0.0215 ,1.0f},


// diffuse material
    {0.07568 ,0.61424 ,0.07568 ,1.0f},


// specular material
    {0.633 ,0.727811 ,0.633 ,1.0f},


// shininess
    {0.6 * 128},
    },

// *******************************************************

// ***** 2nd sphere on 1st column, jade *****
    {
// ambient material
    {0.135 ,0.2225 ,0.1575 ,1.0f},


// diffuse material
    {0.54 ,0.89 ,0.63 ,1.0f},


// specular material
    {0.316228 ,0.316228 ,0.316228 ,1.0f},


// shininess
    {0.1 * 128},
    },

// *******************************************************

// ***** 3rd sphere on 1st column, obsidian *****
// ambient material
    {
    {0.05375 ,0.05 ,0.06625 ,1.0f},


// diffuse material
    {0.18275 ,0.17 ,0.22525 ,1.0f},


// specular material
    {0.332741 ,0.328634 ,0.346435 ,1.0f},


// shininess
    {0.3 * 128},
    },

// *******************************************************

// ***** 4th sphere on 1st column, pearl *****
// ambient material
    {
    {0.25 ,0.20725 ,0.20725 ,1.0f},


// diffuse material
    {1.0 ,0.829 ,0.829 ,1.0f},


// specular material
    {0.296648 ,0.296648 ,0.296648 ,1.0f},


// shininess
    {0.088 * 128},
    },

// *******************************************************

// ***** 5th sphere on 1st column, ruby *****
    {
// ambient material
    {0.1745 ,0.01175 ,0.01175 ,1.0f},


// diffuse material
    {0.61424 ,0.04136 ,0.04136 ,1.0f},


// specular material
    {0.727811 ,0.626959 ,0.626959 ,1.0f},


// shininess
    {0.6 * 128},
    },

// *******************************************************

// ***** 6th sphere on 1st column, turquoise *****
    {
// ambient material
    {0.1 ,0.18725 ,0.1745 ,1.0f},


// diffuse material
    {0.396 ,0.74151 ,0.69102 ,1.0f},


// specular material
    {0.297254 ,0.30829 ,0.306678 ,1.0f},


// shininess
    {0.1 * 128},
    },

// *******************************************************
// *******************************************************
// *******************************************************

// ***** 1st sphere on 2nd column, brass *****
    {
// ambient material

    {0.329412 ,0.223529 ,0.027451 ,1.0f},


// diffuse material
    {0.780392 ,0.568627 ,0.113725 ,1.0f},


// specular material
    {0.992157 ,0.941176 ,0.807843 ,1.0f},


// shininess
    {0.21794872 * 128},
    },

// *******************************************************

// ***** 2nd sphere on 2nd column, bronze *****
    {
// ambient material
    {0.2125 ,0.1275 ,0.054 ,1.0f},


// diffuse material
    {0.714 ,0.4284 ,0.18144 ,1.0f},


// specular material
    {0.393548 ,0.271906 ,0.166721 ,1.0f},


// shininess
    {0.2 * 128},
    },

// *******************************************************

// ***** 3rd sphere on 2nd column, chrome *****
    {
// ambient material
    {0.25 ,0.25 ,0.25 ,1.0f},


// diffuse material
    {0.4 ,0.4 ,0.4 ,1.0f},


// specular material
    {0.774597 ,0.774597 ,0.774597 ,1.0f},


// shininess
    {0.6 * 128},
    },

// *******************************************************

// ***** 4th sphere on 2nd column, copper *****.
    {
// ambient material
    {0.19125 ,0.0735 ,0.0225 ,1.0f},


// diffuse material
    {0.7038 ,0.27048 ,0.0828 ,1.0f},


// specular material
    {0.256777 ,0.137622 ,0.086014 ,1.0f},


// shininess
    {0.1 * 128},
    },

// *******************************************************

// ***** 5th sphere on 2nd column, gold *****
    {
// ambient material
    {0.24725 ,0.1995 ,0.0745 ,1.0f},


// diffuse material
    {0.75164 ,0.60648 ,0.22648 ,1.0f},


// specular material
    {0.628281 ,0.555802 ,0.366065 ,1.0f},


// shininess
    {0.4 * 128},
    },

// *******************************************************

// ***** 6th sphere on 2nd column, silver *****
    {
// ambient material
    {0.19225 ,0.19225 ,0.19225 ,1.0f},


// diffuse material
    {0.50754 ,0.50754 ,0.50754 ,1.0f},


// specular material
    {0.508273 ,0.508273 ,0.508273 ,1.0f},


// shininess
    {0.4 * 128},
    },

// *******************************************************
// *******************************************************
// *******************************************************

// ***** 1st sphere on 3rd column, black *****
    {
// ambient material
    {0.0 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.01 ,0.01 ,0.01 ,1.0f},


// specular material
    {0.50 ,0.50 ,0.50 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************

// ***** 2nd sphere on 3rd column, cyan *****
    {
// ambient material
    {0.0 ,0.1 ,0.06 ,1.0f},


// diffuse material
    {0.0 ,0.50980392 ,0.50980392 ,1.0f},


// specular material
    {0.50196078 ,0.50196078 ,0.50196078 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************

// ***** 3rd sphere on 2nd column, green *****
    {
// ambient material
    {0.0 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.1 ,0.35 ,0.1 ,1.0f},


// specular material
    {0.45 ,0.55 ,0.45 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************

// ***** 4th sphere on 3rd column, red *****
    {
// ambient material
    {0.0 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.5 ,0.0 ,0.0 ,1.0f},


// specular material
    {0.7 ,0.6 ,0.6 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************

// ***** 5th sphere on 3rd column, white *****
// ambient material
    {
    {0.0 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.55 ,0.55 ,0.55 ,1.0f},


// specular material
    {0.70 ,0.70 ,0.70 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************

// ***** 6th sphere on 3rd column, yellow plastic *****
// ambient material
    {
    {0.0 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.5 ,0.5 ,0.0 ,1.0f},


// specular material
    {0.60 ,0.60 ,0.50 ,1.0f},


// shininess
    {0.25 * 128},
    },

// *******************************************************
// *******************************************************
// *******************************************************

// ***** 1st sphere on 4th column, black *****
// ambient material
    {
    {0.02 ,0.02 ,0.02 ,1.0f},


// diffuse material
    {0.01 ,0.01 ,0.01 ,1.0f},


// specular material
    {0.4 ,0.4 ,0.4 ,1.0f},


// shininess
    {0.078125 * 128},
    },

// *******************************************************

// ***** 2nd sphere on 4th column, cyan *****
// ambient material
    {
    {0.0 ,0.05 ,0.05 ,1.0f},


// diffuse material
    {0.4 ,0.5 ,0.5 ,1.0f},


// specular material
    {0.04 ,0.7 ,0.7 ,1.0f},


// shininess
    {0.078125 * 128},
    },

// *******************************************************

// ***** 3rd sphere on 4th column, green *****
// ambient material
    {
    {0.0 ,0.05 ,0.0 ,1.0f},


// diffuse material
    {0.4 ,0.5 ,0.4 ,1.0f},


// specular material
    {0.04 ,0.7 ,0.04 ,1.0f},


// shininess
    {0.078125 * 128},
    },

// *******************************************************

// ***** 4th sphere on 4th column, red *****
// ambient material
    {
    {0.05 ,0.0 ,0.0 ,1.0f},


// diffuse material
    {0.5 ,0.4 ,0.4 ,1.0f},


// specular material
    {0.7 ,0.04 ,0.04 ,1.0f},


// shininess
    {0.078125 * 128},
    },

// *******************************************************

// ***** 5th sphere on 4th column, white *****
// ambient material
    {
    {0.05 ,0.05 ,0.05 ,1.0f},


// diffuse material
    {0.5 ,0.5 ,0.5 ,1.0f},


// specular material
    {0.7 ,0.7 ,0.7 ,1.0f},


// shininess
    {0.078125 * 128},
    },

// *******************************************************

// ***** 6th sphere on 4th column, yellow rubber *****
// ambient material
    {
    {0.05 ,0.05 ,0.0 ,1.0f},


// diffuse material
    {0.5 ,0.5 ,0.4 ,1.0f},


// specular material
    {0.7 ,0.7 ,0.04 ,1.0f},


// shininess
    {0.078125 * 128},
    }
};
