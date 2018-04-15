#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "bittype.h"

/* CHAR_BIT == 8 assumed */
uint16_t le16_to_cpu(const uint8_t *buf)
{
   return ((uint16_t)buf[0]) | (((uint16_t)buf[1]) << 8);
}
uint16_t be16_to_cpu(const uint8_t *buf)
{
   return ((uint16_t)buf[1]) | (((uint16_t)buf[0]) << 8);
}

uint32_t le32_to_cpu(const uint8_t *buf)
{
   return ((uint32_t)buf[0]) 
        | (((uint32_t)buf[1]) << 8)
        | (((uint32_t)buf[2]) << 16)
        | (((uint32_t)buf[3]) << 24)
        ;
}

uint32_t be32_to_cpu(const uint8_t *buf)
{
   return ((uint32_t)buf[3]) 
        | (((uint32_t)buf[2]) << 8)
        | (((uint32_t)buf[1]) << 16)
        | (((uint32_t)buf[0]) << 24)
        ;
}

void cpu_to_le16(uint8_t *buf, uint16_t val)
{
   buf[0] = (val & 0x00FF);
   buf[1] = (val & 0xFF00) >> 8;
}
void cpu_to_be16(uint8_t *buf, uint16_t val)
{
   buf[0] = (val & 0xFF00) >> 8;
   buf[1] = (val & 0x00FF);
}

void cpu_to_le32(uint8_t *buf, uint32_t val)
{
    buf[0] = (val & 0x000000FF);
    buf[1] = (val & 0x0000FF00) >> 8;
    buf[2] = (val & 0x00FF0000) >> 16;
    buf[3] = (val & 0xFF000000) >> 24;
}
void cpu_to_be32(uint8_t *buf, uint32_t val)
{
    buf[0] = (val & 0xFF000000) >> 24;
    buf[1] = (val & 0x00FF0000) >> 16;
    buf[2] = (val & 0x0000FF00) >> 8;
    buf[3] = (val & 0x000000FF);
}

void byte_to_bits(uint8_t *buf)
{
    uint8_t orig;

    assert(buf != NULL);

    orig = buf[0];

    buf[0] = (orig & 0x01) >> 0;
    buf[1] = (orig & 0x02) >> 1;
    buf[2] = (orig & 0x04) >> 2;
    buf[3] = (orig & 0x08) >> 3;
    buf[4] = (orig & 0x10) >> 4;
    buf[5] = (orig & 0x20) >> 5;
    buf[6] = (orig & 0x40) >> 6;
    buf[7] = (orig & 0x80) >> 7;

}
