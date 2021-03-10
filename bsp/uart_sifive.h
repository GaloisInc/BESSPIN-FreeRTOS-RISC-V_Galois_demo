// See https://github.com/riscv/riscv-pk/blob/master/machine/uart.h for the original file
//
// Copyright (c) 2013, The Regents of the University of California (Regents).
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the Regents nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
// SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
// OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
// HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
// MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
#ifndef _RISCV_UART_SIFIVE_H
#define _RISCV_UART_SIFIVE_H

#include <stdint.h>

extern volatile uint32_t* uart;

#define UART_REG_TXFIFO		0
#define UART_REG_RXFIFO		1
#define UART_REG_TXCTRL		2
#define UART_REG_RXCTRL		3
#define UART_REG_DIV		6

#define UART_TXEN		 0x1
#define UART_RXEN		 0x1

void uart_putchar(uint8_t ch);
int uart_getchar(void);
void uart_init(void);

#endif
