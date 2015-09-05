/*----------------------------------------------------------------------*/
/* Petit FatFs sample project for generic uC  (C)ChaN, 2010             */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include "pff.h"

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"

static volatile clock_t tick_48kHz = 0;
static inline clock_t clock(void) {
	return tick_48kHz;
}
#define CLOCKS_PER_SEC 48000
static inline u32 clock_t2us(clock_t c) {
	return (u32) (((u64) (c)) * 1000000 / CLOCKS_PER_SEC);
}

static void sample_interrupt_handler(void* baseaddr_p) {
	(void) baseaddr_p;
	tick_48kHz++;
}

static XIntc intc;

#define die(rc) \
	do{ \
		xil_printf("Failed with rc=%d.\n", (int)rc); \
		return 1; \
	}while(0)

/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/

int main(void) {

	init_platform();

	// Timer stuff.

	XStatus status;

	status = XIntc_Initialize(&intc, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Intc failed with status = %d\n", status);
	}

	status = XIntc_Connect(&intc,
			XPAR_AXI_INTC_0_AUDIO_OUT_O_SAMPLE_INTERRUPT_INTR,
			sample_interrupt_handler, 0);
	if (status != XST_SUCCESS) {
		xil_printf("Failed to connect sample_interrupt with status = %d\n",
				status);
	}

	status = XIntc_Start(&intc, XIN_REAL_MODE);
	if (status != XST_SUCCESS) {
		xil_printf("Failed to start intc with status = %d\n", status);
	}

	XIntc_Enable(&intc, XPAR_AXI_INTC_0_AUDIO_OUT_O_SAMPLE_INTERRUPT_INTR);

	microblaze_enable_interrupts();

	// PFF stuff.

	FATFS fatfs; /* File system object */
	DIR dir; /* Directory object */
	FILINFO fno; /* File information object */
	WORD bw, br, i;
	BYTE buff[64];
	FRESULT rc;

	xil_printf("Mounting a volume...\n");
	rc = pf_mount(&fatfs);
	if (rc)
		die(rc);

	xil_printf("Opening a test file \"MESSAGE.TXT\"...\n");
	rc = pf_open("MESSAGE.TXT");
	if (rc)
		die(rc);

	xil_printf("File content:\n");
	for (;;) {
		rc = pf_read(buff, sizeof(buff), &br);	// Read a chunk of file
		if (rc || !br)
			break;			// Error or end of file
		for (i = 0; i < br; i++) {		// Type the data
			xil_printf("%c", buff[i]);
		}
	}
	xil_printf("\n");
	if (rc)
		die(rc);

#if 0
#if _USE_WRITE
	xil_printf("Open a file \"WRITE.TXT\" for writing...\n");
	rc = pf_open("WRITE.TXT");
	if (rc) die(rc);

	xil_printf("Writing \"Hello world!\" a text data...\n");
	for (;;) {
		rc = pf_write("Hello world!\r\n", 14, &bw);
		if (rc || !bw) break;
	}
	if (rc) die(rc);

	xil_printf("Terminating the file write process...\n");
	rc = pf_write(0, 0, &bw);
	if (rc) die(rc);
#endif
#endif

#if _USE_DIR
	xil_printf("Open root directory...\n");
	rc = pf_opendir(&dir, "");
	if (rc)
		die(rc);

	xil_printf("Directory listing:\n");
	for (;;) {
		rc = pf_readdir(&dir, &fno);	// Read a directory item
		if (rc || !fno.fname[0])
			break;	// Error or end of dir
		if (fno.fattrib & AM_DIR)
			xil_printf("   <dir>  %s\n", fno.fname);
		else
			xil_printf("%8d  %s\n", fno.fsize, fno.fname);
	}
	if (rc)
		die(rc);
#endif

	// Test reading speed.

#if 1
	xil_printf("Testing reading speed...\n");
	xil_printf("Opening a mp3 file...\n");
	rc = pf_open("LINDSE~1.MP3");
	if (rc) {
		xil_printf("Failed opening mp3 file with rc = %d!\n", (int) rc);
		return 1;
	}

	static BYTE read_buff[4096];
	int read_bytes = 0;
//#define CHECKSUM
#ifdef CHECKSUM
	u8 checksum = 0;
#endif

	xil_printf("Reading mp3 file...\n");
	clock_t start = clock();
	for (i = 0; i < 10; i++) {
		rc = pf_read(read_buff, sizeof(read_buff), &br); /* Read a chunk of file */
		if (rc) {
			xil_printf("Error while reading mp3 file with rc = %d!\n", (int) rc);
			return 1;
		}
		if (!br) {
			break;
		}
		read_bytes += br;

#ifdef CHECKSUM
		for(int b = 0; b < br; b++){
			checksum += read_buff[b];
		}
#endif
	}
	clock_t end = clock();
	clock_t read_clks = end - start;
	xil_printf("read time = %dus\n", clock_t2us(read_clks));
	xil_printf("read bytes = %dB\n", read_bytes);
	xil_printf("read bandwidth = %dBps\n", (u32)((u64)read_bytes * CLOCKS_PER_SEC/read_clks));
#ifdef CHECKSUM
	xil_printf("checksum = 0x%02x\n", checksum);
#endif

#endif
	xil_printf("Test completed.\n");

	return 0;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime(void) {
	return ((DWORD) (2010 - 1980) << 25) /* Fixed to Jan. 1, 2010 */
	| ((DWORD) 1 << 21) | ((DWORD) 1 << 16) | ((DWORD) 0 << 11)
			| ((DWORD) 0 << 5) | ((DWORD) 0 >> 1);
}
