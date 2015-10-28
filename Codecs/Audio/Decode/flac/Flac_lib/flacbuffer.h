/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     flacbuffer.h
*  Description:    FLAC���������������Buffer.
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/04/23      1.0
*
*******************************************************************************/

#ifndef __FLAC_BUFFER_H__
#define __FLAC_BUFFER_H__

#include "ordinals.h"


#define FLAC__BITBUFFER_MAX_REMAINDER    (64)  // ÿ��fill bufferʱ���п���ʣ�����ݣ�������Ϊʣ�����ݲ��ᳬ��64��
//#define FLAC__BITBUFFER_DEFAULT_CAPACITY (4096/2)
#define FLAC__BITBUFFER_DEFAULT_CAPACITY (4096/8)	//test
#define FLAC__PCMBUFFER_DEFAULT_CAPACITY (4608)

#define FLAC__MAX_LPC_ORDER (32u)// Ӧ��format.h���һ��
#define FLAC__MAX_CHANNEL 2

/*
 * ���
 * 1��Ŀǰ����ֻ֧�� bits_per_sample = 16 �����;
 * 2���� bits_per_sample > 16 ʱ�����е���� Buffer ��������;
 * 3��FLAC__MAX_LPC_ORDER �� dword ���ڴ�� warmup data;
 *    ʲô��warmup data?
 *    ����һ�������˲���Ϊ�� y(n) = a0 x(n) + a1 x(n-1) + a2 x(n-2),
 *    ��ô��ʼ״̬�� x(n), x(n-1), x(n-2) ����warmup data�� ֻ�л������Щֵ�����ǲ������ y(n)
 */
extern unsigned char g_FlacInputBuffer[]; //FLAC__BITBUFFER_MAX_REMAINDER ���ڴ��buffer�в���������

	// ���,
#ifdef FLAC_MINUS_ZI_DATA
	//modified by evan wu,2009-3-18,for saving zi data
extern	unsigned short g_FlacOutputBufferMidOrLeft[];
extern	unsigned long g_FlacOutputBufferRightOrSide[];
#else

#ifdef HALF_FRAME_BY_HALF_FRAME
extern unsigned long g_FlacOutputBuffer[FLAC__MAX_CHANNEL][FLAC__PCMBUFFER_DEFAULT_CAPACITY/2+FLAC__MAX_LPC_ORDER+FLAC__MAX_LPC_ORDER];
#else
extern unsigned long g_FlacOutputBuffer[FLAC__MAX_CHANNEL][FLAC__PCMBUFFER_DEFAULT_CAPACITY+FLAC__MAX_LPC_ORDER+FLAC__MAX_LPC_ORDER];
#endif


#endif//FLAC_MINUS_ZI_DATA



#ifdef HALF_FRAME_BY_HALF_FRAME
//extern	unsigned short g_FlacCodecBuffer[2][FLAC__PCMBUFFER_DEFAULT_CAPACITY+FLAC__MAX_LPC_ORDER];
extern unsigned short *  g_FlacCodecBuffer[2];
#else
extern	unsigned short g_FlacCodecBuffer[];
#endif



#endif
