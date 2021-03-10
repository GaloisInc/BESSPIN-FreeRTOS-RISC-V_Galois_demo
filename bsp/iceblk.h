#ifndef ICEBLK_H		/* prevent circular inclusions */
#define ICEBLK_H		/* by using protection macros */

/**
 * FreeRTOS drivers for Firesim IceBlk device
 * Host side source:
 * https://github.com/firesim/firesim/blob/master/sim/firesim-lib/src/main/cc/bridges/blockdev.h
 * https://github.com/firesim/firesim/blob/master/sim/firesim-lib/src/main/cc/bridges/blockdev.cc
 */

#if __riscv_xlen == 64
#error "ICEBLK FreeRTOS drivers are not 64-bit compatible"
#endif

#include "icebase.h"
#include "plic_driver.h"
#include "bsp.h"
#include "semphr.h"


#define ICEBLK_DEFAULT_MAX_REQUEST_LENGTH 16
#define ICEBLK_SECTOR_SIZE 512

#define ICEBLK_ADDR 0
#define ICEBLK_OFFSET 8
#define ICEBLK_LEN 12
#define ICEBLK_WRITE 16
#define ICEBLK_REQUEST 17
#define ICEBLK_NREQUEST 18
#define ICEBLK_COMPLETE 19
#define ICEBLK_NCOMPLETE 20
#define ICEBLK_NSECTORS 24
#define ICEBLK_MAX_REQUEST_LENGTH 28

#define ICEBLK_REQ_READ 0
#define ICEBLK_REQ_WRITE 1

#define ICEBLK_TRANSACTION_DELAY_MS 500

typedef struct IceblkDev {
    UINTPTR BaseAddress; /** HW Base Address **/
	SemaphoreHandle_t mutex;  /* Mutex for queue acquisition */
	int qrunning; /* Is queue running? */
	int disk_present; /* Is the disk present? */
	uint32_t nsectors; /* Disk capacity */
	uint32_t max_req_len; /* Max request len, typically 16 */
	uint32_t ntags;
	volatile TaskHandle_t task_handle; /* handle for task that initiated a transaction */
	volatile uint8_t tag;
	/* The buffer has to be sector aligned, because of the iceblk's hardware implementation.
	CacheBlockBytes in rocket chip defaults to 64-bytes, hence the 64-byre aligment.
	Check iceblk scala files for more details. */
	uint8_t addr[ICEBLK_SECTOR_SIZE*ICEBLK_DEFAULT_MAX_REQUEST_LENGTH] __attribute__((aligned(64)));
} IceblkDev;

void iceblk_init(void);
int iceblk_setup(IceblkDev *port);
int iceblk_queue_request(IceblkDev *port, int write, uint8_t *addr, uint32_t len, uint32_t offset);
void iceblk_intr_handler(IceblkDev *port);

extern IceblkDev IceblkDevInstance;

#endif // ICEBLK_H
