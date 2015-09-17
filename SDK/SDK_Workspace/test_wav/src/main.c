/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"

#include "pff.h"
#include "LockFreeFifo.h"

#include "delay.h"

#include "dds.h"

// Params.

#define ENABLE_LOGS 0

#define ENABLE_LOG_NUM_STARVING_SAMPLES 0

#define ENABLE_DDS 0

#define ENABLE_U8_DISABLE_S16_SAMPLES 1

#define NUM_OUT_BUFFS 2


#if ENABLE_U8_DISABLE_S16_SAMPLES
#define WAV_FILE_NAME "LINDSE~1.WAV"
#else
#define WAV_FILE_NAME "LINDSE~2.WAV"
#endif



// Audio stuff.

#if ENABLE_U8_DISABLE_S16_SAMPLES
typedef u8 sample_t;
#else
typedef s16 sample_t;
#endif

#if ENABLE_DDS
static volatile u16 tuning_word = 0;
#endif

static volatile u32* audio_out = (volatile u32*) XPAR_AUDIO_OUT_BASEADDR;

static volatile clock_t tick_48kHz = 0;
static inline clock_t clock(void) {
	return tick_48kHz;
}
#define CLOCKS_PER_SEC 48000
static inline u32 clock_t2us(clock_t c) {
	return (u32)(((u64)(c))*1000000/CLOCKS_PER_SEC);
}

static XIntc intc;

// Buffer stuff.

static LockFreeFifo filled_out_buffs;
static LockFreeFifo empty_out_buffs;
#define OUT_BUFF_LEN 1024


#if ENABLE_LOG_NUM_STARVING_SAMPLES
static volatile u32 num_starving_samples = 0;
#endif

static void sample_interrupt_handler(void* baseaddr_p) {
	(void) baseaddr_p;

	static bool handler_called_twiced_hack = false;
	handler_called_twiced_hack = !handler_called_twiced_hack;
	if(handler_called_twiced_hack){
		return;
	}

	tick_48kHz++;

	static u32 out_buff_read_idx = 0;
	static sample_t* out_buff = NULL;

	// No filled buffer.
	if(!out_buff){
		if(!LockFreeFifo_get(filled_out_buffs, (LockFreeFifo_elem_t*)&out_buff)){
			// No filled buffers.
#if ENABLE_LOG_NUM_EMPTY_SAMPLES
			num_num_starving_samples++;
#endif
			*audio_out = 0;
			return;
		}
	}

#if ENABLE_U8_DISABLE_S16_SAMPLES
	s16 sample = ((u16)out_buff[out_buff_read_idx++]-128) << 8;
#else
	s16 sample = out_buff[out_buff_read_idx++];
#endif

	if (out_buff_read_idx == OUT_BUFF_LEN) {
		assert(LockFreeFifo_put(empty_out_buffs, out_buff));
		out_buff = NULL;
		out_buff_read_idx = 0;
		// Next time will use new buffer.
	}

	*audio_out = ((s32) sample) << 8;
}


int main(void) {

	init_platform();

#if ENABLE_DDS
	tuning_word = dds_freq_to_tunning_word(1000, 48000);
#endif

	filled_out_buffs = LockFreeFifo_create(NUM_OUT_BUFFS);
	empty_out_buffs = LockFreeFifo_create(NUM_OUT_BUFFS);
	for (int b = 0; b < NUM_OUT_BUFFS; b++) {
		void* out_buff = malloc((OUT_BUFF_LEN) * sizeof(sample_t));
		if (!out_buff) {
			xil_printf("Failed to allocate output buffer!");
			return 1;
		}
		assert(LockFreeFifo_put(empty_out_buffs, out_buff));
	}


	// Audio out stuff.

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

#if ENABLE_LOGS
	xil_printf("audio_out = 0x%08x\n", *audio_out);
#endif

	microblaze_enable_interrupts();

#if 0
	// Testing delay_us()
	clock_t start, end;

	start = clock();
	delay_us(1000);
	end = clock();
	// 48 kHz - 48 tick is 1 ms
	xil_printf("ticks = %d\n", end - start);
	xil_printf("time = %dus\n", clock_t2us(end - start));

	start = clock();
	delay_us(10000);
	end = clock();
	// 48 kHz - 480 tick is 10 ms
	xil_printf("ticks = %d\n", end - start);
	xil_printf("time = %dus\n", clock_t2us(end - start));
#endif


	// SD card stuff.

	FATFS fatfs; /* File system object */
	FRESULT rc;

	xil_printf("Mounting a volume...\n");
	rc = pf_mount(&fatfs);
	if (rc) {
		xil_printf("Failed mounting volume with rc = %d!\n", (int) rc);
		return 1;
	}

	xil_printf("Opening a mp3 file...\n");
	rc = pf_open(WAV_FILE_NAME);
	if (rc) {
		xil_printf("Failed opening mp3 file with rc = %d!\n", (int) rc);
		return 1;
	}


	// WAV playing.

	xil_printf("Playing...\n");

	while(true){
		void* out_buff;
		while (true) {
			if (LockFreeFifo_get(empty_out_buffs,
					(LockFreeFifo_elem_t*) &out_buff)) {
				break;
			}
			// No empty buffers, wait a little.
#if ENABLE_LOGS
			xil_printf("Decoding halted...\n");
#endif
			delay_us(1);
		}
#if ENABLE_DDS
		for (int s = 0; s < OUT_BUFF_LEN; s++) {
#if ENABLE_U8_DISABLE_S16_SAMPLES
			((u8*)out_buff)[s] = dds_next_sample(tuning_word) + 128;
#else
			((s16*)out_buff)[s] = ((s16) dds_next_sample(tuning_word)) << 8;
#endif
		}
#else
		WORD br;
		rc = pf_read(out_buff, OUT_BUFF_LEN*sizeof(sample_t), &br);
		if (rc) {
			xil_printf("Failed reading mp3 file with rc = %d!\n", (int) rc);
			return 1;
		}
		if(br != OUT_BUFF_LEN) {
			break; // Exiting.
		}
#endif

		assert(LockFreeFifo_put(filled_out_buffs, out_buff));
	}


	xil_printf("\nExiting...\n");

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

