CC = arm-none-eabi-gcc
CFLAGS = --std=gnu11 -Ofast -nostdlib -mlong-calls -fPIC -mno-pic-data-is-text-relative -msingle-pic-base -mpic-register=r9  -mcpu=cortex-m3 -mthumb -fno-builtin  -ffreestanding   --specs=nosys.specs --specs=nano.specs -Tkmod.ld -Wl,-N -Wl,--just-symbols=../armboy-kernel/src/kernel.elf
OBYCP = arm-none-eabi-objcopy

INCLUDE = -I ../armboy-kernel/src/ -I ../../../tools/ATMEL -I ../../../tools/CMSIS/include
CFILES = ssd1289.c

test.bin : $(CFILES) kmod.S
	$(CC) $(CFLAGS) -o ssd1289.elf $(CFILES) kmod.S $(INCLUDE)
	$(OBYCP) -O binary ssd1289.elf ssd1289.bin

upload: test.bin
	python3 ../tool/deleteAll.py
	../tool/abFlasher.a ssd1289.bin /dev/cu.usbmodem14121
