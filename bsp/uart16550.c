/* See LICENSE for license details. */

#include <string.h>
#include <stdint.h>
#include <FreeRTOSConfig.h>
#include "uart16550.h"
#include "uart.h"

#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t uart_tx_mutex; /* Mutex for TX transmissions */
SemaphoreHandle_t uart_rx_mutex; /* Mutex for RX transmissions */

#define UART_REG_QUEUE        0
#define UART_REG_LINESTAT     ( 5 )
#define UART_REG_STATUS_RX    ( 0x01 )
#define UART_REG_STATUS_TX    ( 0x20 )

#if UART16550_REGSHIFT == 1
    volatile uint8_t* uart16550;
    typedef volatile uint8_t* uart_mmio_t;
#elif UART16550_REGSHIFT == 2
    volatile uint32_t* uart16550;
    typedef volatile uint32_t* uart_mmio_t;
#else
    #error "Unsupported uart reg_shift value"
#endif


bool uart0_rxready(void)
{
    return true;
}

char uart0_rxchar(void)
{
    return (char)uart16550_getchar();
}

char uart0_txchar(char c)
{
    uart16550_putchar(c);
    return c;
}

int uart0_rxbuffer(char *ptr, int len)
{
    configASSERT(uart_rx_mutex != NULL);
    /* First acquire mutex */
    configASSERT(xSemaphoreTake(uart_rx_mutex, portMAX_DELAY) == pdTRUE);
    for (int idx = 0; idx < len; idx++)
    {
        ptr[idx] = uart0_rxchar();
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
    for( int i = 0; i < len; i++ )
    {
        uart16550_putchar( ptr[ i ] );

        if( ptr[ i ] == '\n' )
        {
            uart16550_putchar( '\r' );
        }
    }

    /* Release mutex and return */
    xSemaphoreGive(uart_tx_mutex);
    return len;
}

void uart0_init(void)
{
    uart16550_init(UART16550_BASE);
}


void uart16550_putchar( uint8_t ch )
{
    while( ( uart16550[ UART_REG_LINESTAT ] & UART_REG_STATUS_TX ) == 0 )
    {
    }

    uart16550[ UART_REG_QUEUE ] = ch;
}

int uart16550_getchar()
{
    if( uart16550[ UART_REG_LINESTAT ] & UART_REG_STATUS_RX )
    {
        return uart16550[ UART_REG_QUEUE ];
    }

    return -1;
}


void uart16550_init( unsigned long base )
{
    uart16550 = ( uart_mmio_t ) base;

    uint32_t divisor;
    divisor = configPERIPH_CLOCK_HZ / ( 16 * UART16550_BAUD );

    /* http://wiki.osdev.org/Serial_Ports */
    uart16550[ 1 ] = 0x00;                    /* Disable all interrupts */
    uart16550[ 3 ] = 0x80;                    /* Enable DLAB (set baud rate divisor) */
    uart16550[ 0 ] = divisor & 0xff;          /* Set divisor (lo byte) baud */
    uart16550[ 1 ] = ( divisor >> 8 ) & 0xff; /* Set divisor to (hi byte) baud */
    uart16550[ 3 ] = 0x03;                    /* 8 bits, no parity, one stop bit */
    uart16550[ 2 ] = 0xC7;                    /* Enable FIFO, clear them, with 14-byte threshold */

    uart_tx_mutex = xSemaphoreCreateMutex();
    uart_rx_mutex = xSemaphoreCreateMutex();
}
