/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\WatchDog\WatchDogDevice.h
* Owner: wrm
* Date: 2015.5.19
* Time: 10:16:02
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm     2015.5.19     10:16:02   1.0
********************************************************************************************
*/


#ifndef __DRIVER_WATCHDOG_WATCHDOGDEVICE_H__
#define __DRIVER_WATCHDOG_WATCHDOGDEVICE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/


#define _DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_  __attribute__((section("driver_watchdog_watchdogdevice_common")))
#define _DRIVER_WATCHDOG_WATCHDOGDEVICE_INIT_  __attribute__((section("driver_watchdog_watchdogdevice_init")))
#define _DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_  __attribute__((section("driver_watchdog_watchdogdevice_shell")))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t WDTDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t WDTDev_Write(HDC dev);
extern rk_err_t WDTDev_Read(HDC dev);
extern rk_err_t WDTDev_Delete(uint32 DevID, void * arg);
extern HDC WDTDev_Create(uint32 DevID, void * arg);

extern void WDTDevIntIsr1(void);
extern void WDTDevIntIsr0(void);



#endif
