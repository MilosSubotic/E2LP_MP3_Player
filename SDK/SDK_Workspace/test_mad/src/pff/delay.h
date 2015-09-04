/*
 * delay.h
 *
 *  Created on: Sep 4, 2015
 *      Author: subotic
 */

#ifndef DELAY_H_
#define DELAY_H_

static inline void delay_us(int us) {
	// Busy wait.

	__asm__(
			"outher_loop:          \n"

			"xor   %1,%1,%1        \n"
			"addi  %1,%1,8        \n"
			"inner_loop:           \n"
			"addi  %1,%1,-1        \n"
			"bnei  %1,inner_loop   \n"
			"nop                   \n"
			"nop                   \n"

			"addi  %0,%0,-1        \n"
			"bnei  %0,outher_loop  \n"
			"nop                   \n"
			:
			: "r"(us), "r"(0)
	);

}

#endif /* DELAY_H_ */
