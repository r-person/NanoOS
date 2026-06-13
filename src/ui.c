// NanoOS Alpha version 1.0.0 default UI
// Written by RPerson

#include <stdint.h>
#include <stdbool.h>

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

static char input_buffer[79];
static const char* reboot_command = "reboot";
static const char* echo_command = "echo";
static const char* version_command = "version";
const char* boot_str = "NanoOS Alpha v1.0.0";
const char* reboot_str = "Rebooting...";
static volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
static uint8_t current_row;
static uint8_t current_input_char_col;

static bool check_command(const char* str1, const char* str2){
	int i = 0;
	while (*(str1 + i) != '\0' && *(str2 + i) != '\0' && *(str1 + i) != ' ' && *(str2 + i) != ' '){
		if (*(str1 + i) != *(str2 + i)){
			return false;
		}
		i++;
	}
	if ((*(str1 + i) == ' ' || *(str1 + i) == '\0') && (*(str2 + i) == ' ' || *(str2 + i) == '\0')){
		return true;
	}
	return false;
}

static inline void print_char(char c, uint8_t row, uint8_t col){
	vga[row * 80 + col] = (0x07 << 8) | c;
}

static void clear_screen(){
	for (uint8_t row = 0; row < 25; row++){
		for (uint8_t col = 0; col < 80; col++)
			print_char(' ', row, col);
	}
}

static void print_line(const char* line, uint8_t row, uint8_t pos){
	uint8_t offset = 0;
	while (*line){
		print_char(*line, row, offset + pos);
		offset++;
		line++;
	}
}

static void println(const char* line){
	if (current_row == 78){
		current_row = 0;
		clear_screen();
		print_char('>', 24, 0);
	}
	current_row++;
	print_line(line, current_row - 1, 0);
}

void ui_systemcall(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3){
	char c;
	switch (syscall_num){
		case 0x00:
			clear_screen();
			current_row = 0;
			current_input_char_col = 1;
			println(boot_str);
			print_char('>', 24, 0);
			for (uint8_t i = 0; i < 79; i++){
				input_buffer[i] = '\0';
			}
			break;
		case 0x01:    // frame
			break;
		case 0x02:
			c = (char)arg1;
			if (c == '\n'){
				current_input_char_col = 1;
				for (uint8_t col = 1; col < 80; col++){
					print_char(' ', 24, col);
				}
				if (check_command(input_buffer, reboot_command)){
					println(reboot_str);
					syscall(0x81, 0x00, 0x00, 0x00, 0x00);
				} else if (check_command(input_buffer, echo_command)){
					println((const char*)input_buffer + 5);
				} else if (check_command(input_buffer, version_command)){
					println(boot_str);
				}
			} else {
				input_buffer[current_input_char_col - 1] = c;
				input_buffer[current_input_char_col] = '\0';
				print_char(c, 24, current_input_char_col);
				current_input_char_col++;
			}
			break;
		case 0x03:
			println((const char*)arg1);
			break;
		default:
			break;
	}
}
