#ifndef _X86_REG_H
#define _X86_REG_H

#include <types.h>

union rflags {
    uint64_t val;

    struct {
        uint8_t carry : 1;
        uint8_t reserved0 : 1;
        uint8_t parity : 1;
        uint8_t reserved1 : 1;
        uint8_t auxiliary_carry : 1;
        uint8_t reserved2 : 1;
        uint8_t zero : 1;
        uint8_t sign : 1;
        uint8_t trap : 1;
        uint8_t interrupt : 1;
        uint8_t direction : 1;
        uint8_t overflow : 1;
        uint8_t iopl : 2;
        uint8_t nested_task : 1;
        uint8_t reserved3 : 1;
        uint8_t resume : 1;
        uint8_t virt_8086 : 1;
        uint8_t alignment : 1;
        uint8_t virt_intr : 1;
        uint8_t virt_intr_pending : 1;
        uint8_t id : 1;
        uint32_t reserved4 : 11;
        uint32_t reserved5;
    } __attribute__((packed));
};

#endif
