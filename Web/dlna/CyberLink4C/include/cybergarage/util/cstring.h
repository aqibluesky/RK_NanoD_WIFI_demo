/******************************************************************
*
* CyberUtil for C
*
* Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
* File: cstring.h
*
* Revision:
*
* 01/25/05
*  - first revision
* 03/01/06 Theo Beisch
*  - introduced LONGLONG macro
*  - added cg_strtrimwhite() to trim all
*    leading and trailing non-printable chars
*
******************************************************************/

#ifndef _CG_UTIL_CSTRING_H_
#define _CG_UTIL_CSTRING_H_

#include <stddef.h>
#include <cybergarage/typedef.h>
#include <stdlib.h>

#include <stddef.h>

typedef long  ssize_t;

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
* Define
****************************************/

#define CG_STRING_MEMORY_ALLOCATION_UNIT 64

/* UINT_MAX : 4294967295U */
#define CG_STRING_INTEGER_BUFLEN 16

 /* ULONG_MAX : 4294967295UL */
#define CG_STRING_LONG_BUFLEN 32

 /*  ULLONG_MAX : 18446744073709551615ULL */
#define CG_STRING_LONGLONG_BUFLEN 32

#define CG_STRING_FLOAT_BUFLEN 64
#define CG_STRING_DOUBLE_BUFLEN 64

/****************************************
* Function
****************************************/

char *cg_strdup(const char *str);
size_t cg_strlen(const char *str);
char *cg_strcpy(char *dest, const char *src);
char *cg_strcat(char *dest, const char *src);
int cg_strcmp(const char *str1, const char *str2);
int cg_strncmp(const char *str1, const char *str2, int nchars);
int cg_strcasecmp(const char *str1, const char *str2);
BOOL cg_streq(const char *str1, const char *str2);
BOOL cg_strcaseeq(const char *str1, const char *str2);
ssize_t cg_strchr(const char *str, const char *chars, size_t nchars);
ssize_t cg_strrchr(const char *str, const char *chars, size_t nchars);
ssize_t cg_strstr(const char *haystack, const char *needle);
char *cg_strtrimwhite(char *str);
char *cg_strtrim(char *str, char *delim, size_t ndelim);
char *cg_strltrim(char *str, char *delim, size_t ndelim);
char *cg_strrtrim(char *str, char *delim, size_t ndelim);
char *cg_strncpy(char *str1, const char *str2, size_t cnt);
char *cg_strncat(char *str1, const char *str2, size_t cnt);

const char *cg_int2str(int value, char *buf, size_t bufSize);
const char *cg_long2str(long value, char *buf, size_t bufSize);
const char *cg_float2str(float value, char *buf, size_t bufSize);
const char *cg_double2str(double value, char *buf, size_t bufSize);
const char *cg_sizet2str(size_t value, char *buf, size_t bufSize);
const char *cg_ssizet2str(ssize_t value, char *buf, size_t bufSize);

#define cg_str2int(value) (value ? atoi(value) : 0)
#define cg_str2long(value) (value ? atol(value) : 0)
#define cg_strhex2long(value) (value ? strtol(value, NULL, 16) : 0)
#define cg_strhex2ulong(value) (value ? strtoul(value, NULL, 16) : 0)
#define cg_str2float(value) ((float)(value ? atof(value) : 0.0))
#define cg_str2double(value) (value ? atof(value) : 0.0)
#define cg_str2sizet(value) ((size_t)(value ? atol(value) : 0))
#define cg_str2ssizet(value) ((ssize_t)(value ? atol(value) : 0))

/****************************************
* Data Type
****************************************/

typedef struct _CgString {
    char *value;
    unsigned short memSize;
    unsigned short valueSize;
} CgString;

typedef struct _CgStringTokenizer {
    char *value;
    char *delim;
    size_t delimCnt;
    size_t nextStartPos;
    size_t lastPos;
    char *currToken;
    char *nextToken;
    char repToken;
    BOOL hasNextTokens;
} CgStringTokenizer;

/****************************************
* Function (StringTokenizer)
****************************************/

CgStringTokenizer *cg_string_tokenizer_new(const char *str, const char *delim);
void cg_string_tokenizer_delete(CgStringTokenizer *strToken);
BOOL cg_string_tokenizer_hasmoretoken(CgStringTokenizer *strToken);
char *cg_string_tokenizer_nexttoken(CgStringTokenizer *strToken);
char *cg_string_tokenizer_nextalltoken(CgStringTokenizer *strToken);

void cg_string_tokenizer_print(CgStringTokenizer *strToken);

/****************************************
* Function (String)
****************************************/

CgString *cg_string_new();
void cg_string_delete(CgString *str);
void cg_string_clear(CgString *str);

void cg_string_setvalue(CgString *str, const char *value);
void cg_string_setintvalue(CgString *str, int value);
void cg_string_setlongvalue(CgString *str, long value);
void cg_string_setfloatvalue(CgString *str, float value);
void cg_string_setdoublevalue(CgString *str, double value);
void cg_string_setnvalue(CgString *str, const char *value, size_t len);
void cg_string_setpointervalue(CgString *str, char *value, size_t len);

char *cg_string_getvalue(CgString *str);
#define cg_string_getintvalue(str) cg_str2int(cg_string_getvalue(str))
#define cg_string_getlongvalue(str) cg_str2long(cg_string_getvalue(str))
#define cg_string_getfloatvalue(str) cg_str2float(cg_string_getvalue(str))
#define cg_string_getdoublevalue(str) cg_str2double(cg_string_getvalue(str))

size_t cg_string_length(CgString *str);

char *cg_string_addvalue(CgString *str, const char *value);
char *cg_string_naddvalue(CgString *str, const char *value, size_t valueLen);
char *cg_string_addrepvalue(CgString *str, const char *value, size_t repeatCnt);
char *cg_string_naddrepvalue(CgString *str, const char *value, size_t valueLen, size_t repeatCnt);

char *cg_string_replace(CgString *str, char *fromStr[], char *toStr[], size_t fromStrCnt);

size_t cg_string_getmemorysize(CgString *str);

#ifdef  __cplusplus
}
#endif

#endif
