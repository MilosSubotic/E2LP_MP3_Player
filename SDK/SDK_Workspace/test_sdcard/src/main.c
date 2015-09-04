/*----------------------------------------------------------------------*/
/* Petit FatFs sample project for generic uC  (C)ChaN, 2010             */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include "pff.h"

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"


#define die(rc) \
	do{ \
		xil_printf("Failed with rc=%d.\n", (int)rc); \
		return 1; \
	}while(0)



/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/

int main (void)
{
	FATFS fatfs;			/* File system object */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	WORD bw, br, i;
	BYTE buff[64];
	FRESULT rc;

    init_platform();

	xil_printf("\nMount a volume.\n");
	rc = pf_mount(&fatfs);
	if (rc) die(rc);

	xil_printf("\nOpen a test file (message.txt).\n");
	rc = pf_open("MESSAGE.TXT");
	if (rc) die(rc);

	xil_printf("File content:\n");
	for (;;) {
		rc = pf_read(buff, sizeof(buff), &br);	// Read a chunk of file
		xil_printf("\nbr = %d\n", br);
		if (rc || !br) break;			// Error or end of file
		for (i = 0; i < br; i++) {		// Type the data
			xil_printf("%c", buff[i]);
		}
	}
	if (rc) die(rc);

/*
#if _USE_WRITE
	xil_printf("\nOpen a file to write (write.txt).\n");
	rc = pf_open("WRITE.TXT");
	if (rc) die(rc);

	xil_printf("\nWrite a text data. (Hello world!)\n");
	for (;;) {
		rc = pf_write("Hello world!\r\n", 14, &bw);
		if (rc || !bw) break;
	}
	if (rc) die(rc);

	xil_printf("\nTerminate the file write process.\n");
	rc = pf_write(0, 0, &bw);
	if (rc) die(rc);
#endif
*/

#if _USE_DIR
	xil_printf("\nOpen root directory.\n");
	rc = pf_opendir(&dir, "");
	if (rc) die(rc);

	xil_printf("\nDirectory listing...\n");
	for (;;) {
		rc = pf_readdir(&dir, &fno);	// Read a directory item
		if (rc || !fno.fname[0]) break;	// Error or end of dir
		if (fno.fattrib & AM_DIR)
			xil_printf("   <dir>  %s\n", fno.fname);
		else
			xil_printf("%8d  %s\n", fno.fsize, fno.fname);
	}
	if (rc) die(rc);
#endif

	// Test reading speed.

#if 0
	xil_printf("\nOpening a 1MiB file...\n");
	rc = pf_open("1MiB.dat");
	BYTE buff2[1024];
	if (rc) die(rc);
	for (i = 0;; i++) {
		rc = pf_read(buff2, sizeof(buff2), &br);	/* Read a chunk of file */
		if (rc || !br) break;			/* Error or end of file */
		//xil_printf("%d\n", i);
	}
	if (rc) die(rc);

	xil_printf("\nTest completed.\n");
#endif

	return 0;
}



/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime (void)
{
	return	  ((DWORD)(2010 - 1980) << 25)	/* Fixed to Jan. 1, 2010 */
			| ((DWORD)1 << 21)
			| ((DWORD)1 << 16)
			| ((DWORD)0 << 11)
			| ((DWORD)0 << 5)
			| ((DWORD)0 >> 1);
}
