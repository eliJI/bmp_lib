#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED

#include <stdint.h>



typedef struct {
   uint8_t type[2];                 /* Magic identifier            */
   uint32_t size;                       /* File size in bytes          */
   uint8_t reserved1[2], reserved2[2];
   uint32_t offset;                     /* Offset to image data, bytes */
} HEADER;

typedef struct {
   uint32_t size;               /* Header size in bytes      */
   uint32_t width,height;                /* Width and height of image */
   uint16_t planes;       /* Number of colour planes   */
   uint16_t bits;         /* Bits per pixel            */
   uint32_t compression;        /* Compression type          */
   uint32_t imagesize;          /* Image size in bytes       */
   uint32_t xresolution,yresolution;     /* Pixels per meter          */
   uint32_t ncolours;           /* Number of colours         */
   uint32_t importantcolours;   /* Important colours         */
} INFOHEADER;

typedef struct {
    char red;
    char green;
    char blue;
} PIXEL;


int bmp_loadfromfile(PIXEL *dest, char filename[]);

void bmp_printheader(HEADER *header);
void bmp_printinfoheader(INFOHEADER *infoheader);
void bmp_printpixel(PIXEL *pixel);

int bmp_loadheader(HEADER *dest, FILE *file, char *endian);
int bmp_loadinfoheader(INFOHEADER *dest, FILE *file, const char endian);


#define BMP_ID 19778
#define BMP_HEADER_LEN 14
#define BMP_INFOHEADER_LEN 40

#define LOAD_SUCC                   0
#define LOAD_ERR_OPENING            10
#define LOAD_ERR_READING            11
#define LOAD_ERR_CORUPTED_HEADER    12
#define LOAD_ERR_NO_BMP             13
#define LOAD_ERR_NO_ENDIAN_INFO     14
#define LOAD_ERR_ALLOC_ERR          15

#endif