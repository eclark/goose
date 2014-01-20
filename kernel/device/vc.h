/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */
#ifndef _VC_H
#define _VC_H

#include <system.h>

extern chardev_t vcdev;
extern uint16_t *vga;

void vc_clear(void);

#endif
