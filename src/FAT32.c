// NanoOS's default FAT32 loader
// Written by RPerson

#include <stdint.h>

#define bytes_per_sector *(uint16_t *)((uint8_t *)boot_sector + 0x0B)
#define sectors_per_cluster *(uint8_t *)((uint8_t *)boot_sector + 0x0D)
#define reserved_sectors *(uint16_t *)((uint8_t *)boot_sector + 0x0E)
#define FAT_count *(uint8_t *)((uint8_t *)boot_sector + 0x10)
#define hidden_sectors *(uint32_t *)((uint8_t *)boot_sector + 0x1C)
#define sectors_per_FAT *(uint32_t *)((uint8_t *)boot_sector + 0x24)
#define root_directory_cluster *(uint32_t *)((uint8_t *)boot_sector + 0x2C)

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
} syscall_result_t;

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

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

static uint8_t boot_sector[512];
static uint32_t FAT[128];
static const char* non_vaild_FAT32_err = "Invaild or unsupported FAT32 detected.";
static const char* bad_FAT32_sector_err = "Invaild FAT32 sector detected.";

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

static inline uint32_t cluster_to_sector(uint32_t cluster){
	return (cluster - 2) * sectors_per_cluster + FAT_count * sectors_per_FAT + reserved_sectors + hidden_sectors;
}

static inline uint8_t are_strings_equal(const char* str1, const char* str2, uint8_t length){
	for (int i = 0; i < length; i++){
		if (*(str1 + i) != *(str2 + i)) return 0x00;
	}
	return 0x01;
}

static inline uint32_t next_cluster(uint32_t cluster){
	uint32_t fat_offset = cluster * 4;
	syscall(0x81, 0x01, reserved_sectors + (fat_offset / bytes_per_sector) + hidden_sectors, 0x01, FAT);
	return FAT[(fat_offset % bytes_per_sector) / 4] & 0x0FFFFFFF;
}

static void read_file(uint32_t first_cluster, uint8_t *buffer){
	uint32_t current_cluster = first_cluster; 
	uint8_t *current_pointer = buffer;
	while (current_cluster < 0x0FFFFFF8){
		if (current_cluster == 0 ||
			current_cluster == 0x0FFFFFF7){
				__asm__ volatile ("cli");
				syscall(0x82, 0x03, (uint32_t)bad_FAT32_sector_err, 0x00, 0x00);
				__asm__ volatile ("hlt");
		}
		syscall(0x81, 0x01, cluster_to_sector(current_cluster), sectors_per_cluster, current_pointer);
		current_pointer += sectors_per_cluster * bytes_per_sector;
		current_cluster = next_cluster(current_cluster);
	}
}

static uint32_t find_first_cluster(const char *filename)
{
    uint32_t current_cluster = root_directory_cluster;
	uint8_t directory[sectors_per_cluster * bytes_per_sector];

	while (current_cluster < 0x0FFFFFF8)
	{
		if (current_cluster == 0 ||
			current_cluster == 0x0FFFFFF7){
				__asm__ volatile ("cli");
				syscall(0x82, 0x03, (uint32_t)bad_FAT32_sector_err, 0x00, 0x00);
				__asm__ volatile ("hlt");
		}
		syscall(0x81, 0x01, cluster_to_sector(current_cluster), sectors_per_cluster, directory);
		for (uint32_t i = 0; i < sectors_per_cluster * bytes_per_sector; i += 32)
		{
			uint8_t *entry = &directory[i];
			if (*entry == 0x00)
				return 0;
			if (*entry == 0xE5)
				continue;
			if (*(entry + 11) == 0x0F){
			}
			if (*(entry + 11) & 0x08)
				continue;
			if (are_strings_equal(filename, (const char *)entry, 11))
			{
				uint16_t high = *(uint16_t *)(entry + 20);
				uint16_t low  = *(uint16_t *)(entry + 26);
	
				return ((uint32_t)high << 16) | low;
			}
		}
		current_cluster = next_cluster(current_cluster);
	}
    return 0;
}

static uint32_t number_of_files(){
	uint32_t current_cluster = root_directory_cluster;
	uint8_t directory[sectors_per_cluster * bytes_per_sector];
	int count = 0;

	while (current_cluster < 0x0FFFFFF7){
		if (current_cluster == 0 ||
			current_cluster == 0x0FFFFFF7){
				__asm__ volatile ("cli");
				syscall(0x82, 0x03, (uint32_t)bad_FAT32_sector_err, 0x00, 0x00);
				__asm__ volatile ("hlt");
		}
		syscall(0x81, 0x01, cluster_to_sector(current_cluster), sectors_per_cluster, directory);
		
		for (uint32_t i = 0; i < sectors_per_cluster * bytes_per_sector; i += 32){
			uint8_t *entry = &directory[i];
			if (*entry == 0x00)
				return count;
			if (*entry == 0xE5)
				continue;
			if (*(entry + 11) == 0x0F){
			}
			if (*(entry + 11) & 0x08)
				continue;
			if (entry[11] & 0x20) {
				count++;
			}
		}
		current_cluster = next_cluster(current_cluster);
	}
	
	return count;
}

static void print_all_files(){
	uint32_t current_cluster = root_directory_cluster;
	uint8_t directory[sectors_per_cluster * bytes_per_sector];

	while (current_cluster < 0x0FFFFFF7){
		if (current_cluster == 0 ||
			current_cluster == 0x0FFFFFF7){
				__asm__ volatile ("cli");
				syscall(0x82, 0x03, (uint32_t)bad_FAT32_sector_err, 0x00, 0x00);
				__asm__ volatile ("hlt");
		}
		syscall(0x81, 0x01, cluster_to_sector(current_cluster), sectors_per_cluster, directory);
		
		for (uint32_t i = 0; i < sectors_per_cluster * bytes_per_sector; i += 32){
			uint8_t *entry = &directory[i];
			if (*entry == 0x00)
				return;
			else if (*entry == 0xE5)
				continue;
			else if (*(entry + 11) == 0x0F){
			}
			else if (*(entry + 11) & 0x08)
				continue;
			else{
				if (entry[11] & 0x10){
					char str[18];
					int k = 0;

					str[k++] = 'D';
					str[k++] = 'I';
					str[k++] = 'R';
					str[k++] = ' ';
					str[k++] = '-';
					str[k++] = ' ';

					for (int i = 0; i < 11; i++) {
						str[k++] = entry[i];
					}

					str[k] = '\0';
					syscall(0x82, 0x03, (uint32_t)str, 0x00, 0x00);
				}
				if (entry[11] & 0x20){
					char str[18];
					int k = 0;

					str[k++] = 'F';
					str[k++] = 'I';
					str[k++] = 'L';
					str[k++] = 'E';
					str[k++] = ' ';
					str[k++] = '-';
					str[k++] = ' ';

					for (int i = 0; i < 8; i++) {
						str[k++] = entry[i];
					}
					
					str[k++] = '\0';
					
					for (int i = 8; i < 11; i++) {
						str[k++] = entry[i];
					}

					str[k] = '\0';
					syscall(0x82, 0x03, (uint32_t)str, 0x00, 0x00);
				}
			}
		}
		current_cluster = next_cluster(current_cluster);
	}
}

static inline uint32_t clusters_in_file(uint32_t first_cluster){
	uint32_t current_cluster = first_cluster;
	uint32_t count = 0x00;
	while (current_cluster != 0x00 && current_cluster < 0x0FFFFFF7){
		current_cluster = next_cluster(current_cluster);
		count++;
	}
	return count;
}

void fat32_systemcall(uint32_t call_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, regs_t *r){
	switch (call_number){
		case 0x00:{
			syscall(0x81, 0x01, 0x800, 0x01, boot_sector);
			if (bytes_per_sector != 512){
				__asm__ volatile ("cli");
				syscall(0x82, 0x03, (uint32_t)non_vaild_FAT32_err, 0x00, 0x00);
				char bytes_per_sector_str[16];
				uint32_to_str(bytes_per_sector, bytes_per_sector_str);
				syscall(0x82, 0x03, (uint32_t)bytes_per_sector_str, 0x00, 0x00);
				__asm__ volatile ("hlt");
			}
			break;
		}
		case 0x01:{
			uint32_t first_cluster = find_first_cluster((const char*)arg1);
			r->eax = clusters_in_file(first_cluster) * sectors_per_cluster * bytes_per_sector;
			break;
		}
		case 0x02:{
			uint32_t first_cluster = find_first_cluster((const char*)arg1);
			if (first_cluster != 0x00)
				read_file(first_cluster, (uint8_t *)arg2);
			break;
		}
		case 0x03:{
			r->eax = number_of_files();
			break;
		}
		case 0x04:{
			print_all_files();
			break;
		}
		default:{
			break;
		}
	}
}
