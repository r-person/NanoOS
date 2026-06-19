// NanoKernel v0.1.0
// Written by RPerson

#include <stdint.h>
#define PIC1         0x20
#define PIC1_DATA    0x21
#define PIC2         0xA0
#define PIC2_DATA    0xA1

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void __systemcall();
extern void __intersystemcall();
extern void __uisystemcall();
extern void __filesystemsystemcall();
extern void gdt_flush();
extern void enable_paging();

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

struct idt_entry{
	uint16_t base_low;
	uint16_t segment_selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t base_high;
} __attribute__((packed));
struct idt_ptr{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

const char scancode_to_ascii[128] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x1E] = 'a',
    [0x30] = 'b',
    [0x2E] = 'c',
    [0x20] = 'd',
    [0x12] = 'e',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x17] = 'i',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x32] = 'm',
    [0x31] = 'n',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x10] = 'q',
    [0x13] = 'r',
    [0x1F] = 's',
    [0x14] = 't',
    [0x16] = 'u',
    [0x2F] = 'v',
    [0x11] = 'w',
    [0x2D] = 'x',
    [0x15] = 'y',
    [0x2C] = 'z',
	
	[0x1C] = '\n',
	[0x39] = ' '
};

void *isr[48] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39,
    isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
};

volatile uint64_t ticks_count = 0;
uint8_t sc;

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
} syscall_result_t;

#define syscall(n, a, b, c, d)                    \
({                                                \
    syscall_result_t r;                           \
    uint32_t _eax = (a);                          \
    uint32_t _ebx = (b);                          \
    uint32_t _ecx = (c);                          \
    uint32_t _edx = (d);                          \
                                                  \
    asm volatile (                                \
        "int $" #n                                \
        : "+a"(_eax), "+b"(_ebx), "+c"(_ecx)      \
        : "d"(_edx)                               \
        : "memory"                                \
    );                                            \
                                                  \
    r.eax = _eax;                                 \
    r.ebx = _ebx;                                 \
    r.ecx = _ecx;                                 \
    r;                                            \
})

inline void outb(uint16_t port, uint8_t val){
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline uint8_t inb(uint16_t port){
	uint8_t ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;

    asm volatile (
        "inw %1, %0"
        : "=a"(result)
        : "Nd"(port)
    );

    return result;
}

inline void cli(){
	__asm__ volatile ("cli");
}

inline void sti(){
	__asm__ volatile ("sti");
}

inline void println(const char* str){
	syscall(0x82, 0x03, (uint32_t)str, 0x00, 0x00);
}

// Reads sectors from legacy ATA (PATA) 
void ata_read_sectors(uint32_t lba, uint8_t sectors_amount, uint8_t *buffer){
    while (inb(0x1F7) & 0x80);

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sectors_amount);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    while (inb(0x1F7) & 0x80);

    for (int i = 0; i < 256 * sectors_amount; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }
}

void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                  uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_init()
{
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);

    gdt_set_gate(1,
        0x00000000,
        0xFFFFFFFF,
        0x9A,
        0xCF
    );

    gdt_set_gate(2,
        0x00000000,
        0xFFFFFFFF,
        0x92,
        0xCF
    );

    asm volatile ("lgdt (%0)" : : "r" (&gp));
}

static void idt_set_gate(uint8_t num, uint32_t base, uint8_t flags){
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	idt[num].segment_selector = 0x08;
	idt[num].reserved = 0;
	idt[num].flags = flags;
}

static inline void idt_load(struct idt_ptr* ptr)
{
    asm volatile("lidt (%0)" : : "r"(ptr));
}

void idt_init(){
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = (uint32_t)idt;
	idt_load(&idtp);
}

inline void pic_remap(int offset1, int offset2)
{
    uint8_t a1, a2;
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    outb(PIC1, 0x11);
    outb(PIC2, 0x11);

    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
	
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

inline void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2, 0x20);

    outb(PIC1, 0x20);
}

void isr_handler(int int_num){
	if (int_num < 32){
		switch (int_num){
			case 0x00:
				println("Divide by 0 fault. Not handled by the correct NanoOS version.");
				break;
			case 0x01:
				println("Debug trap. Not handled by the correct NanoOS version.");
				break;
			case 0x02:
				println("Non-maskable interrupt. Not handled by the correct NanoOS version.");
				break;
			case 0x03:
				println("Breakpoint trap. Not handled by the correct NanoOS version.");
				break;
			case 0x04:
				println("Overflow trap. Not handled by the correct NanoOS version.");
				break;
			case 0x05:
				println("Out of bounds fault. Not handled by the correct NanoOS version.");
				break;
			case 0x06:
				println("Invaild opcode fault. Not handled by the correct NanoOS version.");
				break;
			case 0x07:
				println("Device not available fault. Caused by a missing FPU. Not handled by the correct NanoOS version.");
				break;
			case 0x0A:
				println("Invaild TSS fault. Not handled by the correct NanoOS version.");
				break;
			case 0x0B:
				println("Sgment not present fault. Not handled by the correct NanoOS version.");
				break;
			case 0x0C:
				println("Stack segment fault. Not handled by the correct NanoOS version.");
				break;
			case 0x0D:
				println("General protection fault. Not handled by the correct NanoOS version.");
				break;
			case 0x0E:
				println("Page fault. Not handled by the correct NanoOS version.");
				break;
			case 0x10:
				println("Floating point fault. Not handled by the correct NanoOS version.");
				break;
			case 0x11:
				println("Alignment check fault. Not handled by the correct NanoOS version.");
				break;
			case 0x12:
				println("Machine check exception. Not handled by the correct NanoOS version.");
				break;
			case 0x13:
				println("SIMD floating point fault. Not handled by the correct NanoOS version.");
				break;
			case 0x14:
				println("Virtualization fault. Not handled by the correct NanoOS version.");
				break;
			case 0x15:
				println("Control protection fault. Not handled by the correct NanoOS version.");
				break;
			case 0x1C:
				println("Hypervisor injection fault. Not handled by the correct NanoOS version.");
				break;
			case 0x1D:
				println("VMM compunication fault. Not handled by the correct NanoOS version.");
				break;
			case 0x1E:
				println("Security fault. Not handled by the correct NanoOS version.");
				break;
			default:
				println("Unknown CPU exception.");
				break;
		}
		__asm__ volatile ("hlt");
	} else {
		int_num -= 32;
		switch (int_num){
			case 0x00:
				ticks_count++;
				syscall(0x82, 0x01, 0x00, 0x00, 0x00);
				break;
			case 0x01:
				sc = inb(0x60);
				if (!(sc & 0x80)){
					char c = scancode_to_ascii[sc];
					syscall(0x82, 0x02, (uint32_t)c, 0x00, 0x00);
				}
				break;
			default:
				break;
		}
		pic_send_eoi((uint8_t)int_num);
	}
}

void systemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (call_number){
		case 0x00:
			r->eax = 0;
			r->ebx = 1;
			r->ecx = 0;
			break;
		case 0x01:
			cli();
			println((const char*)arg1);
			break;
		default:
			break;
	}
}
void intersystemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (call_number){
		case 0x00:
			isr8();
			break;
		case 0x01:
			ata_read_sectors(arg1, (uint8_t)arg2, (uint8_t*)arg3);
		default:
			break;
	}
}

void kernel_main(void){
	// Sets up interrupts
	gdt_init();
	gdt_flush();
	pic_remap(0x20, 0x28);
	for (int i = 0; i < 48; i++) {
		idt_set_gate(i, (uint32_t)isr[i], 0x8E);
	}
	idt_set_gate(0x80, (uint32_t)__systemcall, 0xEE);
	idt_set_gate(0x81, (uint32_t)__intersystemcall, 0x8E);
	idt_set_gate(0x82, (uint32_t)__uisystemcall, 0x8E);
	idt_set_gate(0x83, (uint32_t)__filesystemsystemcall, 0x8E);
	idt_init();
	// Sets up paging
	for (int i = 0; i < 1024; i++){
		page_directory[i] = 0;
	}
	for (int i = 0; i < 1024; i++) {
		first_page_table[i] = (i * 0x1000) | 3;
	}
	page_directory[0] = ((uint32_t)first_page_table) | 3;
	// Sets up other things
	syscall(0x82, 0x00, 0x00, 0x00, 0x00);
	syscall(0x83, 0x00, 0x00, 0x00, 0x00);
	asm volatile("sti");
}
