/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name��   timer.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#undef  EXT
#ifdef  _IN_TIMER_
#define EXT
#else
#define EXT extern
#endif


extern  uint32 TimerTickBak;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
/********************************************************************
**                      ���⺯���ӿ�����                            *
********************************************************************/
extern uint32 TimerStart(eTIMER_NUM timerNum,              //����TIMER
                            uint32 msTick,
                            pFunc callBack);
extern uint32 TimerStop(eTIMER_NUM timerNum);              //ֹͣTIMER
extern uint32 TimerGetCount(eTIMER_NUM timerNum);          //��ѯ��ǰ����ֵ
extern uint32 TimerUpdateApbFreq(uint32 APBfreq);           //APB CLK�ı�ʱ����������ֵ

//For test
extern uint32 TimerMask(eTIMER_NUM timerNum);              //MASK Timer�ж�
extern uint32 TimerUnmask(eTIMER_NUM timerNum);            //UNMASK Timer�ж�


//#else

//typedef void   (*pTimerEnable)(void);
//typedef void   (*pTimerDisable)(void);
//typedef uint32 (*pTimerPeriodGet)(UINT32 PCLK);
//typedef UINT32 (*pTimer_GetCount)(void);
//typedef void   (*pTimerIntEnable)(void);
//typedef void   (*pTimerIntDisable)(void);
//typedef uint32 (*pTimerClearIntFlag)(void);
//typedef UINT32 (*pTimerIntStateGet)(void);
//
//#define TimerEnable()        (((pTimerEnable      )(Addr_TimerEnable      ))())
//#define TimerDisable()       (((pTimerDisable     )(Addr_TimerDisable     ))())
//#define TimerPeriodGet(PCLK) (((pTimerPeriodGet   )(Addr_TimerPeriodGet   ))(PCLK))
//#define Timer_GetCount()     (((pTimer_GetCount   )(Addr_Timer_GetCount   ))())
//#define TimerIntEnable()     (((pTimerIntEnable   )(Addr_TimerIntEnable   ))())
//#define TimerIntDisable()    (((pTimerIntDisable  )(Addr_TimerIntDisable  ))())
//#define TimerClearIntFlag()  (((pTimerClearIntFlag)(Addr_TimerClearIntFlag))())
//#define TimerIntStateGet()   (((pTimerIntStateGet )(Addr_TimerIntStateGet ))())
//
//#endif

extern BOOL  TimerPeriodSet( UINT32 usTick);

/*
********************************************************************************
*
*                         End of timer.h
*
********************************************************************************
*/
#endif
