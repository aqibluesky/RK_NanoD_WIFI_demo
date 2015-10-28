/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SpiFlashDev.c
* Owner: wangping
* Date: 2015.6.23
* Time: 10:29:54
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     10:29:54   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__

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
#include "SFC.h"
#include "SPIFlash.h"
#include "SFNand.c"
#include "SFNor.c"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define SPIFLASH_DEV_NUM 1

typedef  struct _SPIFLASH_DEVICE_CLASS
{
    DEVICE_CLASS stSpiFlashDevice;
    xSemaphoreHandle osSpiFlashOperSem;
    uint32 capacity;

} SPIFLASH_DEVICE_CLASS;

typedef __packed struct _SFC_STRUCT
{
    uint total_size;    //the total data size need to send/read
    uint complete_size; //
    uint8 *payload;      //current data point
    pSemaphore rw_complete_sema;
} SFC_STRUCT, * P_SFC_STRUCT;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SPIFLASH_DEVICE_CLASS * gpstSpiFlashDevISR[SPIFLASH_DEV_NUM] = {(SPIFLASH_DEVICE_CLASS *)NULL};

static SFLASH_DRIVER *pSFlashDrv;

static uint32 SPI_IDB_SIZE;
SFC_STRUCT sfc_ctrl;
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
rk_err_t SpiFlashDevShellTest(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellDel(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellMc(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellPcb(HDC dev, uint8 * pstr);
void SpiFlashDevIntIsr0(void);
void SpiFlashDevIntIsr(uint32 DevID);
rk_err_t SpiFlashDevDeInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev);
rk_err_t SpiFlashDevInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev);
rk_err_t SpiFlashDevResume(HDC dev);
rk_err_t SpiFlashDevSuspend(HDC dev);
uint32 SFlashRead(uint32 sec, uint32 nSec, void *pData);
uint32 SFlashWrite(uint32 sec, uint32 nSec, void *pData);
uint32 SFlashGetCapacity(void);
void SpiFlash_Test(HDC dev);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t SpiFlashDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SpiFlashDev Write...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstSpiFlashDev->osSpiFlashOperSem, MAX_DELAY);
    SFlashWrite(LBA, len, buffer);
    rkos_semaphore_give(pstSpiFlashDev->osSpiFlashOperSem);
}
/*******************************************************************************
** Name: SpiFlashDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t SpiFlashDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SpiFlashDev Read...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstSpiFlashDev->osSpiFlashOperSem, MAX_DELAY);
    SFlashRead(LBA, len, buffer);
    rkos_semaphore_give(pstSpiFlashDev->osSpiFlashOperSem);
}

_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t  SpiFlashDev_GetSize(HDC dev, uint32 * Size)
{
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev = (SPIFLASH_DEVICE_CLASS *)dev;
    if (pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

    *Size = pstSpiFlashDev->capacity;
    return RK_SUCCESS;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: sfc_int_enable
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
void sfc_int_enable(void)
{
    IntEnable(INT_ID_SFC);
}

/*******************************************************************************
** Name: sfc_int_disable
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
void sfc_int_disable(void)
{
    IntDisable(INT_ID_SFC);
}

/*******************************************************************************
** Name: sfc_dev_init
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
void sfc_dev_init(void)
{
    SetSFCFreq(XIN24M,12000000);
    SfcDevHwInit();
    IntRegister(INT_ID_SFC,SpiFlashDevIntIsr0);
    IntPendingClear(INT_ID_SFC);
    IntDisable(INT_ID_SFC);
    sfc_bsp_init();
    sfc_set_fifo_threshold(0,0);
    memset(&sfc_ctrl, 0, sizeof(SFC_STRUCT));

    sfc_ctrl.rw_complete_sema = rkos_semaphore_create(1, 0);
}

/*******************************************************************************
** Name: SFC_Request
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SFC_Request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data)
{
    SFCCMD_DATA     cmd;
    int32           ret = SFC_OK;
    cmd.d32 = sfcmd;
#if 1
    if (SFC_WRITE == cmd.b.rw)
    {
        sfc_set_intmask(0xFF & (~8));
    }
    else if(SFC_READ == cmd.b.rw)
    {

        sfc_set_intmask(0xFF & (~1));
    }
    else
    {
        sfc_set_intmask(0xFF);
    }
    sfc_clear_int_flag();
#endif

    ret = _sfc_request(sfcmd, sfctrl, addr, data);

    if(ret != SFC_OK)
    {
        return ret;
    }

#if 1
    if(cmd.b.datasize)
    {
        sfc_ctrl.total_size = cmd.b.datasize;
        sfc_ctrl.complete_size = 0;
        sfc_ctrl.payload = data;
        if (SFC_WRITE == cmd.b.rw)
        {
            //sfc_ctrl.complete_size = sfc_write_fifo(uint8 *data , sfc_ctrl.total_size);
            sfc_int_enable();
            rkos_semaphore_take(sfc_ctrl.rw_complete_sema, MAX_DELAY);

        }
        else if(SFC_READ == cmd.b.rw)
        {
            sfc_int_enable();
            rkos_semaphore_take(sfc_ctrl.rw_complete_sema, MAX_DELAY);
        }
    }
#endif

    return ret;
}


/*******************************************************************************
** Name: SFlashRead
** Input:uint32 sec, uint32 nSec, void *pData
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashRead(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->read(sec, nSec, pData);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashWrite
** Input:uint32 sec, uint32 nSec, void *pData
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashWrite(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->write(sec, nSec, pData);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashErase
** Input:uint32 SecAddr
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashErase(uint32 SecAddr)
{

    if (pSFlashDrv)
        return pSFlashDrv->erase(SecAddr);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashGetCapacity
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashGetCapacity(void)
{
    SFLASH_INFO *pInfo;

    if (!pSFlashDrv)
        return 0;

    pInfo = (SFLASH_INFO *)pSFlashDrv->pDev;
    return pInfo->capacity;
}

/*******************************************************************************
** Name: SFlashReadID
** Input:uint8* data, bool bNand
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SFlashReadID(uint8* data, bool bNand)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;               //ID �ֽ�����2-3B, ��ͬ���̲�һ��

    sfctrl.d32 = 0;
    if (bNand)
    {
        sfcmd.b.addrbits = SFC_ADDR_XBITS;
        sfctrl.b.addrbits = 8;
    }
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, 0, data);

    return ret;
}

/*******************************************************************************
** Name: SFlashInit
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashInit(void)
{
    int32 ret = ERROR;
    uint8 data[3];
    uint8 bSpiNand;
    uint32 i;
    SFLASH_INFO *spec;
    uint32 SPI_IDB_NUM;

    pSFlashDrv = NULL;
    SPI_IDB_SIZE = 0;
    //SFC_Init();
    sfc_dev_init();
    if (SFC_OK != SFlashReadID(data, FALSE))
        return ERROR;

    //DEBUG("SPI FLASH ID:%x %x %x\n", data[0], data[1], data[2]);
    //��ʹ��data[0]�ж�, �ǿ��Ǽ���nand, ��nandʱdata[0]=0xFF(IOĬ������������)
    if ((0xFF==data[2] && 0xFF==data[1]) || (0x00==data[2] && 0x00==data[1]))
    {
        return ERROR;
    }

    //bSpiNand = 0;
    bSpiNand = (0xFF==data[0])? 1 : 0;

#if 1//def SPINAND_DRIVER
    if (bSpiNand)
    {
        ret = SNAND_Init(&data[1], &pSFlashDrv);
    }
    else
#endif
    {
        //#ifdef SPINOR_DRIVER
        ret = SNOR_Init(data, &pSFlashDrv);
        //#endif
    }

    if (ret != SFC_OK)
        return ret;

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;
    SPI_IDB_NUM = (bSpiNand == 1)? 2 : 1;
    SPI_IDB_SIZE = SPI_IDB_NUM*spec->BlockSize;
    return ret;
}

/*******************************************************************************
** Name: SpiFlashDevIntIsr0
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN void SpiFlashDevIntIsr0(void)
{
    //Call total int service...

    uint32 int_status = sfc_get_int_status();

    sfc_clear_int_flag();
    if(int_status & 0x1) //read fifo full
    {
        if(sfc_ctrl.total_size != sfc_ctrl.complete_size)
        {
            sfc_ctrl.complete_size += sfc_read_fifo(sfc_ctrl.payload+sfc_ctrl.complete_size ,
                                                    sfc_ctrl.total_size- sfc_ctrl.complete_size);

            if(sfc_ctrl.total_size == sfc_ctrl.complete_size)
            {
                sfc_int_disable();
                rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
            }
        }
        else
        {
            sfc_int_disable();
            rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
        }
    }

    if(int_status & (1<<3))//tx fifo empty
    {
        if(sfc_ctrl.total_size != sfc_ctrl.complete_size)
        {
            sfc_ctrl.complete_size += sfc_write_fifo(sfc_ctrl.payload+sfc_ctrl.complete_size ,
                                      sfc_ctrl.total_size- sfc_ctrl.complete_size);

//            if(sfc_ctrl.total_size == sfc_ctrl.complete_size)
//            {
//                sfc_int_disable();
//                rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
//            }
        }
        else
        {
            sfc_int_disable();
            rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
        }
    }

}

/*******************************************************************************
** Name: SpiFlashDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN rk_err_t SpiFlashDevResume(HDC dev)
{
    //SpiFlashDev Resume...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: SpiFlashDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN rk_err_t SpiFlashDevSuspend(HDC dev)
{
    //SpiFlashDev Suspend...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API rk_err_t SpiFlashDev_Delete(uint32 DevID, void * arg)
{
    //Check SpiFlashDev is not exist...
    if(gpstSpiFlashDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    //SpiFlashDev deinit...
    SpiFlashDevDeInit(gpstSpiFlashDevISR[DevID]);

    //Free SpiFlashDev memory...
    rkos_semaphore_delete(gpstSpiFlashDevISR[DevID]->osSpiFlashOperSem);
    rkos_memory_free(gpstSpiFlashDevISR[DevID]);

    //Delete SpiFlashDev...
    gpstSpiFlashDevISR[DevID] = NULL;

    //Delete SpiFlashDev Read and Write Module...
#ifdef __SYSTEM_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveModule(MODULE_ID_SPIFLASH_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
//_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API HDC SpiFlashDev_Create(uint32 DevID, void * arg)
{
    SPIFLASH_DEV_ARG * pstSpiFlashDevArg;
    DEVICE_CLASS* pstDev;
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev;

//  if(arg == NULL)
//  {
//      return (HDC)RK_PARA_ERR;
//  }

    // Create handler...
    pstSpiFlashDevArg = (SPIFLASH_DEV_ARG *)arg;
    pstSpiFlashDev =  rkos_memory_malloc(sizeof(SPIFLASH_DEVICE_CLASS));
    memset(pstSpiFlashDev, 0, sizeof(SPIFLASH_DEVICE_CLASS));
    if (pstSpiFlashDev == NULL)
    {
        return NULL;
    }

    //init handler...

    pstSpiFlashDev->osSpiFlashOperSem  = rkos_semaphore_create(1,1);

    if((pstSpiFlashDev->osSpiFlashOperSem) == 0)
    {
        rkos_memory_free(pstSpiFlashDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstSpiFlashDev;
    pstDev->suspend = SpiFlashDevSuspend;
    pstDev->resume  = SpiFlashDevResume;

    //init arg...
    //pstSpiFlashDev->usbmode = pstSpiFlashDevArg->usbmode;

    //device init...
    gpstSpiFlashDevISR[DevID] = NULL;
    if(SpiFlashDevInit(pstSpiFlashDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstSpiFlashDev->osSpiFlashOperSem);
        rkos_memory_free(pstSpiFlashDev);
        return (HDC) RK_ERROR;
    }
    gpstSpiFlashDevISR[DevID] = pstSpiFlashDev;
    //SpiFlash_Test(pstSpiFlashDev);
    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDevDeInit
** Input:SPIFLASH_DEVICE_CLASS * pstSpiFlashDev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT FUN rk_err_t SpiFlashDevDeInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev)
{
    //If ip device first step unregister int
    IntRegister(INT_ID_SFC,NULL);
    IntPendingClear(INT_ID_SFC);
    IntDisable(INT_ID_SFC);

    ScuClockGateCtr(CLK_SFC_GATE,0);
    ScuClockGateCtr(HCLK_SFC_GATE,0);
    return RK_ERROR;
}

/*******************************************************************************
** Name: SpiFlashDevInit
** Input:SPIFLASH_DEVICE_CLASS * pstSpiFlashDev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT FUN rk_err_t SpiFlashDevInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev)
{
    SfcDevHwInit();
    IntRegister(INT_ID_SFC,SpiFlashDevIntIsr0);
    IntPendingClear(INT_ID_SFC);
    IntDisable(INT_ID_SFC);
    sfc_bsp_init();
    sfc_set_fifo_threshold(0,0);
    memset(&sfc_ctrl, 0, sizeof(SFC_STRUCT));

    sfc_ctrl.rw_complete_sema = rkos_semaphore_create(1, 0);
    if(sfc_ctrl.rw_complete_sema == NULL)
    {
        return RK_ERROR;
    }

    if(SFlashInit())
    {
        rkos_semaphore_delete(sfc_ctrl.rw_complete_sema);
    }

    pstSpiFlashDev->capacity = SFlashGetCapacity();
    return RK_SUCCESS;

}


#ifdef _SPIFLASH_DEV_SHELL_
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
static char* ShellSpiFlashName[] =
{
    "pcb",
    "mc",
    "del",
    "test",
    "...",
    "\b",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
void SpiFlash_Test(HDC dev)
{
    int i;
    uint8 *readbuf = rkos_memory_malloc(2048);
    uint8 *writebuf = rkos_memory_malloc(2048);

    memset(readbuf, 0, 512);
    memset(writebuf, 0, 512);

    for(i=0; i<256; i++)
    {
        writebuf[i] = i;
        writebuf[i+256] = i;
    }

    SpiFlashDev_Write(dev, 128*30, writebuf, 1);
    SpiFlashDev_Read(dev, 128*30, readbuf, 1);
    for(i=0; i<256; i++)
    {
        if(readbuf[i] != i || readbuf[i+256] != i)
        {
            rk_print_string("SpiFlashDev test fail\n");
            rkos_memory_free(readbuf);
            rkos_memory_free(writebuf);
            return;
        }

    }
    rk_print_string("SpiFlashDev test ok\n");
    rkos_memory_free(readbuf);
    rkos_memory_free(writebuf);
}
/*******************************************************************************
** Name: SpiFlashDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL API rk_err_t SpiFlashDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSpiFlashName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;
    switch (i)
    {
        case 0x00:
            ret = SpiFlashDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = SpiFlashDevShellMc(dev,pItem);
            break;

        case 0x02:
            ret = SpiFlashDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = SpiFlashDevShellTest(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hSpiFlashDev;
    uint32 DevID;

    //Get SpiFlashDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open SpiFlashDev...
    hSpiFlashDev = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);
    if((hSpiFlashDev == NULL) || (hSpiFlashDev == (HDC)RK_ERROR) || (hSpiFlashDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("SpiFlashDev open failure");
        return RK_SUCCESS;
    }

    //do test....

    //close SpiFlashDev...
    SpiFlash_Test(hSpiFlashDev);
    RKDev_Close(hSpiFlashDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    //Get SpiFlashDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }
    if(RKDev_Delete(DEV_CLASS_SPIFLASH, DevID) != RK_SUCCESS)
    {
        rk_print_string("SPIFLASHDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellMc(HDC dev, uint8 * pstr)
{
    SPIFLASH_DEV_ARG stSpiFlashDevArg;
    rk_err_t ret;
    uint32 DevID;

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Init SpiFlashDev arg...
    //stSpiFlashDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create SpiFlashDev...
    ret = RKDev_Create(DEV_CLASS_SPIFLASH, DevID, &stSpiFlashDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("SpiFlashDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hSpiFlashDev;
    uint32 DevID;

    //Get SpiFlashDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if(gpstSpiFlashDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("SpiFlashDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("	osSpiFlashOperSem---0x%08x", gpstSpiFlashDevISR[DevID]->osSpiFlashOperSem);
        //rk_printf_no_time("   usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("   usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("SpiFlashDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}


#endif

#endif
