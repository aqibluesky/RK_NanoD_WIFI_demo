/******************************************************************************
*
*                  Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
*  File name :     rk_eq.h
*  Description:
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     06/12/30      1.0
*           HuWeiGuo     07/09/27      2.0          �޸��˽ӿں���
*
*******************************************************************************/

#ifndef __EQ_H__
#define __EQ_H__

rk_err_t rk_eq_init();
rk_err_t rk_eq_deinit();
void RockEQAdjust(long SmpRate, short *g, short db, RKEffect *userEQ); // db ΪԤ��˥����db ��, 0: ˥��0dB; 1:˥��6dB; 2:˥��12dB;
void RockEQProcess(short *pData, long PcmLen, RKEffect *userEQ);
#endif
