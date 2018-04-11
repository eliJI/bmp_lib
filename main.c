#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(void)
{
    int status;
    PIXEL *pixels = NULL;

    status = bmp_loadfromfile(pixels, "image.bmp");

    printf("status: %i\n", status);

    return EXIT_SUCCESS;
}