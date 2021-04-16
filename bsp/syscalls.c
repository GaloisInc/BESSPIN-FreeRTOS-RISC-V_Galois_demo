#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/time.h>
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

void *_sbrk(int nbytes);
int _write(int file, char *ptr, int len);
int _close(int fd);
int _fstat(int fd, void *buffer);
long _lseek(int fd, long offset, int origin);
int _read(int fd, void *buffer, unsigned int count);
int _isatty(int fd);
int _kill(int pid, int sig);
int _getpid(int n);
void _exit(int n);

#if PLATFORM_QEMU_VIRT
void vTerminate(int32_t lExitCode);
#endif

void _exit(int n) {
    (void)n;
    #if PLATFORM_QEMU_VIRT
        vTerminate( n );
    #endif
    configASSERT(0);
}

void *_sbrk(int nbytes)
{
    (void)nbytes;
    errno = ENOMEM;
    return (void *)-1;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    return uart0_txbuffer(ptr, len);
}

int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

long _lseek(int fd, long offset, int origin)
{
    (void)fd;
    (void)offset;
    (void)origin;
    errno = EBADF;
    return -1;
}

int _read(int fd, void *buffer, unsigned int count)
{
    (void)fd;
    (void)buffer;
    (void)count;
    errno = EBADF;
    return -1;
}

int _fstat(int fd, void *buffer)
{
    (void)fd;
    (void)buffer;
    errno = EBADF;
    return -1;
}

int _isatty(int fd)
{
    (void)fd;
    errno = EBADF;
    return 0;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EBADF;
    return -1;
}

int _getpid(int n)
{
    (void)n;
    return 1;
}

#if PLATFORM_QEMU_VIRT
#ifndef SIFIVE_TEST_BASE
    #define SIFIVE_TEST_BASE    0x100000
#endif

#define TEST_PASS               0x5555
#define TEST_FAIL               0x3333

void vTerminate( int32_t lExitCode )
{
    volatile uint32_t * sifive_test = ( uint32_t * ) SIFIVE_TEST_BASE;
    uint32_t test_command = TEST_PASS;

    if( lExitCode != 0 )
    {
        test_command = ( lExitCode << 16 ) | TEST_FAIL;
    }

    while( sifive_test != NULL )
    {
        *sifive_test = test_command;
    }
}
#endif