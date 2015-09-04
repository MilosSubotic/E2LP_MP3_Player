/*------------------------------------------------------------------------/
/  Bitbanging MMCv3/SDv1/SDv2 (in SPI mode) control module for PFF
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/--------------------------------------------------------------------------/
 Features:

 * Very Easy to Port
   It uses only 4-6 bit of GPIO port. No interrupt, no SPI port is used.

 * Platform Independent
   You need to modify only a few macros to control GPIO ports.

/-------------------------------------------------------------------------*/


#include "diskio.h"

#define AXI_SPI 1

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#ifdef AXI_SPI

#include "xspi.h"
#include "xparameters.h"

#include "delay.h"

#include <string.h>

static XSpi Spi;

#define DLY_US(n)	delay_us(n)	/* Delay n microseconds */

#else

//#include <hardware.h>			/* Include hardware specific declareation file here */

#include "xparameters.h"
#include "xgpio.h"

/*
 *
NET axi_gpio_0_GPIO_IO_pin[0] LOC = "R23"  |  IOSTANDARD = "LVCMOS33"; # MMC_CLK, SCK
NET axi_gpio_0_GPIO_IO_pin[1] LOC = "T19"  |  IOSTANDARD = "LVCMOS33"; # MMC_DATA0, MISO
NET axi_gpio_0_GPIO_IO_pin[2] LOC = "R24"  |  IOSTANDARD = "LVCMOS33"; # MMC_CMD, MOSI
NET axi_gpio_0_GPIO_IO_pin[3] LOC = "R19"  |  IOSTANDARD = "LVCMOS33"; # MMC_DATA3, SS

0 = I/O pin configured as output.
1 = I/O pin configured as input.

 */

void init_port(){
	XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 4, 0b0010);
}
void dly_us(int n) {
	int i, j;
	for(i = 0; i < n; i++){
		for(j = 0; j < 100; j++){
		}
	}
}

void bset(int i) {

}

#define	INIT_PORT()	init_port()	/* Initialize MMC control port (CS/CLK/DI:output, DO:input) */
#define DLY_US(n)	dly_us(n)	/* Delay n microseconds */
//#define	FORWARD(d)	forward(d)	/* Data in-time processing function (depends on the project) */


#define	CS_H()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) | 0b1000) /* Set MMC CS "high" */
#define CS_L()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) & 0b0111) /* Set MMC CS "low" */
#define CK_H()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) | 0b0001) /* Set MMC SCLK "high" */
#define	CK_L()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) & 0b1110) /* Set MMC SCLK "low" */
#define DI_H()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) | 0b0100) /* Set MMC DI "high" */
#define DI_L()		XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, 0, XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) & 0b1011) /* Set MMC DI "low" */
#define DO			((XGpio_ReadReg(XPAR_GPIO_0_BASEADDR, 0) & 0b0010) == 0b0010) /* Get MMC DO value (high:true, low:false) */


#endif

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */



static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */



/*-----------------------------------------------------------------------*/
/* Transmit a byte to the MMC (bitbanging)                               */
/*-----------------------------------------------------------------------*/

static
void xmit_mmc (
	BYTE d			/* Data to be sent */
)
{
#if AXI_SPI
	//xil_printf("xmit_mmc\r\n");
	XStatus Status = XSpi_Transfer(&Spi, &d, 0, 1);
 	if(Status != XST_SUCCESS) {
		xil_printf("Error in write transfer\r\n");
	}

	//DLY_US(1);
#else
	if (d & 0x80) DI_H(); else DI_L();	/* bit7 */
	CK_H(); CK_L();
	if (d & 0x40) DI_H(); else DI_L();	/* bit6 */
	CK_H(); CK_L();
	if (d & 0x20) DI_H(); else DI_L();	/* bit5 */
	CK_H(); CK_L();
	if (d & 0x10) DI_H(); else DI_L();	/* bit4 */
	CK_H(); CK_L();
	if (d & 0x08) DI_H(); else DI_L();	/* bit3 */
	CK_H(); CK_L();
	if (d & 0x04) DI_H(); else DI_L();	/* bit2 */
	CK_H(); CK_L();
	if (d & 0x02) DI_H(); else DI_L();	/* bit1 */
	CK_H(); CK_L();
	if (d & 0x01) DI_H(); else DI_L();	/* bit0 */
	CK_H(); CK_L();
#endif
}



/*-----------------------------------------------------------------------*/
/* Receive a byte from the MMC (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static
BYTE rcvr_mmc (void)
{
	BYTE r;

#if AXI_SPI
	//xil_printf("rcvr_mmc\r\n");
	BYTE w = 0xff;
	XStatus Status = XSpi_Transfer(&Spi, &w, &r, 1);
 	if(Status != XST_SUCCESS) {
		xil_printf("Error in read transfer\r\n");
		return XST_FAILURE;
	}

	//DLY_US(1);
#else
	DI_H();	/* Send 0xFF */

	r = 0;   if (DO) r++;	/* bit7 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit6 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit5 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit4 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit3 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit2 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit1 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit0 */
	CK_H(); CK_L();
#endif

	return r;
}



/*-----------------------------------------------------------------------*/
/* Skip bytes on the MMC (bitbanging)                                    */
/*-----------------------------------------------------------------------*/

static
void skip_mmc (
	WORD n		/* Number of bytes to skip */
)
{
#if AXI_SPI
	do {
		BYTE w = 0xff;
		//xil_printf("skip_mmc\r\n");
		XStatus Status = XSpi_Transfer(&Spi, &w, 0, 1);
	 	if(Status != XST_SUCCESS) {
			xil_printf("Error in skip transfer\r\n");
			xil_printf("Status = %d\r\n", Status);
		}
		//DLY_US(1);
	} while (--n);
#else
	DI_H();	/* Send 0xFF */

	do {
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
	} while (--n);
#endif
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
#if AXI_SPI
	XStatus Status = XSpi_SetSlaveSelect(&Spi, 1);
	if(Status != XST_SUCCESS) {
   		xil_printf("Failure Slave Select release_spi\r\n");
	}

	//DLY_US(1);
#else
	CS_H();
	rcvr_mmc();
#endif
}


/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

#if AXI_SPI
	XStatus Status = XSpi_SetSlaveSelect(&Spi, 0);
	if(Status != XST_SUCCESS) {
   		xil_printf("Failure Slave Select 0\r\n");
   		xil_printf("Status = %d\r\n", Status);
	}
	rcvr_mmc();
	Status = XSpi_SetSlaveSelect(&Spi, 1);
	if(Status != XST_SUCCESS) {
   		xil_printf("Failure Slave Select 1\r\n");
	}
	rcvr_mmc();
#else
	/* Select the card */
	CS_H(); rcvr_mmc();
	CS_L(); rcvr_mmc();
#endif

	/* Send a command packet */
	xmit_mmc(cmd);					/* Start + Command index */
	xmit_mmc((BYTE)(arg >> 24));	/* Argument[31..24] */
	xmit_mmc((BYTE)(arg >> 16));	/* Argument[23..16] */
	xmit_mmc((BYTE)(arg >> 8));		/* Argument[15..8] */
	xmit_mmc((BYTE)arg);			/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) */
	xmit_mmc(n);

	/* Receive a command response */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = rcvr_mmc();
	} while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	BYTE n, cmd, ty, buf[4];
	UINT tmr;


#if AXI_SPI
	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	XStatus Status = XSpi_Initialize(&Spi, XPAR_SDCARD_SPI_DEVICE_ID);
	if(Status != XST_SUCCESS) {\
   		xil_printf("Failure INIT\r\n");
		return XST_FAILURE;
	}
	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
						XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
   		xil_printf("Failure Options\r\n");
		return XST_FAILURE;
	}

	Status = XSpi_SetSlaveSelect(&Spi, 1);
	if(Status != XST_SUCCESS) {
   		xil_printf("Failure Slave Select\r\n");
		return XST_FAILURE;
	}

	XSpi_Start(&Spi);
	XSpi_IntrGlobalDisable(&Spi);

	DLY_US(1);
#else
	INIT_PORT();

	CS_H();
#endif
	skip_mmc(10);			/* Dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2 */
			for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();	/* Get trailing return value of R7 resp */
			if (buf[2] == 0x01 && buf[3] == 0xAA) {			/* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 1000; tmr; tmr--) {				/* Wait for leaving idle state (ACMD41 with HCS bit) */
					if (send_cmd(ACMD41, 1UL << 30) == 0) break;
					DLY_US(1000);
				}
				if (tmr && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();
					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 (HC or SC) */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			for (tmr = 1000; tmr; tmr--) {			/* Wait for leaving idle state */
				if (send_cmd(cmd, 0) == 0) break;
				DLY_US(1000);
			}
			if (!tmr || send_cmd(CMD16, 512) != 0)			/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	release_spi();

	return ty ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE *buff,		/* Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) */
	DWORD lba,		/* Sector number (LBA) */
	WORD ofs,		/* Byte offset to read from (0..511) */
	WORD cnt		/* Number of bytes to read (ofs + cnt mus be <= 512) */
)
{
	DRESULT res;
	BYTE d;
	WORD tmr;
	static BYTE read_buff[514];
	static BYTE write_buff[514];
	if(!write_buff[0]){
		memset(write_buff, 0xff, 514);
	}

	if (!(CardType & CT_BLOCK)) lba *= 512;		/* Convert to byte address if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, lba) == 0) {		/* READ_SINGLE_BLOCK */

		tmr = 1000;
		do {							/* Wait for data packet in timeout of 100ms */
			DLY_US(100);
			d = rcvr_mmc();
		} while (d == 0xFF && --tmr);

		if (d == 0xFE) {				/* A data packet arrived */
			XStatus Status = XSpi_Transfer(&Spi, write_buff, read_buff, 514);
		 	if(Status != XST_SUCCESS) {
				xil_printf("Error in read transfer\r\n");
				return res;
			}
		 	if(buff){
		 		memcpy(buff, read_buff + ofs, cnt);
		 	}

			res = RES_OK;
		}
	}

	release_spi();

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE

DRESULT disk_writep (
	const BYTE *buff,	/* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
	DWORD sa			/* Number of bytes to send, Sector number (LBA) or zero */
)
{
	DRESULT res;
	WORD bc, tmr;
	static WORD wc;


	res = RES_ERROR;

	if (buff) {		/* Send data bytes */
		bc = (WORD)sa;
		while (bc && wc) {		/* Send data bytes to the card */
			xmit_mmc(*buff++);
			wc--; bc--;
		}
		res = RES_OK;
	} else {
		if (sa) {	/* Initiate sector write process */
			if (!(CardType & CT_BLOCK)) sa *= 512;	/* Convert to byte address if needed */
			if (send_cmd(CMD24, sa) == 0) {			/* WRITE_SINGLE_BLOCK */
				xmit_mmc(0xFF); xmit_mmc(0xFE);		/* Data block header */
				wc = 512;							/* Set byte counter */
				res = RES_OK;
			}
		} else {	/* Finalize sector write process */
			bc = wc + 2;
			while (bc--) xmit_mmc(0);	/* Fill left bytes and CRC with zeros */
			if ((rcvr_mmc() & 0x1F) == 0x05) {	/* Receive data resp and wait for end of write process in timeout of 300ms */
				for (tmr = 10000; rcvr_mmc() != 0xFF && tmr; tmr--)	/* Wait for ready (max 1000ms) */
					DLY_US(100);
				if (tmr) res = RES_OK;
			}
			release_spi();
		}
	}

	return res;
}
#endif
