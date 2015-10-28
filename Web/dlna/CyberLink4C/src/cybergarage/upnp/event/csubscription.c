/******************************************************************
*
*    CyberLink for C
*
*    Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*    File: csubscription.c
*
*    Revision:
*
*    06/20/05
*        - first revision
*    02/12/06 Theo Beisch
*        - changed time(NULL) to cg_getcurrentsystemtime()
*    18-Jan-06 Aapo Makela
*        - Modified to fix negative timeout value
*
******************************************************************/
#include "source_macro.h"
#ifdef __WIFI_UPNP_C__
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/event/cevent.h>
#include <cybergarage/util/clog.h>
#include <cybergarage/upnp/cupnp_function.h>

#include <stdio.h>
#include <stdlib.h>
#include <config.h>
//#pragma arm section code = "upnpCode", rodata = "upnpCode", rwdata = "upnpdata", zidata = "upnpdata"

/****************************************
* CG_UPNP_NOUSE_SUBSCRIPTION (Begin)
****************************************/

#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)

/****************************************
* cg_upnp_event_subscription_totimeoutheaderstring
****************************************/

const char *cg_upnp_event_subscription_totimeoutheaderstring(CgTime time, CgString *buf)
{
    char timeBuf[CG_STRING_LONG_BUFLEN];

    cg_log_debug_l4("Entering...\n");

    if (time != CG_UPNP_SUBSCRIPTION_INFINITE_VALUE) {
        cg_string_setvalue(buf, CG_UPNP_SUBSCRIPTION_TIMEOUT_HEADER);
        cg_string_addvalue(buf, cg_long2str(time, timeBuf, sizeof(timeBuf)));
    }
    else
        cg_string_setvalue(buf, CG_UPNP_SUBSCRIPTION_INFINITE_STRING);
    return cg_string_getvalue(buf);

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_upnp_event_subscription_gettimeout
****************************************/

CgTime cg_upnp_event_subscription_gettimeout(const char *headerValue)
{
    ssize_t minusIdx;
    long timeout;

    cg_log_debug_l4("Entering...\n");

    minusIdx = cg_strstr(headerValue, "-");
    if (minusIdx < 0)
        return CG_UPNP_SUBSCRIPTION_INFINITE_VALUE;
    timeout = cg_str2long(headerValue + minusIdx + 1);
    if (timeout == 0)
        return CG_UPNP_SUBSCRIPTION_INFINITE_VALUE;
    if (timeout < 0)
        timeout =-timeout;
    return timeout;

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_upnp_event_subscription_createsid
****************************************/

const char *cg_upnp_event_subscription_createsid(char *buf, size_t bufSize)
{
    cg_log_debug_l4("Entering...\n");
    cg_upnp_createuuid(buf, bufSize);
    cg_log_debug_l4("Leaving...\n");
    return buf;
}

/****************************************
* cg_upnp_event_subscription_tosidheaderstring
****************************************/

const char *cg_upnp_event_subscription_tosidheaderstring(const char *sid, char *buf, size_t bufSize)
{
    ssize_t colonIdx;

    cg_log_debug_l4("Entering...\n");

    colonIdx = cg_strstr(sid, CG_UPNP_SUBSCRIPTION_UUID);
    if (0 <= colonIdx) {
        cg_strncpy(buf, sid, bufSize);
        buf[bufSize-1] = '\0';
        return buf;
    }

#if defined(HAVE_SNPRINTF)
    snprintf(buf, bufSize,
#else
    sprintf(buf,
#endif
        "%s%s",
        CG_UPNP_SUBSCRIPTION_UUID,
        sid);
    return buf;

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_upnp_event_subscription_getsid
****************************************/

const char *cg_upnp_event_subscription_getsid(const char *headerValue)
{
    ssize_t colonIdx;

    cg_log_debug_l4("Entering...\n");

    colonIdx = cg_strstr(headerValue, ":");
    if (colonIdx < 0)
        return headerValue;

    cg_log_debug_l4("Leaving...\n");

    return (headerValue + colonIdx + 1);
}

/****************************************
* CG_UPNP_NOUSE_SUBSCRIPTION (End)
****************************************/

#endif
//#pragma arm section code
#endif
