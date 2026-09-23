// The default UI of NanoOS
// Written by RPerson

#include "stdint.h"
#include "stddef.h"

#define INPUT_BUFFER_SIZE 0x100

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
} syscall_result_t;

struct open_file{
	uint32_t descriptor;
	uint32_t current_cluster;
	uint16_t current_pointer;
	uint8_t mode;
};

struct process{
	struct open_file files[3];
};

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

extern struct process *current_process;

static uint8_t input_buffer[INPUT_BUFFER_SIZE + 0x01];
static uint16_t input_buffer_size = 0x00;
static uint8_t current_row = 0x00;
static uint8_t current_col = 0x00;
static uint8_t current_input_char_col = 0x01;
static const char* reboot_command = "reboot";
static const char* echo_command = "echo";
static const char* version_command = "version";
static const char* filenum_command = "filenum";
static const char* dir_command = "dir";
static const char* run_command = "run";
static const char* version_str = "NanoOS Beta v1.0.0\n";
static const char* reboot_str = "Rebooting...\n";
static volatile uint16_t* vga = (volatile uint16_t*)0xC00B8000;

static void uint32_to_str(uint32_t value, char *buffer)
{
    char temp[10];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

static uint8_t check_command(const char* str1, const char* str2){
	int i = 0;
	while (*(str1 + i) != '\0' && *(str2 + i) != '\0' && *(str1 + i) != ' ' && *(str2 + i) != ' '){
		if (*(str1 + i) != *(str2 + i)){
			return 0;
		}
		i++;
	}
	if ((*(str1 + i) == ' ' || *(str1 + i) == '\0') && (*(str2 + i) == ' ' || *(str2 + i) == '\0')){
		return 1;
	}
	return 0;
}

static inline void print_char(char c, uint8_t row, uint8_t col){
	vga[row * 80 + col] = (0x07 << 8) | c;
}

static inline void clear_screen(){
	for (uint8_t row = 0; row < 25; row++){
		for (uint8_t col = 0; col < 80; col++)
			print_char(' ', row, col);
	}
}


static void print(const char* str){
	if (str == NULL)
		asm volatile("hlt");
	uint16_t index = 0x00;
	while ((*(str + index)) != 0x00){
		for (; *(str + index) != '\n' && *(str + index) != 0x00; ){
			print_char(*(str + index), current_row, current_col);
			current_col++;
			index++;
		}
		if (*(str + index) == '\n'){
			if (current_row == 0x18){
				current_row = 0x00;
				clear_screen();
				if (current_process == NULL)
					print_char('>', 0x18, 0x00);
				else
					print_char(' ', 0x18, 0x00);
			} else {
				current_row++;
			}
			current_col = 0x00;
			index++;
		}
	}
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
	
	print(str);
}

void ui_start(){
	clear_screen();
	print_char('>', 0x18, 0x00);
	for (uint16_t i = 0x00; i < INPUT_BUFFER_SIZE; i++){
		input_buffer[i] = 0x00;
	}
}

void ui_systemcall(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (syscall_num){
		case 0x00:{
			ui_start();
			break;
		}
		case 0x01:{
			uint8_t c = (uint8_t)arg1;
			if (c == '\n'){
				if (current_process == NULL){
					for (uint8_t col = 0x01; col < 80; col++){
						print_char(' ', 0x18, col);
					}
					if (check_command(input_buffer, reboot_command)){
						print(reboot_str);
						syscall(0x81, 0x00, 0x00, 0x00, 0x00);
					} else if (check_command(input_buffer, echo_command)){
						input_buffer[input_buffer_size] = '\n';
						input_buffer_size++;
						print((const char*)input_buffer + 0x05);
					} else if (check_command(input_buffer, version_command)){
						print(version_str);
					} else if (check_command(input_buffer, filenum_command)){
						char output_str[0x10];
						syscall_result_t result;
						if (*(input_buffer + 7) != '\0'){
							result = syscall(0x83, 0x03, (uint32_t)((const char*)(input_buffer + 0x08)), 0x00, 0x00);
						} else {
							result = syscall(0x83, 0x03, (uint32_t)((const char*)(input_buffer + 0x08)), 0x00, 0x00);
						}
						uint32_t number_of_files = result.eax;
						uint32_to_str(number_of_files, output_str);
						print((const char*)output_str);
					} else if (check_command(input_buffer, dir_command)){
						if (*(input_buffer + 0x03) != '\0'){
							syscall(0x83, 0x04, (uint32_t)((const char*)(input_buffer + 0x04)), 0x00, 0x00);
						} else{
							const char *output = "\0";
							syscall(0x83, 0x04, (uint32_t)output, 0x00, 0x00);
						}
					} else if (check_command(input_buffer, run_command)){
						if (*(input_buffer + 0x03) != '\0'){
							uint8_t ELF_name[15] = "BIN\\\0\0\0\0\0\0\0\0\0\0\0";
							const char* input_buffer_filename = (const char*)(input_buffer + 0x04);
							uint8_t i = 0x00;
							while (*(input_buffer_filename + i) != '\0'){
								*(ELF_name + 0x04 + i) = *(input_buffer_filename + i);
								i++;
							}
							*(ELF_name + 0x04 + i) = '.';
							*(ELF_name + 0x04 + i + 0x01) = 'E';
							*(ELF_name + 0x04 + i + 0x02) = 'L';
							*(ELF_name + 0x04 + i + 0x03) = 'F';
							for (uint8_t i = 0x00; i < input_buffer_size; i++){
								input_buffer[i] = 0x00;
							}
							input_buffer_size = 0x00;
							current_input_char_col = 0x01;
							syscall(0x84, 0x06, (uint32_t)ELF_name, 0x00, 0x00);
						}
					}
					for (uint8_t i = 0x00; i < input_buffer_size; i++){
						input_buffer[i] = 0x00;
					}
					input_buffer_size = 0x00;
				} else {
					if (input_buffer_size != INPUT_BUFFER_SIZE){
						input_buffer[input_buffer_size] = c;
						input_buffer_size++;
						input_buffer[input_buffer_size] = 0x00;
					}
				}
				current_input_char_col = 0x01;
			} else {
				if (input_buffer_size != INPUT_BUFFER_SIZE){
					input_buffer[input_buffer_size] = c;
					input_buffer_size++;
					input_buffer[input_buffer_size] = 0x00;
				}
				if (current_process == NULL){
					print_char(c, 0x18, current_input_char_col);
					current_input_char_col++;
				}
			}
			break;
		}
		case 0x02:{
			print((const char*)arg1);
			break;
		}
		case 0x03:{
			uint16_t temp2 = input_buffer_size;
			if (input_buffer_size > 0x00){
				uint16_t temp = input_buffer_size;
				uint8_t c = input_buffer[0x00];
				if (input_buffer_size > INPUT_BUFFER_SIZE){
					print_hex32(input_buffer_size);
					asm volatile("hlt");
				}
				for (uint16_t i = 0x01; i < input_buffer_size; i++){
					input_buffer[i - 0x01] = input_buffer[i];
				}
				input_buffer[input_buffer_size] = 0x00;
				input_buffer_size--;
				if (input_buffer_size > INPUT_BUFFER_SIZE){
					print_hex32(input_buffer_size);
					print_hex32(temp);
					print_hex32(temp2);
					asm volatile("hlt");
				}
				r->eax = c;
			} else {
				r->eax = 0x00;
			}
			break;
		}
		case 0x04:{
			clear_screen();
			break;
		}
		default:{
			break;
		}
	}
}

void ui_main(){}
