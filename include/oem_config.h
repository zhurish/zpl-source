/*
 * oem_config.h
 *
 *  Created on: Oct 24, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_OEM_CONFIG_H_
#define INCLUDE_OEM_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "gitversion.h"

#define OEM_BUG_ADDRESS "https://bugzilla.quagga.net"

/* this OEM auther name */
#define OEM_PACKAGE_AUTHER "zhurish"

/* this OEM base of Quagga version */
#define OEM_PACKAGE_BASE "Quagga 1.2.0"//PACKAGE_STRING

/* Define to the address where bug reports for this OEM package should be
   sent. */
#define OEM_PACKAGE_BUGREPORT "zhurish@163.com"

/* this copyright for OEM version */
#define OEM_PACKAGE_COPYRIGHT "Copyright 2016/03/03 - 2017/04/15 zhurish(zhurish@163.com) et al."

/* Name of OEM package */
#define OEM_PACKAGE_NAME "routing-plaform"

/* this OEM base on Quagga version number(xx.xx.00) */
#define OEM_PACKAGE_VERSION (0x00010200)

/* Define to the version of this OEM package. */
#define OEM_VERSION 	"V0.0.0.1"


#define OEM_DEVICE_NAME	 	"L3-RouteGateway"
#define OEM_HW_VERSION	 	"V0.0.0.1"

/*
#ifdef GIT_VERSION
#define OEM_VERSION GIT_VERSION
#endif
*/
#ifdef PL_BUILD_VERSION
#undef OEM_VERSION
#define OEM_VERSION PL_BUILD_VERSION
#endif

#ifdef GIT_RELEASE
#define OEM_GIT_RELEASE GIT_RELEASE
#else
#define OEM_GIT_RELEASE "master"
#endif

#ifdef GIT_COMMIT
#define OEM_GIT_COMMIT GIT_COMMIT
#else
#define OEM_GIT_COMMIT "0000000000000000000"
#endif

#ifdef MAKE_DATE
#define OEM_MAKE_DATE MAKE_DATE
#else
#define OEM_MAKE_DATE "20181022"
#endif

#define OEM_GIT_SUFFIX  ""
#define OEM_GIT_INFO	""


#define OEM_PROGNAME 	OEM_PACKAGE_NAME

#define OEM_BASE_VERSION(a,b,c)	((a)<<16)|((b)<<8)|(c)

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_OEM_CONFIG_H_ */
