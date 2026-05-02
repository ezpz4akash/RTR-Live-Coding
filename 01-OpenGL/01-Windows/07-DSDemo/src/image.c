#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

PUCHAR GetImageData(PCHAR fileName, PINT width, PINT height, PINT nrChannels){
    unsigned char* imageData = stbi_load(fileName, width, height, nrChannels, STBI_rgb_alpha);
    if (imageData == NULL) {
        fprintf(gpFile, TEXT("Loading Image Failed For File: %s\n"), fileName);
    }
    return imageData;
}

void FreeImageData(PUCHAR imageData){
    stbi_image_free(imageData);
}