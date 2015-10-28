/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   Timer.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*               anzhiguo        2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_TIMER_

#include <typedef.h>
#include <DriverInclude.h>
pFunc       g_timerIRQ[2];  //����TIMER�жϻص�����
uint32      g_timerTick[2]; //����TIMER��ʱ������
//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : TimerEnable(void)
  Author        : anzhiguo
  Description   : enable timer

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void TimerEnable(void)
{
    pTIMER_REG timerReg;
    timerReg = (pTIMER_REG)TIMER_BASE;
//    timerReg->ControlReg |= TIMR_EN ;      //enable timer
}
/*
--------------------------------------------------------------------------------
  Function name : TimerDisable(void)
  Author        : anzhiguo
  Description   : disable timer

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
                anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void TimerDisable(void)
{
    pTIMER_REG timerReg;
    timerReg = (pTIMER_REG)TIMER_BASE;
//    timerReg->ControlReg &= (~TIMR_EN) ;      //disable timer
}
//#endif

/*----------------------------------------------------------------------
Name	: Timer1ISR
Desc	: TIMER1�жϴ�����
Params  :
Return  :
Notes   :
----------------------------------------------------------------------*/
void Timer0ISR(void)
{
    pFunc callback;
    uint32 intstatus;
    pTIMER_REG timerReg;
    timerReg = (pTIMER_REG)TIMER_BASE;
    intstatus = timerReg->Timer1EOI;


    callback = g_timerIRQ[0];
    if (callback)
    {
        callback();
    }
}

/*----------------------------------------------------------------------
Name	: Timer1ISR
Desc	: TIMER1�жϴ�����
Params  :
Return  :
Notes   :
----------------------------------------------------------------------*/
void Timer1ISR(void)
{
    pFunc callback;
    uint32 intstatus;
    pTIMER_REG timerReg;
    timerReg = (pTIMER_REG)TIMER_BASE;
    intstatus = timerReg->Timer1EOI;

    callback = g_timerIRQ[1];
    if (callback)
    {
        callback();
    }
}

/*----------------------------------------------------------------------
Name	: TimerSetCount
Desc	: ����APBƵ������TIMER������ֵ�Ȳ���
Params  : timerNum:TIMER���
          usTick:TIMER��ʱ΢����
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerSetCount(eTIMER_NUM timerNum, uint32 loadCount)
{
    pTIMER_REG timerReg;
    uint32   Retval = TRUE;

    if(loadCount > 0x00ffffff)
	{
        loadCount = 0x00ffffff;
        Retval = FALSE;
	}

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        default:
            break;
    }

//    timerReg->Timer1ControlReg = 0;            //disable the timer
    timerReg->Timer1LoadCount = loadCount;     //load the init count value
    timerReg->Timer1ControlReg |= 0x03;        //enable the timer

    return(Retval);
}

/*----------------------------------------------------------------------
Name	: TimerGetCount
Desc	: ��ȡTIMER�ļ���ֵ
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerGetCount(eTIMER_NUM timerNum)
{
    uint32 timeCount;
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
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        default:
            break;
    }

    return (timerReg->Timer1CurrentValue);
}



/*----------------------------------------------------------------------
Name	: TimerMask
Desc	: ����TIMER�ж����
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerMask(eTIMER_NUM timerNum)
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
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        default:
            break;
    }
    timerReg->Timer1ControlReg |= 0x04;

    return(0);

}

/*----------------------------------------------------------------------
Name	: TimerUnmask
Desc	: ����TIMER�ж����
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerUnmask(eTIMER_NUM timerNum)
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
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        default:
            break;
    }
    timerReg->Timer1ControlReg &= ~0x04;
    return(0);

}

/*----------------------------------------------------------------------
Name	: TimerInit
Desc	: TIMER��ʼ��
Params  : timerNum:TIMER���
Return  : 0:�ɹ�
Notes   :
----------------------------------------------------------------------*/
uint32 TimerInit(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;

    g_timerIRQ[timerNum] = 0;

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            timerReg->Timer1ControlReg = 0;//disable timer1
            IntRegister(INT_ID_TIMER0, Timer0ISR);
            IntPendingClear(INT_ID_TIMER0);
            IntEnable(INT_ID_TIMER0);
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)TIMER_BASE;
            timerReg->Timer1ControlReg = 0;//disable timer1
            IntRegister(INT_ID_TIMER1, Timer1ISR);
            IntPendingClear(INT_ID_TIMER1);
            IntEnable(INT_ID_TIMER1);
            break;
        default:
            break;
    }

    return(0);
}
/*----------------------------------------------------------------------
Name	: TimerStop
Desc	: ֹͣһ��TIMER
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

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        default:
            break;
    }
    timerReg->Timer1ControlReg = 0;

    return(0);
}
/*----------------------------------------------------------------------
Name	: TimerStart
Desc	: ����һ��TIMER
Params  : timerNum:TIMER���
          usTick:TIMER��ʱ΢����
          callBack:�жϻص�����
Return  : 0:�ɹ�
Notes   : ֱ�ӵ��øú����Ϳ�������һ������������Ҫ��INTC��ʼ��֮��
----------------------------------------------------------------------*/
uint32 TimerStart(eTIMER_NUM timerNum, uint32 usTick, pFunc CallBack)
{
    uint32 loadCount;

    if(timerNum >= TIMER_MAX)
    {
        printf("timerNum is err\n");
        return (-1);
    }

    if(TimerInit(timerNum))
    {
        printf("TimerInit is err\n");
        return(-1);
    }

    g_timerIRQ[timerNum] = CallBack;
    g_timerTick[timerNum] = usTick;

    if ((TIMER0 == timerNum) || (TIMER1 == timerNum))
    {
        loadCount = 24*1000;//apb clk��λkHz
        loadCount = loadCount/1000*usTick;
    }

    TimerStop(timerNum);
    if(TimerSetCount(timerNum, loadCount))
    {
        return(-1);
    }
    #if 0
    if(TimerGetCount(timerNum)>loadCount)
    {
        while(1)
        PRINTF("Timer set ERROR!!!\n");

    }
    #endif
    return(0);
}


//#endif
/*
********************************************************************************
*
*                         End of timer.c
*
********************************************************************************
*/

