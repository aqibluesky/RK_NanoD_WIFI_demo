/*
********************************************************************************************
*
*				  Copyright (c): 2014 - 2014 + 5, zhuzhe
*							   All rights reserved.
*
* FileName: Web\Task\WebTask.h
* Owner: zhuzhe
* Date: 2014.6.16
* Time: 15:15:58
* Desc:
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    zhuzhe     2014.6.16     15:15:58   1.0
********************************************************************************************
*/

#ifndef __WEB_TASK_WEBTASK_H__
#define __WEB_TASK_WEBTASK_H__

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
extern void Web_NetifRx(struct netif *netif, uint8 * buf, int len);
extern  struct netif * WebGetNetif(void);
extern xQueueHandle WebGetQueue(void);
extern void WebDNS(void);
extern rk_err_t WebTask_Shell(HDC dev, uint8 * pstr);
extern void WebTask_Enter(void);
extern rk_err_t WebTask_DeInit(void *pvParameters);
extern rk_err_t WebTask_Init(void *pvParameters);

#endif

