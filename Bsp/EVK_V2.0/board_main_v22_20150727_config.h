/*
********************************************************************************
*
*     Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* File Name  board_main_v22_20150727_config.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               wangping          2015-8-25          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef __BOARD_MAIN_V22_20150727_CONFIG_H__
#define __BOARD_MAIN_V22_20150727_CONFIG_H__

/*
*-------------------------------------------------------------------------------
*
*                            Key Driver Configuration
*
*-------------------------------------------------------------------------------
*/
//Key Num Define
#define KEY_NUM_4            4
#define KEY_NUM_5            5
#define KEY_NUM_6            6
#define KEY_NUM_7            7
#define KEY_NUM_8            8

//Config KeyDriver
#define KEY_NUM              KEY_NUM_7

//Key Val Define
#define KEY_VAL_NONE        ((UINT32)0x0000)
#define KEY_VAL_PLAY        ((UINT32)0x0001 << 0)
#define KEY_VAL_MENU        ((UINT32)0x0001 << 1)
#define KEY_VAL_FFD         ((UINT32)0x0001 << 2)
#define KEY_VAL_FFW         ((UINT32)0x0001 << 3)
#define KEY_VAL_UP          ((UINT32)0x0001 << 4)
#define KEY_VAL_DOWN        ((UINT32)0x0001 << 5)
#define KEY_VAL_ESC         ((UINT32)0x0001 << 6)
//#define KEY_VAL_VOL         ((UINT32)0x0001 << 7)

#define KEY_VAL_UPGRADE     KEY_VAL_MENU
#define KEY_VAL_POWER       KEY_VAL_PLAY
#define KEY_VAL_HOLD        (KEY_VAL_MENU | KEY_VAL_PLAY)
#define KEY_VAL_VOL         KEY_VAL_ESC

#define KEY_VAL_MASK        ((UINT32)0x0fffffff)
#define KEY_VAL_UNMASK      ((UINT32)0xf0000000)

//Bit position define for AD keys.
#define KEY_VAL_ADKEY2        KEY_VAL_MENU
#define KEY_VAL_ADKEY3        KEY_VAL_UP
#define KEY_VAL_ADKEY4      KEY_VAL_FFD
#define KEY_VAL_ADKEY5      KEY_VAL_FFW
#define KEY_VAL_ADKEY6      KEY_VAL_DOWN
#define KEY_VAL_ADKEY7      KEY_VAL_ESC

//AdKey Reference Voltage define
//ADC ref = LDO Output
#define ADKEY2_MIN          ((0   +   0) / 2)
#define ADKEY2_MAX          ((0   + 147) / 2)

#define ADKEY3_MIN          ((147 +   0) / 2) //up
#define ADKEY3_MAX          ((147 + 330) / 2)

#define ADKEY4_MIN          ((330 + 147) / 2) //FFD
#define ADKEY4_MAX          ((330 + 522) / 2)

#define ADKEY5_MIN          ((522 + 330) / 2) //FFW
#define ADKEY5_MAX          ((522 + 780) / 2)

#define ADKEY6_MIN          ((780 + 522) / 2) //Down
#define ADKEY6_MAX          ((780 + 956) / 2)

#define ADKEY7_MIN          ((956 + 780) / 2) //ESC
#define ADKEY7_MAX          ((956 + 1024) / 2)
/*
*-------------------------------------------------------------------------------
*
*                            GPIO Config
*
*-------------------------------------------------------------------------------
*/
#define SD_DET              GPIOPortD_Pin2
#define AP6181_POWER_ON_GPIO_CH     GPIO_CH2
#define AP6181_POWER_ON_GPIO_PIN    GPIOPortA_Pin2
#define AP6181_HOST_WAKE_GPIO_CH    GPIO_CH2
#define AP6181_HOST_WAKE_GPIO_PIN   GPIOPortA_Pin0



/*
*-------------------------------------------------------------------------------
*
*                            light config
*
*-------------------------------------------------------------------------------
*/
/*  system start LED B5 */
#define SYSTEM_START_LED_GPIO_CH    GPIO_CH2
#define SYSTEM_START_LED_GPIO_PIN   GPIOPortB_Pin5
#define SYSTEM_START_LED_ON_STATE  ( GPIO_LOW)
#define SYSTEM_START_LED_OFF_STATE  ( GPIO_HIGH)

/*  wifi LED B4 */
#define WIFI_STATE_LED_GPIO_CH    GPIO_CH2
#define WIFI_STATE_LED_GPIO_PIN   GPIOPortB_Pin4//wifi connect status
#define WIFI_START_LED_ON_STATE  ( GPIO_HIGH )
#define WIFI_START_LED_OFF_STATE  ( GPIO_LOW )

/*
********************************************************************************
*
*                         End of board_main_v22_2015027_config.h
*
********************************************************************************
*/

#endif

