#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "bittype.h"
#include "bmp.h"

/* POINTER MIGHT BE WRONG */
void freecolortable(COLORTABLE **colortable, uint16_t bits)
{
    uint16_t i;
    
    if(colortable == NULL)
        return;

    for(i = 0; i < bits; i++)
        free(colortable[i]);
    free(colortable);
}

COLORTABLE **malloccolortable(uint16_t bits)
{
    unsigned int i, j;
    COLORTABLE **colortable;

    assert(bits > 0);

    if((colortable = (COLORTABLE **) malloc(bits * sizeof(COLORTABLE *))) == NULL)
        return NULL;
    
    for (i = 0; i < bits; i++)
    {
        if ((colortable[i] = (COLORTABLE *) malloc(sizeof(COLORTABLE))) == NULL)
        {
            for (j = 0; j < i; j++)
                free(colortable[j]);
            free(colortable);
            return NULL;
        }
    }

    return colortable;
}

PIXEL **mallocpixel(size_t size)
{
    unsigned int i, j;
    PIXEL **pixel;

    assert(size > 0);

    if((pixel = (PIXEL **) malloc(size * sizeof(PIXEL *))) == NULL)
        return NULL;

    for(i = 0; i < size; i++)
    {
        if((pixel[i] = (PIXEL *) malloc(sizeof(PIXEL))) == NULL)
        {
            for(j = 0; j < i; j++)
                free(pixel[i]);
            free(pixel);

            return NULL;
        }
    }

    return pixel;
}

void freepixel(PIXEL **pixel, size_t size)
{
    unsigned int i;

    assert(size > 0);

    if(pixel == NULL)
        return;
    for(i = 0; i < size; i++)
        free(pixel[i]);
    free(pixel);
}

unsigned int calcpixelsize(const HEADER *header, const INFOHEADER *infoheader);

int bmp_loadfromfile(PIXEL *dest, const char filename[])
{
    FILE *file = NULL;
    HEADER *header = NULL;
    INFOHEADER *infoheader = NULL;
    COLORTABLE **colortable = NULL;
    PIXEL **pixel = NULL;
    unsigned int datasize, pixelcount;
    unsigned short int padding;
    int status;
    unsigned int i, j;
    char endian = 0;
    uint8_t buf[3];

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

    if(infoheader->planes != 1 )
    {
        free(header);
        fclose(file);
        free(infoheader);
        return LOAD_ERR_CORUPTED_HEADER;
    }

    if(infoheader->compression != 0)
    {
        free(header);
        fclose(file);
        free(infoheader);
        return LOAD_ERR_NOT_SUPPORTED; 
    }

    bmp_printinfoheader(infoheader);

    if(infoheader->bits == 1 || infoheader->bits == 4)
    {
        if((colortable = malloccolortable(infoheader->bits)) == NULL)
        {
            free(header);
            fclose(file);
            free(infoheader);
            return status;
        }        
    }

    /* bmp_printtcolortable(colortable); */

    if(infoheader->bits != 24)      /* DURING DEV THIS SHOULD BE CHANGED */
    {
        free(header);
        fclose(file);
        free(infoheader);
        freecolortable(colortable, infoheader->bits);
        return LOAD_ERR_NOT_SUPPORTED;
    }
    
    datasize = calcpixelsize(header, infoheader);
    pixelcount = infoheader->width * infoheader->height;

    padding = (infoheader->width % 4);

    if((pixel = mallocpixel(pixelcount)) == NULL)
    {
        free(header);
        fclose(file);
        free(infoheader);
        freecolortable(colortable, infoheader->bits);
        return status;
    }

    bmp_printline();
    printf("datasize:\t\t<%lu>\n", datasize);
    printf("padding:\t\t<%i>\n", padding);
    bmp_printline();
    for(i = 0; i < infoheader->width * infoheader->height; i++)
    {
        if(padding && !(i % infoheader->width) && i != 0)
        {
            fread(buf, padding, 1, file);
            printf("DISCARDED:\t");
            for(j = 0; j < padding; j++)
                printf("%i ", buf[j]);
            putchar('\n');
        }
        fread(buf, 3, 1, file);
        pixel[i]->red    = buf[2];
        pixel[i]->green  = buf[1];
        pixel[i]->blue   = buf[0];
        printf("%05i ", i);
        bmp_printpixel(pixel[i]);
        /* if((i + 125) % 250 == 0) */
    }


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
        /*printf("LITTLE ENDIAN!!!\n");*/
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
    else if(buf[1] == 'B' && buf[0] == 'M') /* LITTLE ENDIAN */
    {
        /*printf("BIG ENDIAN!!!\n");*/
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
unsigned int calcpixelsize(const HEADER *header, const INFOHEADER *infoheader)
{
    unsigned int datasize;

    assert(header != NULL);
    assert(infoheader != NULL);

    datasize = header->size - BMP_HEADER_LEN - BMP_INFOHEADER_LEN;
    switch (infoheader->bits)
    {
    case 1:
        datasize -= 4 * BMP_BBP_1;
        break;

    case 4:
        datasize -= 4 * BMP_BBP_4;
        break;

    default:
        break;
    }

    return datasize;
}

void bmp_printheader(HEADER *header)
{
    assert(header != NULL);
    
    bmp_printline();
    printf("type:\t\t\t<%lu>\t(\"%c%c\")\n", header->type, header->type[0], header->type[1]);
    printf("size:\t\t\t<%i>\n", header->size);
    printf("offset:\t\t\t<%lu>\n", header->offset);
    
}

void bmp_printinfoheader(INFOHEADER *infoheader)
{
    assert(infoheader != NULL);

    bmp_printline();
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

void bmp_printline(void)
{
    printf("----------------------------------\n");
}