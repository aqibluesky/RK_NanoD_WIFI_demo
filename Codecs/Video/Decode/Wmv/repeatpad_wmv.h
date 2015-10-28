//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) 1996 -- 1998  Microsoft Corporation

Module Name:

	repeatpad_wmv.h

Abstract:


Author:


Revision History:

*************************************************************************/

/////////// main external API's /////////////////

#define RepeatRef0YArgs 	U8_WMV* ppxlcRef0Y,     \
                        	CoordI  iStart,         \
                            CoordI  iEnd,           \
                        	I32_WMV     iOldLeftOffet,  \
                        	Bool_WMV    fTop,           \
                            Bool_WMV    fBottom,        \
                            I32_WMV     iWidthY,        \
                            I32_WMV     iWidthYPlusExp, \
                            I32_WMV     iWidthPrevY,  \
                            Bool_WMV    bProgressive


#define RepeatRef0UVArgs	U8_WMV* ppxlcRef0U,     \
                        	U8_WMV* ppxlcRef0V,     \
   	                        CoordI  iStart,         \
                            CoordI  iEnd,           \
	                        I32_WMV     iOldLeftOffet,  \
	                        Bool_WMV    fTop,           \
                            Bool_WMV    fBottom,        \
                            I32_WMV     iWidthUV,       \
                            I32_WMV     iWidthUVPlusExp,\
                            I32_WMV     iWidthPrevUV,   \
                            Bool_WMV    bProgressive


extern Void_WMV (*g_pRepeatRef0UV) (RepeatRef0UVArgs);
extern Void_WMV (*g_pRepeatRef0Y) (RepeatRef0YArgs);

///////////////////////////////////////////////////////


extern  Void_WMV g_RepeatRef0Y     (RepeatRef0YArgs);
extern  Void_WMV g_RepeatRef0UV     (RepeatRef0UVArgs);
extern  Void_WMV g_RepeatRef0Y_AP  (RepeatRef0YArgs);
extern  Void_WMV g_RepeatRef0UV_AP     (RepeatRef0UVArgs);

Void_WMV g_RepeatRef0Y_MMX (RepeatRef0YArgs);
Void_WMV g_RepeatRef0UV_MMX (RepeatRef0UVArgs);
Void_WMV g_InitRepeatRefInfo (Bool_WMV bAdvancedProfile);
Void_WMV g_RepeatRef0Y_AP  (RepeatRef0YArgs);
Void_WMV g_RepeatRef0UV_AP (RepeatRef0UVArgs);