/*
 * dds.h
 *
 *  Created on: Aug 29, 2015
 *      Author: subotic
 */

#ifndef DDS_H_
#define DDS_H_


#include "xil_types.h"

u16 dds_freq_to_tunning_word(u32 freq_Hz, u32 sample_freq_Hz);
s8 dds_next_sample(u16 tunning_word);


#endif /* DDS_H_ */
