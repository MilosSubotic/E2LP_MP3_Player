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
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"

#include "pff.h"
#include "mad.h"

#define IN_BUFF_LEN 32768

static int in_chunk_cnt = 0;
static int out_chunk_cnt = 0;

BYTE* in_buff;

static enum mad_flow input_fun(void *data, struct mad_stream *stream) {
	FRESULT rc;
	WORD br;

	(void)data;

	//xil_printf("in_chunk_cnt = %d\n", in_chunk_cnt);

	// Read buffer.
	rc = pf_read(in_buff, sizeof(in_buff), &br);
	// Error or end of file.
	if (rc || !br){
		return MAD_FLOW_STOP;
	}

	mad_stream_buffer(stream, in_buff, sizeof(in_buff));

	in_chunk_cnt++;

	return MAD_FLOW_CONTINUE;
}

static enum mad_flow output_fun(
	void *data,
	struct mad_header const *header,
	struct mad_pcm *pcm
) {
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	xil_printf("out_chunk_cnt = %d\n", out_chunk_cnt);
	out_chunk_cnt++;

	return MAD_FLOW_CONTINUE;
}

static
enum mad_flow error_fun(
	void *data,
	struct mad_stream *stream,
	struct mad_frame *frame
){
	(void)data;

	xil_printf(
		"decoding error 0x%04x (%s) at byte offset %d\n",
		stream->error, mad_stream_errorstr(stream),
		IN_BUFF_LEN*in_chunk_cnt + (stream->this_frame - in_buff)
	);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

#include "decoder.h"

int main (void)
{
	FATFS fatfs;			/* File system object */
	FRESULT rc;

    init_platform();

	xil_printf("\nMounting a volume...\n");
	rc = pf_mount(&fatfs);
	if(rc){
		xil_printf("Failed mounting volume with rc = %d!\n", (int)rc);
		return 1;
	}

	xil_printf("\nOpening a mp3 file...\n");
	rc = pf_open("LINDSE~1.MP3");
	if(rc){
		xil_printf("Failed opening mp3 file with rc = %d!\n", (int)rc);
		return 1;
	}

	in_buff = (BYTE*)malloc(IN_BUFF_LEN);
	if(!in_buff){
		xil_printf("Failed to allocate input buffer!");
		return 1;
	}

	struct mad_decoder decoder;

	/* configure input, output, and error functions */
	mad_decoder_init(
		&decoder,
		0 /* user data */,
		input_fun,
		0 /* header fun */,
		0 /* filter fun */,
		output_fun,
		error_fun,
		0 /* message fun */
	);

	/* start decoding */
	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	if(result){
		xil_printf("Decoding MP3 failed with rc = %d!", result);
	}

	/* release the decoder */
	mad_decoder_finish(&decoder);

	free(in_buff);

	xil_printf("\nTest completed.\n");

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

