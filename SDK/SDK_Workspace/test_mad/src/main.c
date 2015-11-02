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
#include "mad.h"
#include "mp3_parsing.h"
#include "LockFreeFifo.h"

#include "delay.h"

#include "dds.h"

// Params.

#define ENABLE_LOGS 0

#define ENABLE_LOG_NUM_STARVING_SAMPLES 1

#define NUM_OUT_BUFFS 2

#define IN_BUFF_LEN 8192


// Audio stuff.


static volatile u16 tuning_word = 0;


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

// MP3 stuff.

static int in_chunk_cnt = 0;
static int out_chunk_cnt = 0;

static u8* in_buff;

static clock_t read_clks = 0;
static clock_t decode_clks = 0;
static clock_t play_clks = 0;
static clock_t start_decode;

static enum mad_flow input_fun(void *data, struct mad_stream *stream) {
	FRESULT rc;
	WORD br;

	decode_clks += clock() - start_decode;

	(void) data;

#if ENABLE_LOGS
	xil_printf("in_chunk_cnt = %d\n", in_chunk_cnt);
#endif

#if 0
	// Just for debug decode only 10 frames.
	const int num_int_chunks = 10;
	if (in_chunk_cnt == num_int_chunks) {
		xil_printf("read time = %dus\n", clock_t2us(read_clks));
		xil_printf("decode time = %dus\n", clock_t2us(decode_clks));
		xil_printf("decode + read time = %dus\n", clock_t2us(read_clks + decode_clks));
		xil_printf("play time = %dus\n", clock_t2us(play_clks));
		xil_printf("read bandwidth = %dBps\n", (u32)((u64)(num_int_chunks*IN_BUFF_LEN) * CLOCKS_PER_SEC/read_clks));
		return MAD_FLOW_STOP;
	}
#endif

	clock_t start_read = clock();
	// Read buffer.
	rc = pf_read(in_buff, IN_BUFF_LEN, &br);
	// TODO Handle end of file and error on different way.
	// Error or end of file.
	if (rc || br != IN_BUFF_LEN) {
		return MAD_FLOW_STOP;
	}

	// Search for sync word.
	for(int i = 0; i < IN_BUFF_LEN; i++){
		// Possible sync word.
		if(in_buff[i] == 0xff && ((in_buff[i+1] & 0xe0) == 0xe0)){
			xil_printf("\n\nFound 1st\n");

			// To check is it real sync word calculate frame length
			// from header and check is another sync word after header.

			// Big endian.

			xil_printf("in_buff[i] = 0x%02x\n", (int)in_buff[i]);
			xil_printf("in_buff[i+1] = 0x%02x\n", (int)in_buff[i+1]);
			xil_printf("in_buff[i+2] = 0x%02x\n", (int)in_buff[i+2]);
			xil_printf("in_buff[i+3] = 0x%02x\n", (int)in_buff[i+3]);
			MP3HEADER header = parse_mp3_header(&in_buff[i]);
			xil_printf("header.framesync = 0x%04x\n", header.framesync);
			xil_printf("header.MPEGID = %d\n", header.MPEGID);
			xil_printf("header.layer = %d\n", header.layer);
			xil_printf("header.protectbit = %d\n", header.protectbit);

			u32 h32 = in_buff[i+1] << 8 | in_buff[i];
			u16 h16 = *(u16*)&in_buff[i];
			xil_printf("h16 = 0x%04x\n", h16);
			xil_printf("h32 = 0x%08x\n", h32);

			int bitrate = mp3_bitrate(header);
			int sample_freq = mp3_sample_freq(header);
			int frm_len;
			if(header.layer == LAYER1){
				frm_len = (12* bitrate / sample_freq + header.paddingbit) * 4;
			}else{
				frm_len = 144 * bitrate / sample_freq + header.paddingbit;
			}
			xil_printf("i = %d\n", i);
			xil_printf("frm_len = %d\n", frm_len);
			xil_printf("bitrate = %d\n", bitrate);
			xil_printf("header.samplefreq = %d\n", header.samplefreq);
			xil_printf("header.layer = %d\n", header.layer);
			xil_printf("header.paddingbit = %d\n", header.paddingbit);

			// Check for next sync.
			if(in_buff[i + frm_len] == 0xff && ((in_buff[i+1 + frm_len] & 0xe0) == 0xe0)){
				xil_printf("Found 2nd\n");
			}
		}
	}

	return MAD_FLOW_STOP;

	mad_stream_buffer(stream, in_buff, IN_BUFF_LEN);
	read_clks += clock() - start_read;

	in_chunk_cnt++;

	start_decode = clock();

	return MAD_FLOW_CONTINUE;
}

static LockFreeFifo filled_out_buffs;
static LockFreeFifo empty_out_buffs;
#define OUT_BUFF_LEN 1152

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

#if ENABLE_LOG_NUM_STARVING_SAMPLES
static volatile u32 num_starving_samples = 0;
#endif

static enum mad_flow output_fun(void *data, struct mad_header const *header,
		struct mad_pcm *pcm) {
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	decode_clks += clock() - start_decode;

#if ENABLE_LOGS
	xil_printf("out_chunk_cnt = %d\n", out_chunk_cnt);
	xil_printf("tick_48kHz = %d\n", (int)tick_48kHz);
#endif

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

#if ENABLE_LOG_NUM_STARVING_SAMPLES
	if(num_starving_samples){
		xil_printf("num_starving_samples = %d\n", num_starving_samples);
		num_starving_samples = 0;
	}
#endif


#if ENABLE_LOGS
	xil_printf("nsamples = %d\n", nsamples);
#endif

	if (nsamples != OUT_BUFF_LEN) {
		xil_printf("nsamples isn't %d as we expected!\n", OUT_BUFF_LEN);
	}
	play_clks += nsamples;

	s16* out_buff;
	while(true){
		if(LockFreeFifo_get(empty_out_buffs, (LockFreeFifo_elem_t*)&out_buff)){
			break;
		}
		// No empty buffers, wait a little.
#if ENABLE_LOGS
		xil_printf("Decoding halted...\n");
#endif
		delay_us(1);
	}
	for (int s = 0; s < OUT_BUFF_LEN; s++) {
		out_buff[s] = scale(left_ch[s]);
	}

	assert(LockFreeFifo_put(filled_out_buffs, out_buff));

	out_chunk_cnt++;

	start_decode = clock();

	return MAD_FLOW_CONTINUE;
}


static void sample_interrupt_handler(void* baseaddr_p) {
	(void) baseaddr_p;

	static bool handler_called_twiced_hack = false;
	handler_called_twiced_hack = !handler_called_twiced_hack;
	if(handler_called_twiced_hack){
		return;
	}

	tick_48kHz++;

	static u32 out_buff_read_idx = 0;
	static s16* out_buff = NULL;

	static bool filling_fifo_on_start = true;
	if(filling_fifo_on_start){
		if(LockFreeFifo_full(filled_out_buffs)){
			filling_fifo_on_start = false;
		}else{
			*audio_out = 0;
			return;
		}
	}

	// No filled buffer.
	if(!out_buff){
		if(!LockFreeFifo_get(filled_out_buffs, (LockFreeFifo_elem_t*)&out_buff)){
			// No filled buffers.
#if ENABLE_LOG_NUM_STARVING_SAMPLES
			num_starving_samples++;
#endif
			*audio_out = 0;
			return;
		}
	}

	s16 sample = out_buff[out_buff_read_idx++];

	if (out_buff_read_idx == OUT_BUFF_LEN) {
		assert(LockFreeFifo_put(empty_out_buffs, out_buff));
		out_buff = NULL;
		out_buff_read_idx = 0;
		// Next time will use new buffer.
	}

	*audio_out = ((s32) sample) << 8;
}

static enum mad_flow error_fun(void *data, struct mad_stream *stream,
		struct mad_frame *frame) {
	(void) data;

	decode_clks += clock() - start_decode;

#if ENABLE_LOGS
	xil_printf("decoding error 0x%04x (%s) at byte offset %d\n", stream->error,
			mad_stream_errorstr(stream),
			IN_BUFF_LEN * (in_chunk_cnt - 1) + (stream->this_frame - in_buff));
#endif

	start_decode = clock();

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

int main(void) {

	init_platform();

	tuning_word = dds_freq_to_tunning_word(1000, 48000);

	in_buff = (BYTE*) malloc(IN_BUFF_LEN);
	if (!in_buff) {
		xil_printf("Failed to allocate input buffer!");
		return 1;
	}

	filled_out_buffs = LockFreeFifo_create(NUM_OUT_BUFFS);
	empty_out_buffs = LockFreeFifo_create(NUM_OUT_BUFFS);
	for (int b = 0; b < NUM_OUT_BUFFS; b++) {
		void* out_buff = malloc((OUT_BUFF_LEN) * sizeof(s16));
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

	microblaze_enable_interrupts();


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
	rc = pf_open("LINDSE~1.MP3");
	if (rc) {
		xil_printf("Failed opening mp3 file with rc = %d!\n", (int) rc);
		return 1;
	}

	// MP3 stuff.

	xil_printf("Decoding...\n");

	struct mad_decoder decoder;

	/* configure input, output, and error functions */
	mad_decoder_init(&decoder, 0 /* user data */, input_fun, 0 /* header fun */,
			0 /* filter fun */, output_fun, error_fun, 0 /* message fun */
			);

	start_decode = clock();
	/* start decoding */
	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	if (result) {
		xil_printf("Decoding MP3 failed with rc = %d!\n", result);
	}

	/* release the decoder */
	mad_decoder_finish(&decoder);

	// TODO Out buffs cleanup.
	free(in_buff);

	xil_printf("Exiting...\n");

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

