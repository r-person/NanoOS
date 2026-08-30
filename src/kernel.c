// NanoKernel
// Written by RPerson

#include "stdint.h"
#include "stddef.h"
#include "string.h"

#define PIC1         0x20
#define PIC1_DATA    0x21
#define PIC2         0xA0
#define PIC2_DATA    0xA1

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   (0x18 | 0x03) // 0x1B
#define USER_DS   (0x20 | 0x03) // 0x23

#define NUMBER_OF_PAGE_TABLES 0x04
#define HZ 250
#define HEAP_SIZE 0x800000
#define MEMORY_MAP_MAX_SIZE 0x100
#define MAX_OPEN_FILES 0x03

#define TRUE 0x01
#define FALSE 0x00
#define KERNEL_BASE 0xC0000000
#define STDIN_FILENO 0x00
#define STDOUT_FILENO 0x01
#define STDERR_FILENO 0x02
#define FILE_READ_MODE 0x00
#define FILE_WRITE_MODE 0x01

#define VirtualToPhysical(x) ((uint32_t)(x) - KERNEL_BASE)
#define PhysicalToVirtual(x) ((void *)((uint32_t)(x) + KERNEL_BASE))

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
extern void __linux_syscall();
extern void gdt_flush();
extern void enable_paging();
extern void thread_jmp();
extern void __hlt();

extern uint32_t stack_bottom[0x4000];
extern uint32_t stack_top;
extern uint32_t kernel_page_tables[0x0400 * NUMBER_OF_PAGE_TABLES];
extern uint32_t page_directory[0x0400];

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

static struct idt_entry{
	uint16_t base_low;
	uint16_t segment_selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t base_high;
} __attribute__((packed));
static struct idt_ptr{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
static struct far_jmp{
	uint32_t address;
	uint16_t segment_selector;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtp;

static struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

static struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss_entry
{
    uint32_t prev_tss;

    uint32_t esp0;
    uint32_t ss0;

    uint32_t esp1;
    uint32_t ss1;

    uint32_t esp2;
    uint32_t ss2;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;

    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t ldt;

    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

struct multiboot_info {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms[4];
	uint32_t mmap_length;
	uint32_t mmap_addr;
};

struct multiboot_memory_map {
    uint32_t size;

    uint64_t addr;
    uint64_t len;

    uint32_t type;
};

struct Block {
    size_t size;
    uint8_t free;
    struct Block *next;
} __attribute__((packed));

struct open_file{
	uint32_t descriptor;
	uint32_t current_cluster;
	uint16_t current_pointer;
	uint8_t mode;
};

struct process{
	struct open_file files[MAX_OPEN_FILES];
};

static const char scancode_to_ascii[128] = {
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
	[0x39] = ' ',
	[0x35] = '\\',
	[0x4E] = '+',
	[0x4A] = '-',
	[0x0C] = '_',
	[0x0C] = '=',
	[0x34] = '.'
};

static void *isr[48] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39,
    isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
};

uint32_t current_process = NULL;

static uint32_t tick_count = 0x00;
static uint8_t elf_buffer[0x1000];
static uint32_t userspace_page_tables[0x0400 * NUMBER_OF_PAGE_TABLES] __attribute__((aligned(0x0400)));
static const char *ELF_err = "Incorrect or incomptiable ELF. \n";
static struct tss_entry tss;
static uint8_t heap[HEAP_SIZE];
static struct Block *first_heap_block = (struct Block *)heap;
static struct multiboot_memory_map memory_map[MEMORY_MAP_MAX_SIZE];
static uint8_t memory_map_size = 0x00;
static struct gdt_entry gdt[0x06];
static struct gdt_ptr gp;
static uint32_t address = NULL; // Please don't shoot me for this

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

static inline void outb(uint16_t port, uint8_t val){
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port){
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

static void print_hex32(uint32_t value);

static inline void println(const char* str){
	syscall(0x82, 0x02, (uint32_t)str, 0x00, 0x00);
}

static void print_hex32(uint32_t value)
{
	const char hex[] = "0123456789ABCDEF";
	char str[12];
	
	str[0] = '0';
	str[1] = 'x';
	
	for (int i = 0; i < 8; i++) {
		str[2 + i] = hex[(value >> (28 - i * 4)) & 0xF];
	}
	
	str[10] = '\n';
	str[11] = '\0';
	
	println(str);
}

void *malloc(size_t size){
	struct Block *current_block_pointer = first_heap_block;
	while ((*current_block_pointer).size < size || !(*current_block_pointer).free){
		current_block_pointer = (*current_block_pointer).next;
		if (current_block_pointer == NULL){
			return NULL;
		}
	}
	if ((*current_block_pointer).size > size + sizeof(struct Block)){
		struct Block *new_block = (struct Block *)((uint8_t *)current_block_pointer + sizeof(struct Block) + size);
		(*new_block).size = (*current_block_pointer).size - size - sizeof(struct Block);
		(*new_block).free = TRUE;
		(*new_block).next = (*current_block_pointer).next;
		(*current_block_pointer).next = new_block;
		(*current_block_pointer).size = size;
	}
	(*current_block_pointer).free = 0x00;
	return (uint8_t *)current_block_pointer + sizeof(struct Block);
}

void free(void *ptr){
	struct Block *block_pointer = ptr - sizeof(struct Block);
	(*block_pointer).free = 0x01;
	while ((*(*block_pointer).next).free == 0x01){
		(*block_pointer).size += (*(*block_pointer).next).size + sizeof(struct Block);
		(*block_pointer).next = (*(*block_pointer).next).next;
	}
}

void optimize_memory_map(void)
{
	for (uint8_t i = 0; i < memory_map_size; i++) {
		uint8_t j = i + 1;
		
		while (j < memory_map_size) {
			if (memory_map[i].type == memory_map[j].type) {
				if (memory_map[i].addr + memory_map[i].len == memory_map[j].addr) {
					memory_map[i].len += memory_map[j].len;
					memory_map[j] = memory_map[memory_map_size - 1];
					memory_map_size--;
					continue;
				}
				if (memory_map[j].addr + memory_map[j].len == memory_map[i].addr) {
					memory_map[i].addr = memory_map[j].addr;
					memory_map[i].len += memory_map[j].len;
					memory_map[j] = memory_map[memory_map_size - 1];
					memory_map_size--;
					continue;
				}
			}
			
			j++;
		}
	}
}

uint32_t allocate_physical_memory(size_t size){
	uint32_t i = 0x00;
	while (memory_map[i].len < size || !memory_map[i].type || memory_map[i].addr == NULL){
		i++;
	}
	if (i >= memory_map_size){
		address = NULL;
		return NULL;
	}
	if (memory_map[i].len > size){
		if (memory_map_size >= MEMORY_MAP_MAX_SIZE){
			address = NULL;
			return NULL;
		}
		memory_map[memory_map_size].type = TRUE;
		memory_map[memory_map_size].size = sizeof(struct multiboot_memory_map);
		memory_map[memory_map_size].len = memory_map[i].len - size;
		memory_map[memory_map_size].addr = memory_map[i].addr + size;
		memory_map_size++;
		memory_map[i].len = size;
	}
	memory_map[i].type = FALSE;
	address = memory_map[i].addr;
	optimize_memory_map();
	return address;
}

void free_physical_memory(uint32_t physical_adress){
	for (uint8_t i = 0x00; i < memory_map_size; i++){
		if (memory_map[i].addr == physical_adress){
			memory_map[i].type = TRUE;
			break;
		}
	}
	optimize_memory_map();
}

// Reads sectors from legacy ATA (PATA) 
static void ata_read_sectors(uint32_t lba, uint8_t sectors_amount, uint8_t *buffer){
    while (inb(0x1F7) & 0x80);

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sectors_amount);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

	uint8_t status;
	
	do {
		status = inb(0x1F7);
	} while (status & 0x80);
	
	if (status & 0x01) {
		println("ATA error");
		asm volatile("hlt");
	}
	
	if ((status & 0x08)){
		for (uint16_t i = 0; i < 256 * sectors_amount; i++) {
			uint16_t data = inw(0x1F0);
			buffer[i * 2] = data & 0xFF;
			buffer[i * 2 + 1] = data >> 8;
		}
	} else {
		println("ATA error");
		asm volatile("hlt");
	}
}

static void tss_init()
{
	uint8_t *ptr = (uint8_t *)&tss;
	
	for (uint32_t i = 0x00; i < sizeof(tss); i++)
		ptr[i] = 0x00;
	
	tss.ss0 = 0x10;				// kernel data selector
	tss.esp0 = (uint32_t)(&stack_bottom) + sizeof(stack_bottom);
	
	tss.iomap_base = sizeof(tss);
}

static void gdt_set_tss(uint16_t num, uint32_t base, uint32_t limit)
{
	gdt[num].base_low    = base & 0xFFFF;
	gdt[num].base_middle = (base >> 0x10) & 0xFF;
	gdt[num].base_high   = (base >> 0x18) & 0xFF;
	
	gdt[num].limit_low = limit & 0xFFFF;
	gdt[num].granularity = (limit >> 0x10) & 0x0F;
	
	gdt[num].access = 0x89;
}

static void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit,
                  uint8_t access, uint8_t gran)
{
	gdt[num].base_low    = base & 0xFFFF;
	gdt[num].base_middle = (base >> 0x10) & 0xFF;
	gdt[num].base_high   = (base >> 0x18) & 0xFF;
	
	gdt[num].limit_low   = limit & 0xFFFF;
	gdt[num].granularity = (limit >> 0x16) & 0x0F;
	
	gdt[num].granularity |= gran & 0xF0;
	gdt[num].access      = access;
}

static void gdt_init()
{
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (uint32_t)&gdt;

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
	
	gdt_set_gate(3,
		0x00000000,
		0xFFFFFFFF,
		0xFA,
		0xCF
	);
	
	gdt_set_gate(4,
		0x00000000,
		0xFFFFFFFF,
		0xF2,
		0xCF
	);
	
	tss_init();
	gdt_set_tss(5, (uint32_t)&tss, sizeof(tss)-1);
	
	asm volatile ("lgdt (%0)" : : "r" (&gp));
	asm volatile (
		"mov $0x28, %%ax\n"
		"ltr %%ax"
		:
		:
		: "ax"
	);
}

static void idt_set_gate(uint8_t num, uint32_t base, uint8_t flags){
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	idt[num].segment_selector = KERNEL_CS;
	idt[num].reserved = 0;
	idt[num].flags = flags;
}

static inline void idt_load(struct idt_ptr* ptr)
{
    asm volatile("lidt (%0)" : : "r"(ptr));
}

static void idt_init(){
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = (uint32_t)idt;
	idt_load(&idtp);
}

static inline void pic_remap(int offset1, int offset2)
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

static inline void pic_send_eoi(uint8_t irq)
{
    if (irq >= 0x08)
        outb(PIC2, 0x20);

    outb(PIC1, 0x20);
}

static void update_paging(){
	for (uint16_t i = 0; i < 0x400 * NUMBER_OF_PAGE_TABLES; i++)
		kernel_page_tables[i] = (i * 0x1000) | 0x03;
	if (address == NULL)
		allocate_physical_memory(NUMBER_OF_PAGE_TABLES * 0x400000);
	for (uint16_t i = 0; i < 0x400 * NUMBER_OF_PAGE_TABLES; i++)
		userspace_page_tables[i] = (address + i * 0x1000) | 0x07;
	
	uint32_t *__page_directory = PhysicalToVirtual(page_directory);
	
	for (uint16_t i = 0; i < NUMBER_OF_PAGE_TABLES; i++)
		__page_directory[i] = ((uint32_t)((uint8_t*)VirtualToPhysical(userspace_page_tables) + 0x1000 * i)) | 0x07;
	for (uint16_t i = 0; i < NUMBER_OF_PAGE_TABLES; i++)
		__page_directory[768 + i] = ((uint32_t)((uint8_t*)kernel_page_tables + 0x1000 * i)) | 0x03;
}

static void load_elf(const char* filename){
	update_paging();
	
	uint32_t syscall_0x83_0x02_data[] = {0x08, 0x00};
	uint32_t first_cluster;
	first_cluster = syscall(0x83, 0x05, (uint32_t)filename, 0, 0).eax;
	syscall(0x83, 0x02, first_cluster, (uint32_t)elf_buffer, (uint32_t)syscall_0x83_0x02_data);
	
	if (!(elf_buffer[0x00] == 0x7F && elf_buffer[0x01] == 'E' && elf_buffer[0x02] == 'L' && elf_buffer[0x03] == 'F')){
		println(ELF_err);
		println(filename);
		return;
	}
	if (!(elf_buffer[0x04] == 0x01 && elf_buffer[0x05] == 0x01 && elf_buffer[18] == 0x03 && elf_buffer[19] == 0x00)){
		println(ELF_err);
		println(filename);
		return;
	}
	if (!(elf_buffer[17] == 0x00 && elf_buffer[16] == 0x02)){
		println(ELF_err);
		println(filename);
		return;
	}
	
	uint16_t program_header_entry_size = *(uint16_t *)(elf_buffer + 42);
	uint16_t program_header_entries = *(uint16_t *)(elf_buffer + 44);
	uint32_t program_header_offset = *(uint32_t *)(elf_buffer + 28);
	uint32_t eip = *(uint32_t *)(elf_buffer + 0x18);
	if (program_header_entries * program_header_entry_size + program_header_offset > 0x1000){			// Checks wheather the program header fits in the buffer
		println(ELF_err);
		println(filename);
		return;
	}
	uint32_t program_header_sector = program_header_offset / 512;
	syscall_0x83_0x02_data[0x01] = program_header_sector;
	syscall(0x83, 0x02, first_cluster, (uint32_t)elf_buffer, (uint32_t)syscall_0x83_0x02_data);
	
	for (uint16_t i = 0x00; i < program_header_entries; i++){
		uint16_t index = i * program_header_entry_size;
		if (*(uint32_t *)(elf_buffer + program_header_offset + index) == 0x01){
			uint32_t load_sector = *(uint32_t *)(elf_buffer + program_header_offset + index + 0x04);
			load_sector = (load_sector - load_sector % 512) / 512;
			uint32_t p_offset = *(uint32_t *)(elf_buffer + program_header_offset + index + 0x04);
			uint32_t p_vaddr = *(uint32_t *)(elf_buffer + program_header_offset + index + 0x08);
			uint32_t p_filesz = *(uint32_t *)(elf_buffer + program_header_offset + index + 0x10);
			uint32_t p_memsz = *(uint32_t *)(elf_buffer + program_header_offset + index + 0x14);
			
			if (p_filesz > p_memsz) {
				println(ELF_err);
				println(filename);
				return;
			}
			
			if (p_vaddr < 0x400000 || p_vaddr + p_memsz > NUMBER_OF_PAGE_TABLES * 0x400000){
				println(ELF_err);
				println(filename);
				return;
			}
			
			uint32_t sectors = (p_filesz + p_offset % 512 + 511) / 512;
			
			syscall_0x83_0x02_data[0] = sectors;
			syscall_0x83_0x02_data[1] = load_sector;
			syscall(0x83, 0x02, first_cluster, p_vaddr - p_offset % 512, (uint32_t)syscall_0x83_0x02_data);
			
			if (p_memsz > p_filesz){
				for (uint32_t __i = 0; __i < p_memsz - p_filesz; __i++){
					*(uint8_t*)(p_vaddr + p_filesz + __i) = 0x00;
				}
			}
		}
	}
	
	struct process* loaded_process = malloc(sizeof(struct process));
	loaded_process->files[0x00].descriptor = STDIN_FILENO;
	loaded_process->files[0x00].mode = FILE_READ_MODE;
	loaded_process->files[0x01].descriptor = STDOUT_FILENO;
	loaded_process->files[0x01].mode = FILE_WRITE_MODE;
	loaded_process->files[0x02].descriptor = STDERR_FILENO;
	loaded_process->files[0x02].mode = FILE_WRITE_MODE;
	
	current_process = loaded_process;
	asm volatile(
		"mov %0, %%ebx\n\t"
		"jmp thread_jmp\n\t"
		:
		: "r"(eip)
		: "ebx"
	);
}

void linux_systemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	println("The OS does not support Linux comptiability yet.");
	free(current_process);
	asm volatile("jmp __hlt");
	/*switch (call_number){
		default:{
			break;
		}
	}*/
}

void systemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (call_number){
		case 0x00:{
			r->eax = 0x01;
			r->ebx = 0x00;
			r->ecx = 0x00;
			break;
		}
		case 0x01:{
			if (arg2 < KERNEL_BASE){
				if (arg1 == STDOUT_FILENO || arg1 == STDERR_FILENO) {
					char print_str[arg3 + 0x01];
					for (uint32_t i = 0x00; i < arg3; i++){
						print_str[i] = *((const char*)arg2 + i);
					}
					print_str[arg3] = 0x00;
					println(print_str);
					r->eax = arg3;
				} else {
					r->eax = -0x01;
				}
			} else {
				r->eax = -0x02;
			}
			break;
		}
		case 0x02:{
			current_process = NULL;
			const char* exit_text = "Process exited.\n";
			const char* crash_text = "Process failed.\n";
			if (arg1 == 0x00)
				println(exit_text);
			else
				println(crash_text);
			asm volatile("jmp __hlt");
			break;
		}
		case 0x03:{
			uint32_t __tick_count = (arg1 * HZ) / 1000;
			uint32_t last_tick = tick_count;
			while (__tick_count > 0){
				asm volatile(
					"sti\n\t"
					"hlt\n\t"
					"cli\n\t"
					:
					:
					:
				);
				__tick_count = __tick_count - (tick_count - last_tick);
				last_tick = tick_count;
			}
			break;
		}
		case 0x04:{
			r->eax = tick_count / HZ;
			break;
		}
		case 0x05:{
			uint8_t *user_buffer = arg2;
			if (user_buffer < KERNEL_BASE){
				if (arg1 == STDIN_FILENO) {
					r->eax = 0x00;
					uint32_t chars_left = arg3;
					while (chars_left > 0x00){
						uint8_t c = syscall(0x82, 0x03, 0x00, 0x00, 0x00).eax;
						if (c == 0x00)
							break;
						*user_buffer = c;
						user_buffer++;
						chars_left--;
						r->eax++;
					}
				} else {
					r->eax = -0x01;
				}
			} else { // Nice try, user
				r->eax = -0x02;
			}
			break;
		}
		case 0x06:{	// It is recommended not calling it. Because NanoOS is currently a single tasking OS it replaces the 
					// current app running. However, it will not kill the calling process once multitasking will be added.
			load_elf((const char*)arg1);
			break;
		}
		default:{
			break;
		}
	}
}
void intersystemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (call_number){
		case 0x00:{
			isr8();
			break;
		}
		case 0x01:{
			ata_read_sectors(arg1, (uint8_t)arg2, (uint8_t*)arg3);
			break;
		}
		case 0x02:{
			idt_set_gate(arg1, arg2, 0x8E);
			break;
		}
		default:{
			break;
		}
	}
}

void isr_handler(uint32_t int_num, uint32_t error_code, uint32_t old_eip){
	if (int_num < 32){
		switch (int_num){
			case 0x00:
				println("Divide by 0 fault. Not handled by the current NanoOS version.");
				break;
			case 0x01:
				println("Debug trap. Not handled by the current NanoOS version.");
				break;
			case 0x02:
				println("Non-maskable interrupt. Not handled by the current NanoOS version.");
				break;
			case 0x03:
				println("Breakpoint trap. Not handled by the current NanoOS version.");
				break;
			case 0x04:
				println("Overflow trap. Not handled by the current NanoOS version.");
				break;
			case 0x05:
				println("Out of bounds fault. Not handled by the current NanoOS version.");
				break;
			case 0x06:
				println("Invaild opcode fault. Not handled by the current NanoOS version.");
				break;
			case 0x07:
				println("Device not available fault. Caused by a missing FPU. Not handled by the current NanoOS version.");
				break;
			case 0x0A:
				println("Invaild TSS fault. Not handled by the current NanoOS version.");
				break;
			case 0x0B:
				println("Sgment not present fault. Not handled by the current NanoOS version.");
				break;
			case 0x0C:
				println("Stack segment fault. Not handled by the current NanoOS version.");
				break;
			case 0x0D:
				println("General protection fault. Not handled by the current NanoOS version.\n");
				println("The faulting EIP is:\n");
				print_hex32(old_eip);
				break;
			case 0x0E:{
				println("Page fault. Not handled by the current NanoOS version. CR2, and the error code are:\n");
				uint32_t cr2;
				asm volatile(
					"mov %%cr2, %0\n\t"
					: "=r"(cr2)
					:
					: "memory"
				);
				print_hex32(cr2);
				print_hex32(error_code);
				println("The faulting EIP is:\n");
				print_hex32(old_eip);
				break;
			}
			case 0x10:
				println("Floating point fault. Not handled by the current NanoOS version.");
				break;
			case 0x11:
				println("Alignment check fault. Not handled by the current NanoOS version.");
				break;
			case 0x12:
				println("Machine check exception. Not handled by the current NanoOS version.");
				break;
			case 0x13:
				println("SIMD floating point fault. Not handled by the current NanoOS version.");
				break;
			case 0x14:
				println("Virtualization fault. Not handled by the current NanoOS version.");
				break;
			case 0x15:
				println("Control protection fault. Not handled by the current NanoOS version.");
				break;
			case 0x1C:
				println("Hypervisor injection fault. Not handled by the current NanoOS version.");
				break;
			case 0x1D:
				println("VMM compunication fault. Not handled by the current NanoOS version.");
				break;
			case 0x1E:
				println("Security fault. Not handled by the current NanoOS version.");
				break;
			default:
				println("Unknown CPU exception.");
				break;
		}
		__asm__ volatile ("hlt");
	} else {
		int_num -= 32;
		switch (int_num){
			case 0x01:{
				uint8_t sc = inb(0x60);
				if (!(sc & 0x80)){
					char c = scancode_to_ascii[sc];
					pic_send_eoi((uint8_t)int_num);
					syscall(0x82, 0x01, (uint32_t)c, 0x00, 0x00);
				} else {
					pic_send_eoi((uint8_t)int_num);
				}
				break;
			}
			default:
				pic_send_eoi((uint8_t)int_num);
				break;
		}
	}
}

void irq0_handler(uint32_t eflags, uint16_t ds, uint16_t es, uint16_t fs, uint16_t gs, uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax){
	tick_count++;
	outb(0x20, 0x20);
}

__attribute__((section(".boot.text")))
void set_paging(uint32_t *temp_page_directory, uint32_t *temp_page_tables){
	for (uint16_t i = 0; i < 1024 * NUMBER_OF_PAGE_TABLES; i++)
		temp_page_tables[i] = (i * 0x1000) | 0x07;

	for (uint16_t i = 0; i < NUMBER_OF_PAGE_TABLES; i++)
		temp_page_directory[i] = (uint32_t)((temp_page_tables + 0x400 * i)) | 0x07;
	for (uint16_t i = 0; i < NUMBER_OF_PAGE_TABLES; i++)
		temp_page_directory[768 + i] = (uint32_t)((temp_page_tables + 0x400 * i)) | 0x03;
}

__attribute__((section(".boot.text")))
void build_memory_map(struct multiboot_info *mbi){
	struct multiboot_memory_map *entry;
	
	entry = (void *)(*mbi).mmap_addr;
	struct multiboot_memory_map *__memory_map = VirtualToPhysical(memory_map);
	uint8_t *__memory_map_size = VirtualToPhysical(&memory_map_size);
	(*__memory_map_size) = 0x00;
	
	while ((uint32_t)entry < (*mbi).mmap_addr + (*mbi).mmap_length && (*__memory_map_size) < MEMORY_MAP_MAX_SIZE)
	{
		(*(struct multiboot_memory_map*)((uint8_t*)__memory_map + (*__memory_map_size) * sizeof(struct multiboot_memory_map))).size = (*entry).size;
		(*(struct multiboot_memory_map*)((uint8_t*)__memory_map + (*__memory_map_size) * sizeof(struct multiboot_memory_map))).addr = (*entry).addr;
		(*(struct multiboot_memory_map*)((uint8_t*)__memory_map + (*__memory_map_size) * sizeof(struct multiboot_memory_map))).len = (*entry).len;
		if ((*entry).type == 0x01)
			(*(struct multiboot_memory_map*)((uint8_t*)__memory_map + (*__memory_map_size) * sizeof(struct multiboot_memory_map))).type = TRUE;
		else
			(*(struct multiboot_memory_map*)((uint8_t*)__memory_map + (*__memory_map_size) * sizeof(struct multiboot_memory_map))).type = FALSE;
		entry = (void *)((uint32_t)entry + entry->size + sizeof(entry->size));
		(*__memory_map_size)++;
	}
}

void kernel_main(void){
	first_heap_block->size = HEAP_SIZE - sizeof(struct Block);
	first_heap_block->free = TRUE;
	first_heap_block->next = NULL;
	gdt_init();
	gdt_flush();
	pic_remap(0x20, 0x28);
	for (int i = 0; i < 48; i++) {
		idt_set_gate(i, (uint32_t)isr[i], 0x8E);
	}
	idt_set_gate(0x80, (uint32_t)__linux_syscall, 0xEE);
	idt_set_gate(0x81, (uint32_t)__intersystemcall, 0x8E);
	idt_set_gate(0x82, (uint32_t)__uisystemcall, 0x8E);
	idt_set_gate(0x83, (uint32_t)__filesystemsystemcall, 0x8E);
	idt_set_gate(0x84, (uint32_t)__systemcall, 0xEE);
	idt_init();
	syscall(0x82, 0x00, 0x00, 0x00, 0x00);
	syscall(0x83, 0x00, 0x00, 0x00, 0x00);
	uint16_t divisor = 1193182 / HZ;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 0x08) & 0xFF));
}
