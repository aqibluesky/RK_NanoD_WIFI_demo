/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\USBMsc\UsbMscDevice.c
* Owner: wrm
* Date: 2015.5.25
* Time: 10:28:43
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm     2015.5.25     10:28:43   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_USBMSC_USBMSCDEVICE_C__

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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define SUPPORT_HOST

/* Command Block Wrapper */
typedef __packed struct _HOSTMSC_CBW
{
    uint32      dCBWSignature;
    uint32      dCBWTag;
    uint32      dCBWDataTransLen;
    uint8       bCBWFlags;
    uint8       bCBWLUN;
    uint8       bCDBLength;
    uint8       CBWCDB[CBWCDBLENGTH];
} HOST_CBW, MSC_CBW, *pHOST_CBW;


/* Command Status Wrapper */
typedef __packed struct _HOSTMSC_CSW
{
    uint32      dCSWSignature;
    uint32      dCSWTag;
    uint32      dCSWDataResidue;
    uint8       bCSWStatus;
} HOST_CSW, MSC_CSW;

typedef volatile struct _USB_BULK_XFER
{
    uint32  LBA;
    uint16  NendSec;
    uint16  TransSec;
    uint32  BufLen;
    uint32  BufEp;
    uint8   *buf;
} USB_BULK_XFER, *pUSB_BULK_XFER;

typedef struct MSC_LUN
{
    uint8   LunNum;
    uint8   changed[4];
    uint32  DiskID[4];
} MSC_LUN;

typedef volatile struct _MSC_DEVICE
{
    MSC_CBW         *cbw;
    MSC_CSW         csw;
    uint16          ActualCBWLen;
    uint8           mode;
    uint8           config;
    uint8           AlterInterface;
    uint8           BulkIn;
    uint8           BulkOut;
    USB_BULK_XFER   BulkXfer;
    uint8           connected;
    MSC_LUN         FileDisk;
} MSC_DEVICE, *pMSC_DEVICE;

// Host Device Disagreement Matrix
enum _HOST_DEV_DISAGREE
{
    CASEOK = 0,
    CASE1,
    CASE2,
    CASE3,
    CASE4,
    CASE5,
    CASE6,
    CASE7,
    CASE8,
    CASE9,
    CASE10,
    CASE11,
    CASE12,
    CASE13,
    CASECBW,
    CASECMDFAIL
};

typedef struct _USBMSC_EPDATA
{
    uint8    ep;
    uint32   size;
    uint8    buf[512];
    uint32   listlen;    //������
    struct _USBMSC_EPDATA *next;
} USBMSC_EPDATA;

typedef struct _TransData
{
    //uint8  ep[129];
    //uint32 size[129];
    //uint8  buf[512*129];
    uint8  ep[241];
    uint32 size[241];
    uint8  buf[512*241];
    uint32 datablock;
} TransData;


typedef  struct _USBMSC_DEVICE_CLASS
{
    DEVICE_CLASS stUSBMSCDevice;

    xSemaphoreHandle osUSBMSCReadReqSem;
    xSemaphoreHandle osUSBMSCReadSem;
    xSemaphoreHandle osUSBMSCWriteReqSem;
    xSemaphoreHandle osUSBMSCWriteSem;
    xSemaphoreHandle osUSBMSCControlReqSem;

    xSemaphoreHandle osUSBMSCOperReqSem;
    xSemaphoreHandle osUSBMSCOperSem;

    uint8            speed;
    HDC              hOtg;
    HDC              hLun;

    MSC_DEVICE       mscdev;
    VERIFY_PARAMS    verify_params;
    uint8            SCSIDATA_RequestSense[18];
    int32            mutux_status;        //ʹ���ź�����״̬��־
    //USBMSC_EPDATA   EpData_Head;
    TransData       *EpData;
    uint8            usbmode;
    int              write_status;
    int              invalid_cbw;

    /**Host Part**/
    uint32           CBWTag;
    SCSI_CMD         SCSICmd;
    HOST_INTERFACE   *interfaceDes;
    MSC_HOST         gMscHost;


}USBMSC_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static USBMSC_DEVICE_CLASS * gpstUSBMSCDevISR[USBMSC_DEV_NUM]=
{
    (USBMSC_DEVICE_CLASS *)NULL,(USBMSC_DEVICE_CLASS *)NULL
};


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//USB �豸������
__align(4) USB_DEVICE_DESCRIPTOR HSDeviceDescr =
{
    sizeof(USB_DEVICE_DESCRIPTOR),          //descriptor's size 18(1B)
    USB_DT_DEVICE,                          //descriptor's type 01(1B)
    0x0200,                                 //USB plan distorbution number (2B)
    0,                                      //1type code (point by USB)(1B),0x00
    0, 0,                                   //child type and protocal (usb alloc)(2B)
    EP0_PACKET_SIZE,                        //endpoint 0 max package length(1B)
    0x071b,                                 //VID(ID Vendo r)
    0x3203,                                 //PID(ID Product)
    0x0200,                                 // device serial number
    USB_STRING_MANUFACTURER,                // 1
    USB_STRING_PRODUCT,                     // 2
    USB_STRING_SERIAL,                      //producter,produce,device serial number index(3B)
    1                                       //feasible configuration parameter(1B)
};

//����������
__align(4) USB_CONFIGURATION_DESCRIPTOR HSFsgConfigs =
{
    sizeof(USB_CONFIGURATION_DESCRIPTOR),   //�������Ĵ�С9(1B)
    USB_DT_CONFIG,                          //������������02(1B)
    sizeof(USB_CONFIGURATION_DESCRIPTOR),
    1,                                      //������֧�ֵĽӿ���(1B)
    1,                                      //��ΪSet configuration��һ������ѡ������ֵ(1B)
    0,                                      //�������������ַ���������(1B)
    0x80,                                   //λͼ,���߹���&Զ�̻���(1B)
    200,
};

//�ӿ�������
__align(4) USB_INTERFACE_DESCRIPTOR HSDeviceInterface =
{
    sizeof(USB_INTERFACE_DESCRIPTOR),        //�������Ĵ�С9(1B)
    USB_DT_INTERFACE,                        //������������04(1B)
    0,                                       //�ӿڵı��(1B)
    0,                                       //����Ϊ��һ���ֶοɹ��滻������(1B)
    2,                                       //ʹ�õĶ˵���(�˵�0����)(1B)
    USB_DEVICE_CLASS_STORAGE,                //1���ʹ���(��USB����)(1B),USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_SCSI,                  //1�����ʹ���(��USB����)(1B),"0x06=Reduced Block Commands(RBC)"
    USB_PROTOCOL_CODE_BULK,                  //1Э�����(��USB����)(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    0,                                       //�ַ�������������(1B)
};

//�˵�������
__align(4) USB_ENDPOINT_DESCRIPTOR HSDeviceEnDes_IN =
{
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_IN_EP|0x80,
    USB_EPTYPE_BULK,
    HS_BULK_TX_SIZE,                        //HS_BULK_RX_SIZE,
    0,                                      //bulk trans invailed
};

//�˵�������
__align(4) USB_ENDPOINT_DESCRIPTOR HSDeviceEnDes_OUT =
{
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_OUT_EP,
    USB_EPTYPE_BULK,
    HS_BULK_RX_SIZE,                       //HS_BULK_RX_SIZE,
    0                                      //bulk trans invailed
};

//�����豸����������
__align(4) HS_DEVICE_QUALIFIER HS_Device_Qualifier=
{
    sizeof(HS_DEVICE_QUALIFIER),           //length of HS Device Descriptor
    0x06,                                  //HS Device Qualifier Type
    0x0200,                                // USB 2.0 version
    USB_DEVICE_CLASS_STORAGE,              //Device class
    USB_SUBCLASS_CODE_SCSI,                // Device SubClass
    USB_PROTOCOL_CODE_BULK,                //Device Protocol Code
    FS_BULK_RX_SIZE,                       //Maximum Packet SIze for other speed
    0x01,                                  //Number of Other speed configurations
    0x00                                   //Reserved
};

__align(4) USB_FSG_CONFIGS_DESCRIPTOR allDescriptor =
{
//����������
    sizeof(USB_CONFIGURATION_DESCRIPTOR),  //�������Ĵ�С9(1B)
    USB_DT_CONFIG,                         //������������02(1B)
    sizeof(USB_FSG_CONFIGS_DESCRIPTOR),
    1,                                     //������֧�ֵĽӿ���(1B)
    1,                                     //��ΪSet configuration��һ������ѡ������ֵ(1B)
    0,                                     //�������������ַ���������(1B)
    0x80,                                  //λͼ,���߹���&Զ�̻���(1B)
    200,
//�ӿ�������
    sizeof(USB_INTERFACE_DESCRIPTOR),      //�������Ĵ�С9(1B)
    USB_DT_INTERFACE,                      //������������04(1B)
    0,                                     //�ӿڵı��(1B)
    0,                                     //����Ϊ��һ���ֶοɹ��滻������(1B)
    2,                                     //ʹ�õĶ˵���(�˵�0����)(1B)
    USB_DEVICE_CLASS_STORAGE,              //1���ʹ���(��USB����)(1B),USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_SCSI,                //1�����ʹ���(��USB����)(1B),"0x06=Reduced Block Commands(RBC)"
    USB_PROTOCOL_CODE_BULK,                //1Э�����(��USB����)(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    0,
//�˵�������IN
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_IN_EP|0x80,
    USB_EPTYPE_BULK,
    HS_BULK_TX_SIZE,                       //HS_BULK_RX_SIZE,
    0,                                     //bulk trans invailed
//�˵�������OUT
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_OUT_EP,
    USB_EPTYPE_BULK,
    HS_BULK_RX_SIZE,                      //HS_BULK_RX_SIZE,
    0                                     //bulk trans invailed
};

//�ַ���������,LangId
__align(4) const uint8 USBLangId[4]=
{
    0x04,
    0x03,
    0x09, 0x04                           //English--0x0409
    //0x04, 0x08                         //Chinese--0x0804
};

// string descriptor
__align(4) const uint8 StringProduct[18] =
{
    18,
    0x03,

    'R', 0x00,
    'O', 0x00,
    'C', 0x00,
    'K', 0x00,
    ' ', 0x00,
    'M', 0x00,
    'P', 0x00,
    '3', 0x00
};

// string descriptor
__align(4) const uint8 StringManufacture[18] =
{
    18,
    0x03,

    'R', 0x00,
    'o', 0x00,
    'c', 0x00,
    'k', 0x00,
    'C', 0x00,
    'h', 0x00,
    'i', 0x00,
    'p', 0x00
};

// string number
__align(4) const uint8 StringSerialNumbel[18] =
{
    18,
    0x03,

    'U', 0x00,
    'S', 0x00,
    'B', 0x00,
    'V', 0x00,
    '1', 0x00,
    '.', 0x00,
    '0', 0x00,
    '0', 0x00
};

__align(4) uint8 StringConfig[]=
{
    18,
    0x03,

    'C', 0x00,
    'O', 0x00,
    'N', 0x00,
    'F', 0x00,
    'I', 0x00,
    'G', 0x00,
    ' ', 0x00,
    ' ', 0x00
};

__align(4) uint8 StringInterface[] =
{
    18,
    0x03,

    'I', 0x00,
    'N', 0x00,
    'T', 0x00,
    'E', 0x00,
    'R', 0x00,
    'F', 0x00,
    'A', 0x00,
    'C', 0x00
};

__align(4) uint8 SCSI_INQUIRY[] =
{
    0x00,                               //Peripheral Device Type    0, 5:CD-ROM
    0x80,                               //RMB   1
    0x00,                               //ISO Version+ECMA Version+ANSI Version 2
    0x00,                               //Response Data Format  3
    0x1f,                               //Additional Length(31) 4

    0x00,0x00,0x00,                     //reserved  5~7

    'R','o','c','k','C','h','i','p',    //Vendor Information        8~15

    'U','S','B',' ','M','P','3',        //Product Idernification    16~31
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',

    '1','.','0','0'                     //version : 1.00    //32~35
};


__align(4) uint8 SCSI_INQUIRY_SD[] =
{
    0x00,                               //Peripheral Device Type    0
    0x80,                               //RMB   1
    0x00,                               //ISO Version+ECMA Version+ANSI Version 2
    0x01,                               //Response Data Format  3
    0x1f,                               //Additional Length(31) 4

    0x00,0x00,0x00,                     //reserved  5~7

    'R','o','c','k','C','h','i','p',    //Vendor Information        8~15

    'U','S','B',' ',' ','S','D',        //Product Idernification    16~31
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',

    '1','.','0','0'                     //version : 1.00    //32~35
};

__align(4) uint8  SCSIDATA_Sense_FLASH[] =
{
    0x03,0x00,0x00,0x00,               //��3�ֽ�Ϊ0x80��ʾд����
    0x01,0x0a,0x00,0x10,
    0x00,0x00,0x00,0x00
};

static USBOGT_DESCRIPTOR usbMscDes;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t USBMSCDevShellTest(HDC dev, uint8 * pstr);
rk_err_t USBMSCDevShellTestRead(HDC dev,  uint8 * pstr);
rk_err_t USBMSCDevShellTestWrite(HDC dev,  uint8 * pstr);
rk_err_t USBMSCDevShellDel(HDC dev, uint8 * pstr);
rk_err_t USBMSCDevShellMc(HDC dev, uint8 * pstr);
rk_err_t USBMSCDevShellPcb(HDC dev, uint8 * pstr);
void     USBMSCDevIntIsr1(void);
void     USBMSCDevIntIsr0(void);
void     USBMSCDevIntIsr(uint32 DevID);
rk_err_t USBMSCDevDeInit(USBMSC_DEVICE_CLASS * pstUSBMSCDev);
rk_err_t USBMSCDevInit(USBMSC_DEVICE_CLASS * pstUSBMSCDev);
rk_err_t USBMSCDevResume(HDC dev);
rk_err_t USBMSCDevSuspend(HDC dev);


static void  MscSendCSW(MSC_DEVICE *pFsg);
static void  MscSenseData(uint8 SenseKey, uint8 ASC, uint8 ASCQ);
static void  MscTestUnitReady(MSC_DEVICE *pMsc);
static void  MscCSWHandler(MSC_DEVICE *pMsc, uint8 HostDevCase,uint16 DeviceTrDataLen);
static void  MscRequestSense(MSC_DEVICE *pMsc);
static void  MscInquiry(MSC_DEVICE *pMsc);
static void  MscPreventAllowMediumRemoval(MSC_DEVICE *pMsc);
static void  MscReadFormatCapacities(MSC_DEVICE *pMsc);
static void  MscStartStopUnit(MSC_DEVICE *pMsc);
static void  MscModeSense06(MSC_DEVICE *pMsc);
static void  MscReadCapacity(MSC_DEVICE *pMsc);
static void  MscRead10(MSC_DEVICE *pMsc);
static int   WriteData_To_Flash(MSC_DEVICE *pMsc);
static void  MscWrite10(MSC_DEVICE *pMsc);
static void  MscVerify10(MSC_DEVICE *pMsc);
static void  MscModeSense10(MSC_DEVICE *pMsc);
static void  MscReqest(uint8 ep, uint32 param, MSC_DEVICE *pMsc);
static void  MscClassReq(USB_CTRL_REQUEST *ctrl);
static int32 MscSetup(USB_CTRL_REQUEST *ctrl);
//static void  MscEpData_Insert (USBMSC_EPDATA *New_EpData);
//static void  MscEpData_Delete (void);
//static int32 MscCBWValidVerify(MSC_DEVICE *pMsc, uint8 *buf);


#ifdef SUPPORT_HOST
static int32  MscHostProbe(HOST_INTERFACE* intf);
static int32  MscHostGetPipe(MSC_HOST *pMscHost, HOST_INTERFACE* intf);
static int32  MscHostGetMaxLun(uint8 *pLun);
static int32  MscHostClearFeature(HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex);
static int32  MscHostGetInfo(MSC_HOST *pMscHost, uint8 lun);
static int32  MscHostInquiry(uint8 lun, void *pInquiry);
static void   MscHostnptxfemp(int32 ret);
static int32  MscHostReadFormatCapacity(uint8 lun, uint32 *pCapacity);
static uint32 MscHostSwap32(uint32 input);
static int32  MscHostTestUnitReady(uint8 lun);
static int32  MscHostRequestSense(uint8 lun);
static int32  MscHostRead10(uint8 lun, uint32 start, uint32 blks, void *pBuf);
static uint32 MscHostWrite10(uint8 lun, uint32 start, uint32 blks, void *pBuf);
static int32  MscHostBulkOnlyXfer(SCSI_CMD *sc);

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: USBMSCDev_Task_Enter
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API void USBMSCDev_Task_Enter(void)
{
    MSC_DEVICE *pMsc = &gpstUSBMSCDevISR[0]->mscdev;
    USB_CTRL_REQUEST *ctrl;
    TransData *EpData = NULL;
    uint32 *datablock = NULL;
    int i = 0;

    EpData = rkos_memory_malloc(sizeof(TransData));
    if (EpData == NULL)
    {
        printf ("\n malloc memory failure\n");
    }
    gpstUSBMSCDevISR[0]->EpData = EpData;
    memset (gpstUSBMSCDevISR[0]->EpData->ep, 0, 241);
    memset (gpstUSBMSCDevISR[0]->EpData->size, 0, 241);
    memset (gpstUSBMSCDevISR[0]->EpData->buf, 0, 241*512);
    gpstUSBMSCDevISR[0]->EpData->datablock = 0;

    datablock = &gpstUSBMSCDevISR[0]->EpData->datablock;

    while (1)
    {
        rkos_semaphore_take(gpstUSBMSCDevISR[0]->osUSBMSCOperSem, MAX_DELAY);
        if (gpstUSBMSCDevISR[0]->usbmode == DEVICE_MODE)
        {
            //Msc Device Mode
            if (*datablock > 0)
            {
                pMsc->BulkXfer.BufEp  = gpstUSBMSCDevISR[0]->EpData->ep[0];
                pMsc->BulkXfer.BufLen = gpstUSBMSCDevISR[0]->EpData->size[0];
                pMsc->BulkXfer.buf    = &gpstUSBMSCDevISR[0]->EpData->buf[0];

                if ((pMsc->BulkXfer.BufEp == 1 )&&(pMsc->BulkXfer.BufLen <= 512))
                {
                        pMsc->cbw = (MSC_CBW *)pMsc->BulkXfer.buf;
                        MscReqest(pMsc->BulkXfer.BufEp, pMsc->BulkXfer.BufLen, pMsc);
                        *datablock = *datablock -1;
                        if (*datablock > 0)
                        {
                            WriteData_To_Flash(pMsc);
                        }
                        if (gpstUSBMSCDevISR[0]->invalid_cbw == 1)
                        {
                            gpstUSBMSCDevISR[0]->invalid_cbw = 0;
                            goto NEXT;
                        }
                        MscSendCSW(pMsc);
NEXT:
                }
                else if ((pMsc->BulkXfer.BufEp == 0 )&&(pMsc->BulkXfer.BufLen <= 512))
                {
                    ctrl = (USB_CTRL_REQUEST *)pMsc->BulkXfer.buf;
                    MscSetup(ctrl);
                    *datablock = *datablock -1;
                }
                else
                {
                    printf ("ep=%d  size=%d *datablock=%d\n",
                            pMsc->BulkXfer.BufEp,
                            pMsc->BulkXfer.BufLen,
                            *datablock);
                }
            }
        }
        else
        {
       		rk_err_t ret;
        	LUN_DEV_ARG stLunArg;
			HDC hStorage, hLun, hPar, hFileDev;
			PAR_DEV_ARG stCreateArg;
			FAT_DEV_ARG stCreateFatArg;
			VOLUME_INF stVolumeInf;
							
            rkos_memory_free(EpData);
            //Msc Host Mode
            printf ("MSC host Sever Mode Start:%p\n",gpstUSBMSCDevISR[0]->interfaceDes);
            ret = MscHostProbe(gpstUSBMSCDevISR[0]->interfaceDes);
			printf ("MSC Host size %d ret= %d\n",gpstUSBMSCDevISR[0]->gMscHost.capacity[0],ret);
            printf ("?????????????????????????????????????????????????????????????????????????????????????????????????\n");

			hStorage = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
		    if (hStorage == NULL)
		    {
		        printf ("\nUsb Host hStorage open failure\n");
		        return ;
		    }

			stLunArg.dwEndLBA = gpstUSBMSCDevISR[0]->gMscHost.capacity[0];
			stLunArg.dwStartLBA = 0;
			stLunArg.dwEndLBA--;
			stLunArg.hStorage = hStorage;
			stLunArg.pfStorgeRead = USBMSCHost_Read;
			stLunArg.pfStorgeWrite = USBMSCHost_Write;
			
			ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);
			
			if (ret != RK_SUCCESS)
			{
				printf("LUN 4 Create Failure:%d",ret);
				return;
			}
			rk_print_string("\nUSB LUN 4 Create Success");
			
			
			hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);
			
			if((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
			{
			   rk_print_string("\n lun0 open failure");
			   return;
			}
			
			stCreateArg.hLun = hLun;
			stCreateArg.ParStartLBA = 0;
			
			ret = LunDev_GetSize(hLun, &stCreateArg.ParTotalSec);
			
			if(ret != RK_SUCCESS)
			{
			   rk_print_string("\nlun0 get size failure");
			   return;
			}
			rk_printf("\nlun Size %d \r\n",stCreateArg.ParTotalSec);	
			
			ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateArg);
			
			if(ret != RK_SUCCESS)
			{
			   rk_print_string("\npar0 create failure");
			   return;
			}
			rk_print_string("\npar0 create Success");
			
			hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
			if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
			{
				rk_print_string("\npar0 open failure");
				return;
			}
			rk_print_string("\npar0 open Success");
			stCreateFatArg.hPar = hPar;
			
			ret = RKDev_Create(DEV_CLASS_FAT, 0, &stCreateFatArg);
			
			if (ret != RK_SUCCESS)
			{
				rk_print_string("\n usbhost fat0 create failure");
				return;
			}
			rk_print_string("\n usbhost fat0 create Success"); 
			
			ret = RKDev_Create(DEV_CLASS_FILE, 0, NULL);
			
			if (ret != RK_SUCCESS)
			{
				rk_print_string("\nfile create failure");
				return;
			}
			rk_print_string("\nfile create Success");
			hFileDev = RKDev_Open(DEV_CLASS_FILE, 0, READ_WRITE_CTRL);
			
			if ((hFileDev == NULL) || (hFileDev == (HDC)RK_ERROR) || (hFileDev == (HDC)RK_PARA_ERR))
			{
				rk_print_string("\nfile open failure");
				return;
			}
			rk_print_string("\nfile open Success");
			stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);
			
			stVolumeInf.VolumeID = 'C';
			
			stVolumeInf.VolumeType = VOLUME_TYPE_FAT32;
			
			ret = FileDev_AddVolume(hFileDev, &stVolumeInf);
			if (ret != RK_SUCCESS)
			{
				rk_print_string("\nadd volume failure");
				return;
			}
			rk_print_string("\nadd volume Success\n");
			RKDev_Close(hFileDev);


			
        }
    }
    //rkos_memory_free(EpData);
}

/*******************************************************************************
** Name: MscScsiCmdHandle
** Input:ep buf size
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: �������ݴ���SCSI�������
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API static void MscScsiCmdHandle(uint8 ep, uint8* buf, uint32 size)
{
    MSC_DEVICE *pMsc = &gpstUSBMSCDevISR[0]->mscdev;
    uint32 *datablock = &(gpstUSBMSCDevISR[0]->EpData->datablock);
    gpstUSBMSCDevISR[0]->EpData->ep[*datablock] = ep;
    gpstUSBMSCDevISR[0]->EpData->size[*datablock] = size;
    memcpy(&gpstUSBMSCDevISR[0]->EpData->buf[*datablock * 512],
            buf,
            size);
    *datablock = *datablock + 1;
    gpstUSBMSCDevISR[0]->usbmode = DEVICE_MODE;

    if (gpstUSBMSCDevISR[0]->write_status == 1)
    {
        if (pMsc->BulkXfer.NendSec < *datablock)
        {
            rkos_semaphore_give(gpstUSBMSCDevISR[0]->osUSBMSCWriteSem);
        }
    }

    //�ͷ��ź�������Task�н������ݴ���
    rkos_semaphore_give(gpstUSBMSCDevISR[0]->osUSBMSCOperSem);

    return ;
}

/*******************************************************************************
** Name: UsbMscDev_Connect
** Input:HDC dev, uint32 Speed
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.21
** Time: 9:11:54
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API rk_err_t UsbMscDev_Connect(HDC dev, uint32 Speed)
{
    int ret;
    USBMSC_DEVICE_CLASS * pstUsbMscDev = (USBMSC_DEVICE_CLASS *)dev;
    FREQ_EnterModule(FREQ_MAX);

    ret = UsbOtgDev_Connect (dev,Speed);

    if (ret != RK_SUCCESS)
    {
        //rk_printf ("UsbOtg connect fail!\n");
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

#ifdef SUPPORT_HOST

/*******************************************************************************
** Name: USBMSCHost_Write
** Input:HDC dev, uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API rk_err_t USBMSCHost_Write (HDC   dev,
                                            uint32 start,
                                            void   *pBuf,
                                            uint32 blks)
{
    uint32 ret;
    uint8  lun;
    uint32 zheng = 0,yu = 0;
    int32  i = 0;

    USBMSC_DEVICE_CLASS * pstUSBMSC =  (USBMSC_DEVICE_CLASS *)dev;
    if (pstUSBMSC == NULL)
    {
        return RK_ERROR;
    }
#if 1
    lun = pstUSBMSC->gMscHost.LunMap[0];
    if ((lun!=0)&&(lun!=1)&&(lun!=2))
    {
        printf ("\nFirst Write lun=%d\n",lun);
        lun = 0;
    }
    //printf ("\nWrite lun=%d\n",lun);
#endif
    //USBMSCHost Write...
    if (blks > 10)
    {
        zheng = blks / 10;
        yu = blks % 10;
        for (i=0; i<zheng;i++)
        {
            ret = MscHostWrite10(lun, start+i*10, 10, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
            if (yu > 0)
            {
                pBuf = (uint8*)pBuf+512*10;
            }

        }
        if (yu > 0)
        {
            ret = MscHostWrite10(lun, start+i*10, yu, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
        }
    }
    else
    {
        ret = MscHostWrite10(lun, start, blks, pBuf);
        if (ret < 0)
        {
            return RK_ERROR;
        }
    }

    return blks;
}

/*******************************************************************************
** Name: USBMSCHost_Read
** Input:HDC dev, uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API rk_err_t USBMSCHost_Read (HDC   dev,
                                            uint32 start,
                                            void   *pBuf,
                                            uint32 blks)
{
    int32  ret;
    uint8  lun = 0;
    uint32 zheng = 0,yu = 0;
    int32  i = 0;

    USBMSC_DEVICE_CLASS * pstUSBMSC =  (USBMSC_DEVICE_CLASS *)dev;
    if (pstUSBMSC == NULL)
    {
        return RK_ERROR;
    }
#if 1
    lun = pstUSBMSC->gMscHost.LunMap[0];
    if ((lun!=0)&&(lun!=1)&&(lun!=2))
    {
        printf ("\nFirst Read lun=%d\n",lun);
        lun = 0;
    }
    printf ("\n Read lun=%d start:%d blks:%d\n",lun,start,blks);
#endif
    //USBMSCHost Read...
    if (blks > 10)
    {
        zheng = blks / 10;
        yu = blks % 10;
        for (i=0; i<zheng;i++)
        {
            ret = MscHostRead10(lun, start+i*10, 10, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
//            if (yu > 0)
//            {
                pBuf = ((uint8*)pBuf)+512*10;
 //           }

        }
        if (yu > 0)
        {
            ret = MscHostRead10(lun, start+i*10, yu, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
        }
    }
    else
    {
        ret = MscHostRead10(lun, start, blks, pBuf);
        if (ret < 0)
        {
            return RK_ERROR;
        }
    }

    return blks;
}


/*******************************************************************************
** Name: MscHost
** Input:HOST_INTERFACE* interfaceDes
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON API int32 MscHost(HOST_INTERFACE* interfaceDes)
{

    int32 i, n, ret=0;
    printf ("MscHost:%d\n",interfaceDes->IfDesc.bInterfaceClass);
    gpstUSBMSCDevISR[0]->usbmode = HOST_MODE;
    gpstUSBMSCDevISR[0]->interfaceDes = interfaceDes;
    //��ö�ٺ���������Ȳ��
    gpstUSBMSCDevISR[0]->gMscHost.pDev->toggle[1] = 0;

    rkos_semaphore_give(gpstUSBMSCDevISR[0]->osUSBMSCOperSem);

    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: MscHostGetPipe
** Input:uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostRead10(uint8 lun, uint32 start, uint32 blks, void *pBuf)
{
    MSC_HOST *pMscHost = &gpstUSBMSCDevISR[0]->gMscHost;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;
    if (!pMscHost->valid)
        return -HOST_ERR;

    sc->lun = pMscHost->LunMap[lun];
    sc->dirin = 1;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_READ10;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[2]=((uint8) (start>>24))&0xff;
    sc->cmd[3]=((uint8) (start>>16))&0xff;
    sc->cmd[4]=((uint8) (start>>8))&0xff;
    sc->cmd[5]=((uint8) (start))&0xff;
    sc->cmd[7]=((uint8) (blks>>8))&0xff;
    sc->cmd[8]=(uint8) blks & 0xff;
    sc->cmdlen=0x0a;
    sc->datalen=blks<<9;
    sc->pdata = (uint8*)pBuf;
    return MscHostBulkOnlyXfer(sc);

}

/*******************************************************************************
** Name: MscHostWrite10
** Input:uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static uint32 MscHostWrite10(uint8 lun, uint32 start, uint32 blks, void *pBuf)
{
    MSC_HOST *pMscHost = &gpstUSBMSCDevISR[0]->gMscHost;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;

    if (!pMscHost->valid)
        return -HOST_ERR;

    sc->lun = pMscHost->LunMap[lun];
    sc->dirin = 0;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_WRITE10;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[2]=((uint8) (start>>24))&0xff;
    sc->cmd[3]=((uint8) (start>>16))&0xff;
    sc->cmd[4]=((uint8) (start>>8))&0xff;
    sc->cmd[5]=((uint8) (start))&0xff;
    sc->cmd[7]=((uint8) (blks>>8))&0xff;
    sc->cmd[8]=(uint8) blks & 0xff;
    sc->cmdlen=0x0a;
    sc->datalen=blks<<9;
    sc->pdata = (uint8*)pBuf;

    return MscHostBulkOnlyXfer(sc);
}



/*******************************************************************************
** Name: MscHostGetPipe
** Input:MSC_HOST *pMscHost, HOST_INTERFACE* intf
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostGetPipe(MSC_HOST *pMscHost, HOST_INTERFACE* intf)
{
    int32 i, ret = -HOST_ERR;
    USB_ENDPOINT_DESCRIPTOR *ep = &intf->EpDesc[0];
    uint8 epnum = intf->IfDesc.bNumEndpoints;

    for (i=0; i<epnum; i++, ep++)
    {
        if (USBEpType(ep) == USB_ENDPOINT_XFER_BULK)
        {
            if (USBEpDirIn(ep))
            {
                if (!pMscHost->BulkIn)
                    pMscHost->BulkIn = USBEpNum(ep);
            }
            else
            {
                if (!pMscHost->BulkOut)
                    pMscHost->BulkOut = USBEpNum(ep);
            }
        }
    }

    if (pMscHost->BulkIn && pMscHost->BulkOut)
    {
        pMscHost->SendCtrlPipe = HostCreatePipe(intf->pDev, 0, 0);
        pMscHost->RecvCtrlPipe = HostCreatePipe(intf->pDev, 0, 1);
        pMscHost->SendBulkPipe = HostCreatePipe(intf->pDev, pMscHost->BulkOut, 0);
        pMscHost->RecvBulkPipe = HostCreatePipe(intf->pDev, pMscHost->BulkIn, 1);
        pMscHost->pDev = intf->pDev;
        ret = HOST_OK;
    }

    return ret;
}

/*******************************************************************************
** Name: MscHostGetMaxLun
** Input:uint8 *pLun
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostGetMaxLun(uint8 *pLun)
{
    MSC_HOST *pMscHost = &gpstUSBMSCDevISR[0]->gMscHost;

    return  HostCtrlMsg(pMscHost->pDev, pMscHost->RecvCtrlPipe, 0xA1, USB_BBB_GET_MAX_LUN,
                      0, 0, pLun, 1, USB_CNTL_TIMEOUT*5);
}

/*******************************************************************************
** Name: HostClearFeature
** Input:HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostClearFeature(HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex)
{
    int32 ret;
    ret = HostCtrlMsg(pDev, pipe, type, USB_REQ_CLEAR_FEATURE,
                      wValue, wIndex, NULL, 0, USB_CTRL_SET_TIMEOUT);
    if (ret < 0)
        return ret;

    // toggle is reset on clear
    USBSetToggle(pDev, USBPipeEpNum(pipe), USBPipeDir(pipe), 0);

    return ret;
}


/*******************************************************************************
** Name: MscHostnptxfemp
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
void MscHostnptxfemp(int32 ret)
{
    OtgHostnptxfemp(ret);
}

/*******************************************************************************
** Name: MscHostBulkOnlyXfer
** Input:SCSI_CMD *sc
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostBulkOnlyXfer(SCSI_CMD *sc)
{
    int32 ret, ret1, retry, count=0;
    HOST_CBW cbw;
    HOST_CSW csw;
    MSC_HOST *pMscHost = &gpstUSBMSCDevISR[0]->gMscHost;
    //GINTMSK_DATA gintmsk;

    /* COMMAND phase */
    cbw.dCBWSignature = CBWSIGNATURE;
    cbw.dCBWTag = gpstUSBMSCDevISR[0]->CBWTag++;
    cbw.dCBWDataTransLen = sc->datalen;
    cbw.bCBWFlags = (sc->dirin)? CBWFLAGS_IN : CBWFLAGS_OUT;
    cbw.bCBWLUN = sc->lun;
    cbw.bCDBLength = sc->cmdlen;

    /* copy the command data into the CBW command data buffer */
    memset(cbw.CBWCDB, 0, sizeof(cbw.CBWCDB));
    memcpy(cbw.CBWCDB, sc->cmd, sc->cmdlen);

    REPEAT:
    ret = HostBulkMsg(pMscHost->pDev, pMscHost->SendBulkPipe, &cbw, UMASS_BBB_CBW_SIZE, USB_CNTL_TIMEOUT*5);
    if (ret < 0)
    {
         //gintmsk.d32 = OTGReg->Core.gintmsk;
         //gintmsk.b.nptxfemp = (ret != 0);
         //OTGReg->Core.gintmsk = gintmsk.d32;
         MscHostnptxfemp(ret);
         if (count < 10)
         {
            count++;
            goto REPEAT;
         }
         return ret;
    }

    DelayUs(1);

    /* DATA phase + error handling */
    /* no data, go immediately to the STATUS phase */
    if (sc->datalen == 0)
        goto STATUS;

    ret = HostBulkMsg(pMscHost->pDev, (sc->dirin)? pMscHost->RecvBulkPipe : pMscHost->SendBulkPipe,
                        sc->pdata, sc->datalen, USB_CNTL_TIMEOUT*5);
    if (ret < 0)
    {
        if (-HOST_STALL == ret)
        {
            ret1 = HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, (sc->dirin)? (pMscHost->BulkIn|0x80) : pMscHost->BulkOut);
            if (ret1 < 0)
                return ret1;
        }
        else
            return ret;
    }

    /* STATUS phase + error handling */
STATUS:
    retry = 0;
AGAIN:
    ret1 = HostBulkMsg(pMscHost->pDev, pMscHost->RecvBulkPipe, &csw, UMASS_BBB_CSW_SIZE, USB_CNTL_TIMEOUT*5);
    if (ret1 < 0)
    {
        if (-HOST_STALL == ret1)
        {
            ret1 = HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, (pMscHost->BulkIn|0x80));
            if (ret1 >= 0 && (retry++ < 1))
            {
                goto AGAIN;
            }
        }
        else
            return ret1;
    }
    else
    {
        // Check Csw
        sc->status = csw.bCSWStatus;
    }

    return (ret >= 0)? HOST_OK : ret;
}


/*******************************************************************************
** Name: MscHostInquiry
** Input:uint8 lun, void *pInquiry
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostInquiry(uint8 lun, void *pInquiry)
{
    int32 ret;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;
    int retries = 3;

    sc->lun = lun;
    sc->dirin = 1;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_INQUIRY_CMD;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[4]=36;
    sc->cmdlen=0x06;
    sc->datalen=36;
    sc->pdata = (uint8*)pInquiry;

    do
    {
        ret = MscHostBulkOnlyXfer(sc);
        if(HOST_OK == ret)
        {
            return ret;
        }
    } while(retries--);

    return ret;
}

/*******************************************************************************
** Name: MscHostSwap32
** Input:uint32 input
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static uint32 MscHostSwap32(uint32 input)
{
    return ((input>>24)|(input<<24)|(input>>8&0xff00)|(input<<8&0xff0000));
}


/*******************************************************************************
** Name: MscHostReadFormatCapacity
** Input:uint8 lun, uint32 *pCapacity
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostReadFormatCapacity(uint8 lun, uint32 *pCapacity)
{
    int32 ret;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;
    int retries = 3;
    uint32 cap[4];

    do
    {
        sc->lun = lun;
        sc->dirin = 1;
        memset(&sc->cmd[0], 0, 12);
        sc->cmd[0]=SCSI_RD_FORMAT_CAPAC;
        sc->cmd[1]=sc->lun<<5;
        sc->cmd[8]=0xfc;        //�μ� mass_storage_ufi.pdf
        sc->cmdlen=0x0a;
        sc->datalen=0x0c;
        sc->pdata = (uint8*)&cap[0];

        ret = MscHostBulkOnlyXfer(sc);

        if(HOST_OK == ret)
        {
            if ((MscHostSwap32(cap[2]) & 0xffff)==512)
                *pCapacity = MscHostSwap32(cap[1]);
            printf ("\n--------------------------------------------------------\n");
            printf ("MscHostReadFormatCapacity: %dMB(%lfG)\n",
                    *pCapacity/2/1024,*pCapacity/2.0/1024/1024);
            printf ("\n--------------------------------------------------------\n");
            return ret;
        }
        else if (-HOST_STALL == ret)
        {
            MscHostRequestSense(lun);
            DelayMs(1);
        }
        else
        {
            return ret;
        }
    } while(retries--);

    return ret;
}

/*******************************************************************************
** Name: MscHostRequestSense
** Input:uint8 lun
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostRequestSense(uint8 lun)
{
    int32 ret;
    uint8 *ptr;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;

    sc->lun = lun;
    sc->dirin = 1;
    ptr=(uint8 *)sc->pdata;
    memset(&sc->cmd[0], 0, 12);

    sc->cmd[0]=SCSI_REQ_SENSE;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[4]=18;
    sc->datalen=18;
    sc->pdata= (uint8*)&sc->sense[0];
    sc->cmdlen=0x0a;

    ret = MscHostBulkOnlyXfer(sc);
    sc->pdata = (uint8 *)ptr;

    return ret;
}


/*******************************************************************************
** Name: MscHostTestUnitReady
** Input:uint8 lun
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostTestUnitReady(uint8 lun)
{
    int32 ret;
    SCSI_CMD *sc = &gpstUSBMSCDevISR[0]->SCSICmd;
    int retries = 10;

    do
    {
        sc->lun = lun;
        sc->dirin = 0;
        memset(&sc->cmd[0], 0, 12);
        sc->cmd[0]   = SCSI_TST_U_RDY;
        sc->cmd[1]   = sc->lun<<5;
        sc->cmdlen   = 0x06;
        sc->datalen  = 0;

        ret = MscHostBulkOnlyXfer(sc);
        if(HOST_OK == ret)
        {
            return ret;
        }
        else if (-HOST_STALL == ret)
        {
            MscHostRequestSense(lun);
            DelayMs(1);
        }
        else
        {
            return ret;
        }
    } while(retries--);

    return ret;
}

/*******************************************************************************
** Name: MscHostGetInfo
** Input:MSC_HOST *pMscHost, uint8 lun
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostGetInfo(MSC_HOST *pMscHost, uint8 lun)
{
    int32 ret;
    uint8 TmpBuf[64];

    ret = MscHostInquiry(lun, TmpBuf);
    if (ret < 0)
        return ret;

    if((TmpBuf[0]&0x1f) == 5)//cdrom
    {
        return -HOST_ERR;
    }

    ret = MscHostReadFormatCapacity(lun, &pMscHost->capacity[lun]);
    if (ret < 0)
        return ret;
    #if 0
    ret = MscReadCapacity(lun, &pMsc->capacity[lun]);
    if (ret < 0)
        return ret;
    pMsc->capacity[lun]++;
    #endif
    ret = MscHostTestUnitReady(lun);
    if (ret < 0)
        return ret;

    return HOST_OK;
}


/*******************************************************************************
** Name: MscHostProbe
** Input:HOST_INTERFACE* intf
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
static int32 MscHostProbe(HOST_INTERFACE* intf)
{
    int32 i, n, ret;
    MSC_HOST *pMscHost = &gpstUSBMSCDevISR[0]->gMscHost;

    uint8 LunNum;

    //CBWTag = 0;
    gpstUSBMSCDevISR[0]->CBWTag = 0;
    memset (pMscHost, 0, sizeof(MSC_HOST));

    ret =  MscHostGetPipe(pMscHost, intf);
    if (ret < 0)
        return ret;
	printf(":0x%08x \r\n",pMscHost->RecvBulkPipe);
	printf(":0x%08x \r\n",pMscHost->RecvCtrlPipe);
	printf(":0x%08x \r\n",pMscHost->SendBulkPipe);
	printf(":0x%08x \r\n",pMscHost->SendCtrlPipe);
    ret = MscHostGetMaxLun(&LunNum);
	printf(":%d ret:%d\r\n",LunNum,ret);
    if (ret < 0)
    {
        if (-HOST_STALL == ret)
        {
            HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, 0);
            LunNum = 1;
        }
        else
            return ret;
    }
    else
    {
        LunNum++;
    }

    if (LunNum > MAX_LUN_NUM)
    {
        LunNum = MAX_LUN_NUM;
    }

    for (i=0, n=0; i<LunNum; i++)
    {
        ret = MscHostGetInfo(pMscHost, i);
        if (ret >= 0)
        {
            pMscHost->LunMap[n++] = i;
        }
    }

    if (n > 0)
    {
        ret = HOST_OK;
        pMscHost->LunNum = n;
        pMscHost->valid = 1;
    }
    else
    {
        ret = -HOST_ERR;
    }


    return ret;
}

#endif

#if 0
/*******************************************************************************
** Name: MscEpData_Insert
** Input:New_EpData
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:���յ������ݴ���������
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscEpData_Insert (USBMSC_EPDATA *New_EpData)
{
    USBMSC_EPDATA * Current_EpData = &(gpstUSBMSCDevISR[0]->EpData_Head);
    Current_EpData->listlen++;
    while (Current_EpData->next != NULL)
    {
           Current_EpData = Current_EpData->next;
    }

    Current_EpData->next = New_EpData;
    New_EpData->next = NULL;
}
/*******************************************************************************
** Name: MscEpData_Delete
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:��ִ��������ݴ�������ɾ��
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscEpData_Delete (void)
{
    USBMSC_EPDATA  *Current_EpData = &(gpstUSBMSCDevISR[0]->EpData_Head);
    USBMSC_EPDATA  *Next_EpData = NULL;
    Current_EpData->listlen--;

    if (Current_EpData->next == NULL)
    {
        Current_EpData->listlen++;
        return ;
    }
    else
    {
        Next_EpData = Current_EpData->next;
    }

    Current_EpData->next = Next_EpData->next;

    rkos_memory_free(Next_EpData);

}
#endif
/*******************************************************************************
** Name: MscSendCSW
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:����CSW
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscSendCSW(MSC_DEVICE *pFsg)
{
    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, BULK_IN_EP, (uint8*)&pFsg->csw, 13);
}

/*******************************************************************************
** Name: MscSenseData
** Input:uint8 SenseKey, uint8 ASC, uint8 ASCQ
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:���� sense
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscSenseData(uint8 SenseKey, uint8 ASC, uint8 ASCQ)
{
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[0]=0x70;      //information filed is invalid=0x70,information filed is valid=0xf0,
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[1]=0x00;      //reserve
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[2]=SenseKey;

    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[3]=0x00;      //information
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[4]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[5]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[6]=0x00;

    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[7]=0x0a;      //Addition Sense Length

    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[8]=0x00;      //reserve
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[9]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[10]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[11]=0x00;

    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[12]=ASC;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[13]=ASCQ;

    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[14]=0x00;     //reserve
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[15]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[16]=0x00;
    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense[17]=0x00;
}

/*******************************************************************************
** Name: MscTestUnitReady
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:����׼��0x00
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscTestUnitReady(MSC_DEVICE *pMsc)
{
    if (pMsc->connected == 1)
    {
        MscCSWHandler(pMsc, CASE1,0);
    }
    else
    {
        MscCSWHandler(pMsc, CASE1,0);
        pMsc->csw.bCSWStatus = CSW_FAIL;
        MscSendCSW(pMsc);
        UsbOtgDev_DisConnect();
    }
    //MscSendCSW(pMsc);
}

/*******************************************************************************
** Name:  MscCSWHandler
** Input:FSG_DEVICE *pFsg, uint8 HostDevCase,uint16 DeviceTrDataLen
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: CSW����
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscCSWHandler(MSC_DEVICE *pMsc, uint8 HostDevCase,uint16 DeviceTrDataLen)
{
    MSC_CSW      *csw = &pMsc->csw;
    MSC_CBW      *cbw = pMsc->cbw;
    /* Store and send the Bulk-only CSW */
    csw->dCSWSignature = USB_BULK_CS_SIG;
    csw->dCSWTag = cbw->dCBWTag;
    csw->dCSWDataResidue = cbw->dCBWDataTransLen - DeviceTrDataLen;

    VERIFY_PARAMS *params = &(gpstUSBMSCDevISR[0]->verify_params);

    switch (HostDevCase)
    {
        case CASEOK:
        case CASE1:     /* Hn=Dn*/
        case CASE6:     /* Hi=Di*/
        case CASE12:    /* Ho=Do*/
            csw->bCSWStatus = CSW_GOOD;
            break;

        case CASE4:     /* Hi>Dn*/
        case CASE5:     /* Hi>Di*/
            //USBSetEpSts(1, BULK_IN_EP, 1);
            params->dir = 1;
            params->bulk = BULK_IN_EP;
            params->status = 1;
            csw->bCSWStatus = CSW_FAIL; //CSW_GOOD or CSW_FAIL
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            break;

        case CASE2:     /* Hn<Di*/
        case CASE3:     /* Hn<Do*/
        case CASE7:     /* Hi<Di*/
        case CASE8:     /* Hi<>Do */
            //USBSetEpSts(1, BULK_IN_EP, 1);
            csw->bCSWStatus = CSW_PHASE_ERROR;
            params->dir = 1;
            params->bulk = BULK_IN_EP;
            params->status = 1;
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            break;

        case CASE9:     /* Ho>Dn*/
        case CASE11:    /* Ho>Do*/
            //USBSetEpSts(0, BULK_OUT_EP, 1);
            csw->bCSWStatus = CSW_FAIL;
            params->dir = 0;
            params->bulk = BULK_OUT_EP;
            params->status = 1;
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            break;
        case CASE10:    /* Ho<>Di */
        case CASE13:    /* Ho<Do*/
            //USBSetEpSts(0, BULK_OUT_EP, 1);
            params->dir = 0;
            params->bulk = BULK_OUT_EP;
            params->status = 1;
            csw->bCSWStatus = CSW_PHASE_ERROR;
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            break;
        case CASECMDFAIL:
            csw->bCSWStatus = CSW_FAIL;
            break;
        case CASECBW:   /* invalid CBW */
            //USBSetEpSts(1, BULK_IN_EP, 1);
            params->dir = 1;
            params->bulk = BULK_IN_EP;
            params->status = 1;
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            if (cbw->dCBWDataTransLen && !(cbw->bCBWFlags & 0x80))
            {
                params->dir = 0;
                params->bulk = BULK_OUT_EP;
                params->status = 1;
                //USBSetEpSts(0, BULK_OUT_EP, 1);
                UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, (uint8*)params, sizeof(VERIFY_PARAMS));
            }
            csw->bCSWStatus = CSW_FAIL;
            break;

        default:
            break;
    }
}
#if 0
/*******************************************************************************
** Name:  MscCBWValidVerify
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: �������ЧУ��
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static int32 MscCBWValidVerify(MSC_DEVICE *pMsc, uint8 *buf)
{
    MSC_CBW *cbw = (MSC_CBW *)buf;

    if (cbw->dCBWSignature != USB_BULK_CB_SIG)
    {
        return ERROR;
    }

    /* Is the CBW meaningful? */
    if (cbw->bCBWLUN >= FSG_MAX_LUNS || cbw->bCBWFlags & ~USB_BULK_IN_FLAG ||
            cbw->bCDBLength <= 0 || cbw->bCDBLength > MAX_COMMAND_SIZE)
    {
        return ERROR;
    }

    return OK;
}
#endif
/*******************************************************************************
** Name:  MscRequestSense
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:����0x03
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscRequestSense(MSC_DEVICE *pMsc)
{
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    uint16 residue= MIN (sizeof(gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense), cmnd[4]);

    MscCSWHandler(pMsc, CASE6,residue);
    if (0 == cmnd[4])
    {
        //MscSendCSW(pMsc);
        return;
    }

    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg,
                    BULK_IN_EP,
                    gpstUSBMSCDevISR[0]->SCSIDATA_RequestSense,
                    residue);
    //MscSendCSW(pMsc);
}
/*******************************************************************************
** Name:  MscInquiry
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:��ѯ0x12
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscInquiry(MSC_DEVICE *pMsc)
{
    uint16 residue;
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];

    MscSenseData(SCSI_SENSE_NO_SENSE, 0, 0);
    MscCSWHandler(pMsc, CASE6, cmnd[4]);   //modify by lxs @2007.03.13, for Mac OS 9.2.2

    if (1)
    {
        uint8 *pInquiry = SCSI_INQUIRY;
        residue= MIN(sizeof(SCSI_INQUIRY), cmnd[4]);
        UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, pMsc->BulkIn, pInquiry, residue);
    }
}
/*******************************************************************************
** Name:  MscPreventAllowMediumRemoval
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:��ֹ�����Ƴ�0x1e
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscPreventAllowMediumRemoval(MSC_DEVICE *pMsc)
{
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    if ((cmnd[4] & 0x01) == 0x01)
    {   //���vista �� mac����U���˲����������⣬���ǻ����˳�U�̺���û�������״̬�ġ�
        //BuildSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_PARAMETER, SCSI_SENSEQ_INIT_COMMAND_REQUIRED);
        MscSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB, 0);
        MscCSWHandler(pMsc, CASECMDFAIL,(uint16)pMsc->cbw->dCBWDataTransLen);         //�����Ƴ��豸
        pMsc->csw.bCSWStatus = CSW_FAIL; // comment by hwg, 07-06-30
    }
    else
    {
        MscSenseData(SCSI_SENSE_NO_SENSE,0,0);
        MscCSWHandler(pMsc, CASE1,(uint16)pMsc->cbw->dCBWDataTransLen);          //�����Ƴ��豸
    }


    //MscSendCSW(pMsc);

}
/*******************************************************************************
** Name:  MscStartStopUnit
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:��ͣ����0x1b
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscStartStopUnit(MSC_DEVICE *pMsc)
{
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];

    MscSenseData(SCSI_SENSE_NO_SENSE,0,0);
    MscCSWHandler(pMsc, CASE1,(uint16)pMsc->cbw->dCBWDataTransLen);          //�����Ƴ��豸

    //MscSendCSW(pMsc);
    pMsc->connected = 0;
}

/*******************************************************************************
** Name:  MscModeSense06
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:��ģʽ0x1a
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscModeSense06(MSC_DEVICE *pMsc)
{
    uint8* pSenseData;

    MscCSWHandler(pMsc, CASEOK, 0x04);
    if (0)
    {
        MscSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        MscCSWHandler(pMsc, CASECBW, 0);
        return;
    }

    pSenseData = (uint8*)SCSIDATA_Sense_FLASH;
    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, pMsc->BulkIn, pSenseData, 4);
}

/*******************************************************************************
** Name:  MscReadCapacity
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:������0x25
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscReadCapacity(MSC_DEVICE *pMsc)
{
    uint8  capacity[8] = {0};
    uint32 TotalSecs;
    uint16 BytsPerSec;
    uint32 pSize;

    if (1)
    {
        LunDev_GetSize(gpstUSBMSCDevISR[0]->hLun, &pSize);
        if (pSize)
        {
            TotalSecs = pSize -1 ;
        }
        BytsPerSec=512;
        //printf ("\npSize=%d TotalSecs=%d\n",pSize,TotalSecs);
    }

    MscCSWHandler(pMsc, CASE6,8);

    capacity[0]=(uint8)((TotalSecs >> 24) & 0xff);
    capacity[1]=(uint8)((TotalSecs >> 16) & 0xff);
    capacity[2]=(uint8)(TotalSecs >> 8) & 0xff;
    capacity[3]=(uint8)(TotalSecs & 0xff);
    capacity[4]=0x00;
    capacity[5]=0x00;
    capacity[6]=(uint8)(BytsPerSec >> 8);       //bytes/block
    capacity[7]=(uint8)(BytsPerSec & 0xff);
    #if 0
    printf ("capacity[0]=0x%x\n",capacity[0]);
    printf ("capacity[1]=0x%x\n",capacity[1]);
    printf ("capacity[2]=0x%x\n",capacity[2]);
    printf ("capacity[3]=0x%x\n",capacity[3]);
    printf ("capacity[4]=0x%x\n",capacity[4]);
    printf ("capacity[5]=0x%x\n",capacity[5]);
    printf ("capacity[6]=0x%x\n",capacity[6]);
    printf ("capacity[7]=0x%x\n",capacity[7]);
    #endif
    //USBWriteEp(pFsg->BulkIn, 8, capacity);
    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, pMsc->BulkIn, capacity, 8);
}

/*******************************************************************************
** Name:  MscRead10
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:������0x28
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscRead10(MSC_DEVICE *pMsc)
{
    uint16 nSec;
    uint32 LBA;
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    pUSB_BULK_XFER Xfer = &(pMsc->BulkXfer);
    int i = 0;

    uint8 *buffer = &gpstUSBMSCDevISR[0]->EpData->buf[512];


    MscCSWHandler(pMsc, CASE6, (uint16)pMsc->cbw->dCBWDataTransLen);
    MscSenseData(SCSI_SENSE_NO_SENSE, 0, 0);

    nSec = __get_unaligned_be16((cmnd+7)); //��ʵЭ����������������
    LBA = __get_unaligned_be32((cmnd+2));

    //printf ("Ready read Lun\n");
    LunDev_Read (gpstUSBMSCDevISR[0]->hLun,
                 LBA,
                 buffer,
                 nSec);
    //printf ("read Lun complete\n");
    for (i=0; i<nSec; i++)
    {
        UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, &buffer[i*512], 512);
    }

#if 0
    uint16 nSec;
    uint32 byte;
    uint32 LBA;
    uint8 buffer[512*128] = {0};

    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    pUSB_BULK_XFER Xfer = &(pMsc->BulkXfer);
    int i = 0;


    MscCSWHandler(pMsc, CASE6, (uint16)pMsc->cbw->dCBWDataTransLen);
    MscSenseData(SCSI_SENSE_NO_SENSE, 0, 0);

    nSec = __get_unaligned_be16((cmnd+7)); //��ʵЭ����������������

    if (buffer != NULL)
    {
        byte = (uint32)nSec<<9; //sec's byte 512
        LBA = __get_unaligned_be32((cmnd+2));

        //printf ("Ready read data\n");
        LunDev_Read (gpstUSBMSCDevISR[0]->hLun, LBA, buffer, nSec);
        //printf ("Complete read data\n");
        for (i=0; i<nSec; i++)
        {
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 1, &buffer[512*i], 512);
        }
    }

#endif
}

/*******************************************************************************
** Name: WriteData_To_Flash
** Input:SG_DEVICE *pFsg, uint8* buf, uint32 size
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:д����0x2a
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static int WriteData_To_Flash(MSC_DEVICE *pMsc)
{
    uint32 LBA = pMsc->BulkXfer.LBA;
    uint16 needSec = pMsc->BulkXfer.NendSec;
    uint16 transSec = pMsc->BulkXfer.TransSec;
    int i = 0;

    LunDev_Write (gpstUSBMSCDevISR[0]->hLun,
                  LBA,
                  &gpstUSBMSCDevISR[0]->EpData->buf[512],
                  needSec);

    pMsc->BulkXfer.LBA = 0;
    pMsc->BulkXfer.NendSec = 0;
    pMsc->BulkXfer.TransSec = 0;
    gpstUSBMSCDevISR[0]->EpData->datablock = 0;

#if 0
    uint32 LBA = pMsc->BulkXfer.LBA;
    uint16 needSec = pMsc->BulkXfer.NendSec;
    uint16 transSec = pMsc->BulkXfer.TransSec;
    uint8  buffer[128*512] ={0};
    int i = 0;


    transSec = transSec + 1;
#if 0
    printf ("\n[WriteData_To_Flash]: LBA=0x%x needSec=0x%0x transSec=0x%x\n",
                                                                LBA,
                                                                needSec,
                                                                transSec);
#endif
    while (!(needSec == gpstUSBMSCDevISR[0]->EpData_Head.listlen));

    for (i=0; i<needSec; i++ )
    {
        memcpy(&buffer[i*512],
               gpstUSBMSCDevISR[0]->EpData_Head.next->buf,
               gpstUSBMSCDevISR[0]->EpData_Head.next->size);
        MscEpData_Delete();
    }
    //printf ("Write data  needSec=%d\n",needSec);
    LunDev_Write (gpstUSBMSCDevISR[0]->hLun, LBA, buffer, needSec);
    //printf ("Write Complete data  needSec=%d\n",needSec);
    pMsc->BulkXfer.LBA = 0;
    pMsc->BulkXfer.NendSec = 0;
    pMsc->BulkXfer.TransSec = 0;
    MscSendCSW(pMsc);
#endif
}

/*******************************************************************************
** Name: MscWrite10
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:д����0x2a
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscWrite10(MSC_DEVICE *pMsc)
{
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    uint16 nSec = 0;
    uint32 LBA  = 0;

    MscCSWHandler(pMsc, CASE12, (uint16)pMsc->cbw->dCBWDataTransLen);
    MscSenseData(SCSI_SENSE_NO_SENSE,0,0);

    nSec = __get_unaligned_be16((cmnd+7));
    LBA  = __get_unaligned_be32((cmnd+2));

    //�ȴ�Ҫд������ȫ���������
    //while (!(nSec < gpstUSBMSCDevISR[0]->EpData->datablock));
    gpstUSBMSCDevISR[0]->write_status = 1;
    pMsc->BulkXfer.NendSec = nSec;
    rkos_semaphore_take(gpstUSBMSCDevISR[0]->osUSBMSCWriteSem, MAX_DELAY);
    gpstUSBMSCDevISR[0]->write_status = 0;

    //printf ("\nwrm_Write10: LBA=0x%x nSec=0x%x\n",LBA, nSec);

    pMsc->BulkXfer.NendSec = nSec;
    pMsc->BulkXfer.LBA  = LBA;
    pMsc->BulkXfer.TransSec = 0;

}
/*******************************************************************************
** Name: MscVerify10
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:����:У��0x2f
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscVerify10(MSC_DEVICE *pMsc)
{
    if (pMsc->cbw->dCBWDataTransLen == 0)
    {
        MscCSWHandler(pMsc, CASE1,0);
        MscSenseData(SCSI_SENSE_NO_SENSE,0,0);
        //MscSendCSW(pMsc);
    }
    else
    {
        MscCSWHandler(pMsc, CASE12, (uint16)pMsc->cbw->dCBWDataTransLen);
        MscSenseData(SCSI_SENSE_NO_SENSE,0,0);
        //MscSendCSW(pMsc);
    }
}
/*******************************************************************************
** Name: MscModeSense10
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc:����:0x5a
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscModeSense10(MSC_DEVICE *pMsc)
{
    uint16 len;
    uint8* pSenseData;

    len = pMsc->cbw->dCBWDataTransLen; //modify by lxs @2007.03.13, for Mac OS 10.3.4
    MscCSWHandler(pMsc, CASEOK, len);

    pSenseData = (uint8*)SCSIDATA_Sense_FLASH;

    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, pMsc->BulkIn, pSenseData, len);
}
/*******************************************************************************
** Name:  MscReadFormatCapacities
** Input:pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ����:���ɸ�ʽ������0x23
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscReadFormatCapacities(MSC_DEVICE *pMsc)
{
    uint8 capacity[12];
    uint32 TotalSecs;
    uint16 BytsPerSec;
    uint32 pSize;

    if (1)
    {
        LunDev_GetSize(gpstUSBMSCDevISR[0]->hLun, &pSize);
        if (pSize)
        {
            TotalSecs = pSize;
        }

        BytsPerSec=512;
        //printf ("\nTotalSecs=%d\n",TotalSecs);
    }
    else
    {
        MscSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        MscCSWHandler(pMsc, CASECBW, 0);
        return;
    }

    MscCSWHandler(pMsc, CASE6,12);
    MscSenseData(SCSI_SENSE_NO_SENSE,0,0);
    capacity[0]=0;
    capacity[1]=0;
    capacity[2]=0;
    capacity[3]=8;
    capacity[4]=(uint8)((TotalSecs >> 24) & 0x00ff);
    capacity[5]=(uint8)((TotalSecs >> 16) & 0x00ff);
    capacity[6]=(uint8)((TotalSecs >> 8) & 0x00ff);
    capacity[7]=(uint8)(TotalSecs & 0x00ff);
    capacity[8]=0x01;  //0x02 ��ʾmedia
    capacity[9]=0x00;
    capacity[10]=(uint8)(BytsPerSec >> 8)&0xff;      //bytes/block
    capacity[11]=(uint8)(BytsPerSec & 0x00ff);

    //USBWriteEp(pFsg->BulkIn, 12, capacity);
    UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, pMsc->BulkIn, capacity, 12);
}

/*******************************************************************************
** Name: MscReqest
** Input:ep��event, param, pFsg
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: USBMSC SCSI����
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscReqest(uint8 ep, uint32 param, MSC_DEVICE *pMsc)
{
    uint8 *cmnd = &pMsc->cbw->CBWCDB[0];
    int i = 0;

    switch (cmnd[0])
    {
            //6-byte command
        case K_SCSICMD_TEST_UNIT_READY:             //0x00
            //printf ("usb:CBW_FsgScsiCmdHandle\n");
            MscTestUnitReady(pMsc);
            break;
        case K_SCSICMD_REQUEST_SENSE:               //0x03
            //printf ("usb:CBW_K_SCSICMD_REQUEST_SENSE\n");
            MscRequestSense(pMsc);
            break;
        case K_SCSICMD_INQUIRY:                     //0x12
            //printf ("usb:CBW_K_SCSICMD_INQUIRY\n");
            MscInquiry(pMsc);
            break;
        case K_SCSICMD_MODE_SENSE_06:               //0x1a
            //printf ("usb:CBW_K_SCSICMD_MODE_SENSE_06\n");
            MscModeSense06(pMsc);
            break;
        case K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL://0x1e
            //printf ("usb:CBW_K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL\n");
            MscPreventAllowMediumRemoval(pMsc);
            break;
        case K_SCSICMD_START_STOP_UNIT:             //0x1b
            //printf ("usb:CBW_K_SCSICMD_START_STOP_UNIT\n");
            MscStartStopUnit(pMsc);
            break;

        case K_SCSICMD_MODE_SELECT_06:              //0x15
        case K_SCSICMD_MODE_SELECT_10:              //0x55
            //printf ("usb:CBW_K_SCSICMD_MODE_SELECT_10\n");
            MscCSWHandler(pMsc, CASECMDFAIL, 0);
            break;
            //10-byte command
        case K_SCSICMD_READ_CAPACITY:               //0x25
            //printf ("usb:CBW_K_SCSICMD_READ_CAPACITY\n");
            MscReadCapacity(pMsc);
            break;
        case K_SCSICMD_READ_10:                     //0x28
            //printf ("\nK_SCSICMD_READ_10\n");
            MscRead10(pMsc);
            break;
        case K_SCSICMD_WRITE_10:                    //0x2a
            //printf ("\nusb:CBW_K_SCSICMD_WRITE_10\n");
            //DelayMs(1);
            MscWrite10(pMsc);
            break;
        case K_SCSICMD_VERIFY_10:                   //0x2f
            //printf ("usb:K_SCSICMD_VERIFY_10\n");
            MscVerify10(pMsc);
            break;
        case K_SCSICMD_MODE_SENSE_10:               //0x5a
            //printf ("usb:CBW_K_SCSICMD_MODE_SENSE_10\n");
            MscModeSense10(pMsc);
            break;
            //12-byte command
        case K_SCSICMD_READ_FORMAT_CAPACITIES:      //0x23
            //printf ("usb:CBW_K_SCSICMD_READ_FORMAT_CAPACITIES\n");
            MscReadFormatCapacities(pMsc);
            break;

        case K_RKCMD_FIRMWAVE_UPGRADE:              //0xff
            //printf ("usb:CBW_K_RKCMD_FIRMWAVE_UPGRADE\n");
            //MscFirmwareUpgrade(pMsc);
            break;

        case K_RKCMD_SONY_FWUPDATE:
            //printf ("usb:CBW_K_RKCMD_SONY_FWUPDATE\n");
            //MscFwCmdHandle(pMsc);
            break;

        default:
            //printf ("usb:CBW_default\n");
            gpstUSBMSCDevISR[0]->invalid_cbw = 1;
            MscCSWHandler(pMsc, CASECBW, 0);
            //MscSendCSW(pMsc);
            printf("error cbw = 0x%x\n", cmnd[0]);
            break;
    }
}


/*******************************************************************************
** Name: MscClassReq
** Input:ctrl��pDev
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: USB�������
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static void MscClassReq(USB_CTRL_REQUEST *ctrl)
{
    MSC_DEVICE *pMsc = &gpstUSBMSCDevISR[0]->mscdev;

    /* Handle Bulk-only class-specific requests */
    if (ctrl->bRequest == USB_BULK_GET_MAX_LUN_REQUEST)     //0xfe:GET MAX LUN
    {
        printf("\nGET LUN = %d\n", pMsc->FileDisk.LunNum);
        UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 0, &pMsc->FileDisk.LunNum, 1);
        return;
    }
    else if (ctrl->bRequest == USB_BULK_RESET_REQUEST)      //0xff:Bulk Reset
    {
        printf("BULK RESET\n");
        if (0 == ctrl->wLength)
        {
            UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 0, NULL, 0);
        }
        return;
    }
}

/*******************************************************************************
** Name: MscSetup
** Input:ctrl��pDev
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
** Desc: ��������ṹ��
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN static int32 MscSetup(USB_CTRL_REQUEST *ctrl)
{
    uint8 type = ctrl->bRequestType & USB_TYPE_MASK;
    int32 ret = 0;
    uint8 *buffer = NULL;
    if (type == USB_TYPE_CLASS)
    {
        MscClassReq(ctrl);
    }
    else
    {
        switch (type)
        {
            case USB_TYPE_STANDARD:
                printf ("Usb request is USB_TYPE_STANDARD\n");
                break;
            case USB_TYPE_MASK:
                printf ("Usb request is USB_TYPE_MASK\n");
                break;
            case USB_TYPE_VENDOR:
                printf ("Usb request is USB_TYPE_VENDOR WRM\n");
                UsbOtgDev_Write(gpstUSBMSCDevISR[0]->hOtg, 0, NULL, 0);
                break;
            default:
                printf ("Usb request is USB_TYPE_RESERVED\n");
                break;
        }
        ret = -1;
    }

    return ret;
}



/*******************************************************************************
** Name: USBMSCDevIntIsr1
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN void USBMSCDevIntIsr1(void)
{
    //Call total int service...
    USBMSCDevIntIsr(1);

}
/*******************************************************************************
** Name: USBMSCDevIntIsr0
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN void USBMSCDevIntIsr0(void)
{
    //Call total int service...
    USBMSCDevIntIsr(0);

}
/*******************************************************************************
** Name: USBMSCDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN void USBMSCDevIntIsr(uint32 DevID)
{
    uint32 USBMSCDevIntType;


    //Get USBMSCDev Int type...
    //USBMSCDevIntType = GetIntType();
    if (gpstUSBMSCDevISR[DevID] != NULL)
    {
        //if (USBMSCDevIntType & INT_TYPE_MAP)
        {
            //write serice code...
        }

        //wirte other int service...
    }

}
/*******************************************************************************
** Name: USBMSCDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN rk_err_t USBMSCDevResume(HDC dev)
{
    //USBMSCDev Resume...
    USBMSC_DEVICE_CLASS * pstUSBMSCDev =  (USBMSC_DEVICE_CLASS *)dev;
    if (pstUSBMSCDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: USBMSCDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_COMMON_
COMMON FUN rk_err_t USBMSCDevSuspend(HDC dev)
{
    //USBMSCDev Suspend...
    USBMSC_DEVICE_CLASS * pstUSBMSCDev =  (USBMSC_DEVICE_CLASS *)dev;
    if (pstUSBMSCDev == NULL)
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
** Name: USBMSCDev_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT API rk_err_t USBMSCDev_Task_DeInit(void *pvParameters)
{

}
/*******************************************************************************
** Name: USBMSCDev_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT API rk_err_t USBMSCDev_Task_Init(void *pvParameters)
{
    //HDC hUsbOtgDev;
    //HDC hLun;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;
    uint32 pLun;
    MSC_DEVICE *pMsc = &gpstUSBMSCDevISR[0]->mscdev;


    //printf ("Entry USBMSCDev_Task_Init_1\n");

    //Open UsbOtgDev... for Set Descriptor
#if 0
    hUsbOtgDev = RKDev_Open(DEV_CLASS_USBOTG, 0, NOT_CARE);
    if ((hUsbOtgDev == NULL) || (hUsbOtgDev == (HDC)RK_ERROR) || (hUsbOtgDev == (HDC)RK_PARA_ERR))
    {
        printf("UsbOtgDev open failure for Init USBMSC\n");
        return RK_ERROR;
    }
#endif
    ret = UsbOtgDev_SetDes(gpstUSBMSCDevISR[0]->hOtg,&usbMscDes);
    if (ret == RK_ERROR)
    {
        printf ("Usb Otg Set Descriptor failure\n");
    }
	printf ("Usb Otg Set Descriptor OK\n");

    //Open LunDevice... for Get Lun
#if 0
    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);
    if (hLun == NULL)
    {
        rk_print_string("Lun device open failure");
        return RK_SUCCESS;
    }
#endif
    //LunDev_GetSize(hLun, &pLun);
    pLun = 0;
    pMsc->FileDisk.LunNum = pLun;

    //save otg and lun handler
    //gpstUSBMSCDevISR[0]->hOtg = hUsbOtgDev;
    //gpstUSBMSCDevISR[0]->hLun = hLun;

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: USBMSCDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT API rk_err_t USBMSCDev_Delete(uint32 DevID, void * arg)
{
#if 1
    DevID = 0;
    USBMSC_DEV_ARG* pstUSBMSCDevArg = (USBMSC_DEV_ARG*)arg;
    pstUSBMSCDevArg->hLun = gpstUSBMSCDevISR[DevID]->hLun;
    pstUSBMSCDevArg->hUsbOtgDev = gpstUSBMSCDevISR[DevID]->hOtg;

    //Check USBMSCDev is not exist...
    printf ("\nUSB MSC Delete \n");
    if (gpstUSBMSCDevISR[DevID] == NULL)
    {
        printf ("Delete error\n");
        return RK_ERROR;
    }
    //USBMSCDev deinit...
    USBMSCDevDeInit(gpstUSBMSCDevISR[DevID]);
    //Free USBMSCDev memory...
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCOperReqSem);
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCOperSem);
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCReadReqSem);
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCReadSem);
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCWriteReqSem);
    rkos_semaphore_delete(gpstUSBMSCDevISR[DevID]->osUSBMSCWriteSem);
    rkos_memory_free(gpstUSBMSCDevISR[DevID]);
    //Delete USBMSCDev...
    gpstUSBMSCDevISR[DevID] = NULL;
    //Delete USBMSCDev Read and Write Module...
#ifdef __SYSTEM_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveModule(MODULE_ID_USBMSC_DEV);
#endif

#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: USBMSCDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT API HDC USBMSCDev_Create(uint32 DevID, void * arg)
{
    USBMSC_DEV_ARG * pstUSBMSCDevArg;
    DEVICE_CLASS* pstDev;
    USBMSC_DEVICE_CLASS * pstUSBMSCDev;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    // Create handler...
    pstUSBMSCDevArg = (USBMSC_DEV_ARG *)arg;
    pstUSBMSCDev =  rkos_memory_malloc(sizeof(USBMSC_DEVICE_CLASS));
    memset(pstUSBMSCDev, 0, sizeof(USBMSC_DEVICE_CLASS));
    if (pstUSBMSCDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstUSBMSCDev->osUSBMSCOperReqSem  = rkos_semaphore_create(1,1);
    pstUSBMSCDev->osUSBMSCOperSem  = rkos_semaphore_create(1,0);

    pstUSBMSCDev->osUSBMSCReadReqSem  = rkos_semaphore_create(1,1);
    pstUSBMSCDev->osUSBMSCReadSem  = rkos_semaphore_create(1,0);

    pstUSBMSCDev->osUSBMSCWriteReqSem  = rkos_semaphore_create(1,1);
    pstUSBMSCDev->osUSBMSCWriteSem  = rkos_semaphore_create(1,0);
    if (((pstUSBMSCDev->osUSBMSCOperReqSem) == 0)
            || ((pstUSBMSCDev->osUSBMSCOperSem) == 0)
            || ((pstUSBMSCDev->osUSBMSCReadReqSem) == 0)
            || ((pstUSBMSCDev->osUSBMSCReadSem) == 0)
            || ((pstUSBMSCDev->osUSBMSCWriteReqSem) == 0)
            || ((pstUSBMSCDev->osUSBMSCWriteSem) == 0))
    {
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCOperReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCOperSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCReadReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCReadSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCWriteReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCWriteSem);
        rkos_memory_free(pstUSBMSCDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstUSBMSCDev;
    pstDev->suspend = USBMSCDevSuspend;
    pstDev->resume  = USBMSCDevResume;
    //init arg...
    //pstUSBMSCDev->usbmode = pstUSBMSCDevArg->usbmode;

#ifdef __SYSTEM_FWANALYSIS_FWANALYSIS_C__
    FW_ModuleOverlay(MODULE_ID_USBMSC_DEV, MODULE_OVERLAY_CODE);
#endif
    //device init...
    gpstUSBMSCDevISR[DevID] = NULL;
    pstUSBMSCDev->hOtg = pstUSBMSCDevArg->hUsbOtgDev;
    pstUSBMSCDev->hLun = pstUSBMSCDevArg->hLun;

    gpstUSBMSCDevISR[DevID] = pstUSBMSCDev;
    if (USBMSCDevInit(pstUSBMSCDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCOperReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCOperSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCReadReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCReadSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCWriteReqSem);
        rkos_semaphore_delete(pstUSBMSCDev->osUSBMSCWriteSem);
        rkos_memory_free(pstUSBMSCDev);
        gpstUSBMSCDevISR[DevID] = NULL;
        return (HDC) RK_ERROR;
    }
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
** Name: USBMSCDevDeInit
** Input:USBMSC_DEVICE_CLASS * pstUSBMSCDev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT FUN rk_err_t USBMSCDevDeInit(USBMSC_DEVICE_CLASS * pstUSBMSCDev)
{
    UsbOtgDev_DisConnect();
    //RKDev_Close(pstUSBMSCDev->hLun);
    //RKDev_Close(pstUSBMSCDev->hOtg);
    rkos_memory_free(pstUSBMSCDev->EpData);
    //RKTaskDelete (TASK_ID_USBMSC, 0, SYNC_MODE);

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: USBMSCDevInit
** Input:USBMSC_DEVICE_CLASS * pstUSBMSCDev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_INIT_
INIT FUN rk_err_t USBMSCDevInit(USBMSC_DEVICE_CLASS * pstUSBMSCDev)
{
    MSC_DEVICE *pMsc = &pstUSBMSCDev->mscdev;
    memset(pMsc, 0, sizeof(MSC_DEVICE));
    pMsc->connected = 1;

    pstUSBMSCDev->verify_params.number = VERIFY_NUMBER;
    //pstUSBMSCDev->EpData_Head.listlen = 0;
    pstUSBMSCDev->mutux_status = 0;
    pstUSBMSCDev->write_status = 0;
    pstUSBMSCDev->invalid_cbw  = 0;

    pMsc->BulkOut = BULK_OUT_EP;
    pMsc->BulkIn = BULK_IN_EP;
    pMsc->BulkXfer.TransSec = 0;

    /**Device Part**/
    usbMscDes.pDevDes = &HSDeviceDescr;
    usbMscDes.pConfigDes = (USB_CONFIGURATION_DESCRIPTOR*)&allDescriptor;
    usbMscDes.pInterfaceDes = &HSDeviceInterface;
    usbMscDes.pEnDes[0] = &HSDeviceEnDes_IN;
    usbMscDes.pEnDes[1] = &HSDeviceEnDes_OUT;
    usbMscDes.pEnDes[2] = NULL;
    usbMscDes.pOtherConfigDes = NULL;
    usbMscDes.pHsQualifier = &HS_Device_Qualifier;
    usbMscDes.pUsbPowerDes = NULL;
    usbMscDes.pUsbHubDes = NULL;
    usbMscDes.pUsbLangId = (USB_STRING_DESCRIPTOR*)USBLangId;
    usbMscDes.pDevString = (USB_STRING_DESCRIPTOR*)StringProduct;
    usbMscDes.pMString = (USB_STRING_DESCRIPTOR*)StringManufacture;
    usbMscDes.pSerialNum = (USB_STRING_DESCRIPTOR*)StringSerialNumbel;

    usbMscDes.pConfigString = (USB_STRING_DESCRIPTOR*)StringConfig;
    usbMscDes.pInterfaceString= (USB_STRING_DESCRIPTOR*)StringInterface;
    usbMscDes.pfReadEp = MscScsiCmdHandle;
    usbMscDes.ConfigLen = sizeof(USB_FSG_CONFIGS_DESCRIPTOR);

    /**Host Part**/
    usbMscDes.pMscHost = MscHost;

    RKTaskCreate(TASK_ID_USBMSC, 0, NULL);

    return RK_SUCCESS;

}


#ifdef _USBMSC_DEV_SHELL_
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
static char* ShellUSBMSCName[] =
{
    "pcb",
    "create",
    "delete",
    "test",
    "test_r",
    "test_w",
    "help",
    "\b",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: USBMSCDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL API rk_err_t USBMSCDev_Shell(HDC dev, uint8 * pstr)
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

    ret = ShellCheckCmd(ShellUSBMSCName, pItem, StrCnt);
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
            ret = USBMSCDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = USBMSCDevShellMc(dev,pItem);
            break;

        case 0x02:
            ret = USBMSCDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = USBMSCDevShellTest(dev,pItem);
            break;
        case 0x04:
            ret = USBMSCDevShellTestRead(dev,pItem);
            break;
        case 0x05:
            ret = USBMSCDevShellTestWrite(dev,pItem);
            break;
        case 0x06:
            ret = USBMSCDevShellHelp(dev,pItem);
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
** Name: USBMSCDevShellHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:24:13
*******************************************************************************/
SHELL FUN rk_err_t USBMSCDevShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if ( StrLenA((uint8 *) pstr) != 0)
        return RK_ERROR;

    printf("             UsbMsc����ṩ��һϵ�е������UsbMsc���в���\r\n");
    printf("UsbMsc����������������:           \r\n");
    printf("help      ��ʾhelp��Ϣ           \r\n");
    printf("mc        ��UsbMsc              \r\n");
    printf("del       ɾ��UsbMsc              \r\n");
    printf("test      ����UsbMsc����          \r\n");
    printf("test_r    ����UsbMsc Host Read����          \r\n");
    printf("test_w    ����UsbMsc Host Write����          \r\n");
    printf("pcb       ��ʾpcb��Ϣ  \r\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: USBMSCDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hUSBMSCDev;
    uint32 DevID;

    //Get USBMSCDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }
    //FREQ_EnterModule(FREQ_USB);

    //Open USBMSCDev...
    hUSBMSCDev = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
    if ((hUSBMSCDev == NULL) || (hUSBMSCDev == (HDC)RK_ERROR) || (hUSBMSCDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("USBMSCDev open failure");
        return RK_SUCCESS;
    }
    //do test....
    UsbMscDev_Connect(hUSBMSCDev, 0);
    //close USBMSCDev...
    RKDev_Close(hUSBMSCDev);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: USBMSCDevShellTestRead
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:24:13
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellTestRead(HDC dev,  uint8 * pstr)
{
    HDC hUSBMSC;
    uint8  lun = 0;
    uint32 LBA = 0x0;
    uint32 blks = 5;
    uint8  *pBuf;
    int32  ret = 0;
    int i = 0;


    hUSBMSC = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
    if (hUSBMSC == NULL)
    {
        printf ("Test Usb Host Read failure\n");
        return RK_ERROR;
    }

    pBuf = rkos_memory_malloc(512*blks);
    if (pBuf == NULL)
    {
        printf ("Rkos Alloc memory failure\n");
        return -1;
    }
	memset(pBuf, 0, 512*blks);
    printf ("\nRead lba:%d blks:%d\n",LBA,blks);
    //for (i=0;i<20000;i++)
    //{
        ret = USBMSCHost_Read (hUSBMSC, LBA, (void*)pBuf, blks);
        if (ret < 0)
        {
            printf ("Msc Host Read Data Failure\n");
            //break;
        }
    //}
    printf ("\nRead Data Complete:%d\n",ret);
    printf ("\n----------------------\n");
    printf ("pBuf[0]=%02x\n",pBuf[0]);
    printf ("pBuf[1]=%02x\n",pBuf[1]);
    printf ("pBuf[2]=%02x\n",pBuf[2]);
    printf ("pBuf[3]=%02x\n",pBuf[3]);
    printf ("pBuf[4]=%02x\n",pBuf[4]);
    printf ("pBuf[507]=%02x\n",pBuf[507]);
    printf ("pBuf[508]=%02x\n",pBuf[508]);
    printf ("pBuf[509]=%02x\n",pBuf[509]);
    printf ("pBuf[510]=%02x\n",pBuf[510]);
    printf ("pBuf[511]=%02x\n",pBuf[511]);
	debug_hex((char * )pBuf, 512*blks, 16);
#if 0
    while (1)
    {
    pBuf[0]= 0;
    pBuf[1]= 0;
    pBuf[2]= 0;
    pBuf[3]= 0;
    pBuf[4]= 0;
    pBuf[507]= 0;
    pBuf[508]= 0;
    pBuf[509]= 0;
    pBuf[510]= 0;
    pBuf[511]= 0;

    ret = USBMSCHost_Read (hUSBMSC, lun, LBA, blks, (void*)pBuf);
    printf ("Msc Host Read Data ret=%d\n",ret);
    if (ret < 0)
    {
        printf ("Msc Host Read Data Failure\n");
        //break;
    }

    printf ("\n----------------------\n");
    printf ("pBuf[0]=%d\n",pBuf[0]);
    printf ("pBuf[1]=%d\n",pBuf[1]);
    printf ("pBuf[2]=%d\n",pBuf[2]);
    printf ("pBuf[3]=%d\n",pBuf[3]);
    printf ("pBuf[4]=%d\n",pBuf[4]);
    printf ("pBuf[507]=%d\n",pBuf[507]);
    printf ("pBuf[508]=%d\n",pBuf[508]);
    printf ("pBuf[509]=%d\n",pBuf[509]);
    printf ("pBuf[510]=%d\n",pBuf[510]);
    printf ("pBuf[511]=%d\n",pBuf[511]);
    }
#endif
    rkos_memory_free(pBuf);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: USBMSCDevShellTestWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:24:13
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellTestWrite(HDC dev,  uint8 * pstr)
{
    HDC hUSBMSC;
    uint8  lun = 0;
    uint32 start = 0x0;
    uint32 blks = 3;
    uint8  *pBuf;
    uint32 ret;


    hUSBMSC = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
    if (hUSBMSC == NULL)
    {
        printf ("Test Usb Host Read failure\n");
        return RK_ERROR;
    }

    pBuf = rkos_memory_malloc(1024);
    pBuf[0] = 100;
    pBuf[1] = 101;
    pBuf[2] = 102;

    pBuf[509] = 1;
    pBuf[510] = 2;
    pBuf[511] = 3;

    pBuf[512] = 103;
    pBuf[513] = 104;
    pBuf[514] = 105;

    pBuf[1021] = 4;
    pBuf[1022] = 5;
    pBuf[1023] = 6;


    ret = USBMSCHost_Write (hUSBMSC, start, (void*)pBuf, blks);
    if (ret < 0)
    {
        printf ("Msc Host Write Test Failure\n");
    }
    printf ("Msc Host Write Test Successful\n");

    rkos_memory_free(pBuf);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: USBMSCDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    USBMSC_DEV_ARG pstUSBMSCDevArg;

    //Get USBMSCDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    if (RKDev_Delete(DEV_CLASS_USBMSC, DevID, (void*)&pstUSBMSCDevArg) != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev delete failure");
    }
    RKDev_Close(pstUSBMSCDevArg.hLun);
    RKDev_Close(pstUSBMSCDevArg.hUsbOtgDev);
    RKTaskDelete (TASK_ID_USBMSC, 0, SYNC_MODE);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: USBMSCDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellMc(HDC dev, uint8 * pstr)
{
    USBMSC_DEV_ARG stUSBMSCDevArg;
    rk_err_t ret;
    uint32 DevID;
    HDC hUSBMSCDev;
    HDC hUsbOtgDev;
    HDC hLun;

    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    hUsbOtgDev = RKDev_Open(DEV_CLASS_USBOTG, 0, NOT_CARE);
    if ((hUsbOtgDev == NULL) || (hUsbOtgDev == (HDC)RK_ERROR) || (hUsbOtgDev == (HDC)RK_PARA_ERR))
    {
        printf("UsbOtgDev open failure for Init USBMSC\n");
        return RK_ERROR;
    }

    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);
    if (hLun == NULL)
    {
        rk_print_string("Lun device open failure");
		
        //return RK_SUCCESS;
    }

    //Init USBMSCDev arg...
    stUSBMSCDevArg.hUsbOtgDev = hUsbOtgDev;
    stUSBMSCDevArg.hLun = hLun;


    //Create USBMSCDev...
    ret = RKDev_Create(DEV_CLASS_USBMSC, DevID, &stUSBMSCDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev create failure");
    }

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: USBMSCDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.25
** Time: 10:29:11
*******************************************************************************/
_DRIVER_USBMSC_USBMSCDEVICE_SHELL_
SHELL FUN rk_err_t USBMSCDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hUSBMSCDev;
    uint32 DevID;

    //Get USBMSCDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if (gpstUSBMSCDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("USBMSCDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("    &stUSBMSCDevice---0x%08x", &gpstUSBMSCDevISR[DevID]->stUSBMSCDevice);
        rk_printf_no_time("    osUSBMSCOperReqSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCOperReqSem);
        rk_printf_no_time("    osUSBMSCOperSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCOperSem);
        rk_printf_no_time("    osUSBMSCReadReqSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCReadReqSem);
        rk_printf_no_time("    osUSBMSCReadSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCReadSem);
        rk_printf_no_time("    osUSBMSCWriteReqSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCWriteReqSem);
        rk_printf_no_time("    osUSBMSCWriteSem---0x%08x", gpstUSBMSCDevISR[DevID]->osUSBMSCWriteSem);
        //rk_printf_no_time("    usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("    usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("USBMSCDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}


#endif

#endif
