/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"


struct zpl_errno_msg
{
  zpl_int32 key;
  const char *str;
};

#define ZPL_ERRNO_DESC(m)    {(ZPL_ERRNO_ ##m),(#m)}

struct zpl_errno_msg errno_tbl[] = {
  	{ OK				, "OK"},
  	{ ERROR				, "ERROR"},
  	{ OS_TIMEOUT		, "Timeout"},		//超时
  	{ OS_CLOSE			, "Close"},			//关闭
  	{ OS_TRY_AGAIN		, "Try Again"},		//再一次
  	{ OS_CTRL_X			, "Ctrl X"},		//CTRL 信号
  	{ OS_EXIST			, "Exist"},	//已存在
  	{ OS_NOTEXIST		, "Not Exist"},	//不存在
	{ OS_UNKNOWN_CMD	, "Unknown CMD"},	//
	{ OS_NO_SDKSPUUORT	, "No SDK Support"},
	{ OS_NO_CALLBACK	, "No Callback Func"},

  	{ ZPL_OK			, "OK"},			
  	{ ZPL_ERROR			, "ERROR"},
  	{ ZPL_OS_TIMEOUT	, "Timeout"},
	{ ZPL_OS_CLOSE		, "Close"},
  	{ ZPL_OS_TRY_AGAIN	, "Try Again"},
  	{ ZPL_OS_CTRL_X		, "Ctrl X"},
  	{ ZPL_OS_EXIST		, "Exist"},
  	{ ZPL_OS_NOTEXIST	, "Not Exist"},
	{ ZPL_UNKNOWN_CMD	, "Unknown CMD"},
	{ ZPL_NO_SDKSPUUORT	, "No SDK Support"},
	{ ZPL_NO_CALLBACK	, "No Callback Func"},
	{ ZPL_ERRNO_UNKNOW	, "Unknown error"}
};

#undef ZPL_ERRNO_DESC

void zpl_errno_set(int zpl_errno)
{
	ipstack_errno = zpl_errno;
}

int zpl_errno_get(void)
{
	return ipstack_errno;
}

const char *zpl_strerror(int errnum)
{
	int i = 0;
	const char *s = NULL; 
	for(i = 0; i < sizeof(errno_tbl)/sizeof(errno_tbl[0]); i++)
	{
		if(errno_tbl[i].key == errnum)
			return errno_tbl[i].str;
	}
	s = strerror(errnum);
	if (s != NULL) 
		return s;
#ifdef ZPL_IPCOM_MODULE
	s = ipcom_strerror(errnum);
	if (s != NULL) 
		return s;
#endif
	return "Unknown error";
}

/* Wrapper around strerror to handle case where it returns NULL. */
const char *ipstack_strerror(int errnum) {
	const char *s = zpl_strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}

const char *ipstack_ipcomstrerror(int errnum) {
	const char *s = zpl_strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}

