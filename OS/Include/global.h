/*
********************************************************************************
*                Copyright (c): 2014 - 2014 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* File Name��   Main.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
//system keeping information for music module



#define SYS_SUPPROT_STRING_MAX_LEN 255 // sys support string max len(31, 63, 127, 255(windows))
#define MAX_FILENAME_LEN        (SYS_SUPPROT_STRING_MAX_LEN + 1)//the null of end occupy 2 bytes.

//section define
#define _ATTR_SYS_CODE_         __attribute__((section("SysCode")))
#define _ATTR_SYS_DATA_         __attribute__((section("SysData")))
#define _ATTR_SYS_BSS_          __attribute__((section("SysBss"),zero_init))

#define _ATTR_SYS_INIT_CODE_    __attribute__((section("SysInitCode")))
#define _ATTR_SYS_INIT_DATA_    __attribute__((section("SysInitData")))
#define _ATTR_SYS_INIT_BSS_     __attribute__((section("SysInitBss"),zero_init))

#define _ATTR_SYS_REBOOT_BSS_   __attribute__((section("RebootFlag"),zero_init))


#define _ATTR_BB_SYS_CODE_          __attribute__((used, section("BBSysCode")))
#define _ATTR_BB_SYS_DATA_          __attribute__((used, section("BBSysData")))
#define _ATTR_BB_SYS_BSS_           __attribute__((used, section("BBSysBss"), zero_init))

/*
********************************************************************************
*
*                         End of Main.h
*
********************************************************************************
*/
#endif
