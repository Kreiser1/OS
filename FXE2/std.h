#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"

#define packed __attribute__((__packed__))
#define aligned(alignment) __attribute__((aligned(alignment)))
#define align(value, alignment) ((uint32_t)(value) / (uint32_t)(alignment) * (uint32_t)(alignment))
#define align_padded(value, alignment) ((uint32_t)(value) % (uint32_t)(alignment) ? (align((uint32_t)(value), (alignment)) + (uint32_t)(alignment)) : align((value), (alignment)))
#define aligned_count(value, alignment) ((uint32_t)(value) / (uint32_t)(alignment))
#define aligned_count_padded(value, alignment) ((uint32_t)(value) % (uint32_t)(alignment) ? (aligned_count((value), (alignment)) + 1) : aligned_count((value), (alignment)))
#define lengthof(array) (sizeof((array)) / sizeof((array)[0]))
#define bitsof(value) (sizeof((value)) * 8)
#define asm __asm__ __volatile__