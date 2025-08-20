#pragma once
#include <GL/glew.h> 
#include <GL/gl.h>
#include <GL/glx.h>

#define NO_OF_SPHERES 24

struct Material {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess;
};

struct Material materials[24] = {
    // 1st sphere on 1st column, emerald
    {{0.0215f, 0.1745f, 0.0215f, 1.0f}, {0.07568f, 0.61424f, 0.07568f, 1.0f}, {0.633f, 0.727811f, 0.633f, 1.0f}, 0.6f * 128},
    // 2nd sphere on 1st column, jade
    {{0.135f, 0.2225f, 0.1575f, 1.0f}, {0.54f, 0.89f, 0.63f, 1.0f}, {0.316228f, 0.316228f, 0.316228f, 1.0f}, 0.1f * 128},
    // 3rd sphere on 1st column, obsidian
    {{0.05375f, 0.05f, 0.06625f, 1.0f}, {0.18275f, 0.17f, 0.22525f, 1.0f}, {0.332741f, 0.328634f, 0.346435f, 1.0f}, 0.3f * 128},
    // 4th sphere on 1st column, pearl
    {{0.25f, 0.20725f, 0.20725f, 1.0f}, {1.0f, 0.829f, 0.829f, 1.0f}, {0.296648f, 0.296648f, 0.296648f, 1.0f}, 0.088f * 128},
    // 5th sphere on 1st column, ruby
    {{0.1745f, 0.01175f, 0.01175f, 1.0f}, {0.61424f, 0.04136f, 0.04136f, 1.0f}, {0.727811f, 0.626959f, 0.626959f, 1.0f}, 0.6f * 128},
    // 6th sphere on 1st column, turquoise
    {{0.1f, 0.18725f, 0.1745f, 1.0f}, {0.396f, 0.74151f, 0.69102f, 1.0f}, {0.297254f, 0.30829f, 0.306678f, 1.0f}, 0.1f * 128},
    // 1st sphere on 2nd column, brass
    {{0.329412f, 0.223529f, 0.027451f, 1.0f}, {0.780392f, 0.568627f, 0.113725f, 1.0f}, {0.992157f, 0.941176f, 0.807843f, 1.0f}, 0.21794872f * 128},
    // 2nd sphere on 2nd column, bronze
    {{0.2125f, 0.1275f, 0.054f, 1.0f}, {0.714f, 0.4284f, 0.18144f, 1.0f}, {0.393548f, 0.271906f, 0.166721f, 1.0f}, 0.2f * 128},
    // 3rd sphere on 2nd column, chrome
    {{0.25f, 0.25f, 0.25f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, {0.774597f, 0.774597f, 0.774597f, 1.0f}, 0.6f * 128},
    // 4th sphere on 2nd column, copper
    {{0.19125f, 0.0735f, 0.0225f, 1.0f}, {0.7038f, 0.27048f, 0.0828f, 1.0f}, {0.256777f, 0.137622f, 0.086014f, 1.0f}, 0.1f * 128},
    // 5th sphere on 2nd column, gold
    {{0.24725f, 0.1995f, 0.0745f, 1.0f}, {0.75164f, 0.60648f, 0.22648f, 1.0f}, {0.628281f, 0.555802f, 0.366065f, 1.0f}, 0.4f * 128},
    // 6th sphere on 2nd column, silver
    {{0.19225f, 0.19225f, 0.19225f, 1.0f}, {0.50754f, 0.50754f, 0.50754f, 1.0f}, {0.508273f, 0.508273f, 0.508273f, 1.0f}, 0.4f * 128},
    // 1st sphere on 3rd column, black
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.01f, 0.01f, 0.01f, 1.0f}, {0.50f, 0.50f, 0.50f, 1.0f}, 0.25f * 128},
    // 2nd sphere on 3rd column, cyan
    {{0.0f, 0.1f, 0.06f, 1.0f}, {0.0f, 0.50980392f, 0.50980392f, 1.0f}, {0.50980392f, 0.50980392f, 0.50980392f, 1.0f}, 0.25 * 128},
    // 3rd sphere on 3rd column, green
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.1f, 0.35f, 0.1f, 1.0f}, {0.45f, 0.55f, 0.45f, 1.0f}, 0.25 * 128},
    // 4th sphere on 3rd column, red
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.0f, 0.0f, 1.0f}, {0.7f, 0.6f, 0.6f, 1.0f}, 0.25 * 128},
    // 5th sphere on 3rd column, white
    {{0.0, 0.0, 0.0, 1.0f}, {0.55f, 0.55f, 0.55f, 1.0f}, {0.70f, 0.70f, 0.70f, 1.0f}, 0.25 * 128},
    // 6th sphere on 3rd column, yellow
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f, 0.0f, 1.0f}, {0.60f, 0.60f, 0.50f, 1.0f}, 0.25f * 128},
    // 1st sphere on 4th column, black
    {{0.02f, 0.02f, 0.02f, 1.0f}, {0.01f, 0.01f, 0.01f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, 0.078125f * 128},
    // 2nd sphere on 4th column, cyan
    {{0.0f, 0.05f, 0.05f, 1.0f}, {0.4f, 0.5f, 0.5f, 1.0f}, {0.04f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128},
    // 3rd sphere on 4th column, green
    {{0.0f, 0.05f, 0.0f, 1.0f}, {0.4f, 0.5f, 0.4f, 1.0f}, {0.04f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128},
    // 4th sphere on 4th column, red
    {{0.05f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.4f, 0.4f, 1.0f}, {0.7f, 0.04f, 0.04f, 1.0f}, 0.078125f * 128},
    // 5th sphere on 4th column, white
    {{0.05f, 0.05f, 0.05f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}, {0.7f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128},
    // 6th sphere on 4th column, yellow
    {{0.05f, 0.05f, 0.0, 1.0f}, {0.5f, 0.5f, 0.4f, 1.0f}, {0.7f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128}
};

GLfloat sphereTranslation[24][4] = {
    {1.5f, 14.0f, 0.0f},
    {1.5f, 11.5f, 0.0f},
    {1.5f, 9.0f, 0.0f},
    {1.5f, 6.5f, 0.0f},
    {1.5f, 4.0f, 0.0f},
    {1.5f, 1.5f, 0.0f},
    {7.5f, 14.0f, 0.0f},
    {7.5f, 11.5f, 0.0f},
    {7.5f, 9.0f, 0.0f},
    {7.5f, 6.5f, 0.0f},
    {7.5f, 4.0f, 0.0f},
    {7.5f, 1.5f, 0.0f},
    {13.5f, 14.0f, 0.0f},
    {13.5f, 11.5f, 0.0f},
    {13.5f, 9.0f, 0.0f},
    {13.5f, 6.5f, 0.0f},
    {13.5f, 4.0f, 0.0f},
    {13.5f, 1.5f, 0.0f},
    {19.5f, 14.0f, 0.0f},
    {19.5f, 11.5f, 0.0f},
    {19.5f, 9.0f, 0.0f},
    {19.5f, 6.5f, 0.0f},
    {19.5f, 4.0f, 0.0f},
    {19.5f, 1.5f, 0.0f},
};