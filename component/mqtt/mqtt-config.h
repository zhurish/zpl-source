#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__
/* ============================================================
 * Platform options
 * ============================================================ */
#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"
#ifdef __APPLE__
#  define __DARWIN_C_SOURCE
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__SYMBIAN32__) || defined(__QNX__)
#  define _XOPEN_SOURCE 700
#  define __BSD_VISIBLE 1
#  define HAVE_NETINET_IN_H
#else
#  define _XOPEN_SOURCE 700
#  define _DEFAULT_SOURCE 1
#  define _POSIX_C_SOURCE 200809L
#endif


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#define OPENSSL_LOAD_CONF

/* ============================================================
 * Compatibility defines
 * ============================================================ */
#if defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf sprintf_s
#  define EPROTO ECONNABORTED
#endif

#ifdef WIN32
#  ifndef strcasecmp
#    define strcasecmp strcmpi
#  endif
#  define strtok_r strtok_s
#  define strerror_r(e, b, l) strerror_s(b, l, e)
#endif


#define uthash_malloc(sz) mosquitto__malloc(sz)
#define uthash_free(ptr,sz) mosquitto__free(ptr)


#ifdef WITH_TLS
#  include <openssl/opensslconf.h>
#  if defined(WITH_TLS_PSK) && !defined(OPENSSL_NO_PSK)
#    define FINAL_WITH_TLS_PSK
#  endif
#endif


#ifdef __COVERITY__
#  include <stdint.h>
/* These are "wrong", but we don't use them so it doesn't matter */
#  define _Float32 uint32_t
#  define _Float32x uint32_t
#  define _Float64 uint64_t
#  define _Float64x uint64_t
#  define _Float128 uint64_t
#endif

#define UNUSED(A) (void)(A)

/* Android Bionic libpthread implementation doesn't have pthread_cancel */
#ifndef ANDROID
#  define HAVE_PTHREAD_CANCEL
#endif


/** MQTT QOS等级 */
typedef enum  {
    MQTT_QOS_LEVEL0,  /**< 最多发送一次 */
    MQTT_QOS_LEVEL1,  /**< 最少发送一次  */
    MQTT_QOS_LEVEL2   /**< 只发送一次 */
} mqtt_qos_level;

/** MQTT 连接请求标志位，内部使用 */
typedef enum  {
    MQTT_CONNECT_CLEAN_SESSION  = 0x02,
    MQTT_CONNECT_WILL_FLAG      = 0x04,
    MQTT_CONNECT_WILL_QOS0      = 0x00,
    MQTT_CONNECT_WILL_QOS1      = 0x08,
    MQTT_CONNECT_WILL_QOS2      = 0x10,
    MQTT_CONNECT_WILL_RETAIN    = 0x20,
    MQTT_CONNECT_PASSORD        = 0x40,
    MQTT_CONNECT_USER_NAME      = 0x80
} mqtt_connect_flag;

/** 连接确认标志位 */
typedef enum  {
    MQTT_CONNACK_SP = 0x01 /**< 保留原来会话，以原会话登陆 */
} mqtt_connAck_flag;

/** MQTT 返回码 */
typedef enum  {
    MQTT_CONNACK_ACCEPTED                  = 0, /**< 连接已建立 */
    MQTT_CONNACK_UNACCEPTABLE_PRO_VERSION  = 1, /**< 服务器不支持该版本的MQTT协议*/
    MQTT_CONNACK_IDENTIFIER_REJECTED       = 2, /**< 不允许的客户端ID */
    MQTT_CONNACK_SERVER_UNAVAILABLE        = 3, /**< 服务器不可用 */
    MQTT_CONNACK_BAD_USER_NAME_OR_PASSWORD = 4, /**< 用户名或密码不合法 */
    MQTT_CONNACK_NOT_AUTHORIZED            = 5, /**< 鉴权失败 */

    MQTT_SUBACK_QOS0    = 0x00,  /**< 订阅确认， QoS等级0 */
    MQTT_SUBACK_QOS1    = 0x01,  /**< 订阅确认， QoS等级1 */
    MQTT_SUBACK_QOS2    = 0x02,  /**< 订阅确认， QoS等级2 */
    MQTT_SUBACK_FAILUER = 0x80   /**< 订阅失败 */
} mqtt_ret_code;

/** 数据点类型，内部使用 */
typedef enum  {
    MQTT_DPTYPE_JSON = 1,
    MQTT_DPTYPE_TRIPLE = 2,  /**< 包含数据流名称、时间戳和数据点值 */
    MQTT_DPTYPE_BINARY = 4,   /**< 包含二进制数据的数据点 */
    MQTT_DPTYPE_FLOAT = 7
} mqtt_datapoint_type;

/* 上报数据点，消息支持的格式类型 */
typedef enum {
    kTypeFullJson = 0x01,
    kTypeBin = 0x02,
    kTypeSimpleJsonWithoutTime = 0x03,
    kTypeSimpleJsonWithTime = 0x04,
    kTypeString = 0x05,
    kTypeStringWithTime = 0x06,
    kTypeFloat  = 0x07
} mqtt_savedata_type;

#ifndef WITH_BROKER
#include "zpl_include.h"
#include "memory.h"
#include "vty.h"
#include "buffer.h"
#include "command.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#endif

#endif
