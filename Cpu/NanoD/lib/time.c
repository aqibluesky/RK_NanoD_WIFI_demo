/*
********************************************************************************
*                   Copyright (c): 2014 - 2014 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* File Name��   Timer.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*               anzhiguo        2014-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"
#ifdef __DRIVER_TIME_TIMEDEVICE_C__
#define _IN_TIMER_

#include <typedef.h>
#include <DriverInclude.h>

#if 1
uint32 TimerClrInt(eTIMER_NUM timerNum)
{
    uint32 intstatus = 0;
    pTIMER_REG timerReg;
    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            timerReg->TimerIntStatus = 0x1;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            timerReg->TimerIntStatus = 0x1;
            break;
        default:
            break;
    }
    return (intstatus);
}
/*----------------------------------------------------------------------
Name    : TimerSetCount
Desc    : ����APBƵ������TIMER������ֵ�Ȳ���
Params  : timerNum:TIMER���
          usTick:TIMER��ʱ΢����
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerSetCount(eTIMER_NUM timerNum,uint64 usTick)
{
    pTIMER_REG timerReg;
    uint64 loadCount = 0;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }

    //printf("\nTimer %d SetCount\n", timerNum);
    loadCount = usTick;//24M clk
    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }

    timerReg->TimerLoadCount0 = (loadCount & 0xffffffff );      //load the init count value low 32 bits
    timerReg->TimerLoadCount1 = ((loadCount>>32) & 0xffffffff );//load the init count value high 32 bits

    return(0);
}

/*----------------------------------------------------------------------
Name    : TimerGetCount
Desc    : ��ȡTIMER�ļ���ֵ
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint64 TimerGetCount(eTIMER_NUM timerNum)
{
    uint32 timeCount;
    pTIMER_REG timerReg;

    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
    //printf("\nTimer %d GetCount\n", timerNum);

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }

    return (((timerReg->TimerCurrentValue1)<<32)|timerReg->TimerCurrentValue0);
}

/*----------------------------------------------------------------------
Name    : TimerMask
Desc    : ����TIMER�ж����
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerIntMask(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }
    timerReg->TimerControlReg &= ~TIMR_INOMASK;//timerReg->TimerControlReg |= 0x04;

    return(0);

}

/*----------------------------------------------------------------------
Name    : TimerUnmask
Desc    : ����TIMER�ж����
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerIntUnmask(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
    printf("\nTimer %d Unmask\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }
    timerReg->TimerControlReg |= TIMR_INOMASK;//timerReg->TimerControlReg |= 0x04;
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    return(0);

}


/*----------------------------------------------------------------------
Name    : TimerGetSystime
Desc    : ��ȡϵͳʱ��
Params  : sysTime:ϵͳʱ��ṹ��
Return  :
Notes   : ϵͳʱ���ϵͳ������ʼ���㣬��С��λΪ΢��
----------------------------------------------------------------------*/
#if 0
pTIMER_SYSTIME TimerGetSystime(pTIMER_SYSTIME sysTime)
{
    uint32 count;
    count = TimerGetCount(TIMER1);
    sysTime->ms = g_sysTime.ms;
    sysTime->us = 5000 - count/24;
    return (sysTime);
}
#endif
/*----------------------------------------------------------------------
Name    : TimerInit
Desc    : TIMER��ʼ��
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerInit(eTIMER_NUM timerNum,eTIMER_MODE mode)
{
    pTIMER_REG timerReg;


    if((timerNum >= TIMER_MAX) || (mode >= TIMER_MODE_MAX))
    {
        return (-1);
    }
    //printf("\nTimer %d initial\n", timerNum );
//    g_timerIRQ[timerNum] = 0;

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }

    timerReg->TimerControlReg = TIMR_IMASK | (mode << 1);     //mask interrupt ;free running
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);
    return(0);
}

/*----------------------------------------------------------------------
Name    : TimerStart
Desc    : ����һ��TIMER
Params  : timerNum:TIMER���
Return  : 0:�ɹ�;-1:ʧ��
Notes   : ֱ�ӵ��øú����Ϳ�������һ������������Ҫ��INTC��ʼ��֮��
----------------------------------------------------------------------*/
uint32 TimerStart(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;

    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }

    //printf("\nTimer %d Start\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    timerReg->TimerControlReg |= TIMR_ENABLE;
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    //printf("timerReg->TimerCurrentValue0 = %d\n",timerReg->TimerCurrentValue0);
    //printf("timerReg->TimerCurrentValue1 = %d\n",timerReg->TimerCurrentValue1);

    return(0);
}

/*----------------------------------------------------------------------
Name    : TimerStop
Desc    : ֹͣһ��TIMER
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerStop(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;

    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
    printf("\nTimer %d Stop\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;
        default:
            break;
    }
    timerReg->TimerControlReg = TIMR_DISABLE;
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    return(0);
}
#endif
#endif
/*
********************************************************************************
*
*                         End of timer.c
*
********************************************************************************
*/

