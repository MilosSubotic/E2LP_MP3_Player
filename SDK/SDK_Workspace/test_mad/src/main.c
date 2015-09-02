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
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"

#include "pff.h"
#include "mad.h"

// Params.

#define IN_BUFF_LEN 4096

// Audio stuff.

static volatile u32* audio_out = (volatile u32*) XPAR_AUDIO_OUT_BASEADDR;

static volatile clock_t tick_48kHz = 0;
static inline clock_t clock(void) {
	return tick_48kHz;
}
#define CLOCKS_PER_SEC 48000
static inline u32 clock_t2us(clock_t c) {
	return (u32)(((u64)(c))*1000000/CLOCKS_PER_SEC);
}

static volatile u16 tuning_word = 0;

static s16 get_next_sample(void);

static void sample_interrupt_handler(void* baseaddr_p) {
	(void) baseaddr_p;
	tick_48kHz++;

	*audio_out = ((s32) get_next_sample()) << 8;
}

static XIntc intc;

// MP3 stuff.

static int in_chunk_cnt = 0;
static int out_chunk_cnt = 0;

static BYTE* in_buff;

static clock_t read_clks = 0;
static clock_t decode_clks = 0;
static clock_t play_clks = 0;
static clock_t start_decode;

static enum mad_flow input_fun(void *data, struct mad_stream *stream) {
	FRESULT rc;
	WORD br;

	decode_clks += clock() - start_decode;

	(void) data;

	xil_printf("in_chunk_cnt = %d\n", in_chunk_cnt);

	// TODO Just for debug decode only one frame.
	if (in_chunk_cnt == 10) {
		xil_printf("read time = %dus\n", clock_t2us(read_clks));
		xil_printf("decode time = %dus\n", clock_t2us(decode_clks));
		xil_printf("play time = %dus\n", clock_t2us(play_clks));
		return MAD_FLOW_STOP;
	}

	clock_t start_read = clock();
	// Read buffer.
	rc = pf_read(in_buff, IN_BUFF_LEN, &br);
	// Error or end of file.
	if (rc || br != IN_BUFF_LEN) {
		return MAD_FLOW_STOP;
	}
	mad_stream_buffer(stream, in_buff, IN_BUFF_LEN);
	read_clks += clock() - start_read;

	in_chunk_cnt++;

	start_decode = clock();

	return MAD_FLOW_CONTINUE;
}

#define OUT_BUFF_LEN 1152
#define NUM_OUT_BUFF 2
static s16* out_buffs[NUM_OUT_BUFF];
volatile static bool out_buffs_empty[NUM_OUT_BUFF];
volatile static int current_out_buff = -1;
volatile static u32 out_buff_read_idx = 0;

static inline s16 scale(mad_fixed_t sample) {
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE )
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE )
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output_fun(void *data, struct mad_header const *header,
		struct mad_pcm *pcm) {
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	decode_clks += clock() - start_decode;

	xil_printf("out_chunk_cnt = %d\n", out_chunk_cnt);
	xil_printf("tick_48kHz = %d\n", (int)tick_48kHz);

	xil_printf("current_out_buff = %d\n", current_out_buff);
	xil_printf("out_buff_read_idx = %d\n", out_buff_read_idx);

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	xil_printf("nsamples = %d\n", nsamples);

	if (nsamples != OUT_BUFF_LEN) {
		xil_printf("nsamples isn't %d as we expected!\n", OUT_BUFF_LEN);
	}
	play_clks += nsamples;

	uint num_empty = 0;
	for (int b = 0; b < NUM_OUT_BUFF; b++) {
		xil_printf("out_buffs_empty[%d] = %d\n", b, out_buffs_empty[b]);
		if (out_buffs_empty[b]) {
			num_empty++;
		}
	}
	if (num_empty == NUM_OUT_BUFF) {
		xil_printf("All buffers empty! Audio output starving!\n");
	} else if (num_empty == 0) {
		xil_printf("All buffers full! Hold on with decoding!\n");
		// Busy wait for any empty buffer.
		bool some_buff_empty = false;
		while (!some_buff_empty) {
			for (int b = 0; b < NUM_OUT_BUFF; b++) {
				if (out_buffs_empty[b]) {
					some_buff_empty = true;
					break;
				}
			}
		}

	}
	for (int b = 0; b < NUM_OUT_BUFF; b++) {
		// Fill up first empty buffer.
		if (out_buffs_empty[b]) {
			s16* out_buff = out_buffs[b];
			for (int s = 0; s < OUT_BUFF_LEN; s++) {
				out_buff[s] = scale(left_ch[s]);
			}
			out_buffs_empty[b] = false;
			break;
		}
	}

/*
	// TODO Debug stop on first buffer.
	return MAD_FLOW_STOP;
*/
/*
	 if(out_chunk_cnt == 2){
	 xil_printf("left_ch\n");
	 for(int i = 0; i < 10; i++){
	 xil_printf("0x%08x\n", left_ch[i]);
	 }
	 return MAD_FLOW_STOP;
	 }
*/
	out_chunk_cnt++;

	start_decode = clock();

	return MAD_FLOW_CONTINUE;
}

s16 get_next_sample(void) {
	if (current_out_buff == -1) {
		// Find first filled buffer.
		for (int b = 0; b < NUM_OUT_BUFF; b++) {
			if (!out_buffs_empty[b]) {
				current_out_buff = b;
				break;
			}
		}
		// Still there is no decoded samples. Come back later
		if (current_out_buff == -1) {
			return 0;
		}
	}

	s16* out_buff = out_buffs[current_out_buff];

	s16 sample = out_buff[out_buff_read_idx++];

	if (out_buff_read_idx == OUT_BUFF_LEN) {
		out_buffs_empty[current_out_buff] = true;
		current_out_buff = -1;
		out_buff_read_idx = 0;
		// Next time will use new buffer.
	}

	return sample;
}

static enum mad_flow error_fun(void *data, struct mad_stream *stream,
		struct mad_frame *frame) {
	(void) data;

	decode_clks += clock() - start_decode;

	xil_printf("decoding error 0x%04x (%s) at byte offset %d\n", stream->error,
			mad_stream_errorstr(stream),
			IN_BUFF_LEN * (in_chunk_cnt - 1) + (stream->this_frame - in_buff));

	start_decode = clock();

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

int main(void) {

	init_platform();

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

	xil_printf("audio_out = 0x%08x\n", *audio_out);

	microblaze_enable_interrupts();

/*
	// Should be something like 1ms
	xil_printf("%dus\n", clock()*1000000/CLOCKS_PER_SEC);
	for(int i = 0; i < 100000; i++){
	}
	xil_printf("%d us\n", clock()*1000000/CLOCKS_PER_SEC);
*/

	// SD card stuff.

	FATFS fatfs; /* File system object */
	FRESULT rc;

	xil_printf("\nMounting a volume...\n");
	rc = pf_mount(&fatfs);
	if (rc) {
		xil_printf("Failed mounting volume with rc = %d!\n", (int) rc);
		return 1;
	}

	xil_printf("\nOpening a mp3 file...\n");
	rc = pf_open("LINDSE~1.MP3");
	if (rc) {
		xil_printf("Failed opening mp3 file with rc = %d!\n", (int) rc);
		return 1;
	}

	in_buff = (BYTE*) malloc(IN_BUFF_LEN);
	if (!in_buff) {
		xil_printf("Failed to allocate input buffer!");
		return 1;
	}

	for (int b = 0; b < NUM_OUT_BUFF; b++) {
		out_buffs_empty[b] = true;
		out_buffs[b] = (s16*) malloc(OUT_BUFF_LEN * sizeof(s16));
		if (!out_buffs[b]) {
			xil_printf("Failed to allocate output buffer!");
			return 1;
		}
	}

	// MP3 stuff.

	struct mad_decoder decoder;

	/* configure input, output, and error functions */
	mad_decoder_init(&decoder, 0 /* user data */, input_fun, 0 /* header fun */,
			0 /* filter fun */, output_fun, error_fun, 0 /* message fun */
			);

	start_decode = clock();
	/* start decoding */
	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	if (result) {
		xil_printf("Decoding MP3 failed with rc = %d!", result);
	}

	/* release the decoder */
	mad_decoder_finish(&decoder);

	for (int b = 0; b < NUM_OUT_BUFF; b++) {
		free(out_buffs[b]);
	}
	free(in_buff);

	xil_printf("\nTest completed.\n");

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

