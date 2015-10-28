/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   I2c.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-11-20         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"
#ifdef  __DRIVER_I2C_I2CDEVICE_C__
#define _IN_I2C_
#include "typedef.h"
#include "DriverInclude.h"


const uint32 I2C_GROUP[3] =
{
    I2C0_BASE,
    I2C1_BASE,
    I2C2_BASE,
};

//#define UartReg              ((UART_REG*)UART_GROUP)

#define I2CPORT(n)          ((I2CReg_t*)(I2C_GROUP[n]))

//#define RK_CEIL(x, y) \
//	({ unsigned long __x = (x), __y = (y); (__x + __y - 1) / __y; })


static uint32 RK_CEIL(uint32 x,uint32 y)
{
    uint32 _x = x;
    uint32 _y = y;

    return ((_x + _y -1) / _y);
}

static void I2C_show_regs(eI2C_CHANNEL i2cPort)
{
#ifdef _JTAG_DEBUG_
    uint i;
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    printf("I2C_CON: 0x%08x\n",      *(uint32*)(pReg->I2C_CON));
    printf("I2C_CLKDIV: 0x%08x\n",   *(uint32*)(pReg->I2C_CLKDIV));
    printf("I2C_MRXADDR: 0x%08x\n",  *(uint32*)(pReg->I2C_MRXADDR));
    printf("I2C_MRXRADDR: 0x%08x\n", *(uint32*)(pReg->I2C_MRXRADDR));
    printf("I2C_MTXCNT: 0x%08x\n",   *(uint32*)(pReg->I2C_MTXCNT));
    printf("I2C_MRXCNT: 0x%08x\n",   *(uint32*)(pReg->I2C_MRXCNT));
    printf("I2C_IEN: 0x%08x\n",      *(uint32*)(pReg->I2C_IEN));
    printf("I2C_IPD: 0x%08x\n",      *(uint32*)(pReg->I2C_IPD));
    printf("I2C_FCNT: 0x%08x\n",     *(uint32*)(pReg->I2C_FCNT));

    for (i = 0; i < 8; i ++)
    {
        printf("I2C_TXDATA%d: 0x%08x\n", i, pReg->I2C_TXDATA[i]);
    }
    for (i = 0; i < 8; i ++)
    {
        printf("I2C_RXDATA%d: 0x%08x\n", i, pReg->I2C_RXDATA[i]);
    }
#endif
}

void I2CStart(eI2C_CHANNEL i2cPort)
{
    I2C_send_start_bit(i2cPort);
}

/*
--------------------------------------------------------------------------------
  Function name : I2CStop(void)
  Author        : yangwenjie
  Description   : I2C stop

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void I2CStop(eI2C_CHANNEL i2cPort)
{
    I2C_send_stop_bit(i2cPort);
}


void I2CEnableInt(eI2C_CHANNEL i2cPort, uint32 IntType)
{
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN |= IntType;
}

void I2CDisableInt(eI2C_CHANNEL i2cPort, uint32 IntType)
{
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN &= ~IntType;
}

void I2CDisableIRQ(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN = IRQ_ALL_DISABLE ;
}

uint32 I2CGetInt(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;
    uint32 intstatus;

    pReg = I2CPORT(i2cPort);
    intstatus = pReg->I2C_IPD;

    if (intstatus & I2C_NAKRCVIPD)
    {
        pReg->I2C_IPD = I2C_NAKRCVIPD;
//        printf("I2CGetInt :Error! NAKRCV IPD\n");
    }

    if (intstatus & I2C_STARTIPD )
    {
        pReg->I2C_IPD = I2C_STARTIPD;
//        printf("I2CGetInt : start IPD\n");
    }

    if (intstatus & I2C_STOPIPD)
    {
        pReg->I2C_IPD = I2C_STOPIPD;
//        printf("I2CGetInt : stop IPD\n");
    }

    if (intstatus & I2C_MBTFIPD )
    {
        pReg->I2C_IPD = I2C_MBTFIPD;
//        printf("I2CGetInt : MTXCNT data transmit finished IPD\n");
    }

    if (intstatus & I2C_MBRFIPD )
    {
        pReg->I2C_IPD = I2C_MBRFIPD;
//        printf("I2CGetInt : MRXCNT data received finished IPD\n");
    }
    return intstatus;
}


static int I2C_send_start_bit(eI2C_CHANNEL i2cPort)
{
    int TimeOut = I2C_TIMEOUT_US;

    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;
    pReg->I2C_CON |= I2C_CON_EN | I2C_CON_START; //0x09
    pReg->I2C_IEN = I2C_STARTIEN;

    return OK;
}


static int I2C_send_stop_bit(eI2C_CHANNEL i2cPort)
{
    int TimeOut = I2C_TIMEOUT_US;
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;
    pReg->I2C_CON = I2C_CON_EN | I2C_CON_STOP;  // 0x11 b 10001
    pReg->I2C_IEN = I2C_STOPIEN;

    return OK;
}


int32 I2CSendData(eI2C_CHANNEL i2cPort,
                  UINT16 slaveaddr,
                  UINT16 regaddr,
                  eI2C_Address_Reg_t addr_reg_mode,
                  UINT8* Databuf,
                  uint32 size,
                  eI2C_RW_mode_t rw_mode)
{
    int err = OK;
    int TimeOut = I2C_TIMEOUT_US;
    uint8 *pbuf = Databuf;
    uint bytes_remain_len;
    uint bytes_tranfered_len = 0;
    uint words_tranfered_len = 0;
    uint con = 0;
    uint txData = 0;
    uint txdataCnt = 0;
    uint index = 0;

    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    pReg->I2C_CON &= (~I2C_CON_START);    //clear start signal

    if (slaveaddr != 0) //in irq  slaveaddr set 0 to continus write
    {
        txData = slaveaddr;
        if (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG
                || addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
        {
            index += 1;
        }
        else if (addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG
                 || addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
        {
            index += 2;
        }

        if ( rw_mode == NormalMode)
        {
            txData |= regaddr << ((index % 4) * 8);    //index is slaveaddr length
            if (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG
                    || addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
            {
                index += 1;
            }
            else if (addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG
                     || addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
            {
                index += 2;
            }
        }
        else if ( rw_mode == DirectMode)
        {
            index += 0;
        }
    }

    bytes_remain_len = size + index;//all data numbers( address and data) to be transmit   //j is register length

    if (bytes_remain_len)
    {
        while (1)
        {
            txData |= (*Databuf<<((index % 4) * 8));

            pReg->I2C_TXDATA[index/4] = txData;
//            printf("I2C Write TXDATA[%d] = 0x%08x\n",index/4 ,txData);
            index++;
            Databuf++;
            size--;

            if (index % 4 == 0 || index == 4)
            {
                txData = 0;
            }

            if ((index == 32)||(size == 0))
            {
                bytes_remain_len -= index;
                break;
            }
        }

        pReg->I2C_CON = I2C_CON_EN | I2C_CON_MOD(I2C_MODE_TX);
        pReg->I2C_MTXCNT = index;   //start transmit

        txdataCnt = index;
        index = 0;
    }

//    printf("I2C Write TXDATA[%d] = 0x%08x\n",index/4,pReg->I2C_TXDATA[index/4]);
    return txdataCnt;
}

int32 I2CReadData(eI2C_CHANNEL i2cPort,
                  UINT8 slaver_addr,
                  uint16 reg_addr,
                  eI2C_Address_Reg_t addr_reg_mode,
                  UINT8 *Databuf,
                  uint32 len,
                  eI2C_RW_mode_t rw_mode)
{
    int err = OK;
    int TimeOut ;
    uint8 *pbuf = Databuf;
    uint bytes_remain_len = len;
    uint bytes_tranfered_len = 0;
    uint words_tranfered_len = 0;
    uint con = 0;
    uint rxdata;
    uint rxSize = 0;
    uint i, j;
    I2CReg_t *  pReg;
    TimeOut = I2C_TIMEOUT_US;

    pReg = I2CPORT(i2cPort);

    if ( slaver_addr != 0 )
    {
        pReg->I2C_CON &= (~I2C_CON_START);//clear start signal
        pReg->I2C_CON &= (~I2C_CON_MASK);
        pReg->I2C_CON |= I2C_CON_MOD(I2C_MODE_TRX) /*| I2C_CON_LASTACK*/;

        pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;//clear interrupt pending bit

        if ((addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
                ||(addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG))
        {
            pReg->I2C_MRXADDR = I2C_MRXADDR_SET(1, slaver_addr);
        }
        else if ((addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
                 ||(addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG))
        {
            pReg->I2C_MRXADDR = I2C_MRXADDR_SET(3, slaver_addr);
        }

        if ( rw_mode == NormalMode )
        {
            if ((addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG)
                    ||(addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG))
            {
                pReg->I2C_MRXRADDR = (0x1<<24) | reg_addr;//config device register address,low byte valids
            }
            else if ((addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
                     ||(addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG))
            {
                pReg->I2C_MRXRADDR = (0x3<<24) | reg_addr;//config device register address,low and mid bytes valid
            }
        }
        else if ( rw_mode == DirectMode)
        {
            if ((addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG)
                    ||(addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG)
                    || (addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
                    ||(addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG))
            {
                //config device register address
                //directmode do nothing
                //pReg->I2C_MRXRADDR = (0x0<<24) | reg_addr;
            }
        }
    }


    if (bytes_remain_len)
    {
        if (bytes_remain_len >= RK_I2C_FIFO_SIZE)
        {
            if ( bytes_remain_len == RK_I2C_FIFO_SIZE )
                pReg->I2C_CON |= (I2C_CON_EN|I2C_CON_LASTACK);

            bytes_tranfered_len = 32;
        }
        else
        {
            pReg->I2C_CON |= (I2C_CON_EN | I2C_CON_LASTACK);
            bytes_tranfered_len = bytes_remain_len;
        }

        pReg->I2C_MRXCNT = bytes_tranfered_len;  //set recive data count
        rxSize  = bytes_tranfered_len;

        words_tranfered_len = RK_CEIL(bytes_tranfered_len, 4);
        for (i = 0; i < words_tranfered_len; i++)
        {
            do
            {
                TimeOut--;
            }while(!(pReg->I2C_IPD&I2C_MBRFIPD));

            if( TimeOut )
            {
                //pReg->I2C_IPD |= (I2C_MBRFIPD|I2C_BRFIPD);
                rxdata = pReg->I2C_RXDATA[i];
                //printf("I2c Read RXDATA[%d] = 0x%08x\n", i, rxdata);
                for (j = 0; j < 4; j++)
                {
                    if ((i * 4 + j) == bytes_tranfered_len)
                    {
                        break;
                    }
                    *pbuf++ = (rxdata >> (j * 8)) & 0xff;
                }
            }
            else
            {
                return TIMEOUT;
            }
        }
        bytes_remain_len -= bytes_tranfered_len;
        if ( bytes_remain_len != 0)
        {
            //config recive mode
            pReg->I2C_CON = (pReg->I2C_CON & ~(I2C_CON_MASK)) | I2C_CON_MOD(I2C_MODE_RX);
        }
        //rk_printf("I2C Read bytes_remain_len %d\n", bytes_remain_len);
    }

    return rxSize;
}

static  void I2C_Get_div(int div, int *divh, int *divl)
{
    if (div % 2 == 0)
    {
        *divh = div / 2;
        *divl = div / 2;
    }
    else
    {
        *divh = RK_CEIL(div, 2);
        *divl = div / 2;
    }
}

BOOL I2CSetSpeed(eI2C_CHANNEL i2cPort,UINT16 speed)
{
    UINT32 exp;
    UINT32 rem;
    UINT32 scalespeed;
    UINT32 freqbase;

    uint32 i2c_rate;
    int div, divl, divh;


    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

//	i2c_rate   = (GetPclkFreq() * 1000 ) / 5;//by YWJ 04.19
    i2c_rate   = (GetPclkLogicPreFreq() * 1000 );
    div = RK_CEIL(i2c_rate, speed << 3) - 2;
    if (div < 0)
    {
        divh = divl = 0;
    }
    else
    {
        I2C_Get_div(div, &divh, &divl);
    }
//     printf("divl = %d,divh = %d \n",divl,divh);

    pReg->I2C_CLKDIV = I2C_CLKDIV_VAL(divl, divh);

    printf("I2CSetSpeed %d \n",speed);
    return TRUE;
}

BOOL I2C_Read_SendACK(eI2C_CHANNEL i2cPort,BOOL flg)
{
    return TRUE;
}
/*
--------------------------------------------------------------------------------
  Function name : I2C_Init(UINT8 SlaveAddress, UINT16 nKHz)
  Author        : yangwenjie
  Description   :

  Input         : SlaveAddress :I2C device address
                  nKHz         :I2C max speed

  Return        :TRUE -- ok
                 FALSE -- fail

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
BOOL I2C_Init(eI2C_CHANNEL i2cPort,uint16 speed)
{
    I2CReg_t *  pReg;
    uint32 i2c_rate;

    pReg = I2CPORT(i2cPort);
    pReg->I2C_CON = 0;

    I2CSetSpeed(i2cPort,speed);
    pReg->I2C_CON = I2C_CON_EN;

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
  Description   : Write to i2c memory

  Input         : i2cPort: i2c channel
                  slaveaddr:target i2c address
                  regaddr:	address to send to
                  reglen:
                  buffer:	buffer for send data
                  len:	length of bytes to be send

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
int i2c_write(eI2C_CHANNEL i2cPort,
              uint16 slaveaddr,
              uint16 regaddr,
              eI2C_Address_Reg_t addr_reg_mode,
              uint8 *buffer,
              int len,
              eI2C_RW_mode_t rw_mode)
{
    if ((buffer == NULL) && (len != 0))
    {
        printf ("i2c_write: buf == NULL\n");
        return ERROR;
    }
    if (len == 0)
    {
        printf("i2c_read: len = 0 \n");
        return ERROR;
    }

    return I2CSendData(i2cPort, slaveaddr, regaddr, addr_reg_mode, buffer, len, rw_mode);
}

/*
--------------------------------------------------------------------------------
  Function name : i2c_read(uchar slaveaddr, uint regaddr, int reglen, uchar *buf, int len)
  Author        :
  Description   : Read from i2c memory

  Input         : i2cPort: i2c channel
                  slaveaddr:	target i2c address
                  regaddr:	address to read from
                  reglen:
                  buffer:	buffer for read data
                  len:	length of bytes to be read

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
int i2c_read(eI2C_CHANNEL i2cPort,
             uint8 slaveaddr,
             uint16 regaddr,
             eI2C_Address_Reg_t addr_reg_mode,
             uint8 *buf,
             int len,
             eI2C_RW_mode_t rw_mode)
{
    if ((buf == NULL) && (len != 0))
    {
        printf("i2c_read: buf == NULL\n");
        return ERROR;
    }

    if (len == 0)
    {
        printf("i2c_read: len = 0 \n");
        return ERROR;
    }

    return I2CReadData(i2cPort, slaveaddr, regaddr, addr_reg_mode, buf, len,rw_mode);
}


/*
--------------------------------------------------------------------------------
  Function name : I2C_Read(UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_mode_t mode)
  Author        : yangwenjie
  Description   :

  Input         : RegAddr
                  size -- read size
                  mode -- NormalMode/DirectMode

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
BOOL I2C_ReadConfig(eI2C_CHANNEL i2cPort,BOOL StartBit)
{
    BOOL ret = TRUE;
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    return ret;
}

BOOL I2C_Deinit(eI2C_CHANNEL i2cPort )
{
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);

    pReg->I2C_CON = 0;;

    return TRUE;
}

#endif
/*
********************************************************************************
*
*                         End of I2S.c
*
********************************************************************************
*/
