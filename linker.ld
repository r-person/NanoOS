ENTRY(start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        KEEP(*(.multiboot))
        *(.boot.text)
        *(.boot.data)
        *(.boot.bss)
    }

    . = ALIGN(4096);

    kernel_phys = .;

    . += 0xC0000000;

    .kernel : AT(kernel_phys)
    {
        *(.text*)
        *(.rodata*)
        *(.data*)
        *(.bss*)
    }
}