/*
********************************************************************************************
*
*  Copyright (c): 2014 - 2014 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\I2C\I2cDevice.c
* Owner: chad.Ma
* Date: 2014.11.11
* Time: 9:02:48
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.Ma     2014.11.11     9:02:48   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_I2C_I2CDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "FreeRTOS.h"
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"


//******************************************************************************
//i2c function test
#if 1
#define  RTC_HYM8563_IICAddr   0xA2
#define  RTC_HYM8563_IICSpeed  50

#define   RTC_CTL1      0x00
#define   RTC_CTL2      0x01
#define   RTC_SEC       0x02
#define   RTC_MIN       0x03
#define   RTC_HOUR      0x04
#define   RTC_DAY       0x05
#define   RTC_WEEK      0x06
#define   RTC_MON       0x07
#define   RTC_YEAR      0x08
#define   RTC_A_MIN     0x09
#define   RTC_A_HOUR    0x0A
#define   RTC_A_DAY     0x0B
#define   RTC_A_WEEK    0x0C
#define   RTC_CLKOUT    0x0D
#define   RTC_T_CTL     0x0E
#define   RTC_T_COUNT   0x0F
#define   CENTURY   0x80
#define   TI        0x10
#define   AF        0x08
#define   TF        0x04
#define   AIE       0x02
#define   TIE       0x01
#define   FE        0x80
#define   TE        0x80
#define   FD1       0x02
#define   FD0       0x01
#define   TD1       0x02
#define   TD0       0x01
#define   VL        0x80

#define  Codec_IICAdress   (0x1a<<1)
#define  Codec_IICSpeed    200//100

#define  ALC5633_Codec_IICAdress   (0x1C<<1)
#define  ALC5633_Codec_IICSpeed    200

#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _I2C_TX_ITEM
{
    UINT8 SlaveAddress;
    UINT8 RegAddr;
    eI2C_RW_mode_t RWmode;
    eI2C_Address_Reg_t addr_reg_fmt;
    uint8* DataBuf;
    uint32 size;

}I2C_TX_ITEM;


typedef  struct _I2C_RX_ITEM
{
    UINT8 SlaveAddress;
    UINT8 RegAddr;
    eI2C_RW_mode_t RWmode;
    eI2C_Address_Reg_t addr_reg_fmt;
    uint8* DataBuf;
    uint32 size;
}I2C_RX_ITEM;

typedef  struct _I2C_DEVICE_CLASS
{
    DEVICE_CLASS    stI2CDevice;            //device father class

    pSemaphore osI2CReadReqSem;
    pSemaphore osI2CWriteReqSem;

    pSemaphore osI2CReadSem;
    pSemaphore osI2CWriteSem;


    I2C_TX_ITEM          stTxItem;
    I2C_RX_ITEM          stRxItem;

    uint32           dwTxNeedtoSend;
    uint32           dwRxNeedtoRev;
    uint32           dwTxStatus;
    uint32           dwRxStatus;

    UINT16 speed ;
    eI2C_mode_t mode;
    uint32 Channel;

}I2C_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//static I2C_DEVICE_CLASS * gpstI2cDevHandler[I2C_CH_MAX];
static I2C_DEVICE_CLASS * gpstI2cDevHandler[I2C_CH_MAX];
UINT8     gI2C_flag;
UINT8     gI2C_intType;
UINT8     gI2C_slaveaddr;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t I2CDevShellBspTestSR(HDC dev, uint8 * pstr);
void I2CDevIntIsr2(void);
void I2CDevIntIsr1(void);
void I2CDevIntIsr0(void);
void I2CDevShellIsrHandle(void);
rk_err_t I2CDevShellBspTestSpeed(HDC dev,  uint8 * pstr);
rk_err_t I2CDevShellBspSendData(HDC dev,  uint8 * pstr);
rk_err_t I2CDevShellBspReceiveData(HDC dev,  uint8 * pstr);
rk_err_t I2CDevShellBspSetSpeed(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellOpen(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellClose(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellSuspend(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellResume(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellRead(HDC dev, uint8 * pstr);
rk_err_t I2CDevShellWrite(HDC dev,  uint8 * pstr);
rk_err_t I2CDevShellControl(HDC dev,  uint8 * pstr);
rk_err_t I2CDevShellHelp(HDC dev, uint8 * pstr);


rk_err_t I2CDevResume(HDC dev);
rk_err_t I2CDevSuspend(HDC dev);
rk_err_t I2CDevDelete(HDC dev);
rk_err_t I2CDevDeInit(HDC dev);
rk_err_t I2CDevInit(HDC dev);
void I2CDevIntIsr(uint32 i2cDevID);

BOOL I2C_Init(eI2C_CHANNEL i2cPort,uint16 speed);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDev_Read
** Input:HDC dev
** Return: rk_size_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON API rk_size_t I2cDev_ReadData(HDC dev, uint8* DataBuf,uint32 size,I2C_CMD_ARG *Rx_arg)
{
    eI2C_CHANNEL i2cport;
    rk_size_t realsize;
    I2C_DEVICE_CLASS * pstI2CDev =  (I2C_DEVICE_CLASS *)dev;

    int TimeOut;

    TimeOut = I2C_TIMEOUT_US;   //system tick 10ms,here 2's timeout.



    i2cport = pstI2CDev->stI2CDevice.DevID;
    rkos_semaphore_take(pstI2CDev->osI2CReadReqSem, MAX_DELAY);
//    gpstI2cDevHandler[i2cport] = pstI2CDev;

    rkos_enter_critical();
    pstI2CDev->dwRxStatus  = DEVICE_STATUS_SYNC_PIO_READ;
    pstI2CDev->stRxItem.DataBuf         = DataBuf;
    pstI2CDev->stRxItem.SlaveAddress    = Rx_arg->SlaveAddress;
    pstI2CDev->stRxItem.RegAddr         = Rx_arg->RegAddr;
    pstI2CDev->stRxItem.RWmode          = Rx_arg->RWmode;
    pstI2CDev->stRxItem.addr_reg_fmt    = Rx_arg->addr_reg_fmt;

    pstI2CDev->stRxItem.size = size;
    pstI2CDev->dwRxNeedtoRev = size;

    pstI2CDev->mode = ReadMode;

    //set clock speed
    if (pstI2CDev->speed != Rx_arg->speed)
    {
        I2CSetSpeed(i2cport,Rx_arg->speed);
        pstI2CDev->speed = Rx_arg->speed;
    }
    rkos_exit_critical();

    I2CDisableIRQ(i2cport); //clear all irq

    I2CStart(i2cport);

    do
    {
        TimeOut--;
        __WFI();
    }
    while ((!gI2C_flag) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag = 0;
        TimeOut = I2C_TIMEOUT_US;
    }
    else
    {
        I2C_Deinit(i2cport);
        return TIMEOUT;
    }

    realsize = i2c_read(i2cport,
                        pstI2CDev->stRxItem.SlaveAddress|ReadMode,
                        pstI2CDev->stRxItem.RegAddr,
                        pstI2CDev->stRxItem.addr_reg_fmt,
                        pstI2CDev->stRxItem.DataBuf,
                        pstI2CDev->stRxItem.size,
                        pstI2CDev->stRxItem.RWmode);

    if (realsize == pstI2CDev->dwRxNeedtoRev)
    {
        pstI2CDev->dwRxNeedtoRev = 0;
    }
    else
    {
        pstI2CDev->stRxItem.DataBuf += realsize;
        pstI2CDev->dwRxNeedtoRev    -= realsize;
    }

    I2CEnableInt(i2cport, I2C_MBRFIEN | I2C_NAKRCVIEN);

    do
    {
        TimeOut--;
        __WFI();
    }
    while ((!gI2C_flag) && (TimeOut > 0));

    if ( TimeOut )
    {
        gI2C_flag = 0;
        TimeOut = I2C_TIMEOUT_US;

        if ( gI2C_intType & I2C_NAKRCVIPD )
        {
            rk_printf("I2C_NAKRCVIPD------\n");
            I2CStop(i2cport );
            while (!gI2C_flag)
                __WFI();
            gI2C_flag = 0;

            I2C_Deinit(i2cport);
            I2CDisableInt(i2cport, I2C_MBRFIEN | I2C_NAKRCVIEN);
            return ERROR;   //need retry to send again.
        }
    }
    else
    {
        I2C_Deinit(i2cport);
        I2CDisableInt(i2cport, I2C_MBRFIEN | I2C_NAKRCVIEN);
        rk_printf("I2CRead_TIMEOUT------\n");
        return TIMEOUT;
    }

    rkos_semaphore_take(pstI2CDev->osI2CReadSem, MAX_DELAY);
    I2CDisableInt(i2cport, I2C_MBRFIEN | I2C_NAKRCVIEN);
    rkos_semaphore_give(pstI2CDev->osI2CReadReqSem);
    return size;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDevIntIsr2
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.11.26
** Time: 18:16:50
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN void I2CDevIntIsr2(void)
{
    I2CDevIntIsr(I2C_CH2);
}

/*******************************************************************************
** Name: I2CDevIntIsr1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.11.26
** Time: 18:16:24
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN void I2CDevIntIsr1(void)
{
    I2CDevIntIsr(I2C_CH1);
}

/*******************************************************************************
** Name: I2CDevIntIsr0
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.11.26
** Time: 18:16:01
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN void I2CDevIntIsr0(void)
{
    I2CDevIntIsr(I2C_CH0);
}

/*******************************************************************************
** Name: I2CDevIntIsr
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN void I2CDevIntIsr(uint32 i2cDevID)
{
    uint32 intpending;
    uint32 realsize;
    gI2C_flag = 1;
    //gI2C_intType = I2CGetInt(i2cDevID);

    if (i2cDevID >= I2C_CH_MAX)
        return;

    I2C_DEVICE_CLASS * pstI2CDev =  gpstI2cDevHandler[i2cDevID];

    if ( pstI2CDev == NULL)
    {
        //rk_printf("I2CDevIntIsr : error i2c dev.");
        return;
    }

    intpending = I2CGetInt(i2cDevID);

    if ( intpending & I2C_NAKRCVIPD )
    {
        //printf("I2C_NAKRCVIPD------\n");
        pstI2CDev->dwTxNeedtoSend = 1;
        // Send stop bit
        I2CStop(i2cDevID );
        // Disable Controller
        I2C_Deinit(i2cDevID);

        if ( WriteMode == pstI2CDev->mode )
            rkos_semaphore_give_fromisr(pstI2CDev->osI2CWriteSem );
        else if ( ReadMode == pstI2CDev->mode )
            rkos_semaphore_give_fromisr(pstI2CDev->osI2CReadSem );
        return;
    }

    if ( intpending & I2C_MBTFIPD ) //??????
    {
        if ( DEVICE_STATUS_SYNC_PIO_WRITE == pstI2CDev->dwTxStatus)
        {
            if ( pstI2CDev->dwTxNeedtoSend )
            {   /*in interrupt handle,set slave address 0x0 as continus Tx flag*/
                realsize = i2c_write(i2cDevID,
                                     0,
                                     pstI2CDev->stTxItem.RegAddr,
                                     pstI2CDev->stTxItem.addr_reg_fmt,
                                     pstI2CDev->stTxItem.DataBuf,
                                     pstI2CDev->dwTxNeedtoSend,
                                     pstI2CDev->stTxItem.RWmode);

                pstI2CDev->stTxItem.DataBuf += realsize;
                pstI2CDev->dwTxNeedtoSend -= realsize;

                if (0 == pstI2CDev->dwTxNeedtoSend )//send over
                {
                    rkos_semaphore_give_fromisr(pstI2CDev->osI2CWriteSem);
                    pstI2CDev->dwTxStatus = DEVICE_STATUS_IDLE;
                    I2CStop(i2cDevID);
                    I2C_Deinit(i2cDevID);
                    return;
                }
            }
            else if ( 0 == pstI2CDev->dwTxNeedtoSend )
            {
                rkos_semaphore_give_fromisr(pstI2CDev->osI2CWriteSem);
                pstI2CDev->dwTxStatus = DEVICE_STATUS_IDLE;
                I2CStop(i2cDevID);
                I2C_Deinit(i2cDevID);
                return;
            }
        }
    }

    if ( intpending & I2C_MBRFIPD ) //??????
    {
        if ( DEVICE_STATUS_SYNC_PIO_READ == pstI2CDev->dwRxStatus)
        {
            if ( pstI2CDev->dwRxNeedtoRev )
            {   /*in interrupt handle,set slave address 0x0 as continus Rv flag*/
                realsize = i2c_read(i2cDevID,
                                    0,
                                    pstI2CDev->stRxItem.RegAddr,
                                    pstI2CDev->stRxItem.addr_reg_fmt,
                                    pstI2CDev->stRxItem.DataBuf,
                                    pstI2CDev->dwRxNeedtoRev,
                                    pstI2CDev->stRxItem.RWmode);

                pstI2CDev->stRxItem.DataBuf += realsize;
                pstI2CDev->dwRxNeedtoRev -= realsize;

                if ( 0 == pstI2CDev->dwRxNeedtoRev )//recive over
                {
                    rkos_semaphore_give_fromisr(pstI2CDev->osI2CReadSem);
                    pstI2CDev->dwRxStatus = DEVICE_STATUS_IDLE;
                    I2CStop(i2cDevID);
                    I2C_Deinit(i2cDevID);
                    return;
                }
            }
            else if (0 == pstI2CDev->dwRxNeedtoRev)
            {
                rkos_semaphore_give_fromisr(pstI2CDev->osI2CReadSem);
                pstI2CDev->dwRxStatus = DEVICE_STATUS_IDLE;
                I2CStop(i2cDevID);
                I2C_Deinit(i2cDevID);
                return;
            }
        }
    }
    else
    {
        ;;//rk_printf(" \n  ###### Read OK ######\n");
    }
}
/*******************************************************************************
** Name: I2CDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN rk_err_t I2CDevResume(HDC dev)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: I2CDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON FUN rk_err_t I2CDevSuspend(HDC dev)
{
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2cDev_SendData
** Input:HDC dev,uint8* DataBuf,uint32 size,I2C_DEVICE_ARG *Tx_arg
** Return: rk_size_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:11:38
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_COMMON_
COMMON API rk_size_t I2cDev_SendData(HDC dev,uint8* DataBuf,uint32 size,I2C_CMD_ARG *Tx_arg)
{
    eI2C_CHANNEL    i2cport;
    I2C_DEVICE_CLASS * pstI2CDev;
    rk_size_t realsize = 0;
    rk_size_t doltaSize = 0;  //slave addr and register addr length

    int TimeOut;

    TimeOut = I2C_TIMEOUT_US;   //system tick 10ms,here 2's timeout.


    pstI2CDev =  (I2C_DEVICE_CLASS *)dev;
    i2cport = pstI2CDev->stI2CDevice.DevID;
    //rk_printf("rkos_semaphore_take");
    rkos_semaphore_take(pstI2CDev->osI2CWriteReqSem, MAX_DELAY);
    // rk_printf("rkos_semaphore_take OK");
    rkos_enter_critical();
    pstI2CDev->dwTxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;
    pstI2CDev->stTxItem.DataBuf         =  DataBuf;
    pstI2CDev->stTxItem.SlaveAddress    = Tx_arg->SlaveAddress;
    pstI2CDev->stTxItem.RegAddr         = Tx_arg->RegAddr;
    pstI2CDev->stTxItem.RWmode          = Tx_arg->RWmode;
    pstI2CDev->stTxItem.addr_reg_fmt    = Tx_arg->addr_reg_fmt;

    pstI2CDev->stTxItem.size = size;

    pstI2CDev->dwTxNeedtoSend = size;

    if (Tx_arg->addr_reg_fmt == I2C_7BIT_ADDRESS_8BIT_REG
            || Tx_arg->addr_reg_fmt == I2C_7BIT_ADDRESS_16BIT_REG)
    {
        doltaSize += 1;
    }
    else if (Tx_arg->addr_reg_fmt == I2C_10BIT_ADDRESS_8BIT_REG
             || Tx_arg->addr_reg_fmt == I2C_10BIT_ADDRESS_16BIT_REG)
    {
        doltaSize += 2;
    }

    if ( Tx_arg->RWmode == NormalMode)
    {
        if (Tx_arg->addr_reg_fmt == I2C_7BIT_ADDRESS_8BIT_REG
                || Tx_arg->addr_reg_fmt == I2C_7BIT_ADDRESS_16BIT_REG)
        {
            doltaSize += 1;
        }
        else if (Tx_arg->addr_reg_fmt == I2C_10BIT_ADDRESS_8BIT_REG
                 || Tx_arg->addr_reg_fmt == I2C_10BIT_ADDRESS_16BIT_REG)
        {
            doltaSize += 2;
        }
    }
    else if ( Tx_arg->RWmode == DirectMode)
    {
        doltaSize += 0;
    }
    rkos_exit_critical();

    //set clock speed
    if (pstI2CDev->speed != Tx_arg->speed)
    {
        I2CSetSpeed(i2cport,Tx_arg->speed);
        pstI2CDev->speed = Tx_arg->speed;
    }
    pstI2CDev->mode = WriteMode;

    I2CDisableIRQ(i2cport); //clear all irq

    I2CStart(i2cport);
    do
    {
        TimeOut--;
        __WFI();
    }
    while ((!gI2C_flag) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag = 0;
        TimeOut = I2C_TIMEOUT_US;
    }
    else
    {
        I2C_Deinit(i2cport);
        return TIMEOUT;
    }

    realsize = i2c_write(i2cport,
                         pstI2CDev->stTxItem.SlaveAddress|WriteMode,
                         pstI2CDev->stTxItem.RegAddr,
                         pstI2CDev->stTxItem.addr_reg_fmt,
                         pstI2CDev->stTxItem.DataBuf,
                         pstI2CDev->stTxItem.size,
                         pstI2CDev->stTxItem.RWmode);
    //rk_printf("pstI2CDev->dwTxNeedtoSend=%d",pstI2CDev->dwTxNeedtoSend);
    if ((realsize - doltaSize) == pstI2CDev->dwTxNeedtoSend)
    {
        pstI2CDev->dwTxNeedtoSend = 0;
    }
    else
    {
        pstI2CDev->stTxItem.DataBuf += (realsize - doltaSize);
        pstI2CDev->dwTxNeedtoSend   -= (realsize - doltaSize);
    }
    I2CEnableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);

    do
    {
        TimeOut--;
        __WFI();
    }
    while ((!gI2C_flag) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag = 0;
        TimeOut = I2C_TIMEOUT_US;

        if ( gI2C_intType & I2C_NAKRCVIPD ) //ACK?'�?�???3?'�??????D??��?
        {
            rk_printf("\n------I2C_NAKRCVIPD------\n");
            I2CStop(i2cport );
            while (!gI2C_flag)
                __WFI();
            gI2C_flag = 0;

            gI2C_intType = 0;

            I2C_Deinit(i2cport);
            I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);
            return ERROR;   //need retry to send again.
        }
    }
    else
    {
        I2C_Deinit(i2cport);
        I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);
        rk_printf("I2CSend_TIMEOUT------\n");
        return TIMEOUT;
    }

//     while(needSend)
//     {
//        if ( gI2C_intType & I2C_MBTFIPD )
//        {
//            int realsize = 0;
//            /*Note:in interrupt handle,set slave address 0x0 as continus Tx flag*/
//            realsize = i2c_write(i2cport,
//                                  0,
//                                  pstI2CDev->stTxItem.RegAddr,
//                                  I2C_7BIT_ADDRESS_8BIT_REG,
//                                  pstI2CDev->stTxItem.DataBuf,
//                                  needSend,
//                                  pstI2CDev->stTxItem.RWmode);
//            do
//            {
//                TimeOut--;
//                //__WFI();
//            }while((!gI2C_flag) && (TimeOut > 0));

//            if(TimeOut)
//            {
//                gI2C_flag = 0;
//                TimeOut = I2C_TIMEOUT_US;
//
//                pstI2CDev->stTxItem.DataBuf       += realsize;
//                needSend    -= realsize;
//                sendedSize  += realsize;
//            }
//            else
//            {
//                break;
//            }
//        }

//        if( gI2C_intType & I2C_NAKRCVIPD )  //ACK?'�?�???3?'�??????D??��?
//        {
//            rk_printf("I2C_NAKRCVIPD------\n");
//        	I2CStop(i2cport );
//            while(!gI2C_flag)
//                //__WFI();
//            gI2C_flag = 0;

//            I2C_Deinit(i2cport);
//            I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);
//            gI2C_intType = 0;
//            return ERROR;   //need retry to send again.
//        }
//
//    }//while( needSend );  //�?�??�D?�??�

//    I2CStop(i2cport );
//    do
//    {
//        TimeOut--;
//        //__WFI();
//    }while((!gI2C_flag) && (TimeOut > 0));

//    if(TimeOut)
//    {
//        gI2C_flag = 0;
//
//        I2C_Deinit(i2cport);
//        I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);
//        gI2C_intType = 0;

//        rk_printf(" \n  ###### Write OK ######\n");

//        return sendedSize;
//    }
//    else
//    {
//        I2C_Deinit(i2cport);
//        I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);
//        return TIMEOUT;
//    }


    rkos_semaphore_take(pstI2CDev->osI2CWriteSem, MAX_DELAY);

    I2CDisableInt(i2cport, I2C_MBTFIEN | I2C_NAKRCVIEN);

    rkos_semaphore_give(pstI2CDev->osI2CWriteReqSem);

    return  pstI2CDev->dwTxNeedtoSend;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_INIT_
INIT API rk_err_t I2CDev_Delete(uint32 DevID, void * arg)
{
#if 0
    eI2C_CHANNEL i2cport;

    I2C_DEVICE_CLASS * pstI2CDevHandler = (I2C_DEVICE_CLASS *) dev;
    i2cport = pstI2CDevHandler->stI2CDevice.DevID;
    gpstI2cDevHandler[i2cport] = NULL;
    I2C_Deinit(i2cport);

    switch ( i2cport )
    {
        case I2C_CH0:
            IntDisable(INT_ID_I2C0);
            IntPendingClear(INT_ID_I2C0);
            IntUnregister(INT_ID_I2C0);
            ScuClockGateCtr(PCLK_I2C0_GATE,0);
            break;
        case I2C_CH1:
            IntDisable(INT_ID_I2C1);
            IntPendingClear(INT_ID_I2C1);
            IntUnregister(INT_ID_I2C1);
            ScuClockGateCtr(PCLK_I2C1_GATE,0);
            break;
        case I2C_CH2:
            IntDisable(INT_ID_I2C2);
            IntPendingClear(INT_ID_I2C2);
            IntUnregister(INT_ID_I2C2);
            ScuClockGateCtr(PCLK_I2C2_GATE,0);
            break;

        default:
            break;
    }

//    ScuClockGateCtr(PCLK_I2C0_GATE,0);

    rkos_semaphore_delete( pstI2CDevHandler->osI2CReadReqSem );
    rkos_semaphore_delete( pstI2CDevHandler->osI2CWriteReqSem );
    rkos_semaphore_delete( pstI2CDevHandler->osI2CReadSem );
    rkos_semaphore_delete( pstI2CDevHandler->osI2CWriteSem );

    rkos_memory_free(dev);
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_INIT_
INIT API HDC I2CDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* pstDev;
    I2C_DEVICE_CLASS * psI2CDevHandler;
    I2C_DEVICE_ARG *stI2cArg = (I2C_DEVICE_ARG *)arg;

    //rkos_enter_critical();
    psI2CDevHandler =  rkos_memory_malloc(sizeof(I2C_DEVICE_CLASS));
    if (psI2CDevHandler == NULL)
    {
        rk_print_string("11I2CDev_Create rkos_memory_malloc NULL\r\n");
        return psI2CDevHandler;
    }
    //rk_print_string("22I2CDev_Create rkos_memory_malloc OK\r\n");
    psI2CDevHandler->osI2CReadReqSem  = rkos_semaphore_create(1,1);
    psI2CDevHandler->osI2CWriteReqSem = rkos_semaphore_create(1,1);
    psI2CDevHandler->osI2CReadSem     = rkos_semaphore_create(1,0);
    psI2CDevHandler->osI2CWriteSem    = rkos_semaphore_create(1,0);
    //rk_print_string("I2CDev_RW_semaphore_creat OK\r\n");
    if ((psI2CDevHandler->osI2CReadReqSem
            && psI2CDevHandler->osI2CWriteReqSem
            && psI2CDevHandler->osI2CReadSem
            && psI2CDevHandler->osI2CWriteSem) == 0)
    {
        rk_print_string("i2c_semaphore_busy_free_memory\r\n");
        rkos_semaphore_delete(psI2CDevHandler->osI2CReadReqSem);
        rkos_semaphore_delete(psI2CDevHandler->osI2CWriteReqSem);
        rkos_semaphore_delete(psI2CDevHandler->osI2CReadSem);
        rkos_semaphore_delete(psI2CDevHandler->osI2CWriteSem);

        rkos_memory_free(psI2CDevHandler);

        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)psI2CDevHandler;

    pstDev->suspend = I2CDevSuspend;
    pstDev->resume  = I2CDevResume;
    pstDev->DevID   = DevID;

    psI2CDevHandler->Channel = stI2cArg->Channel;

    gpstI2cDevHandler[DevID] = psI2CDevHandler;

    I2cDevHwInit(DevID,psI2CDevHandler->Channel);

    // one-time initialization code, and do not affect the hardware device
    I2CDevInit(psI2CDevHandler);

    //rkos_exit_critical();
    //rk_print_string("I2CDevice.c I2CDevInit ok\r\n");
    return pstDev;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDevDeInit
** Input:I2C_DEVICE_CLASS * pstI2C
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_INIT_
INIT FUN rk_err_t I2CDevDeInit(HDC dev)
{
    I2C_DEVICE_CLASS * pstI2CDev;

    pstI2CDev = (I2C_DEVICE_CLASS *)dev;
    switch (pstI2CDev->stI2CDevice.DevID)
    {
        case I2C_CH0:
            IntDisable(INT_ID_I2C0);
            IntPendingClear(INT_ID_I2C0);
            IntUnregister(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntDisable(INT_ID_I2C1);
            IntPendingClear(INT_ID_I2C1);
            IntUnregister(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntDisable(INT_ID_I2C2);
            IntPendingClear(INT_ID_I2C2);
            IntUnregister(INT_ID_I2C2);
            break;

        default:
            break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevInit
** Input:I2C_DEVICE_CLASS * pstI2C
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
INIT FUN rk_err_t I2CDevInit(HDC dev)
{
    eI2C_CHANNEL i2cport;
    I2C_DEVICE_CLASS * pstI2CDevHandler = (I2C_DEVICE_CLASS *) dev;

    if (NULL == pstI2CDevHandler )
        return RK_PARA_ERR;

    i2cport = pstI2CDevHandler->stI2CDevice.DevID;
    rk_printf("I2CDevInit FUN\n");

    return RK_SUCCESS;
}

//******************************************************************************




#ifdef _I2C_DEV_SHELL_
_DRIVER_I2C_I2CDEVICE_SHELL_
static char* ShellI2CName[] =
{
    "pcb",
    "open",
    "close",
    "suspend",
    "resume",
    "read",
    "write",
    "control",
    "help",
    "bsp",
    "\b",                          // the end
};

#ifdef SHELL_BSP
static uint8* ShellI2CBspName[] =
{
    "help",
    "setspeed",
    "send",
    "recive",
    "testspeed",
    "testsr",
    "\b",
};

#define     SlaveIICAdress       (0x1a<<1) //wm8987 //(0x1C << 1) alc5633
#define     SlaveRegAddr          WM8987_R15    //reset

/* number of I2C communication retransmission */
#define 	I2C_RETRYCOUNT		10

/*keep current i2c channel*/
int32       gI2C_channal;
UINT8       gI2C_slavereg;
UINT8       gI2C_mode;
UINT8       gI2C_wrmode;
int32       gI2C_WRSize;
int32       gI2C_RxIndex;

int32       gI2C_remainedSize;
int32       gI2C_dotaSize;
int32       gI2C_sendedSize;
int32       gI2C_recivedSize;
UINT32      gI2C_cnt;
uint8       gI2C_Data[2];

uint8       gI2CBuffer[120];
UINT16	    cwFM5807Default[]=
{
    //init data table
    0x0000,
    0x0000,
    0xc081,//0x02		//d881-->9881 for first init don't need voice output,0xb881 no stereo
    0x1a10,			//97100KHZ
    0x0600,			//no interruput
    0x9cad, //05h    0x88, 0xAF
    0x0000,
    0x46c6,
    0x5096,
    0x0000,
    0x0468, //0x0a
    0x650f,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0xf027,//4c08//0x4808,//0x10
    0x9000,
    0xf483,
    0x7001,
    0x40f0,
    0x6180, //0x15
    0x01D6,
    0x10bb,
    0x55a4,
    0xe848,
    0x5080,//0x1a
    0x0000,
    0x37b6,
    0x420c,
    0x079b,
    0x4c1d,
    0x8810,//0x20
    0x45c0,
    0x0923,
    0x98a4,
    0x0500,
    0x1928, //0x25
    0x2d04,
    0x002f,
    0x17af,
    0xd34e,
    0x348c,//0x2a
    0xa004,
    0x7eac,
    0x0014,
    0x0CFF,
    0x3C1E,
    0x0200, //30H:
    0x16F4,
    0xBD00,
    0x830C,
    0xB9FC,
    0x8287, //35H:
    0x0000,
    0x0000,
    0x028B,
    0x8895,
    0x8D9F, //3AH:
    0x92AA,
};

#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL API rk_err_t I2CDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    uint   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    rk_print_string("I2C shell cmd \r\n");
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }
    while (memcmp((uint8*)ShellI2CName[i],pItem,strlen(ShellI2CName[i])) != 0)   //compare the name
    {
        i++;
        if (*((uint8*)ShellI2CName[i]) == '\b')
            return RK_ERROR;
    }

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item
    switch (i)
    {
        case 0x00:  //pcb
            ret = I2CDevShellPcb(dev,pItem);
            break;

        case 0x01:  //open
            ret = I2CDevShellOpen(dev,pItem);
            break;

        case 0x02:  //close
            ret = I2CDevShellClose(dev,pItem);
            break;

        case 0x03:  //suspend
            ret = I2CDevShellSuspend(dev,pItem);
            break;

        case 0x04:  //resume
            ret = I2CDevShellResume(dev,pItem);
            break;

        case 0x05:  //read
            ret = I2CDevShellRead(dev,pItem);
            break;

        case 0x06:  //write
            ret = I2CDevShellWrite(dev,pItem);
            break;

        case 0x07:  //control
            ret = I2CDevShellControl(dev,pItem);
            break;

        case 0x08:  //help
#ifdef SHELL_HELP
            ret = I2CDevShellHelp(dev,pItem);
#endif
            break;

        case 0x09:  //bsp
#ifdef SHELL_BSP
            ret = I2CDevShellBsp(dev,pItem);
#endif
            break;
        default:
            ret = RK_ERROR;
            break;
    }

    return ret;

}


#ifdef SHELL_BSP
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2CDevShellBspTestSR
** Input:HDC dev, uint8 * pstr
** Return: rk_err_T
** Owner:chad.Ma
** Date: 2014.12.7
** Time: 11:08:40
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspTestSR(HDC dev, uint8 * pstr)
{
    uint16  _data;
    uint32 cnt = 1;
    uint16 times = 0;
    uint8 datatoSend[2];
    UINT8 dataRcv[2];



#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.bsp.sr : ??i2c??????????????.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (*pstr == NULL)
    {
        rk_print_string("pstr == NULL");
        cnt = 1;
    }
    else
    {
        while (pstr[0] == ' ')
            pstr++;
        cnt = StringtoNum( pstr );
        rk_printf("cnt==%d",cnt);
        if (cnt == 0)
            return RK_PARA_ERR;

        times = cnt;
    }

#if 1   //WM8987codec i2c_channel0 test
    //RTC_hym8563Test();
    uint8 cmd,second,minute,hour;
    uint32 ret;
    uint8 readBuf[3];
    uint8 time[3];

    I2C_DEVICE_CLASS *dev_8563;
    I2C_DEVICE_ARG stI2cArg;
    I2C_CMD_ARG RTx_arg;
    second = 19;
    minute = 19;
    hour   = 20;

    time[0] = ((second / 10) << 4) + (second % 10);
    time[1] = ((minute / 10) << 4) + (minute % 10);
    time[2] = ((hour / 10) << 4) + (hour % 10);

    rk_printf("dev_8563 inti\n");

    stI2cArg.devID = I2C_CH0;

    rk_printf("RKDev_Create I2C_CH0\n");

    ret = RKDev_Create(DEV_CLASS_I2C, I2C_CH0,&stI2cArg);//&stI2cArg
    if (ret != RK_SUCCESS)
    {
        rk_printf("I2cDev create failure ret=%d\n",ret);
    }

    dev_8563 = (I2C_DEVICE_CLASS*)RKDeviceFind(DEV_CLASS_I2C,I2C_CH0);

    RTx_arg.SlaveAddress = RTC_HYM8563_IICAddr;
    RTx_arg.RegAddr = RTC_SEC;
    RTx_arg.RWmode = NormalMode;//NormalMode;
    RTx_arg.addr_reg_fmt = I2C_7BIT_ADDRESS_8BIT_REG;
    RTx_arg.speed = RTC_HYM8563_IICSpeed;
    ret =  I2cDev_SendData(dev_8563,time,3,&RTx_arg);
    cnt = 10;

    while (ret < 0) //ack not recived,try again.
    {
        DelayMs(5);
        //rk_size_t I2cDev_SendData(HDC dev,uint8* DataBuf,uint32 size,I2C_CMD_ARG *Tx_arg)
        ret =  I2cDev_SendData(dev_8563,time,3,&RTx_arg);

        rk_printf("send data %d \n",ret);
    }

    while (cnt--)
    {
        int i = 0;
        rk_printf(" Set time 20:19:19 \n");
        I2cDev_ReadData(dev_8563,&readBuf[0],3,&RTx_arg);
    }
    rk_printf(" second = %02x \n",readBuf[0]);
    rk_printf(" minute = %02x \n",readBuf[1]);
    rk_printf(" hour   = %02x \n",readBuf[2]);

//    _data = 0x012f;
//    gI2C_flag = 0;
//    gI2C_channal = I2C_CH0;
//    gI2C_slaveaddr = WM8987codec; //FM5800;//WM8987codec;    //use wm8987 / alc5633 as slave device.

//    gI2C_slavereg  = (UINT8)(((SlaveRegAddr << 9) | _data) >> 8);
//    gI2C_mode = WriteMode;
//
//    //open I2C clk
//    ScuClockGateCtr(PCLK_I2C0_GATE, 1);
//    //SetUartFreq(SYS_DEFAULT_FREQ);

//    //open rst I2C ip
//    ScuSoftResetCtr(RST_I2C, 1);
//    DelayMs(1);
//    ScuSoftResetCtr(RST_I2C, 0);

//    I2CDevGpioInit(gI2C_channal,GPIO_CH0);

//    I2C_Init(gI2C_channal,10); // 10 for WM8987codec

//    //I2CSetSpeed(devID, 100);

//    datatoSend[0] = (uint8)_data;
//    i2c_write(gI2C_channal,
//              gI2C_slaveaddr,
//              gI2C_slavereg,
//              I2C_7BIT_ADDRESS_8BIT_REG,
//              &datatoSend[0],
//              1,
//              NormalMode);

//    Delay100cyc(1);


//    gI2C_slavereg = (uint8)(WM8987_R27 << 1);
//    i2c_read(gI2C_channal,
//              gI2C_slaveaddr,
//              gI2C_slavereg,
//              I2C_7BIT_ADDRESS_8BIT_REG,
//              dataRcv,
//              1,
//              NormalMode);
//    rk_printf("i2c read 0x%02x reg ,read data is 0x%02x",gI2C_slavereg,dataRcv[0] );

#else   //fm5807 i2c_channel1 test
    rk_printf("fm5800 i2c_channel1 test\n");
    gI2C_flag = 0;
    gI2C_channal = I2C_CH1;
    gI2C_slaveaddr =FM5800;// FM5800;//FM5800;//WM8987codec;    //use wm8987 / alc5633 as slave device.
    gI2C_slavereg  = 0x02;     //fm5800 no resgister addr
    gI2C_mode = WriteMode;

    //open I2C clk
    ScuClockGateCtr(PCLK_I2C1_GATE, 1);
    //SetUartFreq(SYS_DEFAULT_FREQ);

    I2CDevGpioInit(gI2C_channal,GPIO_CH0);
    //open rst I2C ip
    ScuSoftResetCtr(I2C1_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(I2C1_SRST, 0);

    I2C_Init(gI2C_channal,gI2C_slaveaddr,400); // 10 for WM8987codec,FM5807 has pull-up register
    switch (gI2C_channal)
    {
        case I2C_CH0:
            IntRegister(INT_ID_I2C0 ,I2CDevIntIsr0);
            IntPendingClear(INT_ID_I2C0);
            IntEnable(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntRegister(INT_ID_I2C1 ,I2CDevIntIsr1);
            IntPendingClear(INT_ID_I2C1);
            IntEnable(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntRegister(INT_ID_I2C2 ,I2CDevIntIsr2);
            IntPendingClear(INT_ID_I2C2);
            IntEnable(INT_ID_I2C2);
            break;

        default:
            break;
    }

    _data = 0xc081;

    while (cnt)
    {
        datatoSend[0] = _data >> 8;
        datatoSend[1] = _data & 0xff;
        rk_printf("i2c_write \n");

        i2c_write(gI2C_channal,
                  gI2C_slaveaddr|WriteMode,
                  gI2C_slavereg,
                  I2C_7BIT_ADDRESS_8BIT_REG,
                  datatoSend,
                  2,
                  DirectMode/*NormalMode*/);//driectmode for fm5807
        DelayMs(500);
        cnt--;
    }

    cnt = times;
    rk_printf("i2c_write times = %d \n",times);
    cnt=1;
    while (cnt)
    {
        int ret;
        I2CStart(gI2C_channal);
        ret = i2c_read(gI2C_channal,
                       gI2C_slaveaddr|ReadMode,
                       gI2C_slavereg,
                       I2C_7BIT_ADDRESS_8BIT_REG,
                       &dataRcv[0],
                       2,
                       DirectMode);  //fm5807 use direct mode
        I2CEnableInt(gI2C_channal, I2C_MBRFIEN | I2C_NAKRCVIEN);
        if ( ret != OK)
        {
            _data = dataRcv[0];
            _data = _data << 8 | dataRcv[1];
            rk_printf("i2c read 0x%02x reg ,read data is 0x%04x\n",gI2C_slavereg,_data );

        }
        else if (ret == OK)
        {
            rk_printf("i2c read : ERROR........ \n");
        }

        DelayMs(500);
        cnt--;

    }

    _data = dataRcv[0];
    _data = _data << 8 | dataRcv[1];

#endif
//    rk_printf("i2c read 0x%02x reg ,read data is 0x%04x",gI2C_slavereg,_data );

}
/*******************************************************************************
** Name: I2CDevShellBspTestSpeed
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 14:33:37
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspTestSpeed(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.bsp.testspeed : ??i2c?????????????,????????.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string("ShellBspTestSpeed\r\n");
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}


void I2CDevShellIsrHandle()
{
    uint32 intType;
    uint32 len;

    gI2C_flag = 1;
    intType = I2CGetInt(gI2C_channal);

    if (intType & I2C_NAKRCVIPD)
    {
        return;
    }

    if (intType & I2C_MBTFIPD && gI2C_remainedSize > 0)
    {
        /*note:here slave address use 0,means send data continus*/
        len = i2c_write(gI2C_channal,
                        0,
                        gI2C_slavereg,
                        I2C_7BIT_ADDRESS_8BIT_REG,
                        gI2CBuffer + (gI2C_sendedSize - gI2C_dotaSize),
                        gI2C_remainedSize,
                        DirectMode/*NormalMode*/);//driectmode for fm5807
        gI2C_sendedSize += len;

        gI2C_remainedSize -= len;
        rk_printf("INT handle : gI2C_remainedSize = %d\n",gI2C_remainedSize);
    }

    if (intType & I2C_MBRFIPD && gI2C_remainedSize > 0)
    {
        len = i2c_read(gI2C_channal,
                       0,
                       gI2C_slaveaddr,
                       I2C_7BIT_ADDRESS_8BIT_REG,
                       gI2CBuffer + gI2C_recivedSize,
                       gI2C_remainedSize,
                       DirectMode/*NormalMode*/);//driectmode for fm5807

        gI2C_recivedSize  += len;
        gI2C_remainedSize -= len;
        rk_printf("INT handle : gI2C_remainedSize = %d\n",gI2C_remainedSize);
    }
}

/*******************************************************************************
** Name: I2CDevShellBspSendData
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 12:00:44
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspSendData(HDC dev,  uint8 * pstr)
{
    uint32 devID;
    uint16  _data;
    uint32 cnt = 1;
    uint16 wNakCount = 0;
    uint16 times = 0;
    uint8 datatoSend[2];
    uint32 sendedLen;
    uint8 i;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.bsp.send : ????5A,????????????.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (*pstr == NULL)
    {
        cnt = 1;
    }
    else
    {
        while (pstr[0] == ' ')
            pstr++;
        cnt = StringtoNum( pstr );
        if (cnt <= 0)
            return RK_PARA_ERR;
    }

    _data = 0xc081; //fm5807 test data
    gI2C_flag = 0;
    devID = gI2C_channal = I2C_CH0;
    gI2C_slaveaddr = RTC_HYM8563_IICAddr;//WM8987codec;    //use alc5633 as slave device.

    gI2C_slavereg  = 0;//(UINT8)(((SlaveRegAddr << 9) | _data) >> 8);
    gI2C_mode = WriteMode;

    //open I2C clk
    ScuClockGateCtr(PCLK_I2C0_GATE, 1);
    //SetUartFreq(SYS_DEFAULT_FREQ);

    //open rst I2C ip
    ScuSoftResetCtr(I2C0_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(I2C0_SRST, 0);

    I2CDevGpioInit(devID,GPIO_CH0);

    I2C_Init(devID,RTC_HYM8563_IICSpeed);

    //I2CSetSpeed(devID, 100);

    switch ((int)devID)
    {
        case I2C_CH0:
            IntRegister(INT_ID_I2C0 ,I2CDevIntIsr0);
            IntPendingClear(INT_ID_I2C0);
            IntEnable(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntRegister(INT_ID_I2C1 ,I2CDevIntIsr1);
            IntPendingClear(INT_ID_I2C1);
            IntEnable(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntRegister(INT_ID_I2C2 ,I2CDevIntIsr2);
            IntPendingClear(INT_ID_I2C2);
            IntEnable(INT_ID_I2C2);
            break;

        default:
            break;
    }

    for (i = 0; i < 24; i++)
    {
        gI2CBuffer[i*2] = (cwFM5807Default[i+2] >> 8);
        gI2CBuffer[i*2+1] = (cwFM5807Default[i+2]);
    }
    gI2C_remainedSize = 48; //??????????
    gI2C_dotaSize     = 1;  //????????????????,?fm5807,????slave address
    gI2C_sendedSize   = 0;  //????????

    while (cnt)
    {
//        datatoSend[0] = _data >> 8;
//        datatoSend[1] = _data & 0xff;

//        I2CEnableInt(devID,I2C_STARTIEN);
        I2CStart(devID);
        while (gI2C_flag == 0);
        gI2C_flag = 0;

        gI2C_sendedSize = i2c_write(devID,
                                    gI2C_slaveaddr,
                                    gI2C_slavereg,
                                    I2C_7BIT_ADDRESS_8BIT_REG,
                                    gI2CBuffer,
                                    48,
                                    DirectMode/*NormalMode*/);//driectmode for fm5807

        gI2C_remainedSize -= (gI2C_sendedSize - gI2C_dotaSize);
        I2CEnableInt(devID,I2C_MBTFIEN | I2C_NAKRCVIEN);

        while (gI2C_flag == 0);
        gI2C_flag = 0;


        // Send stop bit
        I2CStop(devID);

        while (gI2C_flag == 0);
        gI2C_flag = 0;
        // Disable Controller
        I2C_Deinit(devID);

        DelayMs(500);
        cnt--;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevShellBspReceiveData
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 11:58:54
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspReceiveData(HDC dev,  uint8 * pstr)
{
    uint32 cnt;
    uint8  regaddr;
    UINT8 _data[2];
    uint16 regData;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.bsp.recive : ????.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (*pstr == NULL)
    {
        cnt = 1;
    }
    else
    {
        while (pstr[0] == ' ')
            pstr++;
        cnt = StringtoNum( pstr );
        if (cnt == 0)
            return RK_PARA_ERR;
    }
    /*use  wm8987*/
//    regaddr = WM8987_R27;
//    gI2C_channal = I2C_CH0;
//    gI2C_slaveaddr = WM8987codec;       //use  wm8987 / alc5633 as slave device.
//    gI2C_slavereg = (uint8)(regaddr << 1);

    /*use  fm5807*/
    gI2C_channal = I2C_CH0;
    gI2C_slaveaddr = RTC_HYM8563_IICAddr;       //use  wm8987 / FM5800 /ALC5633codecas slave device.
    gI2C_slavereg = 0;

    gI2C_cnt = 0;
    gI2C_flag = 0;

    gI2C_WRSize = 2;
    gI2C_RxIndex = 0;

    //open I2C clk
    ScuClockGateCtr(PCLK_I2C0_GATE, 1);

    //open rst I2C ip
    ScuSoftResetCtr(I2C0_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(I2C0_SRST, 0);

    I2CDevGpioInit(gI2C_channal,GPIO_CH0);

    I2C_Init(gI2C_channal,RTC_HYM8563_IICSpeed); //#define  RTC_HYM8563_IICSpeed  50

    //I2CSetSpeed(devID, 100);

    switch (gI2C_channal)
    {
        case I2C_CH0:
            IntRegister(INT_ID_I2C0 ,I2CDevIntIsr0);
            IntPendingClear(INT_ID_I2C0);
            IntEnable(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntRegister(INT_ID_I2C1 ,I2CDevIntIsr1);
            IntPendingClear(INT_ID_I2C1);
            IntEnable(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntRegister(INT_ID_I2C2 ,I2CDevIntIsr2);
            IntPendingClear(INT_ID_I2C2);
            IntEnable(INT_ID_I2C2);
            break;

        default:
            break;
    }

    gI2C_remainedSize = 33;
    gI2C_recivedSize = 0;
    while (cnt)
    {
        I2CStart(gI2C_channal);
        while (gI2C_flag == 0);
        gI2C_flag = 0;

#if 0
        I2CEnableInt(gI2C_channal,I2C_MBRFIEN | I2C_NAKRCVIEN);

        i2c_read(gI2C_channal,
                 gI2C_slaveaddr,
                 gI2C_slavereg,
                 I2C_7BIT_ADDRESS_8BIT_REG,
                 &_data[0],
                 2,
                 DirectMode/*NormalMode*/);

        while (gI2C_flag == 0);
        gI2C_flag = 0;

        // Send stop bit
        I2CStop(gI2C_channal);

        while (gI2C_flag == 0);
        gI2C_flag = 0;
        // Disable Controller
        I2C_Deinit(gI2C_channal);
        regData = _data[0];
        regData <<= 8;
        regData |= _data[1];
        cnt--;
        rk_printf("i2c read reg success,read data is 0x%04x \n",regData);
        DelayMs(500);
        cnt--;
#else
        gI2C_recivedSize = i2c_read(gI2C_channal,
                                    gI2C_slaveaddr,
                                    gI2C_slavereg,
                                    I2C_7BIT_ADDRESS_8BIT_REG,
                                    gI2CBuffer,
                                    gI2C_remainedSize,
                                    DirectMode/*NormalMode*/);

        gI2C_remainedSize -= gI2C_recivedSize;
        I2CEnableInt(gI2C_channal,I2C_MBRFIEN | I2C_NAKRCVIEN);

        while (gI2C_flag == 0);
        gI2C_flag = 0;

        // Send stop bit
        I2CStop(gI2C_channal);

        while (gI2C_flag == 0);
        gI2C_flag = 0;
        // Disable Controller
        I2C_Deinit(gI2C_channal);

        DelayMs(500);
        cnt--;
#endif
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UI2CDevShellBspSetSpeed
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 11:57:27
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspSetSpeed(HDC dev, uint8 * pstr)
{
    uint32 devID;
    uint32 speed;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.bsp.setspeed : ??I2C?????.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if ( *pstr == NULL)
    {
        rk_print_string("Warning!  miss  parameter,default speed 100Khz.\r\n");
        speed = 100;    //default set speed 100khz
    }
    else
    {
        while (pstr[0] == ' ')
            pstr++;

        speed = StringtoNum( pstr );
        if (speed <= 0)
            return RK_PARA_ERR;
    }

    devID = gI2C_channal = I2C_CH0;
    gI2C_slaveaddr = SlaveIICAdress;

    //open I2C clk
    ScuClockGateCtr(PCLK_I2C0_GATE, 1);
    //SetUartFreq(SYS_DEFAULT_FREQ);

    //open rst I2C ip
    ScuSoftResetCtr(I2C0_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(I2C0_SRST, 0);

    I2CDevGpioInit(devID,GPIO_CH0);

    I2C_Init(devID,10);
    DelayMs(10);

    I2CSetSpeed(devID, speed);

    return RK_SUCCESS;

}

#ifdef SHELL_HELP
/*******************************************************************************
** Name: I2CDevShellBspHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 11:41:05
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBspHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if (StrLenA(pstr) != 0)
        return RK_ERROR;

    rk_print_string("i2c.bsp?????????????i2c??????????\r\n");
    rk_print_string("??????:\r\n");
    rk_print_string("setspeed s       ??I2C?????\r\n");
    rk_print_string("send m           ????????????\r\n");
    rk_print_string("recive m         ????????????\r\n");
    rk_print_string("testspeed        ??I2C?????,??????????,???????\r\n");
    rk_print_string("testsr            i2c bsp??????????????\r\n");


    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: I2CDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 11:30:19
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
    rk_print_string("I2CDevShellBsp\r\n");
    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if (StrCnt == 0)
    {
        rk_print_string("I2CDevShellBsp StrCnt RK_ERROR\r\n");
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellI2CBspName, pItem, StrCnt);
    if (ret < 0)
    {
        rk_print_string("I2CDevShellBsp ShellCheckCmd RK_ERROR\r\n");
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                          //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:  //bsp help
#ifdef SHELL_HELP
            ret = I2CDevShellBspHelp(dev,pItem);
#endif

            break;

        case 0x01:  //setspeed
            ret = I2CDevShellBspSetSpeed(dev,pItem);
            break;

        case 0x02:  //send data
            ret = I2CDevShellBspSendData(dev,pItem);

            break;

        case 0x03:  //recive data
            ret = I2CDevShellBspReceiveData(dev,pItem);
            break;

        case 0x04:  //test speed
            ret = I2CDevShellBspTestSpeed(dev,pItem);
            break;

        case 0x05:  //test I2C Device
            ret = I2CDevShellBspTestSR(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}
#endif


/*******************************************************************************
** Name: I2CDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 9:03:45
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellPcb(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.pcb : pcb info cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string("Pcb\r\n");
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevShellOpen
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:28:16
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellOpen(HDC dev, uint8 * pstr)
{
    HDC hI2C;
    I2C_DEVICE_CLASS  * pDev;
    rk_err_t ret;
    uint32 DevID;
    HDC hExDev;
    I2C_DEVICE_ARG stArg;
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }


#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.open : open i2c cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string("Will RKDev_Create\r\n");
    ret = RKDev_Create(DEV_CLASS_I2C, DevID,I2C_CH0);
    rk_print_string("RKDev_Create ok\r\n");
    if (ret != RK_SUCCESS)
    {
        UartDev_Write(dev, "I2C open failure",17,SYNC_MODE,NULL);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevShellClose
** Input:HDC dev, uint8 * pstr
** Return: rk_errt
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:27:19
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellClose(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.close : close i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string(" I2CDevShellClose\r\n");
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevShellSuspend
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:26:55
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellSuspend(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.suspend : suspend i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2CDevShellResume
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:26:25
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellResume(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.resume : resume i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: I2CDevShellRead
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:25:58
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellRead(HDC dev, uint8 * pstr)
{
    uint8 cmd,second,minute,hour;
    uint32 ret;
    uint8 readBuf[3];
    uint8 time[3];
    uint32 cnt = 10;

    second = 59;
    minute = 59;
    hour   = 23;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.read : read data from i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string(" I2CDevShellRead\r\n");
    // TODO:
    //add other code below:
    //...
    time[0] = ((second / 10) << 4) + (second % 10);
    time[1] = ((minute / 10) << 4) + (minute % 10);
    time[2] = ((hour / 10) << 4) + (hour % 10);

    I2C_Init(I2C_CH0,RTC_HYM8563_IICSpeed);

    ret = i2c_write(I2C_CH0,0x51,0x44,I2C_7BIT_ADDRESS_8BIT_REG,time,3,NormalMode);

    rk_print_string("I2CDevShellRead i2c_write \n");
    while (ret <= 0) //ack not recived,try again.
    {
        DelayMs(5);
        ret = i2c_write(I2C_CH0,0x51,0x44,I2C_7BIT_ADDRESS_8BIT_REG,time,3,NormalMode);

        rk_printf("send data %d \n",cmd);
    }
    rk_printf(" Set time:::: \n");
    while (cnt--)
    {
        int i = 0;
        rk_printf(" Set time 23:59:59 \n");
        i2c_read(I2C_CH0,0x51,0x44,I2C_7BIT_ADDRESS_8BIT_REG,readBuf,3,NormalMode);

        rk_printf(" second = %d \n",readBuf[0]);
        rk_printf(" minute = %d \n",readBuf[1]);
        rk_printf(" hour   = %d \n",readBuf[2]);
    }

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: I2CDevShellWrite
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:24:59
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellWrite(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.write : write data to i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string(" I2CDevShellWrite\r\n");

    // TODO:

    //add other code below:
    //...

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: I2CDevShellControl
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_r
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:24:11
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellControl(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2c.control : control i2c device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    rk_print_string(" I2CDevShellControl\r\n");
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: I2CDevShellHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 10:21:35
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_SHELL_
SHELL FUN rk_err_t I2CDevShellHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if ( StrLenA( pstr) != 0)
        return RK_ERROR;

    rk_print_string("i2c?????????????i2c????\r\n");
    rk_print_string("i2c????????:           \r\n");
    rk_print_string("pcb       ??pcb??         \r\n");
    rk_print_string("open      ??i2c             \r\n");
    rk_print_string("close     ??i2c             \r\n");
    rk_print_string("suspend   suspend i2c         \r\n");
    rk_print_string("resume    resume i2c          \r\n");
    rk_print_string("read      read from i2c       \r\n");
    rk_print_string("write     write to i2c        \r\n");
    rk_print_string("control   control i2c         \r\n");
    rk_print_string("bsp    ??I2C?????\r\n");
    rk_print_string("help      ??i2c?????? \r\n");

    return RK_SUCCESS;
}

#endif
#endif
