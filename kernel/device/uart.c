/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */

#include "uart.h"
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

/* See http://wiki.osdev.org/Serial_Ports and
 * http://www.sci.muni.cz/docs/pc/serport.txt */

#define IRQ 4
#define PORT 0x3f8

static ssize_t uart_read(struct chardev_struct *dev, char *buf, size_t nbyte);
static ssize_t uart_write(chardev_t *dev, const char *buf, size_t nbyte);
static int uart_int(regs_t *regs);

#define RECVBUF_LEN 64
static char recvbuf[RECVBUF_LEN];
static int rbpos = 0;

chardev_t uartdev = { uart_read, uart_write };

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

	/* Interrupt enable and DTR,RTS high */
	outb(PORT + 4, 0x0B);

	/* Enable Interrupt on data ready */
	outb(PORT + 1, 0x01);

	bind_vector(IRQBASEVEC + IRQ, uart_int);
	enable_isa_irq(IRQ);

	return 0;
}

static ssize_t
uart_read(struct chardev_struct *dev, char *buf, size_t nbyte)
{
	uint64_t flags;
	size_t read = 0, rs;

	while (1) {
		cli_ifsave(&flags);

		if (rbpos > 0) {
			if (rbpos <= (nbyte - read)) {
				rs = rbpos;
			} else {
				rs = nbyte - read;
			}
			memcpy(buf, recvbuf, rs);
			read += rbpos;
			buf += rbpos;
			if (rbpos - rs > 0)
				memmove(recvbuf, recvbuf + rs, rbpos - rs);
			rbpos -= rs;
		}

		ifrestore(flags);

		if (read >= nbyte)
			break;

		/* Wait for an interrupt */
		asm volatile("hlt":::"memory");
	}

	return read;
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

static int
uart_int(regs_t *regs)
{
	char byt;

	byt = inb(PORT + 0);
	if (rbpos < RECVBUF_LEN)
		recvbuf[rbpos++] = byt;
	else
		kprintf("uart overrun\n");
	/* Drops bytes when the buffer is full */

	send_eoi(IRQ);

	return 1;
}
