/*
 * @license MIT
 * @brief Parsing MP3 file.
 */

#ifndef MP3_PARSING_H_
#define MP3_PARSING_H_

#include <stdint.h>

#define MPEGVersion2_5 0
#define MPEGreserved   1
#define MPEGVersion2   2
#define MPEGVersion1   3

#define LAYER1  3
#define LAYER2  2
#define LAYER3  1
#define LAYER_RESERVED   0
//some macros defined by me... you can do what ever you want with these
#define BITRATE_FREE -2
#define BITRATE_BAD  -1
#define SAMPLE_FREQ_BAD  -1

typedef struct{
#ifdef __BIG_ENDIAN__
    unsigned    framesync   :11;    //Frame synchronizer
    unsigned    MPEGID      : 2;    //MPEG version ID
    unsigned    layer       : 2;    //Layer
    unsigned    protectbit  : 1;    //CRC Protection
    unsigned    bitrateindx : 4;    //Bitrate index
    unsigned    samplefreq  : 2;    //Samplig rate frequency index
    unsigned    paddingbit  : 1;    //Padding
    unsigned    privatebit  : 1;    //Private bit
    unsigned    channel     : 2;    //Channel
    unsigned    modeext     : 2;    //Mode extension (only if Joint Stereo is set)
    unsigned    copyright   : 1;    //Copyright
    unsigned    original    : 1;    //Original
    unsigned    emphasis    : 2;    //Emphasis
#else
    unsigned    emphasis    : 2;    //Emphasis
    unsigned    original    : 1;    //Original
    unsigned    copyright   : 1;    //Copyright
    unsigned    modeext     : 2;    //Mode extension (only if Joint Stereo is set)
    unsigned    channel     : 2;    //Channel
    unsigned    privatebit  : 1;    //Private bit
    unsigned    paddingbit  : 1;    //Padding
    unsigned    samplefreq  : 2;    //Samplig rate frequency index
    unsigned    bitrateindx : 4;    //Bitrate index
    unsigned    protectbit  : 1;    //CRC Protection
    unsigned    layer       : 2;    //Layer
    unsigned    MPEGID      : 2;    //MPEG version ID
    unsigned    framesync   :11;    //Frame synchronizer
#endif
} MP3HEADER;

typedef struct{
    char    tagid  [ 3];    //0-2     3  Tag identifier. Must contain "TAG" string if Tag is valid.
    char    name   [30];    //3-32   30  Song Name
    char    artist [30];    //33-62  30  Artist
    char    album  [30];    //63-92  30  Album
    int     year   :32 ;    //93-96   4  Year
    char    comment[30];    //97-126 30  Comment
    char    genre  : 8 ;    //127  1  Genre
} MP3ID3TAG1;

#pragma pack(1)
typedef struct{
    char    tagid[3];//0-2  TAG identifier. It contains of string "ID3"
    short   tagverm ;//3-4  TAG version
    char    flags   ;//5    Flags
    long    size    ;//6-9  Size of TAG MSB
} MP3ID3TAG2;

int mp3_bitrate_kb(MP3HEADER h);

int mp3_sample_freq(MP3HEADER h);

/**
 * @param data pointer on 4 byte data.
 */
MP3HEADER parse_mp3_header(const uint8_t* data);

#endif /* MP3_PARSING_H_ */
