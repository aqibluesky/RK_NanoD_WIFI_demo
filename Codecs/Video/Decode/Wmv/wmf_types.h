//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 2000

Module Name:

    wmfencode.h

Abstract:

    API's for light WMF SDK Encoder.

Revision History:


*************************************************************************/

#ifndef _WMF_TYPES_H_
#define _WMF_TYPES_H_
//#pragma pack (1)

/*********  Basic type Definitions  *************/

#ifndef WMF_TYPE_DEFINED
#define WMF_TYPE_DEFINED

typedef void Void_WMF;
typedef long I32_WMF;
typedef unsigned long U32_WMF;
typedef short I16_WMF;
typedef unsigned short U16_WMF;
#if defined(_Embedded_x86)
typedef unsigned short U16Char_WMF;
#elif defined(macintosh)
typedef wchar_t U16Char_WMF;
#else
typedef unsigned short U16Char_WMF;
#endif
typedef char I8_WMF;
typedef unsigned char U8_WMF;
typedef long Bool_WMF;
typedef double Double_WMF;
typedef float Float_WMF;
typedef    struct tQWORD_WMF
{
    U32_WMF   dwLo;
    U32_WMF   dwHi;

}   QWORD_WMF;

#ifdef macintosh
typedef unsigned long long QWORD;
#endif

#ifdef _XBOX
typedef unsigned __int64    U64_WMF;
typedef __int64    I64_WMF;
#elif defined(macintosh) || defined(_Embedded_x86)
typedef unsigned long long  U64_WMF;
typedef long long  I64_WMF;
#elif defined(HITACHI)
typedef    struct tU64_WMF
{
    U32_WMF   dwLo;
    U32_WMF   dwHi;
}   U64_WMF;
typedef    struct tI64_WMF
{
    I32_WMF   dwLo;
    I32_WMF   dwHi;
}   I64_WMF;
#else
typedef unsigned __int64    U64_WMF;
typedef __int64    I64_WMF;
#endif

typedef    struct tRect_WMF
{
    I32_WMF iLeft;
    I32_WMF iTop;
    I32_WMF iRight;
    I32_WMF iBottom;
}   Rect_WMF;

#define TRUE_WMF    1
#define FALSE_WMF   0
#define NULL_WMF   0
typedef void * HWMFFILE;

#endif // WMF_TYPE_DEFINED


/************************************************/


// GUID definiton

#ifndef MAKEFOURCC_WMF
#define MAKEFOURCC_WMF(ch0, ch1, ch2, ch3) \
        ((U32_WMF)(U8_WMF)(ch0) | ((U32_WMF)(U8_WMF)(ch1) << 8) |   \
        ((U32_WMF)(U8_WMF)(ch2) << 16) | ((U32_WMF)(U8_WMF)(ch3) << 24 ))

#define mmioFOURCC_WMF(ch0, ch1, ch2, ch3)  MAKEFOURCC_WMF(ch0, ch1, ch2, ch3)
#endif


/******* video output type guids, in preference order  *****/
#define FOURCC_WMVP     mmioFOURCC_WMF('W','M','V','P')
#define FOURCC_WMV3     mmioFOURCC_WMF('W','M','V','3')
#define FOURCC_WMVA     mmioFOURCC_WMF('W','M','V','A')
#define FOURCC_WMV2     mmioFOURCC_WMF('W','M','V','2')
#define FOURCC_WMV1     mmioFOURCC_WMF('W','M','V','1')
#define FOURCC_M4S2     mmioFOURCC_WMF('M','4','S','2')
#define FOURCC_MP43     mmioFOURCC_WMF('M','P','4','3')
#define FOURCC_mp43     mmioFOURCC_WMF('m','m','4','3')
#define FOURCC_MP4S     mmioFOURCC_WMF('M','P','4','S')
#define FOURCC_mp4s     mmioFOURCC_WMF('m','p','4','s')
#define FOURCC_MP42     mmioFOURCC_WMF('M','P','4','2')
#define FOURCC_mp42     mmioFOURCC_WMF('m','m','4','2')
#define FOURCC_MSS1     mmioFOURCC_WMF('M','S','S','1')
#define FOURCC_MSS2     mmioFOURCC_WMF('M','S','S','2')

/***********************************************************/


/******* video intput type guids, in preference order  *****/

#define FOURCC_I420             0x30323449
#define FOURCC_IYUV             0x56555949
#define FOURCC_YV12             0x32315659
#define FOURCC_YUY2             0x32595559
#define FOURCC_UYVY             0x59565955
#define FOURCC_YVYU             0x55595659
#define FOURCC_YVU9             0x39555659
#define FOURCC_BI_RGB           0x00000000
#define FOURCC_BI_BITFIELDS     0x00000003
#define FOURCC_BI_RLE8          0x00000001
#define FOURCC_BI_RLE4          0x00000002

/***********************************************************/

/******* Audio format tags  *****/

#define WAVE_FORMAT_PCM_WMF     0x0001
#define WAVE_FORMAT_WMA1_WMF    0x0160
#define WAVE_FORMAT_WMA2_WMF    0x0161
#define WAVE_FORMAT_WMA3_WMF    0x0162

/***********************************************************/


typedef    struct tagWMFContentDescription
{
    /* *_len: as [in], they specify how large the corresponding
     *        buffers below are.
     *        as [out], they specify how large the returned
     *        buffers actually are.
     */

    U16_WMF uiTitle_len;
    U16_WMF uiAuthor_len;
    U16_WMF uiCopyright_len;
    U16_WMF uiDescription_len;   /* rarely used */
    U16_WMF uiRating_len;        /* rarely used */

    /* these are two-byte strings
     *   for ASCII-type character set, to convert these
     *   into a single-byte characters, simply skip
     *   over every other bytes.
     */

    U16Char_WMF *pchTitle;
    U16Char_WMF *pchAuthor;
    U16Char_WMF *pchCopyright;
    U16Char_WMF *pchDescription;
    U16Char_WMF *pchRating;

} WMFContentDescription;

/******************************************************************
**   Extended content description types:
**   ECD_STRING -- wchar string
**   ECD_BINARY -- binary (byte) data
**   ECD_BOOL   -- BOOL (int) data
**   ECD_WORD  -- contains one word,
**   ECD_DWORD  -- contains one dword,
**   ECD_QWORD  -- contains one qword,
*******************************************************************/

enum {
    ECD_STRING = 0,
    ECD_BINARY = 1,
    ECD_BOOL = 2,
    ECD_DWORD = 3,
    ECD_QWORD = 4,
    ECD_WORD = 5
};

//#pragma pack (1)
typedef    struct _CommandEntry {
    U32_WMF         time;
    U16_WMF         type;
    U16Char_WMF     *param;
} CommandEntry;


typedef    struct _ECD_DESCRIPTOR {
    U16_WMF         cbName;
    U16Char_WMF     *pwszName;
    U16_WMF         data_type;
    U16_WMF         cbValue;
    union {
        U16Char_WMF *pwszString;
        U8_WMF *pbBinary;
        Bool_WMF *pfBool;
        U32_WMF *pdwDword;
        U64_WMF *pqwQword;
        U16_WMF  *pwWord;
    } uValue;
} ECD_DESCRIPTOR;

typedef    struct _MarkerEntry {
    U64_WMF     m_qOffset;
    U64_WMF     m_qtime;
    U16_WMF     m_wEntryLen;
    U32_WMF     m_dwSendTime;
    U32_WMF     m_dwFlags;
    U32_WMF     m_dwDescLen;
    U16Char_WMF *m_pwDescName;
} MarkerEntry;

typedef    struct tIndexEntries
{
    U32_WMF dwPacket;
    U16_WMF wSpan;
} INDEXENTRIES;


typedef    struct tIndexInfo
{
    U16_WMF nStreamId;
    U32_WMF   time_deltaMs;
    U32_WMF   max_packets;
    U32_WMF   num_entries;
    INDEXENTRIES *pIndexEntries;
} INDEXINFO;



//#pragma pack ()


#endif // _WMF_TYPES_H_
