#include "stdio.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef char char8;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint64_t u64;
typedef int sint32;

typedef intptr_t INTPTR;
typedef uintptr_t UINTPTR;

#define DEBUG_ICE_BASE 0

static inline uint64_t ioread64(uintptr_t addr)
{
    uint64_t val = *(volatile u64 *) addr;
    #if DEBUG_ICE_BASE
    printf("icebase: Reading from register %lx: %lx\n", addr, val);
    #endif
    return val;
}

static inline uint32_t ioread32(uintptr_t addr)
{
    uint32_t val = *(volatile u32 *) addr;
    #if DEBUG_ICE_BASE
    printf("icebase: Reading from register %x: %x\n", addr, val);
    #endif
    return val;
}

static inline uint8_t ioread8(uintptr_t addr)
{
    uint8_t val = *(volatile u8 *) addr;
    #if DEBUG_ICE_BASE
    printf("icebase: Reading from register %x: %x\n", addr, val);
    #endif
    return val;
}

static inline uint16_t ioread16(uintptr_t addr)
{
    uint16_t val = *(volatile u16 *) addr;
    #if DEBUG_ICE_BASE
    printf("icebase: Reading from register %x: %x\n", addr, val);
    #endif
    return val;
}

static inline void iowrite8(uint8_t data, uintptr_t addr)
{
    #if DEBUG_ICE_BASE
    printf("icebase: Writing to register %x: %x\n", (unsigned int) addr, data);
    #endif
    *(volatile u8 *) addr = data;
}

static inline void iowrite32(uint32_t data, uintptr_t addr)
{
    #if DEBUG_ICE_BASE
    printf("icebase: Writing to register %x: %lx\n", (unsigned int) addr, data);
    #endif
    *(volatile u32 *) addr = data;
}

static inline void iowrite64(uint64_t data, uintptr_t addr)
{
    #if DEBUG_ICE_BASE
    printf("icebase: 64-bit write to register %x: %llx\n", (unsigned int) addr, data);
    #endif
    // MUST write to lower address first
    iowrite32(data & 0xffffffffL, addr);
    iowrite32((data >> 32), addr+4);
}
