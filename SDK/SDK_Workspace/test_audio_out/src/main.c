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
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"

#include "xintc.h"

#include "dds.h"

static volatile u32* audio_out = (volatile u32*)XPAR_AUDIO_OUT_BASEADDR;

static volatile u64 tick_48kHz = 0;

static volatile u16 tuning_word = 0;

static void sample_interrupt_handler(void* baseaddr_p) {
	(void) baseaddr_p;
	tick_48kHz++;

	*audio_out = ((s32)dds_next_sample(tuning_word)) << 16;
}

static XIntc intc;

int main() {
	XStatus status;

	init_platform();

	status = XIntc_Initialize(&intc, XPAR_INTC_0_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("Intc failed with status = %d\n", status);
	}

	status = XIntc_Connect(&intc, XPAR_AXI_INTC_0_AUDIO_OUT_O_SAMPLE_INTERRUPT_INTR, sample_interrupt_handler, 0);
	if(status != XST_SUCCESS){
		xil_printf("Failed to connect sample_interrupt with status = %d\n", status);
	}

	status = XIntc_Start(&intc, XIN_REAL_MODE);
	if(status != XST_SUCCESS){
		xil_printf("Failed to start intc with status = %d\n", status);
	}

	XIntc_Enable(&intc, XPAR_AXI_INTC_0_AUDIO_OUT_O_SAMPLE_INTERRUPT_INTR);

	xil_printf("audio_out = 0x%08x\n", *audio_out);

	tuning_word = dds_freq_to_tunning_word(1000, 48000);
	xil_printf("tuning_word = %d\n", tuning_word);

	microblaze_enable_interrupts();

	while(1){
		//*audio_out = ((s32)(tick_48kHz % 2048) - 1024) << 11;
	}

	xil_printf("\nTest completed.\n");

	return 0;
}
