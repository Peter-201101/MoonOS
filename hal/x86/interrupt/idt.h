#ifndef IDT_H
#define IDT_H

#include <types.h>

#define IDT_ENTRIES 256

/* IDT gate types */
#define IDT_TASK_GATE       0x5
#define IDT_INTERRUPT_GATE  0xE
#define IDT_TRAP_GATE       0xF

/* Privilege levels */
#define IDT_DPL_KERNEL  0
#define IDT_DPL_USER    3

/* IDT entry flags */
#define IDT_PRESENT     (1 << 7)
#define IDT_GATE_INT32  (IDT_PRESENT | IDT_INTERRUPT_GATE)
#define IDT_GATE_TRAP32 (IDT_PRESENT | IDT_TRAP_GATE)
#define IDT_GATE_USER   (IDT_PRESENT | IDT_INTERRUPT_GATE | (IDT_DPL_USER << 5))

/* IDT entry (8 bytes) */
typedef struct {
    uint16_t offset_low;    /* bits 0-15 of handler address */
    uint16_t selector;      /* code segment selector */
    uint8_t  zero;          /* always 0 */
    uint8_t  type_attr;     /* type + attributes */
    uint16_t offset_high;   /* bits 16-31 of handler address */
} __attribute__((packed)) idt_entry_t;

/* IDT pointer (untuk lidt instruction) */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/* Register struct — di-push oleh ISR stub sebelum panggil handler C */
typedef struct {
    /* Segment registers */
    uint32_t gs, fs, es, ds;
    /* General purpose — di-push oleh pusha */
    uint32_t edi, esi, ebp, esp_dummy;
    uint32_t ebx, edx, ecx, eax;
    /* Interrupt info */
    uint32_t int_no;    /* interrupt number */
    uint32_t err_code;  /* error code (0 kalau tidak ada) */
    /* Di-push otomatis oleh CPU */
    uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed)) registers_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags);

#endif