#include "iceblk.h"
#include "task.h"
#include <string.h> /* for memcpy */

// This driver has been adapted from the original Linux driver available here:
// https://github.com/firesim/iceblk-driver/blob/master/iceblk.c

/* Driver instances*/
IceblkDev IceblkDevInstance;

void iceblk_init(void)
{
	printf("iceblk_init\r\n");

	IceblkDevInstance.BaseAddress = ICEBLK_BASEADDR;
	IceblkDevInstance.mutex = xSemaphoreCreateMutex();
	IceblkDevInstance.task_handle = NULL;

	int err = iceblk_setup(&IceblkDevInstance);
	if (err)
	{
		printf("iceblk_init: Device setup failed\n");
		IceblkDevInstance.disk_present = 0;
	}
	else
	{
		printf("iceblk_init: Device setup OK\n");
		IceblkDevInstance.disk_present = 1;
	}
}

int iceblk_setup(IceblkDev *port)
{
	printf("iceblk_setup\r\n");

	configASSERT(PLIC_register_interrupt_handler(&Plic, PLIC_SOURCE_ICEBLK, (void *)iceblk_intr_handler, &IceblkDevInstance));
	printf("iceblk irq handler registered\r\n");

	port->nsectors = ioread32(port->BaseAddress + ICEBLK_NSECTORS);

	if (port->nsectors == 0)
	{
		printf("iceblk: No disk attached.\n");
		return 1;
	}

	port->ntags = ioread8(port->BaseAddress + ICEBLK_NREQUEST);
	port->max_req_len = ioread32(port->BaseAddress + ICEBLK_MAX_REQUEST_LENGTH);
	port->qrunning = 1;

	printf("Iceblk: disk loaded; "
		   "%lu sectors, %lu tags, %lu max request length\n",
		   (long unsigned int) port->nsectors, (long unsigned int) port->ntags, (long unsigned int) port->max_req_len);

	configASSERT(port->max_req_len <= ICEBLK_DEFAULT_MAX_REQUEST_LENGTH);

	return 0;
}

void iceblk_intr_handler(IceblkDev *port)
{
	uint8_t ncomplete = ioread8(port->BaseAddress + ICEBLK_NCOMPLETE);

	for (uint8_t i = 0; i < ncomplete; i++)
	{
		/* Read tag to clear the interrupt, again we assume only one pending request at a time */
		ioread8(port->BaseAddress + ICEBLK_COMPLETE);
	}

	configASSERT(port->task_handle != NULL);
	static BaseType_t askForContextSwitch = pdFALSE;
	vTaskNotifyGiveFromISR(port->task_handle, &askForContextSwitch);
}

/**
 * IceblkDev *port - IceBlk driver instance
 * int write - 0/1 read/write request
 * uint8_t* buffer - source/destination address, depending on R/W request
 * uint32_t len - length of the request [0..16]
 * uint32_t offset - multiple of sector len
 * 
 * Returns:
 * 0 - OK
 * 1 - Device error
 * -1 - timeout occured
 */
int iceblk_queue_request(IceblkDev *port, int write, uint8_t *buffer, uint32_t len, uint32_t offset)
{
	int returnval;

	/* Assume we are writting/reading only one sector at a time */
	configASSERT(len <= port->max_req_len);

	/* Lock the handler */
	configASSERT(port->mutex != NULL);
	configASSERT(xSemaphoreTake(port->mutex, portMAX_DELAY) == pdTRUE);

	if (write == 1)
	{
		/* Copy the contents from source buffer to our intermediary buffer */
		memcpy(port->addr, buffer, ICEBLK_SECTOR_SIZE*len);
	}

	/* Get task handle */
	port->task_handle = xTaskGetCurrentTaskHandle();
	if (ioread8(port->BaseAddress + ICEBLK_NREQUEST) == 0)
	{
		port->qrunning = 0;
		printf("iceblk_queue_request: queue not running, abort!");
		/* Release mutex and return */
		xSemaphoreGive(port->mutex);
		return 1;
	}

	taskENTER_CRITICAL();
	iowrite64((uint64_t)(uintptr_t)port->addr, port->BaseAddress + ICEBLK_ADDR);
	iowrite32(offset, port->BaseAddress + ICEBLK_OFFSET);
	iowrite32(len, port->BaseAddress + ICEBLK_LEN);
	iowrite8(write, port->BaseAddress + ICEBLK_WRITE);
	port->tag = ioread8(port->BaseAddress + ICEBLK_REQUEST);
	taskEXIT_CRITICAL();

	/* Short delay is necessary here */
	msleep(1);

	/* wait for notification */
	/* Note that we use ulTaskNotifyTake() here to reset the calling */
	/* task's notification to Zero after the call                    */
	if (ulTaskNotifyTake (pdTRUE, pdMS_TO_TICKS(ICEBLK_TRANSACTION_DELAY_MS)))
	{
		/* Transaction finished */
		if (write == 0)
		{
			/* Copy the contents from our intermediary buffer to the destination buffer */
			memcpy(buffer, port->addr, ICEBLK_SECTOR_SIZE*len);
		}
		returnval = 0;
	}
	else
	{
		/* timeout occured */
		printf("iceblk_queue_request: timeout\n\n");
		returnval = -1;
	}

	/* Release mutex and return */
	xSemaphoreGive(port->mutex);
	return returnval;
}
