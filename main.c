#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(void)
{
    int status;
    BITMAP *bitmap = NULL;

    status = bmp_loadfromfile(bitmap, "images/mono.bmp");

    bmp_printline();
    printf("status: %i\n", status);
    bmp_printline();
    bmp_printstatus(status);
    bmp_printline();
    bmp_unload(bitmap);
    return EXIT_SUCCESS;
}