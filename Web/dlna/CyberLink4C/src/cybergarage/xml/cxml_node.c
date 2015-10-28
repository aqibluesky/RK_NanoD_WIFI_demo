/******************************************************************
*
*    CyberXML for C
*
*    Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006-2007 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*    File: cxml_node.cpp
*
*    Revision:
*
*    02/07/05
*        - first revision
*    12/13/07 Aapo Makela
*        - Fixes to work in out-of-memory situations
*    05/22/08
*        - Added cg_xml_nodelist_getbyname() instead of cg_xml_nodelist_get().
*        - Added cg_xml_node_getchildnodebyname() instead of cg_xml_node_getchildnode().
*        - Added cg_xml_nodelist_getbyxpath() and cg_xml_node_getchildnodebyxpath() to get a node simply.
*        - Deprecated cg_xml_nodelist_get() and cg_xml_node_getchildnode().*
*
******************************************************************/
#include "source_macro.h"
#ifdef __WIFI_UPNP_C__
#include <cybergarage/xml/cxml.h>
#include <cybergarage/util/clog.h>
#include <rk_heap.h>

//#pragma arm section code = "upnpCode", rodata = "upnpCode", rwdata = "upnpdata", zidata = "upnpdata"


/****************************************
* cg_xml_node_new
****************************************/
int node_new_cnt=0;

static int is_cg_xml_init = 0;
static char * cg_xml_buf = NULL;
#define CG_XML_BUF_SIZE 352
static int * use_flag = NULL;

CgXmlNode *cg_xml_node_new()
{
    CgXmlNode *node = NULL;
    int i;
    //cg_log_debug_l4("Entering...\n");

#if 0
    node = (CgXmlNode *)malloc(sizeof(CgXmlNode));

    if ( NULL != node )
    {
        cg_list_node_init((CgList *)node);

        node->name = cg_string_new();
        node->value = cg_string_new();
        node->attrList = cg_xml_attributelist_new();
        node->nodeList = cg_xml_nodelist_new();
        node->parentNode = NULL;
        node->userData = NULL;
        //node->userDataDestructorFunc = NULL;
    }
    node_new_cnt++;
#endif

    if(!is_cg_xml_init)
    {
        is_cg_xml_init = 1;
        cg_xml_buf = malloc(sizeof(CgXmlNode)*CG_XML_BUF_SIZE);
        if(NULL == cg_xml_buf)
        {
            printf("cg_xml_buf malloc fail\n");
            return node;
        }
        use_flag = (unsigned int *)malloc(CG_XML_BUF_SIZE/8);
        if(NULL == use_flag)
        {
            free(cg_xml_buf);
            printf("use_flag malloc fail\n");
            return node;
        }
        memset(use_flag, 0 , CG_XML_BUF_SIZE/8);
    }
    //cg_log_debug_l5("Entering...\n");
    for(i=0; i<CG_XML_BUF_SIZE;i++)
    {   int group = i/32;
        int index = i%32;
        if((use_flag[group] & (1<< index)) == 0)
        {
            use_flag[group] |= (1<< index);

            node = (CgXmlNode *)(cg_xml_buf+i*sizeof(CgXmlNode));

            cg_list_node_init((CgList *)node);
            node->name = cg_string_new();
            node->value = cg_string_new();
            node->attrList = cg_xml_attributelist_new();
            node->nodeList = cg_xml_nodelist_new();
            node->parentNode = NULL;
            node->userData = NULL;
            node_new_cnt++;


            break;
        }
    }

    //printf("node_new_cnt =%d\n",node_new_cnt);

    if(NULL == node)
    {
        node = (CgXmlNode *)malloc(sizeof(CgXmlNode));

        if ( NULL != node )
        {
            cg_list_node_init((CgList *)node);

            node->name = cg_string_new();
            node->value = cg_string_new();
            node->attrList = cg_xml_attributelist_new();
            node->nodeList = cg_xml_nodelist_new();
            node->parentNode = NULL;
            node->userData = NULL;
            //node->userDataDestructorFunc = NULL;
        }

    }

    //cg_log_debug_l4("Leaving...\n");

    return node;
}

/****************************************
* cg_xml_node_delete
****************************************/

void cg_xml_node_delete(CgXmlNode *node)
{
    cg_log_debug_l4("Entering...\n");

    //if (node->userDataDestructorFunc != NULL)
        //node->userDataDestructorFunc(node->userData);
#if 0
    free(node);
#endif

    if(NULL != node)
    {
        cg_list_remove((CgList *)node);
        cg_string_delete(node->name);
        cg_string_delete(node->value);
        cg_xml_attributelist_delete(node->attrList);
        cg_xml_nodelist_delete(node->nodeList);

        if(((unsigned int)node >= (unsigned int)cg_xml_buf) && ((unsigned int)node) < (unsigned int)cg_xml_buf + sizeof(CgXmlNode)*CG_XML_BUF_SIZE)
        {
            int i = ((unsigned int)node-(unsigned int)cg_xml_buf)/sizeof(CgXmlNode);
            int group = i/32;
            int index = i%32;
            use_flag[group] &= ~(1<< index);
            node_new_cnt--;

            if(node_new_cnt == 0)
            {
                is_cg_xml_init = 0;
                free(cg_xml_buf);
                free(use_flag);
            }
        }
        else
        {
           free(node);
        }
    }
    //cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_xml_node_setparentnode
****************************************/

CgXmlNode *cg_xml_node_getrootnode(CgXmlNode *node)
{
    CgXmlNode *rootNode;
    CgXmlNode *parentNode;

    cg_log_debug_l4("Entering...\n");

    rootNode = NULL;
    parentNode = cg_xml_node_getparentnode(node);
    while (parentNode != NULL) {
         rootNode = parentNode;
         parentNode = cg_xml_node_getparentnode(rootNode);
    }

    cg_log_debug_l4("Leaving...\n");

    return rootNode;
}

/****************************************
* cg_xml_node_addchildnode
****************************************/

void cg_xml_node_addchildnode(CgXmlNode *node, CgXmlNode *cnode)
{
    cg_log_debug_l4("Entering...\n");

    cg_xml_nodelist_add(node->nodeList, cnode);
    cg_xml_node_setparentnode(cnode, node);

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_xml_node_setchildnode
****************************************/

void cg_xml_node_setchildnode(CgXmlNode *node, const char *name, const char *value)
{
    CgXmlNode *childNode;

    cg_log_debug_l4("Entering...\n");

    childNode = cg_xml_node_getchildnode(node, name);
    if (childNode != NULL) {
        cg_xml_node_setvalue(childNode, value);
        return;
    }
    childNode = cg_xml_node_new();
    cg_xml_node_setname(childNode, name);
    cg_xml_node_setvalue(childNode, value);

    cg_xml_node_addchildnode(node, childNode);

    cg_log_debug_l4("Leaving...\n");
}

/*****************************************************************************
* Get childnode with some specific namespace prefix, or ignore namespace prefix.
* If ns is NULL, name string must match completely
*****************************************************************************/

CgXmlNode *cg_xml_node_getchildnodewithnamespace(CgXmlNode *node, const char *name, const char *ns, BOOL ignoreNs)
{
    char *nameWithPrefix = NULL;
    size_t nameLen = 0;
    ssize_t nameIdx;
    CgXmlNode *result = NULL;
    CgXmlNode *child = NULL;
    char *nodeName = NULL;

    cg_log_debug_l4("Entering...\n");

    if (node == NULL)
        return NULL;

    if (name == NULL)
        return NULL;

    if (ignoreNs == TRUE)
    {
        for (child = cg_xml_node_getchildnodelist(node); child != NULL; child = cg_xml_node_next(child))
        {
            nodeName = cg_xml_node_getname(child);
            if (nodeName == NULL)
                continue;

            nameIdx = cg_strstr(nodeName, ":");
            if (nameIdx < 0)
            {
                /* No prefix (no ':') */
                if (cg_strcasecmp(nodeName, name) == 0)
                    return child;
            }
            else
            {
                if (cg_strcasecmp(&(nodeName[nameIdx+1]), name) == 0)
                    return child;
            }
        }

        return NULL;
    }
    else
    {
        if (ns == NULL)
        {
            /* When ns is NULL, this works like normal ..._getchildnode */
            return cg_xml_node_getchildnode(node, name);
        }

        nameLen = cg_strlen(name) + cg_strlen(ns) + 1; /* Not including the terminating \0 */
        nameWithPrefix = (char*) malloc(nameLen + 1);
        if (nameWithPrefix == NULL)
            return NULL;

#if defined(HAVE_SNPRINTF)
        snprintf(nameWithPrefix, nameLen + 1, "%s:%s", ns, name);
#else
        sprintf(nameWithPrefix, "%s:%s", ns, name);
#endif

        result = cg_xml_node_getchildnode(node, nameWithPrefix);

        free(nameWithPrefix);
        return result;
    }

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_xml_node_getchildnodevalue
****************************************/

const char *cg_xml_node_getchildnodevalue(CgXmlNode *node, const char *name)
{
    CgXmlNode *childNode;

    cg_log_debug_l4("Entering...\n");

    childNode = cg_xml_node_getchildnode(node, name);
    if (childNode != NULL)
        return cg_xml_node_getvalue(childNode);

    cg_log_debug_l4("Leaving...\n");

    return NULL;
}

/****************************************
* cg_xml_node_removeattribute
****************************************/

void cg_xml_node_removeattribute(CgXmlNode *node, const char *name)
{
    CgXmlAttribute *attr;

    if (!node || !name)
        return;

    attr = cg_xml_node_getattribute(node, name);
    if (!attr)
        return;

    cg_xml_attribute_remove(attr);
}

/****************************************
 * cg_xml_node_copy
 ****************************************/

void cg_xml_node_copy(CgXmlNode *dstNode, CgXmlNode *srcNode)
{
    CgXmlAttribute *attr;
    CgXmlNode *dstChildNode;
    CgXmlNode *srcChildNode;

    cg_log_debug_l4("Entering...\n");

    if (!dstNode || !srcNode)
        return;

    cg_xml_node_setname(dstNode, cg_xml_node_getname(srcNode));
    cg_xml_node_setvalue(dstNode, cg_xml_node_getvalue(srcNode));

    for (attr = cg_xml_node_getattributes(srcNode); attr != NULL; attr = cg_xml_attribute_next(attr))
        cg_xml_node_setattribute(dstNode, cg_xml_attribute_getname(attr), cg_xml_attribute_getvalue(attr));

    for (srcChildNode = cg_xml_node_getchildnodes(srcNode); srcChildNode != NULL; srcChildNode = cg_xml_node_next(srcChildNode)) {
        dstChildNode = cg_xml_node_new();
        cg_xml_node_copy(dstChildNode, srcChildNode);
        cg_xml_node_addchildnode(dstNode, dstChildNode);
    }

     cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_xml_node_attribute_tostring
****************************************/

static char *cg_xml_node_attribute_tostring(CgXmlNode *node, CgString *str)
{
    CgXmlAttribute *attr;
    const char *name;
    const char *value;
    CgString *valueStr;

    cg_log_debug_l4("Entering...\n");

    valueStr = cg_string_new();
    if (valueStr == NULL) return NULL;

    for (attr = cg_xml_node_getattributes(node); attr != NULL; attr = cg_xml_attribute_next(attr)) {
        name = cg_xml_attribute_getname(attr);
        value = cg_xml_attribute_getvalue(attr);

        cg_string_setvalue(valueStr, value);
        cg_xml_escapechars(valueStr);

        /* All the following functions return NULL only when memory
           allocation fails, so we can check them all */
        if (!cg_string_naddvalue(str, " ", 1) ||
            !cg_string_addvalue(str, name) ||
            !cg_string_naddvalue(str, "=\"", 2) ||
            !cg_string_addvalue(str, cg_string_getvalue(valueStr)) ||
            !cg_string_naddvalue(str, "\"", 1))
        {
            /* Memory allocation failed */
            cg_string_delete(valueStr);
            return NULL;
        }
    }
    cg_string_delete(valueStr);

    cg_log_debug_l4("Leaving...\n");

    return cg_string_getvalue(str);
}

/****************************************
* cg_xml_node_tostring_indent
****************************************/

static char *cg_xml_node_tostring_indent(CgXmlNode *node, int indentLevel, BOOL withChildNode, CgString *str)
{
    char *name;
    char *value;
    CgString *valueStr;
    CgXmlNode *childNode;

    cg_log_debug_l4("Entering...\n");

    name = cg_xml_node_getname(node);
    value = cg_xml_node_getvalue(node);

    if (cg_xml_node_haschildnodes(node) == FALSE || withChildNode == FALSE) {
        cg_string_addrepvalue(str, CG_XML_INDENT_STRING, indentLevel);
        if (!cg_string_naddvalue(str, "<", 1) ||
            !cg_string_addvalue(str, name) ||
            !cg_xml_node_attribute_tostring(node, str))
            /* Memory allocation failed */
            return NULL;

        valueStr = cg_string_new();
        if (!valueStr)
            /* Memory allocation failed */
            return NULL;

        cg_string_setvalue(valueStr, value);
        cg_xml_escapechars(valueStr);

        if (!cg_string_naddvalue(str, ">", 1) ||
            !cg_string_addvalue(str, cg_string_getvalue(valueStr)) ||
            !cg_string_naddvalue(str, "</", 2) ||
            !cg_string_addvalue(str, name) ||
            !cg_string_naddvalue(str, ">", 1) ||
            !cg_string_addvalue(str, "\n"))
        {
            /* Memory allocation failed */
            cg_string_delete(valueStr);
            return NULL;
        }

        cg_string_delete(valueStr);

        return cg_string_getvalue(str);
    }

    cg_string_addrepvalue(str, CG_XML_INDENT_STRING, indentLevel);
    if (!cg_string_naddvalue(str, "<", 1) ||
        !cg_string_addvalue(str, name) ||
        !cg_xml_node_attribute_tostring(node, str) ||
        !cg_string_naddvalue(str, ">", 1) ||
        !cg_string_addvalue(str, "\n"))
        /* Memory allocation failed */
        return NULL;

    for (childNode = cg_xml_node_getchildnodes(node); childNode != NULL; childNode = cg_xml_node_next(childNode))
        if (!cg_xml_node_tostring_indent(childNode, indentLevel+1, TRUE, str))
            /* Memory allocation failed */
            return NULL;

    cg_string_addrepvalue(str, CG_XML_INDENT_STRING, indentLevel);
    if (!cg_string_naddvalue(str, "</", 2) ||
        !cg_string_addvalue(str, name) ||
        !cg_string_naddvalue(str, ">", 1) ||
        !cg_string_addvalue(str, "\n"))
        /* Memory allocation failed */
        return NULL;

    cg_log_debug_l4("Leaving...\n");

    return cg_string_getvalue(str);
}

/****************************************
* cg_xml_node_tostring
****************************************/

char *cg_xml_node_tostring(CgXmlNode *node, BOOL withChildNode, CgString *str)
{
    cg_log_debug_l4("Entering...\n");

    return cg_xml_node_tostring_indent(node, 0, withChildNode, str);

    cg_log_debug_l4("Leaving...\n");
}

/****************************************
 * cg_xml_node_print
 ****************************************/

void  cg_xml_node_print(CgXmlNode *node)
{
    CgString *str = cg_string_new();
    cg_xml_node_tostring(node, TRUE, str);
    printf("%s", cg_string_getvalue(str));
    cg_string_delete(str);
}
//#pragma arm section code
#endif

