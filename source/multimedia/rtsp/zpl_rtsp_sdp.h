/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_SDP_H__
#define __RTSP_SDP_H__
#ifdef __cplusplus
extern "C" {
#endif



enum {
    SDP_V_VERSION_0 = 0
};
enum {
    SDP_C_NETWORK_UNKNOWN=0,
    SDP_C_NETWORK_IN
};
enum {
    SDP_C_ADDRESS_UNKNOWN=0,
    SDP_C_ADDRESS_IP4,
    SDP_C_ADDRESS_IP6
};
enum {
    SDP_A_INACTIVE = 0,
    SDP_A_SENDONLY = 0x01,
    SDP_A_RECVONLY = 0x02,
    SDP_A_SENDRECV = 0x03 /*default*/,
};
enum {
    SDP_M_MEDIA_UNKOWN = 0,
    SDP_M_MEDIA_AUDIO,
    SDP_M_MEDIA_VIDEO,
    SDP_M_MEDIA_TEXT,
    SDP_M_MEDIA_APPLICATION,
    SDP_M_MEDIA_MESSAGE
};


/*
Header               type   support   methods
Accept               R      opt.      entity
Accept-Encoding      R      opt.      entity
Accept-Language      R      opt.      all
Allow                r      opt.      all
Authorization        R      opt.      all
Bandwidth            R      opt.      all
Blocksize            R      opt.      all but OPTIONS, TEARDOWN
Cache-Control        g      opt.      SETUP
Conference           R      opt.      SETUP
Connection           g      req.      all
Content-Base         e      opt.      entity
Content-Encoding     e      req.      SET_PARAMETER
Content-Encoding     e      req.      DESCRIBE, ANNOUNCE
Content-Language     e      req.      DESCRIBE, ANNOUNCE
Content-Length       e      req.      SET_PARAMETER, ANNOUNCE
Content-Length       e      req.      entity
Content-Location     e      opt.      entity
Content-Type         e      req.      SET_PARAMETER, ANNOUNCE
Content-Type         r      req.      entity
CSeq                 g      req.      all
Date                 g      opt.      all
Expires              e      opt.      DESCRIBE, ANNOUNCE
From                 R      opt.      all
If-Modified-Since    R      opt.      DESCRIBE, SETUP
Last-Modified        e      opt.      entity
Proxy-Authenticate
Proxy-Require        R      req.      all
Public               r      opt.      all
Range                R      opt.      PLAY, PAUSE, RECORD
Range                r      opt.      PLAY, PAUSE, RECORD
Referer              R      opt.      all
Require              R      req.      all
Retry-After          r      opt.      all
RTP-Info             r      req.      PLAY
Scale                Rr     opt.      PLAY, RECORD
Session              Rr     req.      all but SETUP, OPTIONS
Server               r      opt.      all
Speed                Rr     opt.      PLAY
Transport            Rr     req.      SETUP
Unsupported          r      req.      all
User-Agent           R      opt.      all
Via                  g      opt.      all
WWW-Authenticate     r      opt.      all
*/
typedef struct sdp_header_s
{
    char *Accept;             //               R      opt.      entity
    char *Accept_Encoding;    //      R      opt.      entity
    char *Accept_Language;    //      R      opt.      all
    char *Allow;              //                r      opt.      all
    char *Authorization;      //        R      opt.      all
    char *Bandwidth;          //            R      opt.      all
    char *Blocksize;          //            R      opt.      all but OPTIONS, TEARDOWN
    char *Cache_Control;      //        g      opt.      SETUP
    char *Conference;         //           R      opt.      SETUP
    char *Connection;         //           g      req.      all
    char *Content_Base;       //         e      opt.      entity
    char *Content_Encoding;   //     e      req.      SET_PARAMETER
    char *Content_Language;   //     e      req.      DESCRIBE, ANNOUNCE
    char *Content_Length;     //       e      req.      SET_PARAMETER, ANNOUNCE
    char *Content_Location;   //     e      opt.      entity
    char *Content_Type;       //         e      req.      SET_PARAMETER, ANNOUNCE
    int CSeq;                 //                 g      req.      all
    char *Date;               //                 g      opt.      all
    char *Expires;            //              e      opt.      DESCRIBE, ANNOUNCE
    char *From;               //                 R      opt.      all
    char *If_Modified_Since;  //    R      opt.      DESCRIBE, SETUP
    char *Last_Modified;      //        e      opt.      entity
    char *Proxy_Authenticate; //
    char *Proxy_Require;      //        R      req.      all
    char *Public;             //               r      opt.      all
    char *Range;              //                R      opt.      PLAY, PAUSE, RECORD
    char *Referer;            //              R      opt.      all
    char *Require;            //              R      req.      all
    char *Retry_After;        //          r      opt.      all
    char *RTP_Info;           //             r      req.      PLAY
    char *Scale;              //                Rr     opt.      PLAY, RECORD
    char *Session;            //              Rr     req.      all but SETUP, OPTIONS
    char *Server;             //               r      opt.      all
    char *Speed;              //                Rr     opt.      PLAY
    char *Transport;          //            Rr     req.      SETUP
    char *Unsupported;        //          r      req.      all
    char *User_Agent;         //           R      opt.      all
    char *Via;                //                  g      opt.      all
    char *WWW_Authenticate;   //     r      opt.      all
} sdp_header_t;

/**
 * The RTSP_MAX_SDP_FMT macro defines maximum format in a media line.
 */
#ifndef RTSP_MAX_SDP_FMT
#define RTSP_MAX_SDP_FMT 32
#endif

/**
 * The RTSP_MAX_SDP_BANDW macro defines maximum bandwidth information
 * lines in a media line.
 */
#ifndef RTSP_MAX_SDP_BANDW
#define RTSP_MAX_SDP_BANDW 4
#endif

/**
 * The RTSP_MAX_SDP_ATTR macro defines maximum SDP attributes in media and
 * session descriptor.
 */
#ifndef RTSP_MAX_SDP_ATTR
#define RTSP_MAX_SDP_ATTR (RTSP_MAX_SDP_FMT * 2 + 4)
#endif

/**
 * The RTSP_MAX_SDP_MEDIA macro defines maximum SDP media lines in a
 * SDP session descriptor.
 */
#ifndef RTSP_MAX_SDP_MEDIA
#define RTSP_MAX_SDP_MEDIA 16
#endif


#ifndef RTSP_MAX_SDP_REPEAT
#define RTSP_MAX_SDP_REPEAT 8
#endif

#ifndef RTSP_MAX_SDP_ZONE
#define RTSP_MAX_SDP_ZONE 8
#endif

#ifndef RTSP_MAX_SDP_EMAIL
#define RTSP_MAX_SDP_EMAIL 8
#endif

#ifndef RTSP_MAX_SDP_PHONE
#define RTSP_MAX_SDP_PHONE 8
#endif
/* **************************************************************************
 * SDP ATTRIBUTES
 ***************************************************************************
 */

/**
 * Generic representation of attribute.
 */
struct sdp_attr
{
    char *name;  /**< Attribute name.    */
    char *value; /**< Attribute value.   */
};

/**
 * This structure declares SDP \a rtpmap attribute.
 */
struct sdp_rtpmap
{
    int pt;            /**< Payload type.	    */
    char enc_name[16];      /**< Encoding name.	    */
    unsigned clock_rate; /**< Clock rate.	    */
    char param[32];         /**< Parameter.	    */
};
/**
 * This structure describes SDP \a fmtp attribute.
 */
typedef struct sdp_fmtp
{
    char *fmt;       /**< Format type.		    */
    char *fmt_param; /**< Format specific parameter. */
} sdp_fmtp;

/**
 * This structure describes SDP \a rtcp attribute.
 */
typedef struct sdp_rtcp_attr
{
    unsigned port;   /**< RTCP port number.	    */
    char *net_type;  /**< Optional network type.	    */
    char *addr_type; /**< Optional address type.	    */
    char *addr;      /**< Optional address.	    */
} sdp_rtcp_attr;

/**
 * This structure describes SDP \a ssrc attribute.
 */
typedef struct sdp_ssrc_attr
{
    uint32_t ssrc; /**< RTP SSRC.	*/
    char *cname;   /**< RTCP CNAME.	*/
} sdp_ssrc_attr;

/* **************************************************************************
 * SDP CONNECTION INFO
 ****************************************************************************
 */
struct sdp_conn
{
    char *net_type;  /**< Network type ("IN").		*/
    char *addr_type; /**< Address type ("IP4", "IP6").	*/
    char *addr;      /**< The address.			*/
};
/* **************************************************************************
 * SDP BANDWIDTH INFO
 ****************************************************************************
 */
typedef struct sdp_bandw
{
    char *bwtype; /**< Bandwidth modifier.		*/
    char *bandwidth; /**< Bandwidth value.	                */
} sdp_bandw;

/* **************************************************************************
 * SDP MEDIA INFO/LINE
 ****************************************************************************
 */
struct sdp_encryption
{
    char* method;
    char* key;
};

struct sdp_origin
{
    char *user;       /**< User 				*/
    uint32_t id;      /**< Session ID			*/
    uint32_t version; /**< Session version		*/
    char *net_type;   /**< Network type ("IN")		*/
    char *addr_type;  /**< Address type ("IP4", "IP6")	*/
    char *addr;       /**< The address.			*/
};

struct sdp_repeat
{
    /** Session time (r= line)	*/
    time_t interval;
    time_t duration;
    time_t offsets[RTSP_MAX_SDP_REPEAT];
    size_t offsets_count;
};

struct sdp_timezone
{
    /**< Subject line (z=)		*/
    time_t adjust;
    time_t offset;
};


struct sdp_media
{
    /** Media descriptor line ("m=" line) */
    struct
    {
        char *media;                 /**< Media type ("audio", "video")  */
        uint16_t port;               /**< Port number.		    */
        unsigned port_count;         /**< Port count, used only when >2  */
        char *proto;             /**< Transport ("RTP/AVP")	    */
        unsigned fmt_count;          /**< Number of formats.		    */
        int fmt_capacity;
        char *fmt[RTSP_MAX_SDP_FMT]; /**< Media formats.	    */
    } desc;
    char *title;                                /**< Subject line (i=)		*/
    int conn_capacity;
    struct sdp_conn conn;                       /**< Optional connection info.	    */

    unsigned bandw_count;                       /**< Number of bandwidth info.	    */
    int bandw_capacity;
    struct sdp_bandw bandw[RTSP_MAX_SDP_BANDW]; /**< Bandwidth info.  */

    struct sdp_encryption encrypt_key;                          /**< Subject line (k=)		*/
    unsigned attr_count;                        /**< Number of attributes.	    */
    int attr_capacity;
    struct sdp_attr attr[RTSP_MAX_SDP_ATTR];    /**< Attributes.	    */
};

struct sdp_misc
{
    char *sdp_data; // raw source string
    char *sdp_start;
    uint32_t sdp_offset; // parse offset
    uint32_t sdp_len;

    char *url;
    char *version;
    char result_string[128];
};

struct sdp_auth
{
    int     type;
    char    *username;
    char    *password;
    char    *realm; // raw source string
    char    *nonce;
    bool    stale;
};


/* **************************************************************************
 * SDP SESSION DESCRIPTION
 ****************************************************************************
 */
struct sdp_session
{
    rtsp_method method;
    int code;

    struct sdp_misc misc;

    struct sdp_auth auth;

    sdp_header_t header;
    /** Session origin (o= line) */
    struct sdp_origin origin;

    char *name;                       /**< Subject line (s=)		*/
    char *information;                /**< Subject line (i=)		*/
    char *uri;                        /**< Subject line (u=)		*/
    char *emails[RTSP_MAX_SDP_EMAIL]; /**< Subject line (e=)		*/
    size_t emails_count;
    char *phones[RTSP_MAX_SDP_PHONE]; /**< Subject line (p=)		*/
    size_t phones_count;
    struct sdp_conn conn; /**< Connection line (c=)		*/

    /**< Bandwidth info array (b=)	*/
    int bandw_capacity;
    unsigned bandw_count; /**< Number of bandwidth info (b=)	*/
    struct sdp_bandw bandw[RTSP_MAX_SDP_BANDW];


    /** Session time (t= line)	*/
    struct
    {
        uint32_t start; /**< Start time.			*/
        uint32_t stop;  /**< Stop time.			*/
        struct sdp_repeat repeat[RTSP_MAX_SDP_REPEAT];
        size_t repeat_count;
    } time;

    struct sdp_timezone timezones[RTSP_MAX_SDP_ZONE];
    size_t timezones_count;

    struct sdp_encryption encrypt_key; /**< Subject line (k=)		*/

    int attr_capacity;
    unsigned attr_count;                     /**< Number of attributes.  */
    struct sdp_attr attr[RTSP_MAX_SDP_ATTR]; /**< Attributes array.   */

    int media_capacity;
    unsigned media_count;                       /**< Number of media.	    */
    struct sdp_media media[RTSP_MAX_SDP_MEDIA]; /**< Media array.   */
    bool media_start;
};

RTSP_API void sdp_skip_space(struct sdp_session* sdp);
RTSP_API int sdp_token_word(struct sdp_session* sdp, const char* escape);
RTSP_API int sdp_token_crlf(struct sdp_session* sdp);
RTSP_API char* sdp_get_string(const char *src, const char*brk);
RTSP_API int sdp_get_string_offset(const char *src, const char*brk);
RTSP_API char * sdp_scopy(const char *src, int len);
RTSP_API int sdp_get_intcode(const char *src, const char*brk);
RTSP_API char* sdp_get_datetime(time_t t);



RTSP_API char * sdp_attr_find(struct sdp_attr *attr, uint32_t n, const char *name, uint32_t *step);
RTSP_API char * sdp_attr_find_value(struct sdp_attr *attr, uint32_t n, const char *name, uint32_t *step);
RTSP_API struct sdp_media *sdp_media_find(struct sdp_media *media, uint32_t n, const char *name, uint32_t *step);

RTSP_API int sdp_header_check(char *src, uint32_t len);
RTSP_API int sdp_header_parse(char *src, uint32_t len, struct sdp_session *session);
RTSP_API int sdp_header_init(struct sdp_session *session);
RTSP_API int sdp_header_free(struct sdp_session *session);
RTSP_API int sdp_header_debug(struct sdp_session *session);

RTSP_API int sdp_attr_check(char *src, uint32_t len);
RTSP_API int sdp_attr_parse(char *src, uint32_t len, struct sdp_session *session, bool m);
RTSP_API int sdp_attr_debug(struct sdp_session *session);
RTSP_API int sdp_attr_free(struct sdp_session *session);
RTSP_API int sdp_attr_rtpmap_get(const char* rtpmapstr, struct sdp_rtpmap *rtpmap);

RTSP_API int sdp_text_init(struct sdp_session *session, const char *raw, uint32_t len);
RTSP_API int sdp_text_free(struct sdp_session *session);
RTSP_API int sdp_text_debug(struct sdp_session *session);
RTSP_API int sdp_text_prase(bool srv, struct sdp_session *session);

RTSP_API const char *sdp_code_get(int code);
RTSP_API const char *sdp_method_get(rtsp_method method);


RTSP_API int sdp_build_sessionID(struct sdp_session *sdp, uint8_t *src, uint32_t sessionid);
/* client */
RTSP_API int sdp_build_request_header(struct sdp_session *sdp, uint8_t *src,
                                      const char *clientname, rtsp_method method, const char *url, int cseq);
/* server */
RTSP_API int sdp_build_respone_header(uint8_t *src, const char *srvname,
                                      char *base, int code, int CSeq, uint32_t sessionid);
RTSP_API int sdp_build_respone_option(uint8_t *src, const char *srvname, int code, int CSeq);



RTSP_API int rtsp_sdp_hdr_prase_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SDP_H__ */
