#ifndef _PFLAC_DEC_H_
#define _PFLAC_DEC_H_

//#include "../buffer/buffer.h"
#include "ape_globalvardeclaration.h"

//////////////////////////////////

// ע��:��������������APEDec.h��ĺ�APE_PCM_BUFFER_SIZE\APE_ENCODED_DATA_SIZEһ��, HuWeiGuo

//#define  PCM_BUFFER_SIZE    (BLOCKS_PER_DECODE * 2)//���Ա���32λPCM���
#define  PCM_BUFFER_SIZE    (BLOCKS_PER_DECODE)//��֧��32λPCM���
#define  ENCODED_DATA_SIZE  ((BLOCKS_PER_BITSTREAMREAD+16)*4)//�Ժ�Ҫ�޸�����Buffer���Ⱦ��޸�BLOCKS_PER_BITSTREAMREAD��

#if 0
///�����ļ������ṹ������,���ԱΪ����ָ������ added by hxd 20070723
typedef struct{
	int (*ApeReadBase)(void* ptr ,int size ,int nitems , FILE* strem);
	int (*ApeSeekBase)(FILE* stream , long offset , int fromwhere);
	int (*ApeTellBase)(FILE* stream);
	int (*ApeWriteBase)(void* ptr ,int size, int nitems, FILE* stream);
}tAPEFileIo;
#endif

typedef struct
{

  //DecoderList *workspace;
  //eDecoderStatus status;
  long             status;
  //sDecoderBitstream bitstream;
  //sDecoderOutput output;

  //tmInstance *pmInstance;
  //tmScratch *pmScratch;


  char pcEncodedData[ENCODED_DATA_SIZE];//��������Buffer,���������Buffer,(4096+16)*4 bytes<=> ape_uint32 ape_gInputBuffer[BLOCKS_PER_BITSTREAMREAD+16]


  short psLeft[PCM_BUFFER_SIZE];//���������������Buffer,��������������Buffer
  short psRight[PCM_BUFFER_SIZE];//���������������Buffer,��������������Buffer

  //BufferState *pOutput;
  int buffer_ptr[2];

  //unsigned short usValid;

  unsigned long usSampleRate;

  unsigned char ucChannels;

  //unsigned char ucIsVBR;

  //unsigned long ulFirstFrame;

  unsigned long ulLength;

  // The number of samples in each encoded block of audio.
  unsigned short usSamplesPerBlock;	// block

  // The length of the file in milliseconds.
  unsigned long ulTimeLength;

  unsigned long ulBitRate;

  // The number of samples that have been encoded/decoded.
  unsigned long ulTimePos;

} tAPE;


extern tAPE gAPEStruct;           // APE��������ṹ��
extern void *pApeRawFileCache;       // APE�ļ�ָ��

extern long ApeFread(void *pHandle, void *pBuf, unsigned long size);
extern long ApeFseek(void *pHandle, long offset, unsigned long mode);
extern long ApeFtell(void *pHandle);

#endif
