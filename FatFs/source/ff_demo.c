/*----------------------------------------------------------------------*/
/* Petit FatFs sample project for generic uC  (C)ChaN, 2010             */
/* From: https://github.com/pabigot/FatFs/tree/pf/master                */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include "ff.h" /* Declarations of FatFs API */
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

FATFS FatFs; /* FatFs work area needed for each volume */
FIL Fil;     /* File object needed for each open file */

void sd_demo(void);

/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/

void sd_demo(void)
{
    UINT bw;
    FRESULT res;

    printf("Mounting filesystem\r\n");
    f_mount(&FatFs, "", 0); /* Give a work area to the default drive */

    printf("Opening file\r\n");
    if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    { /* Create a file */
        printf("Writing to file\r\n");
        res = f_write(&Fil, "It works!\r\n", 11, &bw); /* Write data to the file */
        printf("Written %u bytes, result = %u\r\n", bw, res);
        f_close(&Fil); /* Close the file */
        printf("File closed, write OK\r\n");
    }
    else
    {
        printf("Opening file failed\r\n");
    }

    printf("Opening file\r\n");
    if (f_open(&Fil, "newfile.txt", FA_READ) == FR_OK)
    { /* Create a file */
        printf("Reading from file\r\n");
        char readbuf[32];
        res = f_read(&Fil, readbuf, sizeof(readbuf), &bw);
        readbuf[bw] = '\0'; /* Terminate the string for easy printing */
        printf("Read %u bytes, result = %u, content: %s\r\n", bw, res,readbuf);
        f_close(&Fil); /* Close the file */
        printf("File closed, read OK\r\n");
    }
    else
    {
        printf("Opening file failed\r\n");
    }

    printf("Demo done!\r\n");
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
