#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "bittype.h"
#include "bmp.h"

void initbitmap(BITMAP *bitmap)
{
    bitmap->header       = NULL;
    bitmap->infoheader   = NULL;
    bitmap->colortable   = NULL;
    bitmap->pixel        = NULL;
}

void freecolortable(COLORTABLE **colortable, uint16_t bits)
{
    uint16_t i;
    
    if(colortable == NULL)
        return;

    for(i = 0; i < bits; i++)
        free(colortable[i]);
    free(colortable);
}

COLORTABLE **malloccolortable(size_t num_of_elem)
{
    unsigned int i, j;
    COLORTABLE **colortable;

    assert(num_of_elem > 0);

    if ((colortable = (COLORTABLE **)malloc(num_of_elem * sizeof(COLORTABLE *))) == NULL)
        return NULL;

    for (i = 0; i < num_of_elem; i++)
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

int bmp_loadfromfile(BITMAP *dest, const char filename[])
{
    FILE *file = NULL;
    BITMAP *bitmap = NULL;
    char endian = 0;
    int status;
    unsigned int i, j;
    unsigned short int padding;
    unsigned int datasize, pixelcount;

    uint8_t buf[8];

    assert(filename != NULL);
    assert(dest == NULL);

    bmp_printline();
    printf("opening file: \"%s\"\n", filename);

    if((file = fopen(filename, "rb")) == NULL)
        return LOAD_ERR_OPENING;
    
    if((bitmap = malloc(sizeof(BITMAP))) == NULL)
    {
        fclose(file);
        return LOAD_ERR_ALLOC_ERR;
    }
    
    if((bitmap->header = malloc(sizeof(HEADER))) == NULL)
    {
        bmp_unload(bitmap);
        fclose(file);
        return LOAD_ERR_ALLOC_ERR;
    }
        
    if((bitmap->infoheader = malloc(sizeof(INFOHEADER))) == NULL)
    {
        bmp_unload(bitmap);
        fclose(file);
        return LOAD_ERR_ALLOC_ERR;
    }

    if((status = bmp_loadheader(bitmap->header, file, &endian)) != LOAD_SUCC)
    {
        bmp_unload(bitmap);
        fclose(file);
        return status;
    }

    bmp_printheader(bitmap->header);    

    if((status = bmp_loadinfoheader(bitmap->infoheader, file, endian)) != LOAD_SUCC)
    {
        bmp_unload(bitmap);
        fclose(file);
        return status;
    }

    if(bitmap->infoheader->planes != 1 )
    {
        bmp_unload(bitmap);
        fclose(file);
        return LOAD_ERR_CORUPTED_HEADER;
    }

    if(bitmap->infoheader->compression != 0)
    {
        bmp_unload(bitmap);
        fclose(file);
        return LOAD_ERR_NOT_SUPPORTED; 
    }

    bmp_printinfoheader(bitmap->infoheader);

    if(bitmap->infoheader->bits == 1)
    {
        if(((bitmap->colortable = malloccolortable(2)) == NULL) || bmp_loadcolortable(bitmap->colortable, 2, file) != LOAD_SUCC)
        {
            bmp_unload(bitmap);
            fclose(file);
            return status;
        }

    }
    else if(bitmap->infoheader->bits == 4)
    {
        if (((bitmap->colortable = malloccolortable(16)) == NULL) || bmp_loadcolortable(bitmap->colortable, 16, file) != LOAD_SUCC)
        {
            bmp_unload(bitmap);
            fclose(file);
            return status;
        }
    }

    if(bitmap->infoheader->bits != 24 && bitmap->infoheader->bits != 1)
    {
        bmp_unload(bitmap);
        fclose(file);
        return LOAD_ERR_NOT_SUPPORTED;
    }
    
    datasize = calcpixelsize(bitmap->header, bitmap->infoheader);
    pixelcount = bitmap->infoheader->width * bitmap->infoheader->height;

    padding = (bitmap->infoheader->width % 4);

    if((bitmap->pixel = mallocpixel(pixelcount)) == NULL)
    {
        bmp_unload(bitmap);
        fclose(file);
        return status;
    }

    bmp_printline();
    printf("datasize:\t\t<%lu>\n", datasize);
    printf("padding:\t\t<%i>\n", padding);
    bmp_printline();

    switch(bitmap->infoheader->bits)
    {
        case 1:
            printf("COLORTABLES \n");
            printf("<0> ");
            bmp_printcolortable(bitmap->colortable[0]);
            printf("<1> ");
            bmp_printcolortable(bitmap->colortable[1]);
            bmp_printline();

            for(i = 0; i < (pixelcount + 1) / 8; i++)
            {
                if(fread(buf, 1, 1, file) != 1)
                {
                    bmp_unload(bitmap);
                    fclose(file);
                    return LOAD_ERR_READING;
                }

                /* if(i % 2 == 0)
                    putchar(' ');
                if(i % 16 == 0)
                    putchar('\n');
                printf("%02x", buf[0]); */
                

                byte_to_bit(buf);
                for(j = 0; j < 8; j++)
                {
                    printf("%i ", buf[j]);
                }
                putchar('\n');
            }
            break;

        case 24:
            for (i = 0; i < pixelcount; i++)
            {
                if (padding && !(i % bitmap->infoheader->width) && i != 0)
                {
                    if(fread(buf, padding, 1, file) != 1)
                    {
                        bmp_unload(bitmap);
                        fclose(file);
                        return LOAD_ERR_READING;
                    }
                }
                if(fread(buf, 3, 1, file) != 1)
                {
                    bmp_unload(bitmap);
                    fclose(file);
                    return LOAD_ERR_READING;
                }
                bitmap->pixel[i]->red = buf[2];
                bitmap->pixel[i]->green = buf[1];
                bitmap->pixel[i]->blue = buf[0];
            }
            break;
    }
    

    dest = bitmap;
    return LOAD_SUCC; 
}

void bmp_unload(BITMAP *bitmap)
{
    if(bitmap == NULL)
        return;
        
    if(bitmap->header != NULL)
    {
        if(bitmap->infoheader != NULL)
        {
            freecolortable(bitmap->colortable, bitmap->infoheader->bits);
            freepixel(bitmap->pixel, bitmap->infoheader->width * bitmap->infoheader->height);
            free(bitmap->infoheader);
        }
        free(bitmap->header);
    }
    free(bitmap);
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

int bmp_loadcolortable(COLORTABLE **colortable, size_t num, FILE *file)
{
    unsigned int i;
    uint8_t buf[4];

    assert(colortable != NULL);
    assert(num > 0);

    for(i = 0; i < num; i++)
    {
        if(fread(buf, 4, 1, file) != 1)
            return LOAD_ERR_READING;
        colortable[i]->red = buf[2];
        colortable[i]->green = buf[1];
        colortable[i]->blue = buf[0];
    }

    return LOAD_SUCC;
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

void bmp_map_colortable_to_pixel(PIXEL *pixel, COLORTABLE *colortable)
{
    assert(pixel != NULL);
    assert(colortable != NULL);
    
    pixel->blue     = colortable->blue;
    pixel->green    = colortable->green;
    pixel->red      = colortable->red;
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

void bmp_printcolortable(COLORTABLE *colortable)
{
    assert(colortable != NULL);

    printf("RGB:\t%3i, %3i, %3i\n", colortable->red, colortable->green, colortable->blue);
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

void bmp_printstatus(int status)
{
    switch(status)
    {
        case LOAD_SUCC:
            printf("loading successfull! <LOAD_SUCC>\n");
            break;

        case LOAD_ERR_OPENING:
            printf("error opening file! <LOAD_ERR_OPENING>\n");
            break;

        case LOAD_ERR_READING:
            printf("error reading file! <LOAD_ERR_READING>\n");
            break;

        case LOAD_ERR_CORUPTED_HEADER:
            printf("the header of the file is corrupted! <LOAD_ERR_CORUPTED_HEADER>\n");
            break;

        case LOAD_ERR_NO_BMP:
            printf("the specified file is not a bmp! <LOAD_ERR_NO_BMP>\n");
            break;

        case LOAD_ERR_NO_ENDIAN_INFO:
            printf("couldn't find endian information! <LOAD_ERR_NO_ENDIAN_INFO>\n");
            break;

        case LOAD_ERR_ALLOC_ERR:
            printf("an error occured during memory allocation! <LOAD_ERR_ALLOC_ERR>\n");
            break;

        case LOAD_ERR_NOT_SUPPORTED:
            printf("this filetype isn't supported (yet)! <LOAD_ERR_NOT_SUPPORTED>\n");
            break;

        case LOAD_ERR_TMPFILE:
            printf("error during creation of a temporary file! <LOAD_ERR_TMPFILE>\n");
            break;

        default:
            printf("unknown status!");
            break;
    }
}