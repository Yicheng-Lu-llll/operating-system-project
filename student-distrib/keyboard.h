#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "terminal.h"

void init_keyboard(void);

extern void keyboard_handler(void);

void up_handler();
void down_handler();
void tab_handler();

int32_t sysdir_read_for_tab(void* buf, uint32_t  n);


int32_t special_scancode_handler(uint8_t scan_code);

int32_t copy_buffer(void* buf);
#endif

