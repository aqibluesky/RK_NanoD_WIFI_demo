/*
********************************************************************************************
*
*				  Copyright (c): 2014 - 2014 + 5, Aaron.sun
*							   All rights reserved.
*
* FileName: ..\Cpu\NanoC\lib\sdmmc.h
* Owner: Aaron.sun
* Date: 2014.2.20
* Time: 10:11:15
* Desc: sd mmc control bsp driver
* History:
*	 <author>	 <date> 	  <time>	 <version>	   <Desc>
* Aaron.sun     2014.2.20     10:11:15   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_SDMMC_H__
#define __CPU_NANOC_LIB_SDMMC_H__

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
#define FIFO_DEPTH        (0x20)       //FIFO depth = 32 word

#if 0
#define FOD_FREQ              (400)    //��ʶ��׶�ʹ�õ�Ƶ��,��λKHz,Э��涨���400KHz
//���������������Ƶ��ΪFREQ_HCLK_MAX/8
#define SD_FPP_FREQ           (24000)  //��׼SD����������Ƶ�ʣ���λKHz��Э��涨���25MHz
#define SDHC_FPP_FREQ         (48000)  //SDHC���ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���50MHz
#define MMC_FPP_FREQ          (18000)  //��׼MMC����������Ƶ�ʣ���λKHz��Э��涨���20MHz
#define MMCHS_26_FPP_FREQ     (25000)  //����ģʽֻ֧�����26M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���26MHz
#define MMCHS_52_FPP_FREQ     (50000)  //����ģʽ��֧�����52M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���52MHz
#define MAX_CLKIN_FREQ        (50000)
#else
//���ǵ�ϵͳ��Ƶ�Կ�������Ӱ�죬����Ӱ����OS��Խ��Խ���ԣ����ǵ�ϵͳ�Կ��������ٶ�Ҫ���Ǻܸߣ��ʽ��������̶���24M
//��Ӧ��Ƶ���µ�
#define FOD_FREQ              (400)    //��ʶ��׶�ʹ�õ�Ƶ��,��λKHz,Э��涨���400KHz
//���������������Ƶ��ΪFREQ_HCLK_MAX/8
#define SD_FPP_FREQ           (24000)  //��׼SD����������Ƶ�ʣ���λKHz��Э��涨���25MHz
#define SDHC_FPP_FREQ         (24000)  //SDHC���ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���50MHz
#define MMC_FPP_FREQ          (18000)  //��׼MMC����������Ƶ�ʣ���λKHz��Э��涨���20MHz
#define MMCHS_26_FPP_FREQ     (24000)  //����ģʽֻ֧�����26M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���26MHz
#define MMCHS_52_FPP_FREQ     (24000)  //����ģʽ��֧�����52M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���52MHz
#define MAX_CLKIN_FREQ        (24000)

#endif

/* Command Register */
#define START_CMD           (0x1U << 31) //start command
#define USE_HOLD_REG        (1 << 29)
#define VOLT_SWITCH         (1 << 28)
#define BOOT_MODE           (1 << 27)
#define DISABLE_BOOT        (1 << 26)
#define EXPECT_BOOT_ACK     (1 << 25)
#define ENABLE_BOOT         (1 << 24)
#define CCS_EXPECTED        (1 << 23)
#define READ_CEATA          (1 << 22)

#define UPDATE_CLOCK        (1 << 21)    //update clock register only
#define SEND_INIT           (1 << 15)    //send initialization sequence
#define STOP_CMD            (1 << 14)    //stop abort command
#define NO_WAIT_PREV        (0 << 13)    //not wait previous data transfer complete, send command at once
#define WAIT_PREV           (1 << 13)    //wait previous data transfer complete
#define AUTO_STOP           (1 << 12)    //send auto stop command at end of data transfer
#define BLOCK_TRANS         (0 << 11)    //block data transfer command
#define STREAM_TRANS        (1 << 11)    //stream data transfer command
#define READ_CARD           (0 << 10)    //read from card
#define WRITE_CARD          (1 << 10)    //write to card
#define NOCARE_RW           (0 << 10)    //not care read or write
#define NO_DATA_EXPECT      (0 << 9)     //no data transfer expected
#define DATA_EXPECT         (1 << 9)     //data transfer expected
#define NO_CHECK_R_CRC      (0 << 8)     //do not check response crc
#define CHECK_R_CRC         (1 << 8)     //check response crc
#define NOCARE_R_CRC        CHECK_R_CRC  //not care response crc
#define SHORT_R             (0 << 7)     //short response expected from card
#define LONG_R              (1 << 7)     //long response expected from card
#define NOCARE_R            SHORT_R      //not care response length
#define NO_R_EXPECT         (0 << 6)     //no response expected from card
#define R_EXPECT            (1 << 6)     //response expected from card

//sdc event
#define SDC_EVENT_SDC_ERR       (0x1 << 0)
#define SDC_EVENT_RX_DATA_REQ   (0x01 << 1)
#define SDC_EVENT_TX_DATA_REQ   (0x01 << 2)
#define SDC_EVENT_CMD_OK        (0x01 << 3)
#define SDC_EVENT_DATA_TRANS_OK (0x01 << 4)
#define SDC_EVENT_SDIO          (0X01 << 5)
#define SDC_EVENT_RESP_TIMEOUT  (0X01 << 6)
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
extern rk_err_t SDC_SetChannel(SDMMC_PORT SDCPort, uint32 Channel);
extern void SDC_DisableDMA(SDMMC_PORT SDCPort);
extern void SDC_EnableDMA(SDMMC_PORT SDCPort);
extern uint32 SDC_GetFIFOAddr(SDMMC_PORT SDCPort);
extern rk_err_t SDC_CheckDataBusy(SDMMC_PORT SDCPort);
extern void SDC_ClearIRQ(SDMMC_PORT SDCPort, uint32 intmask);
extern rk_err_t SDC_SetBusWidth(SDMMC_PORT SDCPort, BUS_WIDTH width);
extern void SDC_DevReset(eGPIO_CHANNEL gpio_port, eGPIOPinNum_t GpioNum, BOOL DevReset);
extern void SDC_DevPower(eGPIO_CHANNEL gpio_port, eGPIOPinNum_t GpioNum, BOOL DevPower);
extern uint32 SDC_GetResponse(SDMMC_PORT SDCPort, uint32 ResponseNum);
extern rk_err_t SDC_SetByteCnt(SDMMC_PORT SDCPort, uint32 ByteCnt);
extern rk_err_t SDC_SetCmdArg(SDMMC_PORT SDCPort, uint32 CmdArg);
extern rk_err_t SDC_SetBlockSize(SDMMC_PORT SDCPort, uint32 BlockSize);
extern rk_err_t SDC_SetDmaMode(SDMMC_PORT SDCPort, uint32 OpenDma);
extern uint32 SDC_GetIRQ(SDMMC_PORT SDCPort, uint32 * Int);
extern rk_size_t SDC_ReadData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize);
extern rk_size_t SDC_WriteData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize);
extern rk_err_t SDC_ResetFIFO(SDMMC_PORT SDCPort);
extern rk_err_t SDC_Init(SDMMC_PORT SDCPort, uint32 FreqKHz);
extern rk_err_t SDC_UpdateFreq(SDMMC_PORT SDCPort, uint32 FreqKHz);



#endif

