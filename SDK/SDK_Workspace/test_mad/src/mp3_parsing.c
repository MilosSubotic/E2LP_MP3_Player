/*
 * @license MIT
 * @brief Parsing MP3 file.
 */

#include "mp3_parsing.h"

static const int mp3_bitrates_table[] = {
    BITRATE_FREE, BITRATE_FREE, BITRATE_FREE, BITRATE_FREE, BITRATE_FREE,
    32,  32,  32,  32,    8,
    64,  48,  40,  48,   16,
    96,  56,  48,  56,  24,
    128,  64,  56,  64,  32,
    160,  80,  64,  80,  40,
    192,  96,  80,  96,  48,
    224, 112,  96, 112,  56,
    256, 128, 112, 128,  64,
    288, 160, 128, 144,  80,
    320, 192, 160, 160,  96,
    352, 224, 192, 176, 112,
    384, 256, 224, 192, 128,
    416, 320, 256, 224, 144,
    448, 384, 320, 256, 160,
    BITRATE_BAD, BITRATE_BAD, BITRATE_BAD, BITRATE_BAD, BITRATE_BAD
};

/**
 * [MPEGID][layer]
 */
static const int mp3_sample_freq_table[4][4] = {
	{ 11025, 12000,  8000, SAMPLE_FREQ_BAD }, // MPEG2.5
	{ SAMPLE_FREQ_BAD, SAMPLE_FREQ_BAD, SAMPLE_FREQ_BAD, SAMPLE_FREQ_BAD },
	{ 22050, 24000, 16000, SAMPLE_FREQ_BAD }, // MPEG2
	{ 44100, 48000, 32000, SAMPLE_FREQ_BAD }, // MPEG1
};

int mp3_bitrate_kb(MP3HEADER h) {
    int index=0;
    if( (0x03&h.MPEGID)==MPEGVersion1 ){
        index = 3-h.layer;
    }else if( (0x03&h.MPEGID)==MPEGVersion2 || (0x03&h.MPEGID)==MPEGVersion2_5 ){
        index = (0x03&h.layer) == LAYER1 ? 3 :4 ;
    }
    return mp3_bitrates_table[ index + (0x0f&h.bitrateindx) * 5];
}

int mp3_sample_freq(MP3HEADER h) {
	return mp3_sample_freq_table[h.MPEGID][h.samplefreq];
}

MP3HEADER parse_mp3_header(const uint8_t* data) {
	union {
		uint8_t bytes[4];
		MP3HEADER bitfields;
	} bytes_to_bitfields;

#ifdef __BIG_ENDIAN__
	bytes_to_bitfields.bytes[0] = data[0];
	bytes_to_bitfields.bytes[1] = data[1];
	bytes_to_bitfields.bytes[2] = data[2];
	bytes_to_bitfields.bytes[3] = data[3];
#else
	bytes_to_bitfields.bytes[0] = data[3];
	bytes_to_bitfields.bytes[1] = data[2];
	bytes_to_bitfields.bytes[2] = data[1];
	bytes_to_bitfields.bytes[3] = data[0];
#endif

	return bytes_to_bitfields.bitfields;

}

