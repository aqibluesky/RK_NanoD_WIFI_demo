/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, WJR
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_ape.c
* Owner: WJR
* Date: 2014.11.28
* Time: 16:59:12
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     16:59:12   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_APE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "DriverInclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: APE_Clear
** Input:UINT32 HifiId
** Return: rk_err_t
** ����һ�׸�տ�ʼ����һ�׸�ս���
** Owner:WJR
** Date: 2014.12.5
** Time: 17:49:09
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_APE_COMMON_
COMMON API rk_err_t APE_Clear(UINT32 HifiId)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->APE_CFG |= APE_CLC;
}
/*******************************************************************************
** Name: APE_Set_CFG
** Input:UINT32 HifiId,, int lpc_quant,int version,int_com_level
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 17:01:15
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_APE_COMMON_
COMMON API rk_err_t APE_Set_CFG(UINT32 HifiId,int version,int com_level)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    if(version <3980)
    {
       pHifi->APE_CFG =APE_VERSION_LESS_3980;
    }
    else
    {
       pHifi->APE_CFG =APE_VERSION_EQUAL_OR_GREAT_3980;
    }
    if(com_level == 1000)
    {
       pHifi->APE_CFG |=APE_COMPRESS_LEVEL_1;
    }
    else if(com_level == 2000)
    {
       pHifi->APE_CFG |=APE_COMPRESS_LEVEL_2|APE_LPC_QUANT_16;
    }
    else if(com_level == 3000)
    {
       pHifi->APE_CFG |=APE_COMPRESS_LEVEL_3|APE_LPC_QUANT_64;
    }
    else if(com_level == 4000)
    {
       pHifi->APE_CFG |=APE_COMPRESS_LEVEL_4|APE_LPC_QUANT_32|APE_LPC_QUANT_256;
    }
    else if(com_level == 5000)
    {
       pHifi->APE_CFG |=APE_COMPRESS_LEVEL_5|APE_LPC_QUANT_16|APE_LPC_QUANT_256\
                       | APE_LPC_QUANT_1280;
    }
    // printf("APE_CFG 0x%x \n",pHifi->APE_CFG);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SHELL_APE_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 16:21:38
*******************************************************************************/
#ifdef _HIFI_SHELL_



_CPU_NANOD_LIB_HIFI_APE_SHELL_
extern unsigned long SysTickCounter;

SHELL API rk_err_t ape_test(int com_level,int *input)
{
    int count = 2048;
    int i,j,k;
    int count_rc =0;
    int  SysTick,Tick,Time;
    #ifdef APE_VERSION_3990
    int version = 3990;
    #else
    int version = 3970;
    #endif
    int compress_level = com_level;
    int circle =2048/count;
    SysTick = SysTickCounter;
    for(k =0;k<96;k++)
    {
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_L);//��ʼ�����������ݺͳ�ʼ��ϵ��(����fifo��)
    APE_Set_CFG(0,version,compress_level);
    APE_Clear(0);
    Hifi_Set_ACC_clear(0);//fpga �ڲ���ʵ��
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_L);//���Կ�ʼ��fifo�����ݣ����ҿ���ȡ����
    HIFI_DMA_TO_register();
	  for(i=0;i<circle;i++)
    {
    /*********������************/

        Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_L);//��ʼ�����������ݺͳ�ʼ��ϵ��(����fifo��)
        APE_Set_CFG(0,version,compress_level);
        Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_L);//���Կ�ʼ��fifo�����ݣ����ҿ���ȡ����

#ifdef DMA_TRAN_APE
        HIFI_DMA_TO_ACC(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_L[i*count]);
#else
        count_rc = 0;
        HIFITranData_fifo(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_L[i*count],&count_rc);
        rk_printf_no_time("count_read %d\n ",count_rc);
#endif


        /*********������************/
        Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_R);//��ʼ�����������ݺͳ�ʼ��ϵ��(����fifo��)
        APE_Set_CFG(0,version,compress_level);
        Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_R);//���Կ�ʼ��fifo�����ݣ����ҿ���ȡ����
        count_rc = 0;
        {
#ifdef DMA_TRAN_APE
        HIFI_DMA_TO_ACC(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_R[i*count]);
#else
        HIFITranData_fifo(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_R[i*count],&count_rc);
        rk_printf_no_time("count_read %d\n ",count_rc);
#endif
        }

      }
    }

    Tick = SysTickCounter -  SysTick;
    Time = Tick *10;
    rk_printf_no_time("\nDMA ape %d num is %d(ms)",circle*count,Time);
    HIFI_DMA_TO_Unregister();
    {

        for(i=0;i<count*circle;i++)
        {
           if((test_output_ape_L[i] != output_ape[i])  )
            {
              break;
            }
        }
         for(j=0;j<count*circle;j++)
        {
           if((test_output_ape_R[j] != output_ape[j]))
            {
              break;
            }
        }
        if((i == count*circle) &&(j == count*circle) )
        {
            rk_printf_no_time("%d %d  %d ape test over\n",version,compress_level,circle*count);
            return  RK_SUCCESS;
        }
        else
        {
            rk_printf_no_time("%d %d  %d ape test error from L:%d R:%d \n",version,compress_level,circle*count,i,j);
            return RK_ERROR  ;
        }
    }
}


SHELL API rk_err_t hifi_ape_shell(void)
{
    {
  rk_printf_no_time("\n-------------APE START-------------");
  ape_test(2000,test_data_ape2000);
  //ape_test(3000,test_data_ape3000);
  //ape_test(4000,test_data_ape4000);
  #ifdef APE_VERSION_3990
 // ape_test(5000,test_data_ape5000);
  #endif

    }
}
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/




#endif
