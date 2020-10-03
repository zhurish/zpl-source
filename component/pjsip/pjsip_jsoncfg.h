/*
 * pjsip_jsoncfg.h
 *
 *  Created on: Feb 17, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_JSONCFG_H__
#define __PJSIP_JSONCFG_H__


#define PJSIP_NAMESERVER_MAX		4
#define PJSIP_OUTBOUND_PROXY_MAX	4
#define PJSIP_STUNSERVER_MAX		8


/**
 * SIP User Agent related settings.
 */
typedef struct pjsip_ua_config
{
    /**
     * Maximum calls to support (default: 4). The value specified here
     * must be smaller than the compile time maximum settings
     * PJSUA_MAX_CALLS, which by default is 32. To increase this
     * limit, the library must be recompiled with new PJSUA_MAX_CALLS
     * value.
     */
    unsigned int		maxCalls;

    /**
     * Number of worker threads. Normally application will want to have at
     * least one worker thread, unless when it wants to poll the library
     * periodically, which in this case the worker thread can be set to
     * zero.
     */
    unsigned int		threadCnt;

    /**
     * When this flag is non-zero, all callbacks that come from thread
     * other than main thread will be posted to the main thread and
     * to be executed by Endpoint::libHandleEvents() function. This
     * includes the logging callback. Note that this will only work if
     * threadCnt is set to zero and Endpoint::libHandleEvents() is
     * performed by main thread. By default, the main thread is set
     * from the thread that invoke Endpoint::libCreate()
     *
     * Default: false
     */
    bool		mainThreadOnly;

    /**
     * Array of nameservers to be used by the SIP resolver subsystem.
     * The order of the name server specifies the priority (first name
     * server will be used first, unless it is not reachable).
     */
    char*	nameserver[PJSIP_NAMESERVER_MAX];
    int nameserverCnt;
    /**
     * Specify the URL of outbound proxies to visit for all outgoing requests.
     * The outbound proxies will be used for all accounts, and it will
     * be used to build the route set for outgoing requests. The final
     * route set for outgoing requests will consists of the outbound proxies
     * and the proxy configured in the account.
     */
    char*	outboundProxies[PJSIP_OUTBOUND_PROXY_MAX];
    int outboundProxiesCnt;
    /**
     * Optional user agent string (default empty). If it's empty, no
     * User-Agent header will be sent with outgoing requests.
     */
    char*		userAgent;

    /**
     * Array of STUN servers to try. The library will try to resolve and
     * contact each of the STUN server entry until it finds one that is
     * usable. Each entry may be a domain name, host name, IP address, and
     * it may contain an optional port number. For example:
     *	- "pjsip.org" (domain name)
     *	- "sip.pjsip.org" (host name)
     *	- "pjsip.org:33478" (domain name and a non-standard port number)
     *	- "10.0.0.1:3478" (IP address and port number)
     *
     * When nameserver is configured in the \a pjsua_config.nameserver field,
     * if entry is not an IP address, it will be resolved with DNS SRV
     * resolution first, and it will fallback to use DNS A resolution if this
     * fails. Port number may be specified even if the entry is a domain name,
     * in case the DNS SRV resolution should fallback to a non-standard port.
     *
     * When nameserver is not configured, entries will be resolved with
     * pj_gethostbyname() if it's not an IP address. Port number may be
     * specified if the server is not listening in standard STUN port.
     */
    char*	stunServer[PJSIP_STUNSERVER_MAX];
    int stunServerCnt;
    /**
     * This specifies if the library should try to do an IPv6 resolution of
     * the STUN servers if the IPv4 resolution fails. It can be useful
     * in an IPv6-only environment, including on NAT64.
     *
     * Default: FALSE
     */
    bool	    	stunTryIpv6;

    /**
     * This specifies if the library startup should ignore failure with the
     * STUN servers. If this is set to PJ_FALSE, the library will refuse to
     * start if it fails to resolve or contact any of the STUN servers.
     *
     * Default: TRUE
     */
    bool		stunIgnoreFailure;

    /**
     * Support for adding and parsing NAT type in the SDP to assist
     * troubleshooting. The valid values are:
     *	- 0: no information will be added in SDP, and parsing is disabled.
     *	- 1: only the NAT type number is added.
     *	- 2: add both NAT type number and name.
     *
     * Default: 1
     */
    int			natTypeInSdp;

    /**
     * Handle unsolicited NOTIFY requests containing message waiting
     * indication (MWI) info. Unsolicited MWI is incoming NOTIFY requests
     * which are not requested by client with SUBSCRIBE request.
     *
     * If this is enabled, the library will respond 200/OK to the NOTIFY
     * request and forward the request to Endpoint::onMwiInfo() callback.
     *
     * See also AccountMwiConfig.enabled.
     *
     * Default: PJ_TRUE
     */
    bool	    	mwiUnsolicitedEnabled;

}pjsip_ua_config_t;



/**
 * Logging configuration, which can be (optionally) specified when calling
 * Lib::init().
 */
typedef struct pjsip_log_config
{
    /** Log incoming and outgoing SIP message? Yes!  */
    unsigned int		msgLogging;

    /** Input verbosity level. Value 5 is reasonable. */
    unsigned int		level;

    /** Verbosity level for console. Value 4 is reasonable. */
    unsigned int		consoleLevel;

    /** Log decoration. */
    unsigned int		decor;

    /** Optional log filename if app wishes the library to write to log file.
     */
    char*		filename;

    /**
     * Additional flags to be given to pj_file_open() when opening
     * the log file. By default, the flag is PJ_O_WRONLY. Application
     * may set PJ_O_APPEND here so that logs are appended to existing
     * file instead of overwriting it.
     *
     * Default is 0.
     */
    unsigned int		fileFlags;


}pjsip_log_config_t;

/**
 * This structure describes media configuration, which will be specified
 * when calling Lib::init().
 */
typedef struct pjsip_media_config
{
    /**
     * Clock rate to be applied to the conference bridge.
     * If value is zero, default clock rate will be used
     * (PJSUA_DEFAULT_CLOCK_RATE, which by default is 16KHz).
     */
    unsigned int		clockRate;

    /**
     * Clock rate to be applied when opening the sound device.
     * If value is zero, conference bridge clock rate will be used.
     */
    unsigned int		sndClockRate;

    /**
     * Channel count be applied when opening the sound device and
     * conference bridge.
     */
    unsigned int		channelCount;

    /**
     * Specify audio frame ptime. The value here will affect the
     * samples per frame of both the sound device and the conference
     * bridge. Specifying lower ptime will normally reduce the
     * latency.
     *
     * Default value: PJSUA_DEFAULT_AUDIO_FRAME_PTIME
     */
    unsigned int		audioFramePtime;

    /**
     * Specify maximum number of media ports to be created in the
     * conference bridge. Since all media terminate in the bridge
     * (calls, file player, file recorder, etc), the value must be
     * large enough to support all of them. However, the larger
     * the value, the more computations are performed.
     *
     * Default value: PJSUA_MAX_CONF_PORTS
     */
    unsigned int		maxMediaPorts;

    /**
     * Specify whether the media manager should manage its own
     * ioqueue for the RTP/RTCP sockets. If yes, ioqueue will be created
     * and at least one worker thread will be created too. If no,
     * the RTP/RTCP sockets will share the same ioqueue as SIP sockets,
     * and no worker thread is needed.
     *
     * Normally application would say yes here, unless it wants to
     * run everything from a single thread.
     */
    bool		hasIoqueue;

    /**
     * Specify the number of worker threads to handle incoming RTP
     * packets. A value of one is recommended for most applications.
     */
    unsigned int		threadCnt;

    /**
     * Media quality, 0-10, according to this table:
     *   5-10: resampling use large filter,
     *   3-4:  resampling use small filter,
     *   1-2:  resampling use linear.
     * The media quality also sets speex codec quality/complexity to the
     * number.
     *
     * Default: 5 (PJSUA_DEFAULT_CODEC_QUALITY).
     */
    unsigned int		quality;

    /**
     * Specify default codec ptime.
     *
     * Default: 0 (codec specific)
     */
    unsigned int		ptime;

    /**
     * Disable VAD?
     *
     * Default: 0 (no (meaning VAD is enabled))
     */
    bool		noVad;

    /**
     * iLBC mode (20 or 30).
     *
     * Default: 30 (PJSUA_DEFAULT_ILBC_MODE)
     */
    unsigned int		ilbcMode;

    /**
     * Percentage of RTP packet to drop in TX direction
     * (to simulate packet lost).
     *
     * Default: 0
     */
    unsigned int		txDropPct;

    /**
     * Percentage of RTP packet to drop in RX direction
     * (to simulate packet lost).
     *
     * Default: 0
     */
    unsigned int		rxDropPct;

    /**
     * Echo canceller options (see pjmedia_echo_create())
     *
     * Default: 0.
     */
    unsigned int		ecOptions;

    /**
     * Echo canceller tail length, in miliseconds. Setting this to zero
     * will disable echo cancellation.
     *
     * Default: PJSUA_DEFAULT_EC_TAIL_LEN
     */
    unsigned int		ecTailLen;

    /**
     * Audio capture buffer length, in milliseconds.
     *
     * Default: PJMEDIA_SND_DEFAULT_REC_LATENCY
     */
    unsigned int		sndRecLatency;

    /**
     * Audio playback buffer length, in milliseconds.
     *
     * Default: PJMEDIA_SND_DEFAULT_PLAY_LATENCY
     */
    unsigned int		sndPlayLatency;

    /**
     * Jitter buffer initial prefetch delay in msec. The value must be
     * between jb_min_pre and jb_max_pre below.
     *
     * Default: -1 (to use default stream settings, currently 150 msec)
     */
    int			jbInit;

    /**
     * Jitter buffer minimum prefetch delay in msec.
     *
     * Default: -1 (to use default stream settings, currently 60 msec)
     */
    int			jbMinPre;

    /**
     * Jitter buffer maximum prefetch delay in msec.
     *
     * Default: -1 (to use default stream settings, currently 240 msec)
     */
    int			jbMaxPre;

    /**
     * Set maximum delay that can be accomodated by the jitter buffer msec.
     *
     * Default: -1 (to use default stream settings, currently 360 msec)
     */
    int			jbMax;

    /**
     * Specify idle time of sound device before it is automatically closed,
     * in seconds. Use value -1 to disable the auto-close feature of sound
     * device
     *
     * Default : 1
     */
    int			sndAutoCloseTime;

    /**
     * Specify whether built-in/native preview should be used if available.
     * In some systems, video input devices have built-in capability to show
     * preview window of the device. Using this built-in preview is preferable
     * as it consumes less CPU power. If built-in preview is not available,
     * the library will perform software rendering of the input. If this
     * field is set to PJ_FALSE, software preview will always be used.
     *
     * Default: PJ_TRUE
     */
    bool		vidPreviewEnableNative;
}pjsip_media_config_t;



typedef struct pjsip_ep_config
{
    /** UA config */
	pjsip_ua_config_t		*uaConfig;

    /** Logging config */
    pjsip_log_config_t		*logConfig;

    /** Media config */
    pjsip_media_config_t	*medConfig;
}pjsip_ep_config_t;

/**
 * Credential information. Credential contains information to authenticate
 * against a service.
 */
typedef struct pjsip_auth_cred_info
{
    /**
     * The authentication scheme (e.g. "digest").
     */
    char*	scheme;

    /**
     * Realm on which this credential is to be used. Use "*" to make
     * a credential that can be used to authenticate against any challenges.
     */
    char*	realm;

    /**
     * Authentication user name.
     */
    char*	username;

    /**
     * Type of data that is contained in the "data" field. Use 0 if the data
     * contains plain text password.
     */
    int		dataType;

    /**
     * The data, which can be a plain text password or a hashed digest.
     */
    char*	data;

    /*
     * Digest AKA credential information. Note that when AKA credential
     * is being used, the \a data field of this pjsip_cred_info is
     * not used, but it still must be initialized to an empty string.
     * Please see PJSIP_AUTH_AKA_API for more information.
     */

    /** Permanent subscriber key. */
    char*	akaK;

    /** Operator variant key. */
    char*	akaOp;

    /** Authentication Management Field	*/
    char*	akaAmf;
}pjsip_auth_cred_info_t;


//////////////////////////////////////////////////////////////////////////////

/**
 * TLS transport settings, to be specified in TransportConfig.
 */
typedef struct pjsip_tls_config
{
    /**
     * Certificate of Authority (CA) list file.
     */
	char*		CaListFile;

    /**
     * Public endpoint certificate file, which will be used as client-
     * side  certificate for outgoing TLS connection, and server-side
     * certificate for incoming TLS connection.
     */
	char*		certFile;

    /**
     * Optional private key of the endpoint certificate to be used.
     */
	char*		privKeyFile;

    /**
     * Password to open private key.
     */
	char*		password;

    /**
     * Certificate of Authority (CA) buffer. If CaListFile, certFile or
     * privKeyFile are set, this setting will be ignored.
     */
	char*		CaBuf;

    /**
     * Public endpoint certificate buffer, which will be used as client-
     * side  certificate for outgoing TLS connection, and server-side
     * certificate for incoming TLS connection. If CaListFile, certFile or
     * privKeyFile are set, this setting will be ignored.
     */
	char*		certBuf;

    /**
     * Optional private key buffer of the endpoint certificate to be used.
     * If CaListFile, certFile or privKeyFile are set, this setting will
     * be ignored.
     */
	char*		privKeyBuf;

    /**
     * TLS protocol method from #pjsip_ssl_method. In the future, this field
     * might be deprecated in favor of <b>proto</b> field. For now, this field
     * is only applicable only when <b>proto</b> field is set to zero.
     *
     * Default is PJSIP_SSL_UNSPECIFIED_METHOD (0), which in turn will
     * use PJSIP_SSL_DEFAULT_METHOD, which default value is PJSIP_TLSV1_METHOD.
     */
    pjsip_ssl_method	method;

    /**
     * TLS protocol type from #pj_ssl_sock_proto. Use this field to enable
     * specific protocol type. Use bitwise OR operation to combine the protocol
     * type.
     *
     * Default is PJSIP_SSL_DEFAULT_PROTO.
     */
    unsigned int		proto;

    /**
     * Ciphers and order preference. The Endpoint::utilSslGetAvailableCiphers()
     * can be used to check the available ciphers supported by backend.
     * If the array is empty, then default cipher list of the backend
     * will be used.
     */
    pj_ssl_cipher		*ciphers;
    int ciphersCnt;
    /**
     * Specifies TLS transport behavior on the server TLS certificate
     * verification result:
     * - If \a verifyServer is disabled, TLS transport will just notify
     *   the application via pjsip_tp_state_callback with state
     *   PJSIP_TP_STATE_CONNECTED regardless TLS verification result.
     * - If \a verifyServer is enabled, TLS transport will be shutdown
     *   and application will be notified with state
     *   PJSIP_TP_STATE_DISCONNECTED whenever there is any TLS verification
     *   error, otherwise PJSIP_TP_STATE_CONNECTED will be notified.
     *
     * In any cases, application can inspect pjsip_tls_state_info in the
     * callback to see the verification detail.
     *
     * Default value is false.
     */
    bool		verifyServer;

    /**
     * Specifies TLS transport behavior on the client TLS certificate
     * verification result:
     * - If \a verifyClient is disabled, TLS transport will just notify
     *   the application via pjsip_tp_state_callback with state
     *   PJSIP_TP_STATE_CONNECTED regardless TLS verification result.
     * - If \a verifyClient is enabled, TLS transport will be shutdown
     *   and application will be notified with state
     *   PJSIP_TP_STATE_DISCONNECTED whenever there is any TLS verification
     *   error, otherwise PJSIP_TP_STATE_CONNECTED will be notified.
     *
     * In any cases, application can inspect pjsip_tls_state_info in the
     * callback to see the verification detail.
     *
     * Default value is PJ_FALSE.
     */
    bool		verifyClient;

    /**
     * When acting as server (incoming TLS connections), reject incoming
     * connection if client doesn't supply a TLS certificate.
     *
     * This setting corresponds to SSL_VERIFY_FAIL_IF_NO_PEER_CERT flag.
     * Default value is PJ_FALSE.
     */
    bool		requireClientCert;

    /**
     * TLS negotiation timeout to be applied for both outgoing and incoming
     * connection, in milliseconds. If zero, the SSL negotiation doesn't
     * have a timeout.
     *
     * Default: zero
     */
    unsigned int		msecTimeout;

    /**
     * QoS traffic type to be set on this transport. When application wants
     * to apply QoS tagging to the transport, it's preferable to set this
     * field rather than \a qosParam fields since this is more portable.
     *
     * Default value is PJ_QOS_TYPE_BEST_EFFORT.
     */
    pj_qos_type 	qosType;

    /**
     * Set the low level QoS parameters to the transport. This is a lower
     * level operation than setting the \a qosType field and may not be
     * supported on all platforms.
     *
     * By default all settings in this structure are disabled.
     */
    pj_qos_params 	qosParams;

    /**
     * Specify if the transport should ignore any errors when setting the QoS
     * traffic type/parameters.
     *
     * Default: PJ_TRUE
     */
    bool		qosIgnoreError;
}pjsip_tls_config_t;


/**
 * Parameters to create a transport instance.
 */
typedef struct pjsip_transport_config
{
    /**
     * UDP port number to bind locally. This setting MUST be specified
     * even when default port is desired. If the value is zero, the
     * transport will be bound to any available port, and application
     * can query the port by querying the transport info.
     */
    unsigned int		port;

    /**
     * Specify the port range for socket binding, relative to the start
     * port number specified in \a port. Note that this setting is only
     * applicable when the start port number is non zero.
     *
     * Default value is zero.
     */
    unsigned int		portRange;

    /**
     * Optional address to advertise as the address of this transport.
     * Application can specify any address or hostname for this field,
     * for example it can point to one of the interface address in the
     * system, or it can point to the public address of a NAT router
     * where port mappings have been configured for the application.
     *
     * Note: this option can be used for both UDP and TCP as well!
     */
    char*		publicAddress;

    /**
     * Optional address where the socket should be bound to. This option
     * SHOULD only be used to selectively bind the socket to particular
     * interface (instead of 0.0.0.0), and SHOULD NOT be used to set the
     * published address of a transport (the public_addr field should be
     * used for that purpose).
     *
     * Note that unlike public_addr field, the address (or hostname) here
     * MUST correspond to the actual interface address in the host, since
     * this address will be specified as bind() argument.
     */
    char*		boundAddress;

    /**
     * This specifies TLS settings for TLS transport. It is only be used
     * when this transport config is being used to create a SIP TLS
     * transport.
     */
    pjsip_tls_config_t		*tlsConfig;

    /**
     * QoS traffic type to be set on this transport. When application wants
     * to apply QoS tagging to the transport, it's preferable to set this
     * field rather than \a qosParam fields since this is more portable.
     *
     * Default is QoS not set.
     */
    pj_qos_type		qosType;

    /**
     * Set the low level QoS parameters to the transport. This is a lower
     * level operation than setting the \a qosType field and may not be
     * supported on all platforms.
     *
     * Default is QoS not set.
     */
    pj_qos_params	qosParams;
}pjsip_transport_config_t;















typedef struct pjsip_account_reg_config
{
    /**
     * This is the URL to be put in the request URI for the registration,
     * and will look something like "sip:serviceprovider".
     *
     * This field should be specified if registration is desired. If the
     * value is empty, no account registration will be performed.
     */
    char		*registrarUri;

    /**
     * Specify whether the account should register as soon as it is
     * added to the UA. Application can set this to PJ_FALSE and control
     * the registration manually with pjsua_acc_set_registration().
     *
     * Default: True
     */
    bool		registerOnAdd;
    /**
     * The optional custom SIP headers to be put in the registration
     * request.
     */
    //SipHeaderVector	headers;

    /**
     * Additional parameters that will be appended in the Contact header
     * of the registration requests. This will be appended after
     * \a AccountSipConfig.contactParams;
     *
     * The parameters should be preceeded by semicolon, and all strings must
     * be properly escaped. Example:
     *	 ";my-param=X;another-param=Hi%20there"
     */
    char*	    	contactParams;
    /**
     * Optional interval for registration, in seconds. If the value is zero,
     * default interval will be used (PJSUA_REG_INTERVAL, 300 seconds).
     */
    unsigned int		timeoutSec;

    /**
     * Specify interval of auto registration retry upon registration failure
     * (including caused by transport problem), in second. Set to 0 to
     * disable auto re-registration. Note that if the registration retry
     * occurs because of transport failure, the first retry will be done
     * after \a firstRetryIntervalSec seconds instead. Also note that
     * the interval will be randomized slightly by some seconds (specified
     * in \a reg_retry_random_interval) to avoid all clients re-registering
     * at the same time.
     *
     * See also \a firstRetryIntervalSec and \a randomRetryIntervalSec
     * settings.
     *
     * Default: PJSUA_REG_RETRY_INTERVAL
     */
    unsigned int		retryIntervalSec;

    /**
     * This specifies the interval for the first registration retry. The
     * registration retry is explained in \a retryIntervalSec. Note that
     * the value here will also be randomized by some seconds (specified
     * in \a reg_retry_random_interval) to avoid all clients re-registering
     * at the same time.
     *
     * See also \a retryIntervalSec and \a randomRetryIntervalSec settings.
     *
     * Default: 0
     */
    unsigned int		firstRetryIntervalSec;

    /**
     * This specifies maximum randomized value to be added/substracted
     * to/from the registration retry interval specified in \a
     * reg_retry_interval and \a reg_first_retry_interval, in second.
     * This is useful to avoid all clients re-registering at the same time.
     * For example, if the registration retry interval is set to 100 seconds
     * and this is set to 10 seconds, the actual registration retry interval
     * will be in the range of 90 to 110 seconds.
     *
     * See also \a retryIntervalSec and \a firstRetryIntervalSec settings.
     *
     * Default: 10
     */
    unsigned int		randomRetryIntervalSec;

    /**
     * Specify the number of seconds to refresh the client registration
     * before the registration expires.
     *
     * Default: PJSIP_REGISTER_CLIENT_DELAY_BEFORE_REFRESH, 5 seconds
     */
    unsigned int		delayBeforeRefreshSec;

    /**
     * Specify whether calls of the configured account should be dropped
     * after registration failure and an attempt of re-registration has
     * also failed.
     *
     * Default: FALSE (disabled)
     */
    bool		dropCallsOnFail;

    /**
     * Specify the maximum time to wait for unregistration requests to
     * complete during library shutdown sequence.
     *
     * Default: PJSUA_UNREG_TIMEOUT
     */
    unsigned int		unregWaitMsec;

    /**
     * Specify how the registration uses the outbound and account proxy
     * settings. This controls if and what Route headers will appear in
     * the REGISTER request of this account. The value is bitmask combination
     * of PJSUA_REG_USE_OUTBOUND_PROXY and PJSUA_REG_USE_ACC_PROXY bits.
     * If the value is set to 0, the REGISTER request will not use any proxy
     * (i.e. it will not have any Route headers).
     *
     * Default: 3 (PJSUA_REG_USE_OUTBOUND_PROXY | PJSUA_REG_USE_ACC_PROXY)
     */
    unsigned int		proxyUse;

}pjsip_account_reg_config_t;




/**
 * Various SIP settings for the account. This will be specified in
 * AccountConfig.
 */
typedef struct pjsip_account_sip_config
{
    /**
     * Array of credentials. If registration is desired, normally there should
     * be at least one credential specified, to successfully authenticate
     * against the service provider. More credentials can be specified, for
     * example when the requests are expected to be challenged by the
     * proxies in the route set.
     */
	pjsip_auth_cred_info_t	*authCreds;

    /**
     * Array of proxy servers to visit for outgoing requests. Each of the
     * entry is translated into one Route URI.
     */
    char*	proxies[PJSUA_ACC_MAX_PROXIES];
    int proxiesCnt;

    /**
     * Optional URI to be put as Contact for this account. It is recommended
     * that this field is left empty, so that the value will be calculated
     * automatically based on the transport address.
     */
    char*		contactForced;

    /**
     * Additional parameters that will be appended in the Contact header
     * for this account. This will affect the Contact header in all SIP
     * messages sent on behalf of this account, including but not limited to
     * REGISTER, INVITE, and SUBCRIBE requests or responses.
     *
     * The parameters should be preceeded by semicolon, and all strings must
     * be properly escaped. Example:
     *	 ";my-param=X;another-param=Hi%20there"
     */
    char*		contactParams;

    /**
     * Additional URI parameters that will be appended in the Contact URI
     * for this account. This will affect the Contact URI in all SIP
     * messages sent on behalf of this account, including but not limited to
     * REGISTER, INVITE, and SUBCRIBE requests or responses.
     *
     * The parameters should be preceeded by semicolon, and all strings must
     * be properly escaped. Example:
     *	 ";my-param=X;another-param=Hi%20there"
     */
    char*		contactUriParams;


    /**
     * If this flag is set, the authentication client framework will
     * send an empty Authorization header in each initial request.
     * Default is no.
     */
    bool		authInitialEmpty;

    /**
     * Specify the algorithm to use when empty Authorization header
     * is to be sent for each initial request (see above)
     */
    char*		authInitialAlgorithm;

    /**
     * Optionally bind this account to specific transport. This normally is
     * not a good idea, as account should be able to send requests using
     * any available transports according to the destination. But some
     * application may want to have explicit control over the transport to
     * use, so in that case it can set this field.
     *
     * Default: -1 (PJSUA_INVALID_ID)
     *
     * @see Account::setTransport()
     */
    int		transportId;

}pjsip_account_sip_config_t;

/**
 * Account's call settings. This will be specified in AccountConfig.
 */
typedef struct pjsip_account_call_config
{
    /**
     * Specify how to offer call hold to remote peer. Please see the
     * documentation on pjsua_call_hold_type for more info.
     *
     * Default: PJSUA_CALL_HOLD_TYPE_DEFAULT
     */
    pjsua_call_hold_type holdType;

    /**
     * Specify how support for reliable provisional response (100rel/
     * PRACK) should be used for all sessions in this account. See the
     * documentation of pjsua_100rel_use enumeration for more info.
     *
     * Default: PJSUA_100REL_NOT_USED
     */
    pjsua_100rel_use	prackUse;

    /**
     * Specify the usage of Session Timers for all sessions. See the
     * pjsua_sip_timer_use for possible values.
     *
     * Default: PJSUA_SIP_TIMER_OPTIONAL
     */
    pjsua_sip_timer_use	timerUse;

    /**
     * Specify minimum Session Timer expiration period, in seconds.
     * Must not be lower than 90. Default is 90.
     */
    unsigned int		timerMinSESec;

    /**
     * Specify Session Timer expiration period, in seconds.
     * Must not be lower than timerMinSE. Default is 1800.
     */
    unsigned int		timerSessExpiresSec;

}pjsip_account_call_config_t;

/**
 * Account presence config. This will be specified in AccountConfig.
 */
typedef struct pjsip_account_pres_config
{
    /**
     * The optional custom SIP headers to be put in the presence
     * subscription request.
     */
    //SipHeaderVector	headers;

    /**
     * If this flag is set, the presence information of this account will
     * be PUBLISH-ed to the server where the account belongs.
     *
     * Default: PJ_FALSE
     */
    bool		publishEnabled;

    /**
     * Specify whether the client publication session should queue the
     * PUBLISH request should there be another PUBLISH transaction still
     * pending. If this is set to false, the client will return error
     * on the PUBLISH request if there is another PUBLISH transaction still
     * in progress.
     *
     * Default: PJSIP_PUBLISHC_QUEUE_REQUEST (TRUE)
     */
    bool		publishQueue;

    /**
     * Maximum time to wait for unpublication transaction(s) to complete
     * during shutdown process, before sending unregistration. The library
     * tries to wait for the unpublication (un-PUBLISH) to complete before
     * sending REGISTER request to unregister the account, during library
     * shutdown process. If the value is set too short, it is possible that
     * the unregistration is sent before unpublication completes, causing
     * unpublication request to fail.
     *
     * Value is in milliseconds.
     *
     * Default: PJSUA_UNPUBLISH_MAX_WAIT_TIME_MSEC (2000)
     */
    unsigned int		publishShutdownWaitMsec;

    /**
     * Optional PIDF tuple ID for outgoing PUBLISH and NOTIFY. If this value
     * is not specified, a random string will be used.
     */
    char*		pidfTupleId;
}pjsip_account_pres_config_t;

/**
 * Account MWI (Message Waiting Indication) settings. This will be specified
 * in AccountConfig.
 */
typedef struct pjsip_account_mwi_config
{
    /**
     * Subscribe to message waiting indication events (RFC 3842).
     *
     * See also UaConfig.mwiUnsolicitedEnabled setting.
     *
     * Default: FALSE
     */
    bool		enabled;

    /**
     * Specify the default expiration time (in seconds) for Message
     * Waiting Indication (RFC 3842) event subscription. This must not
     * be zero.
     *
     * Default: PJSIP_MWI_DEFAULT_EXPIRES (3600)
     */
    unsigned int		expirationSec;
}pjsip_account_mwi_config_t;

/**
 * Account's NAT (Network Address Translation) settings. This will be
 * specified in AccountConfig.
 */
typedef struct pjsip_account_nat_config
{
    /**
     * Control the use of STUN for the SIP signaling.
     *
     * Default: PJSUA_STUN_USE_DEFAULT
     */
    pjsua_stun_use 	sipStunUse;

    /**
     * Control the use of STUN for the media transports.
     *
     * Default: PJSUA_STUN_USE_DEFAULT
     */
    pjsua_stun_use 	mediaStunUse;

    /**
     * Specify NAT64 options.
     *
     * Default: PJSUA_NAT64_DISABLED
     */
    pjsua_nat64_opt 	nat64Opt;

    /**
     * Enable ICE for the media transport.
     *
     * Default: False
     */
    bool		iceEnabled;

    /**
     * Set the maximum number of ICE host candidates.
     *
     * Default: -1 (maximum not set)
     */
    int			iceMaxHostCands;

    /**
     * Specify whether to use aggressive nomination.
     *
     * Default: True
     */
    bool		iceAggressiveNomination;

    /**
     * For controlling agent if it uses regular nomination, specify the delay
     * to perform nominated check (connectivity check with USE-CANDIDATE
     * attribute) after all components have a valid pair.
     *
     * Default value is PJ_ICE_NOMINATED_CHECK_DELAY.
     */
    unsigned int		iceNominatedCheckDelayMsec;

    /**
     * For a controlled agent, specify how long it wants to wait (in
     * milliseconds) for the controlling agent to complete sending
     * connectivity check with nominated flag set to true for all components
     * after the controlled agent has found that all connectivity checks in
     * its checklist have been completed and there is at least one successful
     * (but not nominated) check for every component.
     *
     * Default value for this option is
     * ICE_CONTROLLED_AGENT_WAIT_NOMINATION_TIMEOUT. Specify -1 to disable
     * this timer.
     */
    int			iceWaitNominationTimeoutMsec;

    /**
     * Disable RTCP component.
     *
     * Default: False
     */
    bool		iceNoRtcp;

    /**
     * Always send re-INVITE/UPDATE after ICE negotiation regardless of whether
     * the default ICE transport address is changed or not. When this is set
     * to False, re-INVITE/UPDATE will be sent only when the default ICE
     * transport address is changed.
     *
     * Default: yes
     */
    bool		iceAlwaysUpdate;

    /**
     * Enable TURN candidate in ICE.
     */
    bool		turnEnabled;

    /**
     * Specify TURN domain name or host name, in in "DOMAIN:PORT" or
     * "HOST:PORT" format.
     */
    char*		turnServer;

    /**
     * Specify the connection type to be used to the TURN server. Valid
     * values are PJ_TURN_TP_UDP or PJ_TURN_TP_TCP.
     *
     * Default: PJ_TURN_TP_UDP
     */
    pj_turn_tp_type	turnConnType;

    /**
     * Specify the username to authenticate with the TURN server.
     */
    char*		turnUserName;

    /**
     * Specify the type of password. Currently this must be zero to
     * indicate plain-text password will be used in the password.
     */
    int			turnPasswordType;

    /**
     * Specify the password to authenticate with the TURN server.
     */
    char*		turnPassword;

    /**
     * This option is used to update the transport address and the Contact
     * header of REGISTER request. When this option is  enabled, the library
     * will keep track of the public IP address from the response of REGISTER
     * request. Once it detects that the address has changed, it will
     * unregister current Contact, update the Contact with transport address
     * learned from Via header, and register a new Contact to the registrar.
     * This will also update the public name of UDP transport if STUN is
     * configured.
     *
     * See also contactRewriteMethod field.
     *
     * Default: TRUE
     */
    int			contactRewriteUse;

    /**
     * Specify how Contact update will be done with the registration, if
     * \a contactRewriteEnabled is enabled. The value is bitmask combination of
     * \a pjsua_contact_rewrite_method. See also pjsua_contact_rewrite_method.
     *
     * Value PJSUA_CONTACT_REWRITE_UNREGISTER(1) is the legacy behavior.
     *
     * Default value: PJSUA_CONTACT_REWRITE_METHOD
     *   (PJSUA_CONTACT_REWRITE_NO_UNREG | PJSUA_CONTACT_REWRITE_ALWAYS_UPDATE)
     */
    int			contactRewriteMethod;

    /**
     * Specify if source TCP port should be used as the initial Contact
     * address if TCP/TLS transport is used. Note that this feature will
     * be automatically turned off when nameserver is configured because
     * it may yield different destination address due to DNS SRV resolution.
     * Also some platforms are unable to report the local address of the
     * TCP socket when it is still connecting. In these cases, this
     * feature will also be turned off.
     *
     * Default: 1 (yes).
     */
    int			contactUseSrcPort;

    /**
     * This option is used to overwrite the "sent-by" field of the Via header
     * for outgoing messages with the same interface address as the one in
     * the REGISTER request, as long as the request uses the same transport
     * instance as the previous REGISTER request.
     *
     * Default: TRUE
     */
    int			viaRewriteUse;

    /**
     * This option controls whether the IP address in SDP should be replaced
     * with the IP address found in Via header of the REGISTER response, ONLY
     * when STUN and ICE are not used. If the value is FALSE (the original
     * behavior), then the local IP address will be used. If TRUE, and when
     * STUN and ICE are disabled, then the IP address found in registration
     * response will be used.
     *
     * Default: PJ_FALSE (no)
     */
    int			sdpNatRewriteUse;

    /**
     * Control the use of SIP outbound feature. SIP outbound is described in
     * RFC 5626 to enable proxies or registrar to send inbound requests back
     * to UA using the same connection initiated by the UA for its
     * registration. This feature is highly useful in NAT-ed deployemtns,
     * hence it is enabled by default.
     *
     * Note: currently SIP outbound can only be used with TCP and TLS
     * transports. If UDP is used for the registration, the SIP outbound
     * feature will be silently ignored for the account.
     *
     * Default: TRUE
     */
    int			sipOutboundUse;

    /**
     * Specify SIP outbound (RFC 5626) instance ID to be used by this
     * account. If empty, an instance ID will be generated based on
     * the hostname of this agent. If application specifies this parameter, the
     * value will look like "<urn:uuid:00000000-0000-1000-8000-AABBCCDDEEFF>"
     * without the double-quotes.
     *
     * Default: empty
     */
    char*		sipOutboundInstanceId;

    /**
     * Specify SIP outbound (RFC 5626) registration ID. The default value
     * is empty, which would cause the library to automatically generate
     * a suitable value.
     *
     * Default: empty
     */
    char*		sipOutboundRegId;

    /**
     * Set the interval for periodic keep-alive transmission for this account.
     * If this value is zero, keep-alive will be disabled for this account.
     * The keep-alive transmission will be sent to the registrar's address,
     * after successful registration.
     *
     * Default: 15 (seconds)
     */
    unsigned int		udpKaIntervalSec;

    /**
     * Specify the data to be transmitted as keep-alive packets.
     *
     * Default: CR-LF
     */
    char*		udpKaData;
}pjsip_account_nat_config_t;

/**
 * SRTP crypto.
 */
typedef struct pjsip_srtp_crypto
{
    /**
     * Optional key. If empty, a random key will be autogenerated.
     */
    char*	key;

    /**
     * Crypto name.
     */
    char*	name;

    /**
     * Flags, bitmask from #pjmedia_srtp_crypto_option
     */
    unsigned int	flags;
}pjsip_srtp_crypto_t;

/* Array of SRTP cryptos. */

/**
 * SRTP settings.
 */
typedef struct pjsip_srtp_option
{
    /**
     * Specify SRTP cryptos. If empty, all crypto will be enabled.
     * Available crypto can be enumerated using Endpoint::srtpCryptoEnum().
     *
     * Default: empty.
     */
	pjsip_srtp_crypto_t *		cryptos;
	int cryptos_num;
    /**
     * Specify SRTP keying methods, valid keying method is defined in
     * pjmedia_srtp_keying_method. If empty, all keying methods will be
     * enabled with priority order: SDES, DTLS-SRTP.
     *
     * Default: empty.
     */
    int*			keyings;
    int keying_num;
}pjsip_srtp_option_t;

/**
 * RTCP Feedback capability.
 */
typedef struct pjsip_rtcp_fb_cap
{
    /**
     * Specify the codecs to which the capability is applicable. Codec ID is
     * using the same format as in pjmedia_codec_mgr_find_codecs_by_id() and
     * pjmedia_vid_codec_mgr_find_codecs_by_id(), e.g: "L16/8000/1", "PCMU",
     * "H264". This can also be an asterisk ("*") to represent all codecs.
     */
    char*		    codecId;

    /**
     * Specify the RTCP Feedback type.
     */
    pjmedia_rtcp_fb_type    type;

    /**
     * Specify the type name if RTCP Feedback type is PJMEDIA_RTCP_FB_OTHER.
     */
    char*		    typeName;

    /**
     * Specify the RTCP Feedback parameters.
     */
    char*		    param;
}pjsip_rtcp_fb_cap_t;

/**
 * RTCP Feedback settings.
 */
typedef struct pjsip_rtcp_fb_config
{
    /**
     * Specify whether transport protocol in SDP media description uses
     * RTP/AVP instead of RTP/AVPF. Note that the standard mandates to signal
     * AVPF profile, but it may cause SDP negotiation failure when negotiating
     * with endpoints that does not support RTCP Feedback (including older
     * version of PJSIP).
     *
     * Default: false.
     */
    bool		    dontUseAvpf;

    /**
     * RTCP Feedback capabilities.
     */
    pjsip_rtcp_fb_cap_t*	    caps;
}pjsip_rtcp_fb_config_t;

/**
 * Account media config (applicable for both audio and video). This will be
 * specified in AccountConfig.
 */
typedef struct pjsip_account_media_config
{
    /**
     * Media transport (RTP) configuration.
     */
	pjsip_transport_config_t	*transportConfig;

    /**
     * If remote sends SDP answer containing more than one format or codec in
     * the media line, send re-INVITE or UPDATE with just one codec to lock
     * which codec to use.
     *
     * Default: True (Yes).
     */
    bool		lockCodecEnabled;

    /**
     * Specify whether stream keep-alive and NAT hole punching with
     * non-codec-VAD mechanism (see PJMEDIA_STREAM_ENABLE_KA) is enabled
     * for this account.
     *
     * Default: False
     */
    bool		streamKaEnabled;

    /**
     * Specify whether secure media transport should be used for this account.
     * Valid values are PJMEDIA_SRTP_DISABLED, PJMEDIA_SRTP_OPTIONAL, and
     * PJMEDIA_SRTP_MANDATORY.
     *
     * Default: PJSUA_DEFAULT_USE_SRTP
     */
    pjmedia_srtp_use	srtpUse;

    /**
     * Specify whether SRTP requires secure signaling to be used. This option
     * is only used when \a use_srtp option above is non-zero.
     *
     * Valid values are:
     *	0: SRTP does not require secure signaling
     *	1: SRTP requires secure transport such as TLS
     *	2: SRTP requires secure end-to-end transport (SIPS)
     *
     * Default: PJSUA_DEFAULT_SRTP_SECURE_SIGNALING
     */
    int			srtpSecureSignaling;

    /**
     * Specify SRTP settings, like cryptos and keying methods.
     */
    pjsip_srtp_option_t		*srtpOpt;

    /**
     * Specify whether IPv6 should be used on media. Default is not used.
     */
    pjsua_ipv6_use	ipv6Use;

    /**
     * Enable RTP and RTCP multiplexing.
     */
    bool		rtcpMuxEnabled;

    /**
     * RTCP Feedback settings.
     */
    pjsip_rtcp_fb_config_t	*rtcpFbConfig;

}pjsip_account_media_config_t;

/**
 * Account video config. This will be specified in AccountConfig.
 */
typedef struct pjsip_account_video_config
{
    /**
     * Specify whether incoming video should be shown to screen by default.
     * This applies to incoming call (INVITE), incoming re-INVITE, and
     * incoming UPDATE requests.
     *
     * Regardless of this setting, application can detect incoming video
     * by implementing \a on_call_media_state() callback and enumerating
     * the media stream(s) with pjsua_call_get_info(). Once incoming
     * video is recognised, application may retrieve the window associated
     * with the incoming video and show or hide it with
     * pjsua_vid_win_set_show().
     *
     * Default: False
     */
    bool			autoShowIncoming;

    /**
     * Specify whether outgoing video should be activated by default when
     * making outgoing calls and/or when incoming video is detected. This
     * applies to incoming and outgoing calls, incoming re-INVITE, and
     * incoming UPDATE. If the setting is non-zero, outgoing video
     * transmission will be started as soon as response to these requests
     * is sent (or received).
     *
     * Regardless of the value of this setting, application can start and
     * stop outgoing video transmission with pjsua_call_set_vid_strm().
     *
     * Default: False
     */
    bool			autoTransmitOutgoing;

    /**
     * Specify video window's flags. The value is a bitmask combination of
     * pjmedia_vid_dev_wnd_flag.
     *
     * Default: 0
     */
    unsigned int			windowFlags;

    /**
     * Specify the default capture device to be used by this account. If
     * vidOutAutoTransmit is enabled, this device will be used for
     * capturing video.
     *
     * Default: PJMEDIA_VID_DEFAULT_CAPTURE_DEV
     */
    pjmedia_vid_dev_index 	defaultCaptureDevice;

    /**
     * Specify the default rendering device to be used by this account.
     *
     * Default: PJMEDIA_VID_DEFAULT_RENDER_DEV
     */
    pjmedia_vid_dev_index 	defaultRenderDevice;

    /**
     * Rate control method.
     *
     * Default: PJMEDIA_VID_STREAM_RC_SIMPLE_BLOCKING.
     */
    pjmedia_vid_stream_rc_method rateControlMethod;

    /**
     * Upstream/outgoing bandwidth. If this is set to zero, the video stream
     * will use codec maximum bitrate setting.
     *
     * Default: 0 (follow codec maximum bitrate).
     */
    unsigned int			rateControlBandwidth;

    /**
     * The number of keyframe to be sent after the stream is created.
     *
     * Default: PJMEDIA_VID_STREAM_START_KEYFRAME_CNT
     */
    unsigned int			    startKeyframeCount;

    /**
     * The keyframe sending interval after the stream is created.
     *
     * Default: PJMEDIA_VID_STREAM_START_KEYFRAME_INTERVAL_MSEC
     */
    unsigned int			    startKeyframeInterval;

}pjsip_account_video_config_t;

/**
 * Account config specific to IP address change.
 */
typedef struct pjsip_account_IpChange_config
{
    /**
     * Shutdown the transport used for account registration. If this is set to
     * PJ_TRUE, the transport will be shutdown altough it's used by multiple
     * account. Shutdown transport will be followed by re-Registration if
     * AccountConfig.natConfig.contactRewriteUse is enabled.
     *
     * Default: true
     */
    bool    		shutdownTp;

    /**
     * Hangup active calls associated with the acount. If this is set to true,
     * then the calls will be hang up.
     *
     * Default: false
     */
    bool		hangupCalls;

    /**
     * Specify the call flags used in the re-INVITE when \a hangupCalls is set
     * to false. If this is set to 0, no re-INVITE will be sent. The
     * re-INVITE will be sent after re-Registration is finished.
     *
     * Default: PJSUA_CALL_REINIT_MEDIA | PJSUA_CALL_UPDATE_CONTACT |
     *          PJSUA_CALL_UPDATE_VIA
     */
    unsigned int		reinviteFlags;

} pjsip_account_IpChange_config_t;

/**
 * Account configuration.
 */
typedef struct pjsip_account_config
{
    /**
     * Account priority, which is used to control the order of matching
     * incoming/outgoing requests. The higher the number means the higher
     * the priority is, and the account will be matched first.
     */
    int			priority;

    /**
     * The Address of Record or AOR, that is full SIP URL that identifies the
     * account. The value can take name address or URL format, and will look
     * something like "sip:account@serviceprovider".
     *
     * This field is mandatory.
     */
    char*		idUri;

    /**
     * Registration settings.
     */
    pjsip_account_reg_config_t	*regConfig;

    /**
     * SIP settings.
     */
    pjsip_account_sip_config_t	*sipConfig;

    /**
     * Call settings.
     */
    pjsip_account_call_config_t	*callConfig;

    /**
     * Presence settings.
     */
    pjsip_account_pres_config_t	*presConfig;

    /**
     * MWI (Message Waiting Indication) settings.
     */
    pjsip_account_mwi_config_t	*mwiConfig;

    /**
     * NAT settings.
     */
    pjsip_account_nat_config_t	*natConfig;

    /**
     * Media settings (applicable for both audio and video).
     */
    pjsip_account_media_config_t	*mediaConfig;

    /**
     * Video settings.
     */
    pjsip_account_video_config_t	*videoConfig;

    /**
     * IP Change settings.
     */
    pjsip_account_IpChange_config_t *ipChangeConfig;

}pjsip_account_config_t;



typedef struct pjsip_config
{
	pjsip_ep_config_t	endpoint;
	pjsip_account_config_t	pjsip_account_table[8];
	int table_cnt;

}pjsip_config_t;


extern int pjsip_config_load(char *filename, pjsip_config_t *ua);
extern int pjsip_config_write(char *filename, pjsip_config_t *ua);


#endif /* __PJSIP_JSONCFG_H__ */
