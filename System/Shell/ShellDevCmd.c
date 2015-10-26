/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                                         All rights reserved.
*File    : NanoShell.C
*By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            DESCRIPTION
*    RK_NANO_OS is a system designed specifically for real-time embedded SOC operating system ,before using
*RK_NANO_OS sure you read the user's manual
*    This file is to provide Debug shell
*    The FreeRTOS provide these macro to trace:
*    �궨��                                                 ˵��
*    traceTASK_INCREMENT_TICK()                        �ڽ����жϵ��á�
*    traceTASK_SWITCHED_OUT()                        ��ѡ������������ǰ���á���ʱ pxCurrentTCB �����˽���������״̬����ľ����
*    traceTASK_SWITCHED_IN()                        ��ѡ��������������С���ʱ pxCurrentTCB �����˼�����������״̬����ľֲ���
*    traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue)        ����ǰ��������������Ϊ��ͼ��ȡ�ն��У�������ͼ'��ȡ'���źŻ򻥳⡣
*    traceBLOCKING_ON_QUEUE_SEND(pxQueue)            ����ǰ����������Ϊ��ͼд��һ���Ѿ����Ķ��С�
*    traceGIVE_MUTEX_RECURSIVE(pxMutex)                ������д����ɹ��� xSemaphoreGiveRecursive() �е��á�
*    traceQUEUE_CREATE()                            �� xQueueCreate() �е��á�
*    traceQUEUE_CREATE_FAILED()                        ���������Ϊ���ڴ治����������ɹ��� xQueueCreate() �е��á�
*    traceCREATE_MUTEX()                            ������ⴴ���ɹ��� xSemaphoreCreateMutex() �е��á�
*    traceCREATE_MUTEX_FAILED()                        ���������Ϊ���ڴ治����������ɹ��� xSemaphoreCreateMutex() �е��á�
*    traceGIVE_MUTEX_RECURSIVE(pxMutex)                ���'��ȡ'����ɹ��� xSemaphoreGiveRecursive() �е��á�
*    traceGIVE_MUTEX_RECURSIVE_FAILED(pxMutex)        ��������߲��ǻ����������� xSemaphoreGiveRecursive() �е��á�
*    traceTAKE_MUTEX_RECURSIVE(pxMutex)                �� xQueueTakeMutexRecursive() �е��á�
*    traceCREATE_COUNTING_SEMAPHORE()                ����źŵƴ����ɹ��� xSemaphoreCreateCounting() �е��á�
*    traceCREATE_COUNTING_SEMAPHORE_FAILED()        ����źŵ���Ϊ���ڴ治�㴴�����ɹ�ʱ�� xSemaphoreCreateCounting() �е��á�
*    traceQUEUE_SEND(pxQueue)                        �����з��ͳɹ�ʱ���� xQueueSend(), xQueueSendToFront(), xQueueSendToBack(), ���κ��źŵ� '����'�����е��á�
*    traceQUEUE_SEND_FAILED(pxQueue)                ����Ϊ������(�ڴﵽָ������ʱ��)��ɷ���ʧ�ܺ��� xQueueSend(), xQueueSendToFront(), xQueueSendToBack(), �������źŵ� '����' �����е��á�
*    traceQUEUE_RECEIVE(pxQueue)                    �����н��ճɹ����� xQueueReceive() �������źŵ� '��ȡ'�����е��á�
*    traceQUEUE_RECEIVE_FAILED()                    ����Ϊ���п�(�ڴﵽָ������ʱ���)��ɽ���ʧ�ܺ��� xQueueReceive() �������źŵ� '��ȡ' �����е��á�
*    traceQUEUE_PEEK()                                �� xQueuePeek() �е��á�
*    traceQUEUE_SEND_FROM_ISR(pxQueue)                �����ͳɹ�ʱ�� xQueueSendFromISR() �е��á�
*    traceQUEUE_SEND_FROM_ISR_FAILED(pxQueue)        ����Ϊ��������ɷ���ʧ��ʱ�� xQueueSendFromISR() �е��á�
*    traceQUEUE_RECEIVE_FROM_ISR(pxQueue)            �����ճɹ�ʱ�� xQueueReceiveFromISR() �е��á�
*    traceQUEUE_RECEIVE_FROM_ISR_FAILED(pxQueue)    ����Ϊ���п���ɽ���ʧ��ʱ�� xQueueReceiveFromISR() �е��á�
*    traceQUEUE_DELETE(pxQueue)                        �� vQueueDelete() �е��á�
*    traceTASK_CREATE(pxTask)                        �����񴴽��ɹ�ʱ�� xTaskCreate() �е��á�
*    traceTASK_CREATE_FAILED()                        ��������Ϊ�ѿռ䲻�㴴��ʧ��ʱ�� xTaskCreate() �е��á�
*    traceTASK_DELETE(pxTask)                        �� vTaskDelete() �е��á�
*    traceTASK_DELAY_UNTIL()                        �� vTaskDelayUntil() �е��á�
*    traceTASK_DELAY()                                �� vTaskDelay() �е��á�
*    traceTASK_PRIORITY_SET(pxTask,uxNewPriority)    �� vTaskPrioritySet() �е��á�
*    traceTASK_SUSPEND(pxTask)                        �� vTaskSuspend() �е��á�
*    traceTASK_RESUME(pxTask)                        �� vTaskResume() �е��á�
*    traceTASK_RESUME_FROM_ISR(pxTask)                �� xTaskResumeFromISR() �е��á�
*
*
*
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "BspConfig.h"

#ifdef SHELL_DEV_PAR

#include "RKOS.h"
#include "BSP.h"

static char* ShellDeviceName[] =
{
    "list",
    "create",
    "\b"
};

static char* ShellDeviceList[] =
{
    "emmccode",
    "emmcdata",
    "emmcuser1",
    "sdiodevlist",
    "\b"
};

/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellDevCreate(HDC dev, char * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDeviceList, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:
#ifdef __OS_PLUGIN_DEVICEPLUGIN_C__
            ret = DeviceTask_CreateDeviceList(0);
#endif
            if (ret != RK_SUCCESS)
            {
                UartDev_Write(dev,"emmc code create fail",21,SYNC_MODE,NULL);
                ret = RK_SUCCESS;
            }
            break;

        case 0x01:
            //create
            break;

        case 0x02:
            //connect
            break;

        case 0x03:
#ifdef __DRIVER_SDIO_SDIODEVICE_C__
            ret = DeviceTask_CreateDeviceList(DEVICE_LIST_SDIO_FIFO);
#endif
            if (ret != RK_SUCCESS)
            {
                UartDev_Write(dev,"sdc code create fail",20,SYNC_MODE,NULL);
                ret = RK_SUCCESS;
            }
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}



/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellDeviceParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;


    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDeviceName, pItem, StrCnt);

    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:
            //list
            break;

        case 0x01:
            //create
            ret = ShellDevCreate(dev, pItem);
            break;

        case 0x02:
            //remove
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}
#endif

