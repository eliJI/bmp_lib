#include <stdint.h>

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
