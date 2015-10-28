/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @
 * Defines WWD SDIO functions for STM32F2xx MCU
 */
#include "BspConfig.h"

#ifdef __DRIVER_SDIO_SDIODEVICE_C__

#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"

#include "device.h"


#include "wwd_constants.h"
#include "wwd_buffer_interface.h"
#include "wwd_bus_interface.h"
#include "wwd_bus_protocol.h"
#include "wwd_sdio_interface.h"
#include "wwd_bus_protocol_interface.h"
#include "ap6181wifi.h"
/******************************************************
 *             Constants
 ******************************************************/

/******************************************************
 *             Structures
 ******************************************************/


/******************************************************
 *             Variables
 ******************************************************/
HDC hSdio,hFun1,hFun2,hFun3;


/******************************************************
 *             Function declarations
 ******************************************************/

/******************************************************
 *             Function definitions
 ******************************************************/
void wwd_thread_notify_irq( void );
static void sdio_irq_handler( void* arg )
{
    //printf("sdio_irq_handler_f1");

#ifdef _DRIVER_AP6181_WIFI_C__
    wwd_thread_notify_irq( );
#endif
}



static void sdio_irq_handler_f2( void* arg )
{
#ifdef _DRIVER_AP6181_WIFI_C__
    wwd_thread_notify_irq( );

     //printf("sdio_irq_handler_f2\n");
#endif
}
int sdio_device_create(void)
{
    SDC_DEV_ARG stSdcArg;
    SDIO_DEV_ARG stSdioArg;
    HDC hSdc;
    rk_err_t ret;


    /*   ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

       if (ret != RK_SUCCESS)
       {
         printf("dma device create failure");
       }
    */

    stSdcArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    stSdcArg.Channel = 0;
    if (stSdcArg.hDma <= 0)
    {
        return WWD_TIMEOUT;
    }

    ret = RKDev_Create(DEV_CLASS_SDC, 1, &stSdcArg);

    if (ret != RK_SUCCESS)
    {
        printf("sdc1 open failure");
        return WWD_TIMEOUT;
    }


    hSdc = RKDev_Open(DEV_CLASS_SDC, 1, NOT_CARE);

    if ((hSdc == NULL) || (hSdc == (HDC)RK_ERROR) || (hSdc == (HDC)RK_PARA_ERR))
    {
        printf("Sdc Device Open Failure");
        return WWD_TIMEOUT;
    }

    stSdioArg.hSdc = hSdc;

    ret = RKDev_Create(DEV_CLASS_SDIO, 0, &stSdioArg);

    if (ret != RK_SUCCESS)
    {
        printf("sdio Device Create Failure");
        return WWD_TIMEOUT;
    }
    return ret;
}
_ap6181WICE_WIFI_INIT_
wwd_result_t host_platform_bus_init( void )
{

    rk_err_t ret;

    if (DeviceTask_CreateDeviceList(DEVICE_LIST_SDIO_FIFO) != RK_SUCCESS)
    {
        printf("sdio  create fail\n");
    }
/*
#ifdef _JTAG_DEBUG_
    ret = sdio_device_create();
    if (ret != RK_SUCCESS)
    {
        printf("sdio create failure\n");
    }
#endif
*/
    while (1)
    {
        hSdio = RKDev_Open(DEV_CLASS_SDIO, 0, NOT_CARE);
        if (hSdio > 0)
        {
            break;
        }
       printf("sdio open fail\n");
        //vTaskDelay(10);
    }

    if ((hSdio == NULL) || (hSdio == (HDC)RK_ERROR) || (hSdio == (HDC)RK_PARA_ERR))
    {
        printf("sdio Device Open Failure");
        return WWD_TIMEOUT;
    }


    hFun1 = SdioDev_GetFuncHandle(hSdio, 0);
    if ((rk_err_t)hFun1 <= 0)
        goto fail;


    hFun2 = SdioDev_GetFuncHandle(hSdio, 1);
    if ((rk_err_t)hFun2 <= 0)
        goto fail;


    hFun3 = SdioDev_GetFuncHandle(hSdio, 2);
    if ((rk_err_t)hFun3 <= 0)
        goto fail;


    SdioDev_Claim_irq(hFun2, sdio_irq_handler);


    SdioDev_Claim_irq(hFun3, sdio_irq_handler_f2);


    return WWD_SUCCESS;

fail:

    return WWD_TIMEOUT;

}

wwd_result_t host_platform_sdio_enumerate( void )
{
    return WWD_SUCCESS;
}
_ap6181WICE_WIFI_INIT_
wwd_result_t host_platform_bus_deinit( void )
{
    SDIO_DEV_ARG pstSdioArg;
    SDC_DEV_ARG stSdcArg;

    sdio_release_irq(hFun1);
    sdio_release_irq(hFun2);
    sdio_release_irq(hFun3);
    RKDev_Close(hSdio);
    hSdio = NULL;
    if (RKDev_Delete(DEV_CLASS_SDIO, 0, &pstSdioArg) != RK_SUCCESS)
    {
       // printf("SDDev delete failureDevID=%d\n",pstSdioDev->stSdioDevice.DevID);
        return -1;
    }

    if (pstSdioArg.hSdc != NULL)
    {
        rk_printf("pstSdioArg.hSdc != NUL\n");
        if (RKDev_Close(pstSdioArg.hSdc) != RK_SUCCESS)
        {
            rk_printf("hsdc close failure\n");
            return -1;
        }
    }

    if (RKDev_Delete(DEV_CLASS_SDC, 1, &stSdcArg) != RK_SUCCESS)
    {
        printf("sdc1 delete failure\n");
        return -1;
    }

    if(stSdcArg.hDma != NULL)
    {
        if (RKDev_Close(stSdcArg.hDma) != RK_SUCCESS)
        {
            printf("sdmmc hDma close failure\n");
            return -1;
        }
    }
    rk_printf("sdio deinit success\n");
    return WWD_SUCCESS;
}

extern rk_err_t mmc_io_rw_extended(HDC pstSdioDev, uint32 write,
                                       uint32 fn, uint32 addr, uint32 incr_addr, uint8 *buf, uint32 blocks, uint32 blksz, uint8 * out);


extern rk_err_t mmc_io_rw_direct(HDC pstSdioDev, int32 write, uint32 fn,
                                     uint32 addr, uint8 in, uint8 *out);

wwd_result_t wwd_bus_sdio_cmd52( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint8_t value, sdio_response_needed_t response_expected, uint8_t* response)
{
    if (mmc_io_rw_direct(hSdio, direction, function,
                         address, value, response) == RK_SUCCESS)
    {
        return WWD_SUCCESS;
    }
    else
    {
        return WWD_TIMEOUT;
    }

}

wwd_result_t wwd_bus_sdio_cmd53( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, sdio_transfer_mode_t mode, uint32_t address, uint16_t data_size, uint8_t* data, sdio_response_needed_t response_expected,  uint32_t* response )
{
    uint32 n, m;
#define  _BLOCK_SIZE_  64
    //n = data_size / 512;
    //m = data_size % 512;
    n = data_size / _BLOCK_SIZE_;
    m = data_size % _BLOCK_SIZE_;

    if (n)
    {
        if (mmc_io_rw_extended(hSdio, direction, function,
                               address, 1, data, n, _BLOCK_SIZE_, (uint8 *)response) != RK_SUCCESS)

        {
            return WWD_TIMEOUT;
        }
    }

    if (m)
    {
        if (mmc_io_rw_extended(hSdio, direction, function,
                               address, 1, data+n*_BLOCK_SIZE_, 1, m, (uint8 *)response) != RK_SUCCESS)

        {
            return WWD_TIMEOUT;
        }
    }

    return WWD_SUCCESS;

}


void host_platform_enable_high_speed_sdio(void)
{
    return;
}


wwd_result_t host_platform_bus_enable_interrupt(void)
{
    return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt(void)
{
    return  WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{

}

#ifndef WICED_DISABLE_MCU_POWERSAVE
wwd_result_t host_enable_oob_interrupt( void )
{
    return WWD_SUCCESS;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return 0;
}
#endif /* ifndef  WICED_DISABLE_MCU_POWERSAVE */

#endif
