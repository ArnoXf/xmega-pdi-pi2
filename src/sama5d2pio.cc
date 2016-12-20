/*
 * This library wraps the functions of the bcm2835 GPIO 
 * of the Raspberry Pi to the PIO of the Atmel SAMA5D2
 * using /dev/mem to access the PIO related registers.
 *
 * @author	Arno Fuhrmann <arno.fuhrmann@tum.de>
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// Define the size that is mapped using nmap
#define MAPPED_SIZE     0x1000
// Define the start address of the IO port in memory
#define IO_PORT_RAM     0xFC038000

// Define the available registers of the IO port (reference: Atmel 11267)
#define PIO_MSKR        0x000
#define PIO_CFGR        0x004
#define PIO_PDSR        0x008
#define PIO_LOCKSR      0x00C
#define PIO_SODR        0x010
#define PIO_CODR        0x014
#define PIO_ODSR        0x018
#define PIO_IER         0x020
#define PIO_IDR         0x024
#define PIO_IMR         0x028
#define PIO_ISR         0x02C
#define PIO_IOFR        0x03C
#define PIO_WPMR        0x5E0
#define PIO_WPSR        0x5E4
#define OFFSET          0x40
// Config-register: Input (direction: input, pull-up enabled)
#define BCM2835_GPIO_FSEL_INPT  0x200u
// Config-register: Output (direction: output, pull-up enabled)
#define BCM2835_GPIO_FSEL_OUTP  0x100u
// This means pin HIGH, true, 3.3volts on a pin
#define HIGH 0x1
// This means pin LOW, false, 0volts on a pin
#define LOW  0x0

void* base_addr;
int fd;

int bcm2835_init();
void bcm2835_cleanup();
void bcm2835_gpio_clr(uint8_t pin);
void bcm2835_gpio_set(uint8_t pin);
uint8_t bcm2835_gpio_lev(uint8_t pin); 
void bcm2835_gpio_fsel(uint8_t pin, uint16_t mode);
void bcm2835_delayMicroseconds(uint64_t micros);
int* getreg(int off, int io_group);

int bcm2835_init(void){
	// Open /dev/mem file
	if((fd = open("/dev/mem", O_RDWR)) == -1) {
		printf("/dev/mem could not be opened.\n");
		return 0;
	}

	base_addr = mmap(0, MAPPED_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, IO_PORT_RAM);
	return 1;
}

void bcm2835_cleanup(){
	// Unmap the area & error checking
	if (munmap(base_addr,MAPPED_SIZE)==-1){
		perror("Error un-mmapping the file");
	}

	// Close the character device
	close(fd);

	return;
}

void bcm2835_gpio_clr(uint8_t pin){
	*(getreg(PIO_CODR, pin/32)) = 1 << pin%32;
}

void bcm2835_gpio_set(uint8_t pin){
	*(getreg(PIO_SODR, pin/32)) = 1 << pin%32;
}

void bcm2835_gpio_fsel(uint8_t pin, uint16_t mode) {
	*(getreg(PIO_MSKR, pin/32)) = 1 << pin%32;
	*(getreg(PIO_CFGR, pin/32)) = mode;
}

uint8_t bcm2835_gpio_lev(uint8_t pin) {
	return (*(getreg(PIO_PDSR, pin/32)) & (1 << pin%32)) ? HIGH : LOW;
}

void bcm2835_delayMicroseconds(uint64_t micros) {
#ifdef GETTIME
	struct timespec ttime,curtime;

	clock_gettime(CLOCK_REALTIME,&ttime);
	ttime.tv_nsec = 0;

	clock_settime(CLOCK_REALTIME,&ttime);
	clock_gettime(CLOCK_REALTIME,&ttime);
	ttime.tv_nsec += 1000*micros;

	while(true) {
		clock_gettime(CLOCK_REALTIME,&curtime);
		if (curtime.tv_nsec > ttime.tv_nsec)
			break;
	}
#else
#define CYCLES_PER_LOOP 3
	uint32_t l = micros*500/CYCLES_PER_LOOP;
	asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" : : [count]"r"(l) );
#endif

}

inline int* getreg(int reg_off, int io_group) {
	// Returns a pointer to the desired register (reference: Atmel 11267 - 31.7)
	return (int*) ((char*)base_addr + (io_group*OFFSET) + reg_off);
}
