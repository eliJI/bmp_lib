#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "bittype.h"
#include "bmp.h"

int bmp_loadfromfile(PIXEL *dest, char filename[])
{
    FILE *file;
    HEADER *header;
    INFOHEADER *infoheader;
    int status;
    char endian = 0;

    assert(filename != NULL);
    assert(dest == NULL);

    if((header = malloc(sizeof(HEADER))) == NULL)
        return LOAD_ERR_ALLOC_ERR;
        
    if((infoheader = malloc(sizeof(INFOHEADER))) == NULL)
    {
        free(header);
        return LOAD_ERR_ALLOC_ERR;
    }
    
    file = fopen(filename, "rb");

    if(file == NULL)
        return LOAD_ERR_OPENING;

    if((status = bmp_loadheader(header, file, &endian)) != LOAD_SUCC)
    {
        fclose(file);
        free(header);
        free(infoheader);
        return status;
    }

    bmp_printheader(header);    

    if((status = bmp_loadinfoheader(infoheader, file, endian)) != LOAD_SUCC)
    {
        free(header);
        fclose(file);
        free(infoheader);
        return status;
    }

    bmp_printinfoheader(infoheader);

    return LOAD_SUCC; 
}


int bmp_loadheader(HEADER *dest, FILE *file, char *endian)
{
    uint8_t buf[BMP_HEADER_LEN];

    assert(dest != NULL);
    assert(file != NULL);

    if(fread(buf, BMP_HEADER_LEN, 1, file) != 1)
        return LOAD_ERR_READING;
    
    if(buf[0] == 'B' && buf[1] == 'M')    /* BIG ENDIAN */
    {
        printf("LITTLE ENDIAN!!!\n");
        *endian = 'l';
        dest->type[0]       = buf[0];
        dest->type[1]       = buf[1];
        dest->size          = le32_to_cpu(buf + 2);
        dest->reserved1[0]  = buf[6];
        dest->reserved1[1]  = buf[7];
        dest->reserved2[0]  = buf[8];
        dest->reserved2[1]  = buf[9];
        dest->offset        = le32_to_cpu(buf + 10);
        return LOAD_SUCC;

    }
    else if(buf[1] == 'B' && buf[0] == 'M')
    {
        printf("BIG ENDIAN!!!\n");
        *endian = 'b';
        dest->type[0]       = buf[1];
        dest->type[1]       = buf[0];
        dest->size          = be32_to_cpu(buf + 2);
        dest->reserved1[0]  = buf[7];
        dest->reserved1[1]  = buf[6];
        dest->reserved2[0]  = buf[9];
        dest->reserved2[1]  = buf[8];
        dest->offset        = be32_to_cpu(buf + 10);
        return LOAD_SUCC;
    }
    else
        return LOAD_ERR_NO_BMP;
}

int bmp_loadinfoheader(INFOHEADER *dest, FILE *file, const char endian)
{
    uint8_t buf[BMP_INFOHEADER_LEN];

    assert(dest != NULL);
    assert(file != NULL);
    assert(endian == 'l' || endian == 'b');

    if(fread(buf, BMP_INFOHEADER_LEN, 1, file) != 1)
        return LOAD_ERR_READING;

    switch(endian)
    {
        case 'l':
            dest->size              = le32_to_cpu(buf);
            dest->width             = le32_to_cpu(buf + 4);
            dest->height            = le32_to_cpu(buf + 8);
            dest->planes            = le16_to_cpu(buf + 12);
            dest->bits              = le16_to_cpu(buf + 14);
            dest->compression       = le32_to_cpu(buf + 16);
            dest->imagesize         = le32_to_cpu(buf + 20);
            dest->xresolution       = le32_to_cpu(buf + 24);
            dest->yresolution       = le32_to_cpu(buf + 28);
            dest->ncolours          = le32_to_cpu(buf + 32);
            dest->importantcolours  = le32_to_cpu(buf + 36);
            return LOAD_SUCC;

        case 'b':
            dest->size              = be32_to_cpu(buf);
            dest->width             = be32_to_cpu(buf + 4);
            dest->height            = be32_to_cpu(buf + 8);
            dest->planes            = be16_to_cpu(buf + 12);
            dest->bits              = be16_to_cpu(buf + 14);
            dest->compression       = be32_to_cpu(buf + 16);
            dest->imagesize         = be32_to_cpu(buf + 20);
            dest->xresolution       = be32_to_cpu(buf + 24);
            dest->yresolution       = be32_to_cpu(buf + 28);
            dest->ncolours          = be32_to_cpu(buf + 32);
            dest->importantcolours  = be32_to_cpu(buf + 36);
            return LOAD_SUCC;

        default:
            return LOAD_ERR_CORUPTED_HEADER;
    } 
}

void bmp_printheader(HEADER *header)
{
    assert(header != NULL);
    
    printf("----------------------------------");
    printf("BMP HEADER\n");
    printf("type:\t\t\t<%lu>\t(\"%c%c\")\n", header->type, header->type[0], header->type[1]);
    printf("size:\t\t\t<%i>\n", header->size);
    printf("offset:\t\t\t<%lu>\n", header->offset);
    
}

void bmp_printinfoheader(INFOHEADER *infoheader)
{
    assert(infoheader != NULL);

    printf("size:\t\t\t<%i>\n", infoheader->size);
    printf("width:\t\t\t<%i>\n", infoheader->width);
    printf("height:\t\t\t<%i>\n", infoheader->height);
    printf("planes:\t\t\t<%i>\n", infoheader->planes);
    printf("bits:\t\t\t<%i>\n", infoheader->bits);
    printf("compression:\t\t<%i>\n", infoheader->compression);
    printf("imagesize:\t\t<%i>\n", infoheader->imagesize);
    printf("xresolution:\t\t<%i>\n", infoheader->xresolution);
    printf("yresolution:\t\t<%i>\n", infoheader->yresolution);
    printf("ncolours:\t\t<%i>\n", infoheader->ncolours);
    printf("importantcolours:\t<%i>\n", infoheader->importantcolours);
}

void bmp_printpixel(PIXEL *pixel)
{
    assert(pixel != NULL);

    printf("RGB:\t%i, %i, %i\n", pixel->red, pixel->green, pixel->blue);
}
