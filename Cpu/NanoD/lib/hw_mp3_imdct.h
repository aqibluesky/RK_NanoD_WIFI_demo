
/*
********************************************************************************************
*
*				  Copyright (c): 2014 - 2014 + 5, WJR
*							   All rights reserved.
*
* FileName: Cpu\NanoC\lib\hw_mp3_imdct.h
* Owner: WJR
* Date: 2014.11.10
* Time: 17:36:35
* Desc:
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    WJR     2014.11.10     17:36:35   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_HW_MP3_IMDCT_H__
#define __CPU_NANOC_LIB_HW_MP3_IMDCT_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "hw_memap.h"
//Ϊ�˴�����ⷽ�㣬�����������Ӳ���������ݣ����ⲿӲ����ַӳ��ı�����Ӧ����Ӧ�޸�

//base------------------------------------------------------------------------------------
#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//imdct36 memory map------------------------------------------------------------------------
//#define IMDCT_BASEADDR	0x62042000
#define IMDCT_BASEADDR 	    IMDCT_BASE//==(0x60010000UL)

#define IMDCT_CTRL_REG	*((volatile unsigned long*)(IMDCT_BASEADDR + 0x0800))
#define IMDCT_STAT_REG	*((volatile unsigned long*)(IMDCT_BASEADDR + 0x0804))
#define IMDCT_INTR_REG	*((volatile unsigned long*)(IMDCT_BASEADDR + 0x0808))
#define IMDCT_EOIT_REG	*((volatile unsigned long*)(IMDCT_BASEADDR + 0x080C))

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

extern volatile int imdct_finish;
extern volatile int imdct36_auto_output_addr;




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t hw_imdct_shell();
extern void imdct36_hw_pipeline(long *X,long *z);

extern void Hw_imdct_Exit(void);
extern void Hw_imdct_Init(void);

extern void imdct36_handler(void);



#endif


