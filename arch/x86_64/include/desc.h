#ifndef _X86_DESC_H
#define _X86_DESC_H

#include <types.h>

union gdt_desc_64 {
    uint32_t val[4];

    struct {
        uint16_t limit_15_00;
        uint16_t base_15_00;

        uint8_t base_23_16;
        uint8_t accessed : 1;
        uint8_t read_write : 1;
        uint8_t dir_cfr : 1;
        uint8_t segment_type : 1;
        uint8_t descriptor_type : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
        uint8_t limit_19_16 : 4;
        uint8_t reserved0 : 1;
        uint8_t is_long_mode : 1;
        uint8_t size : 1;
        uint8_t granularity : 1;
        uint8_t base_31_24;

        uint32_t base_63_32;
        uint32_t reserved1;
    } __attribute__((packed));
};

union idt_desc_64 {
    uint32_t val[4];

    struct {
        uint16_t offset_15_00;
        uint16_t target_segment;
        uint8_t stack_table : 3;
        uint8_t reserved0 : 5;
        uint8_t type : 1;
        uint8_t reserved1 : 4;
        uint8_t dpl : 2;
        uint8_t present : 1;
        uint16_t offset_31_16;
        uint32_t offset_63_32;
        uint32_t reserved2;
    } __attribute__((packed));
};

struct desc_entry {
    uint16_t limit;
    void *entry;
} __attribute__((packed));

#endif
