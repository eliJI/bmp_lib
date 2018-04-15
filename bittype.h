#ifndef BITTYPE_H_INCLUDED
#define BITTYPE_H_INCLUDED

#include <stdint.h>

/* CHAR_BIT == 8 assumed */
uint16_t le16_to_cpu(const uint8_t *buf);
uint16_t be16_to_cpu(const uint8_t *buf);

uint32_t le32_to_cpu(const uint8_t *buf);
uint32_t be32_to_cpu(const uint8_t *buf);

void cpu_to_le16(uint8_t *buf, uint16_t val);
void cpu_to_be16(uint8_t *buf, uint16_t val);

void cpu_to_le32(uint8_t *buf, uint32_t val);
void cpu_to_be32(uint8_t *buf, uint32_t val);

void byte_to_bits(uint8_t *buf);

#endif