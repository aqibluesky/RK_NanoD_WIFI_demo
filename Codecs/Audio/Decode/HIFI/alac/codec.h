
#ifndef  CODEC_H
#define  CODEC_H

#define in_buf_size 16384
#define out_buf_size 4096*8//192000
#define MAX_CHANNELS 2
#define MAX_BLOCKSIZE 4096




 typedef struct AVPacket {
	unsigned char *data;
	int   size;
} AVPacket;


#endif