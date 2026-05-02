#pragma once
#include "common.h"

extern FILE* gpFile;

PUCHAR GetImageData(PCHAR fileName, PINT width, PINT height, PINT nrChannels);
void FreeImageData(PUCHAR);