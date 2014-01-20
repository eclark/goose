/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */
#ifndef _UART_H
#define _UART_H

#include <system.h>

int uart_init(void);

extern chardev_t uartdev;

#endif
