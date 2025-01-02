#ifndef _NICKEL_TYPES_H
#define _NICKEL_TYPES_H

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
#define NICKEL_X86
#define NICKEL_X86_64
#define NICKEL_64BIT
#elif defined(__i386) || defined(__i386) || defined(i386)
#define NICKEL_X86
#define NICKEL_32BITS
#elif defined(__aarch64__)
#define NICKEL_ARM
#define NICKEL_ARM64
#define NICKEL_64BIT
#elif defined(__arm__)
#define NICKEL_ARM
#define NICKEL_32BIT
#elif defined(__mips__) || defined(mips)
#define NICKEL_MIPS
#endif

typedef char                char8_t;
// typedef short               char16_t;

#ifdef NICKEL_64BIT
typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef unsigned long       size_t;
#else
typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef unsigned int        size_t;
#endif

#define NULL                ((void *)0)

#endif
