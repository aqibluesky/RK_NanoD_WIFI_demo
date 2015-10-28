/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FileStream\StreamControlTask.c
* Owner: aaron.sun
* Date: 2015.6.18
* Time: 16:02:34
* Version: 1.0
* Desc: stream control task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.18     16:02:34   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_FILESTREAM_STREAMCONTROLTASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "audio_globals.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _STREAMCONTROL_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}STREAMCONTROL_RESP_QUEUE;
typedef  struct _STREAMCONTROL_ASK_QUEUE
{
    uint32 cmd;

}STREAMCONTROL_ASK_QUEUE;
typedef  struct _STREAMCONTROL_TASK_DATA_BLOCK
{
    pQueue  StreamControlAskQueue;
    pQueue  StreamControlRespQueue;
    pSemaphore osStreamControl;
    HDC hFile[8];
    HDC hFifo;
    uint32 cmd;
    uint32 data;

}STREAMCONTROL_TASK_DATA_BLOCK;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static STREAMCONTROL_TASK_DATA_BLOCK * gpstStreamControlData;



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
void RegMBoxFileSvc(void);
__irq void AudioDecodingInputFileBuffer(void);
void DeRegMBoxFileSvc(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: StreamControlTask_SendFileHandle
** Input:HDC hFile, uint32 FileNum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.29
** Time: 9:21:27
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_COMMON_
COMMON API rk_err_t StreamControlTask_SendFileHandle(HDC hFile, uint32 FileNum)
{
    while(RKTaskFind(TASK_ID_STREAMCONTROL, 0) == NULL)
    {
        rkos_delay(10);
    }
    rk_printf("file handle = %x, num = %d", hFile, FileNum);

    if(FileNum == 4)
    {
        gpstStreamControlData->hFifo = hFile;
    }
    else
    {
        gpstStreamControlData->hFile[FileNum] = hFile;
    }
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: StreamControlTask_Enter
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 16:04:32
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_COMMON_
COMMON API void StreamControlTask_Enter(void)
{
   FILE_SEEK_OP_t    *gpFileSeekParm;
   FILE_READ_OP_t    *gpFileReadParm;
   FILE_WRITE_OP_t   *gpFileWriteParm;
   FLAC_SEEKFAST_OP_t *gpFlacSeekFastParm;

   rk_printf("stream control enter....");

    while(1)
    {
        rkos_semaphore_take(gpstStreamControlData->osStreamControl, MAX_DELAY);

        switch(gpstStreamControlData->cmd)
        {
            case MEDIA_MSGBOX_CMD_FILE_SEEK:
                {
                    uint8 ret;

                    //rk_printf("file seek");

                    gpFileSeekParm = (FILE_SEEK_OP_t*)gpstStreamControlData->data;

                    if(gpstStreamControlData->hFifo == NULL)
                    {
                        ret = FileDev_FileSeek(gpstStreamControlData->hFile[gpFileSeekParm->handle], gpFileSeekParm->whence, gpFileSeekParm->offset);
                        FileDev_GetFileOffset(gpstStreamControlData->hFile[gpFileSeekParm->handle], &gpFileSeekParm->offset);
                    }
                    else
                    {
                        printf("hFifo ! NULL\n");
                        ret = fifoDev_ReadSeek(gpstStreamControlData->hFifo, gpFileSeekParm->whence, gpFileSeekParm->offset);
                        fifoDev_GetOffset(gpstStreamControlData->hFifo, &gpFileSeekParm->offset);
                    }
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                    MailBoxWriteA2BData(ret, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                }
                break;

            case MEDIA_MSGBOX_CMD_FILE_READ:
                {
                    uint32 ret;
                    uint8  handle;
                    uint8 *dataptr;
                    uint32 num;
                    gpFileReadParm = (FILE_READ_OP_t*)gpstStreamControlData->data;

                    handle  = gpFileReadParm->handle;
                    dataptr = gpFileReadParm->pData;
                    num     = gpFileReadParm->NumBytes;

                    //rk_printf("file read hDc = %x, num = %d", gpstStreamControlData->hFile[handle], num);

                    if(gpstStreamControlData->hFifo == NULL)
                    {

                        ret = FileDev_ReadFile(gpstStreamControlData->hFile[handle], dataptr, num);

                        #if 0
                        {
                            uint32 i;
                            for(i = 0; i < 512; i++)
                            {
                                if(i % 16 == 0)
                                    printf("\n");
                                printf("%02x ", dataptr[i]);
                            }
                        }
                        #endif

                        FileDev_GetFileOffset(gpstStreamControlData->hFile[handle], &gpFileReadParm->NumBytes);


                        MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_READ_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                        MailBoxWriteA2BData(ret, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

                    }
                    else
                    {
                        //rk_printf("dataptr = %x, num = %d", dataptr, num);
                        ret = fifoDev_Read(gpstStreamControlData->hFifo, dataptr, num , SYNC_MODE, NULL);
                        fifoDev_GetOffset(gpstStreamControlData->hFifo, &gpFileReadParm->NumBytes);
                        MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_READ_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                        MailBoxWriteA2BData(ret, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                    }
                }
                break;

            case MEDIA_MSGBOX_CMD_FILE_WRITE:
                {
                    uint32 ret;
                    gpFileWriteParm = (FILE_WRITE_OP_t*)gpstStreamControlData->data;

                    if(gpstStreamControlData->hFifo == NULL)
                    {
                        ret = FileDev_WriteFile(gpstStreamControlData->hFile[gpFileWriteParm->handle],gpFileWriteParm->buf,
                                        gpFileWriteParm->size);
                    }
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                    MailBoxWriteA2BData(ret,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                }
                break;

            case MEDIA_MSGBOX_CMD_FILE_TELL:
                {
                    uint32 offset;
                    if(gpstStreamControlData->hFifo == NULL)
                    {
                        FileDev_GetFileOffset(gpstStreamControlData->hFile[gpFileSeekParm->handle], &offset);
                    }
                    else
                    {
                        fifoDev_GetOffset(gpstStreamControlData->hFifo, &offset);
                    }

                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_TELL_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                    MailBoxWriteA2BData(offset,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                }
                break;

            case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH:
                {
                    uint32 filesize;
                    if(gpstStreamControlData->hFifo == NULL)
                    {
                        FileDev_GetFileSize(gpstStreamControlData->hFile[gpFileSeekParm->handle], &filesize);
                    }
                    else
                    {
                        fifoDev_GetTotalSize(gpstStreamControlData->hFifo, &filesize);
                    }
                    rk_printf("filesize = %d", filesize);
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                    MailBoxWriteA2BData(filesize,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                }
                break;

            default:

                break;
        }
    }
}


/*******************************************************************************
** Name: StreamControlTask_Resume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 16:04:32
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_COMMON_
COMMON API rk_err_t StreamControlTask_Resume(void)
{

}

/*******************************************************************************
** Name: StreamControlTask_Suspend
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 16:04:32
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_COMMON_
COMMON API rk_err_t StreamControlTask_Suspend(void)
{

}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioDecodingInputFileBuffer
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 18:07:20
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_COMMON_
__irq COMMON FUN void AudioDecodingInputFileBuffer(void)
{
    gpstStreamControlData->cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    gpstStreamControlData->data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);

    rkos_semaphore_give_fromisr(gpstStreamControlData->osStreamControl);
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: StreamControlTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 16:04:32
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_INIT_
INIT API rk_err_t StreamControlTask_DeInit(void *pvParameters)
{
    uint8 i;
    rk_printf("StreamControlTask_DeInit\n");

    if (gpstStreamControlData->hFifo != NULL)
    {
        if (RKDev_Close(gpstStreamControlData->hFifo) != RK_SUCCESS)
        {
            printf("hsdc close failure\n");
            return RK_ERROR;
        }
    }
    for(i=0;i<8;i++)
    {
        if (gpstStreamControlData->hFile[i] != NULL)
        {
            if (RKDev_Close(gpstStreamControlData->hFile[i]) != RK_SUCCESS)
            {
                printf("hfile[%d] close failure\n",i);
                return RK_ERROR;
            }
        }
    }
    printf("will rkos_queue_delete \n",i);
    /*
    if(gpstStreamControlData->StreamControlAskQueue)
    {
        rkos_queue_delete(gpstStreamControlData->StreamControlAskQueue);
    }
    if(gpstStreamControlData->StreamControlRespQueue)
    {
        rkos_queue_delete(gpstStreamControlData->StreamControlRespQueue);
    }
    */
    //if(gpstStreamControlData->osStreamControl)
    {
        rkos_semaphore_delete(gpstStreamControlData->osStreamControl);
    }
    rk_printf("rkos_queue_delete\n");
    rkos_memory_free(gpstStreamControlData);

    #ifdef __SYSTEM_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveModule(MODULE_ID_STREAMCONTROL_TASK);
    #endif
    rk_printf("rkos_memory_free\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: StreamControlTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 16:04:32
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_INIT_
INIT API rk_err_t StreamControlTask_Init(void *pvParameters, void *arg)
{

    RK_TASK_CLASS*   pStreamControlTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_STREAMCONTROL_ARG * pArg = (RK_TASK_STREAMCONTROL_ARG *)arg;
    STREAMCONTROL_TASK_DATA_BLOCK*  pStreamControlTaskData;
    if (pStreamControlTask == NULL)
        return RK_PARA_ERR;

    pStreamControlTaskData = rkos_memory_malloc(sizeof(STREAMCONTROL_TASK_DATA_BLOCK));
    memset(pStreamControlTaskData, NULL, sizeof(STREAMCONTROL_TASK_DATA_BLOCK));

    gpstStreamControlData = pStreamControlTaskData;

    gpstStreamControlData->osStreamControl = rkos_semaphore_create(1, 0);
    gpstStreamControlData->hFifo = NULL;

    #ifdef __SYSTEM_FWANALYSIS_FWANALYSIS_C__
    FW_ModuleOverlay(MODULE_ID_STREAMCONTROL_TASK, MODULE_OVERLAY_CODE);
    #endif

    RegMBoxFileSvc();

    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RegMBoxFileSvc
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 15:28:44
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_INIT_
INIT FUN void RegMBoxFileSvc(void)
{
    IntRegister(INT_ID_MAILBOX2, (void*)AudioDecodingInputFileBuffer);
    IntPendingClear(INT_ID_MAILBOX2);
    IntEnable(INT_ID_MAILBOX2);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
}

/*******************************************************************************
** Name: DeRegMBoxFileSvc
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 18:04:54
*******************************************************************************/
_APP_FILESTREAM_STREAMCONTROLTASK_INIT_
INIT FUN void DeRegMBoxFileSvc(void)
{
    IntUnregister(INT_ID_MAILBOX2);
    IntPendingClear(INT_ID_MAILBOX2);
    IntDisable(INT_ID_MAILBOX2);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
