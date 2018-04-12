#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(void)
{
    int status;
    PIXEL *pixels = NULL;

    status = bmp_loadfromfile(pixels, "images/colors/red.bmp");

    bmp_printline();
    printf("status: %i\n", status);

    return EXIT_SUCCESS;
}