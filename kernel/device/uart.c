
#include "uart.h"
#include <ioport.h>

/* See http://wiki.osdev.org/Serial_Ports */

#define PORT 0x3f8

static ssize_t uart_write(chardev_t *dev, const char *buf, size_t nbyte);

chardev_t uartdev = { NULL, uart_write };

int
uart_init(void)
{
	/* Disable Interrupts */
	outb(PORT + 1, 0x00);

	/* Enable Divisor Latch */
	outb(PORT + 3, 0x80);

	/* Set divisor to 1 */
	outb(PORT + 0, 0x01);
	outb(PORT + 1, 0x00);

	/* Set 8N1 */
	outb(PORT + 3, 0x03);

	/* Enable FIFO, Clear buffers, set to 14 byte */
	outb(PORT + 2, 0xC7);

	/* */
	outb(PORT + 4, 0x0B);

	return 0;
}

static ssize_t
uart_write(chardev_t *dev, const char *buf, size_t nbyte)
{
	size_t i;
	for (i=0; i < nbyte; i++) {
		while((inb(PORT + 5) & 0x20) == 0);
		outb(PORT, buf[i]);
	}

	return i;
}
