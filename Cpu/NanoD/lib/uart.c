/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   Uart.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"

#ifdef __DRIVER_UART_UARTDEVICE_C__
#include "typedef.h"
#include "DriverInclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define  IN_DRIVER_UART
#define _IN_UART_

const uint32 UART_GROUP[6] =
{
    UART0_BASE,
    UART1_BASE,
    UART2_BASE,
    UART3_BASE,
    UART4_BASE,
    UART5_BASE,
};

//#define UartReg              ((UART_REG*)UART_GROUP)

#define UARTPORT(n)          ((UART_REG*)(UART_GROUP[n]))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : void UARTRest(void)
  Author        :
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
void UARTRest(eUART_CH uartPort)
{
    UART_REG *  pReg = UARTPORT(uartPort);
    pReg->UART_SRR = UART_RESET | RCVR_FIFO_REST | XMIT_FIFO_RESET;
    pReg->UART_IER = 0;
    pReg->UART_DMASA = 1;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 UARTSetIOP(uint8 useIrDA)
  Author        :
  Description   : set uart role as serial port or IRDA.

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
int32 UARTSetIOP(eUART_CH uartPort,uint8 useIrDA)
{
    //pUART_REG   phwHead   = (pUART_REG)UART1_BASE_ADDR;

    UART_REG *  pReg = UARTPORT(uartPort);
    if ((useIrDA == IRDA_SIR_DISABLED) || (useIrDA == IRDA_SIR_ENSABLED))
    {
        pReg->UART_MCR = useIrDA;
        return (0);
    }

    return (-1);
}

/*
--------------------------------------------------------------------------------
  Function name : UARTSetBaudRate(pUART_REG phead, uint32 baudRate)
  Author        :
  Description   :
                  baudRate:serial initial speed,get by table seek,
  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
int32 UARTSetBaudRate(eUART_CH uartPort,uint32 clk, uint32 baudRate)
{
    uint32  DivLatch;
    UART_REG *  pReg = UARTPORT(uartPort);

    //  DLL and THR(RBR) are mapped the same offset, set LCR[7] to access to
    //  DLL register, otherwise the THR(RBR)

    DivLatch = clk / MODE_X_DIV / baudRate;

    pReg->UART_LCR |= LCR_DLA_EN;

    pReg->UART_DLL  = (DivLatch >> 0) & 0xff;
    pReg->UART_DLH  = (DivLatch >> 8) & 0xff;

    pReg->UART_LCR &= (~LCR_DLA_EN);

    return (0);
}

/*
--------------------------------------------------------------------------------
  Function name : int32 UARTSetLcrReg(uint8 byteSize, uint8 parity,uint8 stopBits )
  Author        :
  Description   :

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
int32 UARTSetLcrReg(eUART_CH uartPort,uint8 byteSize, uint8 parity,uint8 stopBits )
{
    uint32 lcr;
    int32  bRet = 0;

    UART_REG *  pReg = UARTPORT(uartPort);
    lcr = (pReg->UART_LCR) & (~(uint32)(UART_DATABIT_MASK));

    //Data length select
    switch ( byteSize )
    {
        case UART_DATA_5B:
            lcr |= LCR_WLS_5;
            break;
        case UART_DATA_6B:
            lcr |= LCR_WLS_6;
            break;
        case UART_DATA_7B:
            lcr |= LCR_WLS_7;
            break;
        case UART_DATA_8B:
            lcr |= LCR_WLS_8;
            break;
        default:
            bRet = -1;
            break;
    }

    //Parity set
    switch ( parity )
    {
        case UART_ODD_PARITY:
        case UART_EVEN_PARITY:
            lcr |= PARITY_ENABLED;
            lcr |= ((parity) << 4);
            break;
        case UART_PARITY_DISABLE:
            lcr &= ~PARITY_ENABLED;
            break;
        default:
            bRet = -1;
            break;
    }

    //StopBits set
    switch ( stopBits )
    {
        case 0:
            lcr |= ONE_STOP_BIT;
            break;
        case 1:
            lcr |= ONE_HALF_OR_TWO_BIT;
            break;
        default:
            bRet = -1;
            break;
    }

    if (bRet == 0)
    {
        pReg->UART_LCR = lcr;
    }

    return(bRet);
}

/*
--------------------------------------------------------------------------------
  Function name : UARTSetFifoEnabledNumb()
  Author        :
  Description   :

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
void UARTSetFifoEnabledNumb(eUART_CH uartPort,uint32 param)
{
	UART_REG *  pReg = UARTPORT(uartPort);

    pReg->UART_SFE  = (param & 0x01);
    pReg->UART_SRT  = (param >> 1) & 0x03;
    pReg->UART_STET = (param >> 3) & 0x03;
}

/*
--------------------------------------------------------------------------------
  Function name : void UARTSetIntEnabled(uint32 uartIntNumb)
  Author        :
  Description   : ����UART�ڵ��ж�ʹ�ܴ򿪡�

  Input         : uartIntNumb��UART���жϺ�

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
void UARTSetIntEnabled(eUART_CH uartPort,uint32 uartIntNumb)
{
    UART_REG *  pReg = UARTPORT(uartPort);

    pReg->UART_IER |= uartIntNumb;
}

/*
--------------------------------------------------------------------------------
  Function name : void UARTSetIntDisabled(uint32 uartIntNumb)
  Author        :
  Description   : ���ж�ʹ��

  Input         : uartIntNumb��UART���жϺ�

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
void UARTSetIntDisabled(eUART_CH uartPort,uint32 uartIntNumb)
{
    UART_REG *  pReg = UARTPORT(uartPort);
    pReg->UART_IER &= (~uartIntNumb);
}

/*
--------------------------------------------------------------------------------
  Function name : UARTWriteByte(uint8 ch)
  Author        :
  Description   : ����дһ���ֽ�

  Input         : byte:������ֽ�ֵ
                  uartTimeOut:�ȴ���ʱʱ��

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
rk_size_t UARTWriteByte(eUART_CH uartPort,uint8 * pdata, uint32 cnt)
{
    rk_size_t dwRealSize;
    UART_REG *  pReg = UARTPORT(uartPort);

    dwRealSize = 0;
    while (cnt--)
    {
        if(!(pReg->UART_USR & UART_TRANSMIT_FIFO_NOT_FULL))
        {
            break;
        }

        pReg->UART_THR = *pdata++;
        dwRealSize++;
    }

    return dwRealSize;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 UARTReadByte(uint8 *pdata, uint32 uartTimeOut)
  Author        :
  Description   : ���ڶ�ȡһ���ֽ�

  Input         :
                  uartTimeOut:�ȴ���ʱʱ��

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
rk_size_t UARTReadByte(eUART_CH uartPort,uint8 *pdata, uint32 cnt)
{
    rk_size_t dwRealSize;
    UART_REG *  pReg = UARTPORT(uartPort);

    dwRealSize = 0;

    while (cnt--)
    {
        if (!(pReg->UART_USR & UART_RECEIVE_FIFO_NOT_EMPTY))
        {
            break;
        }

        *pdata++ = (uint8 )pReg->UART_RBR;
        dwRealSize++;
    }

    return dwRealSize;
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : int32 UartDmaWrite(uint8 *pdata, uint32 size, uint32 uartTimeOut, pFunc Callback)
  Author        :
  Description   : ͨ��DMA�򴮿�д����

  Input         : pdata:�������ָ��
                  size: ������ݴ�С(byte)
                  uartTimeOut:�ȴ���ʱʱ��
                  Callback:DMA�жϻص�����

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
//int32 UartDmaWrite(uint8 *pdata, uint32 size, uint32 uartTimeOut, pFunc Callback)
//{
//    int32 channel;
//    DMA_CFGX DmaCfg = {DMA_CTLL_M2UART,DMA_CFGL_M2UART,DMA_CFGH_M2UART,0};
//
//    while ((UartReg->UART_USR & UART_TRANSMIT_FIFO_NOT_FULL) != UART_TRANSMIT_FIFO_NOT_FULL)
//    {
//        if (uartTimeOut == 0)
//        {
//            return (-1);
//        }
//        uartTimeOut--;
//    }
//
//    channel = DmaGetChannel();
//    if (channel != DMA_FALSE)
//    {
//        DmaStart((uint32)(channel), (UINT32)(pdata),(uint32)(&(UartReg->UART_THR)),size, &DmaCfg, Callback);
//        return channel;
//    }
//
//    return (-1);
//}

/*
--------------------------------------------------------------------------------
  Function name : int32 UartDmaRead(uint8 *pdata, uint32 size, uint32 uartTimeOut, pFunc Callback)
  Author        :
  Description   : read the uart data by dma.

  Input         : pdata:input data pointer.
                  size: input data size.(byte)
                  uartTimeOut: waiting delay time.
                  Callback:DMA interrupt callback.

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
//int32 UartDmaRead(uint8 *pdata, uint32 size, uint32 uartTimeOut, pFunc Callback)
//{
//    eDMA_CHN channel;
//    DMA_CFGX DmaCfg = {DMA_CTLL_UART2M,DMA_CFGL_UART2M,DMA_CFGH_UART2M,0};
//
//    while ((UartReg->UART_USR & UART_TRANSMIT_FIFO_NOT_FULL) != UART_TRANSMIT_FIFO_NOT_FULL)
//    {
//        if (uartTimeOut == 0)
//        {
//            return (-1);
//        }
//        uartTimeOut--;
//    }
//
//    channel = DmaGetChannel();
//    if (channel != DMA_FALSE)
//    {
//        DmaStart((uint32)(channel), (uint32)(&(UartReg->UART_RBR)),(UINT32)(pdata),size, &DmaCfg, Callback);
//        return channel;
//    }
//
//    return (-1);
//}

//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : int32 UARTInit(eUART_BaudRate baudRate,eUART_dataLen dataBit,eUART_stopBit stopBit,eUART_parity_en parity)
  Author        :
  Description   :

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
int32 UARTInit(eUART_CH uartPort,eUART_BaudRate baudRate,eUART_dataLen dataBit,eUART_stopBit stopBit,eUART_parity_en parity)
{
    UARTRest(uartPort);

    UARTSetIOP(uartPort,IRDA_SIR_DISABLED);

    UARTSetFifoEnabledNumb(uartPort,SHADOW_FIFI_ENABLED | RCVR_TRIGGER_HALF_FIFO | TX_TRIGGER_TWO_IN_FIFO);

    UARTSetLcrReg(uartPort,dataBit,parity,stopBit);

    UARTSetBaudRate(uartPort,24 * 1000 * 1000, baudRate);

    return (0);
}


int32 UARTIntInit(eUART_CH uartPort,eUART_BaudRate baudRate,eUART_dataLen dataBit,eUART_stopBit stopBit,eUART_parity_en parity)
{
    UART_REG *  pReg = UARTPORT(uartPort);

    UARTRest(uartPort);

    UARTSetIOP(uartPort,IRDA_SIR_DISABLED);

    UARTSetFifoEnabledNumb(uartPort,SHADOW_FIFI_ENABLED | RCVR_TRIGGER_ONE | TX_TRIGGER_EMPTY);

    UARTSetLcrReg(uartPort,dataBit,parity,stopBit);

    UARTSetBaudRate(uartPort,24 * 1000 * 1000, baudRate);
    pReg->UART_SRR = RCVR_FIFO_REST | XMIT_FIFO_RESET;

    return (0);
}

/*
*********************************************************************************************************
*                                     UARTGetIntType(void)
*
* Description : �жϴ����ж�����.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint32 UARTGetIntType(eUART_CH uartPort)
{
    uint32 dwUartIntStaus;
    uint event;
    UART_REG *  pReg = UARTPORT(uartPort);

    dwUartIntStaus = pReg->UART_IIR & UART_IF_MASK;
    event = 0;

    if(dwUartIntStaus & UART_IF_REC_DATA)
    {
        event |= UART_INT_READ_FIFO_NOT_EMPTY;
    }

    if(dwUartIntStaus & UART_IF_THR_EMPTY)
    {
        event |= UART_INT_WRITE_FIFO_EMPTY;
    }

    return event;
}


uint32 Memory_to_Uart( eDMA_CHN dmaPort,
                    eUART_CH uartPort,
                    uint32 srcAddr,
                    uint32 size
                    )
{
    UART_REG *  pReg ;
    uint32 dstAddr;
    DMA_CFGX DmaCfg;

    pReg = UARTPORT(uartPort);
    dstAddr = (uint32)&(pReg->UART_THR);

    switch( uartPort)
    {
        case UART_DEV0:
        case UART_DEV2:
            DmaCfg.CTLL = DMA_CTLL_M2UART_02;
            DmaCfg.CFGL = DMA_CFGL_M2UART_02;
            DmaCfg.CFGH = DMA_CFGH_M2UART_02;
            DmaCfg.pLLP = 0;
            break;

        case UART_DEV1:
            DmaCfg.CTLL = DMA_CTLL_M2UART_1;
            DmaCfg.CFGL = DMA_CFGL_M2UART_1;
            DmaCfg.CFGH = DMA_CFGH_M2UART_1;
            DmaCfg.pLLP = 0;
            break;

        case UART_DEV3:

            break;

        case UART_DEV4:

            break;

        case UART_DEV5:

            break;
    }

    DmaConfig(dmaPort, srcAddr,dstAddr, size, &DmaCfg, NULL);

    return 0;
}

uint32 Uart_to_Memory( eDMA_CHN dmaPort,
                    eUART_CH uartPort,
                    uint32 dstAddr,
                    uint32 size)
{
    UART_REG *  pReg ;
    uint32 srcAddr;
    DMA_CFGX DmaCfg;

    pReg = UARTPORT(uartPort);
    srcAddr = (uint32)&(pReg->UART_RBR);

    switch( uartPort)
    {
        case UART_DEV0:
        case UART_DEV2:
            DmaCfg.CTLL = DMA_CTLL_UART_022M;
            DmaCfg.CFGL = DMA_CFGL_UART_022M;
            DmaCfg.CFGH = DMA_CFGH_UART_022M;
            DmaCfg.pLLP = 0;

            break;

        case UART_DEV1:
            DmaCfg.CTLL = DMA_CTLL_UART_12M;
            DmaCfg.CFGL = DMA_CFGL_UART_12M;
            DmaCfg.CFGH = DMA_CFGH_UART_12M;
            DmaCfg.pLLP = 0;

            break;

        case UART_DEV3:
            break;

        case UART_DEV4:
            break;

        case UART_DEV5:
            break;
    }

    DmaConfig(dmaPort, srcAddr,dstAddr, size, &DmaCfg, NULL);

    return 0;
}

void UARTEnableFlowControl(eUART_CH uartPort)
{
    UART_REG *  pReg ;

    pReg = UARTPORT(uartPort);
    pReg->UART_MCR = AUTO_FLOW_ENSABLED;
    pReg->UART_FCR = 0x1;
}

/*0 = cts_n input is de-asserted (logic 1)
1 = cts_n input is asserted (logic 0)
*/
uint32 UARTGetCTSState(eUART_CH uartPort)
{
    UART_REG *  pReg ;
    uint32 cts_status;

    pReg = UARTPORT(uartPort);
    cts_status = pReg->UART_MSR & (1<<4);

    return cts_status;
}


void UARTSetAutoRTS(eUART_CH uartPort)
{
    UART_REG *  pReg ;
    pReg = UARTPORT(uartPort);

    pReg->UART_FCR = IRDA_SIR_DISABLED <<6 | 0<<4 | 1<<0;
    pReg->UART_MCR = AUTO_FLOW_ENSABLED | 1<<1; //request to send
}

#endif
/*
********************************************************************************
*
*                         End of Uart.c
*
********************************************************************************
*/

