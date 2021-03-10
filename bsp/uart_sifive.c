// See https://github.com/riscv/riscv-pk/blob/master/machine/uart.c for the original file
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
#include "bsp.h"
#include "uart.h"
#include "uart_sifive.h"
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t uart_tx_mutex; /* Mutex for TX transmissions */
SemaphoreHandle_t uart_rx_mutex; /* Mutex for RX transmissions */
volatile uint32_t *uart;

bool uart0_rxready(void)
{
    return true;
}

char uart0_rxchar(void)
{
    return (char)uart_getchar();
}

char uart0_txchar(char c)
{
    uart_putchar(c);
    return c;
}

int uart0_rxbuffer(char *ptr, int len)
{
    configASSERT(uart_rx_mutex != NULL);
    /* First acquire mutex */
    configASSERT(xSemaphoreTake(uart_rx_mutex, portMAX_DELAY) == pdTRUE);
    for (int idx = 0; idx < len; idx++)
    {
        ptr[idx] = uart_getchar();
    }
    /* Release mutex and return */
    xSemaphoreGive(uart_rx_mutex);
    return len;
}

int uart0_txbuffer(char *ptr, int len)
{
    configASSERT(uart_tx_mutex != NULL);
    /* First acquire mutex */
    configASSERT(xSemaphoreTake(uart_tx_mutex, portMAX_DELAY) == pdTRUE);
    for (int idx = 0; idx < len; idx++)
    {
        uart_putchar(ptr[idx]);
    }
    /* Release mutex and return */
    xSemaphoreGive(uart_tx_mutex);
    return len;
}

void uart0_init(void)
{
    uart_init();
}

void uart_putchar(uint8_t ch)
{
#ifdef __riscv_atomic
    int32_t r;
    do
    {
        __asm__ __volatile__(
            "amoor.w %0, %2, %1\n"
            : "=r"(r), "+A"(uart[UART_REG_TXFIFO])
            : "r"(ch));
    } while (r < 0);
#else
    volatile uint32_t *tx = uart + UART_REG_TXFIFO;
    //  TODO: change this to something better than a busy wait
    while ((int32_t)(*tx) < 0)
        ;

#if VERILATOR_SIM
    // This is not normally needed, but the simulator
    // will exit before the characters actually come out
    // to the shell if you don't insert some wait time
    volatile unsigned int i;
    i = 0;
    while (i < 868)
    {
        i = i + 1;
    }
#endif
    *tx = ch;
#endif
}

int uart_getchar()
{
    int32_t ch = uart[UART_REG_RXFIFO];
    if (ch < 0)
        return -1;
    return ch;
}

void uart_init()
{
    // Enable Rx/Tx channels
    uart = (void *)(uintptr_t)XPAR_UARTNS550_0_BASEADDR;
    uart[UART_REG_TXCTRL] = UART_TXEN;
    // NOTE: 27 is a magic divisor that brings the UART baudrate
    // to the magic baudrate of 3686400 which FireSim uses
    // Running at 100MHz effective rate.
    // More info:
    // FYI on FireSim's UART - the UART device actually communicates with another UART device on the FPGA.
    // That "receiver" has a hard-coded divisor / baud rate. Based on our current builds,
    // this number is derived from the 100MHz "effective" bus frequency
    // with a baud rate of 3686400 to obtain a divisor of 27.
    //
    // TODO: make configurable if necessary
    uart[UART_REG_DIV] = 27;
    uart[UART_REG_RXCTRL] = UART_RXEN;

    uart_tx_mutex = xSemaphoreCreateMutex();
    uart_rx_mutex = xSemaphoreCreateMutex();
}
