#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <types.h>
#include <log.h>

typedef struct chardev_struct {
	ssize_t (*read)(struct chardev_struct *dev, char *buf, size_t nbyte);
	ssize_t (*write)(struct chardev_struct *dev, const char *buf, size_t nbyte);
	void *data;
} chardev_t;

int multiboot_init(uint32_t magic, uint32_t addr);

size_t strlen(const char *s);
char * strnrev(char *s, size_t nbytes);

char * ltoa(long value, char *str, int base);

extern chardev_t *kconsole;

#endif
