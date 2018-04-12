#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(void)
{
    int status;
    BITMAP *bitmap = NULL;

    status = bmp_loadfromfile(bitmap, "images/colors/red.bmp");

    bmp_printline();
    printf("status: %i\n", status);

    return EXIT_SUCCESS;
}