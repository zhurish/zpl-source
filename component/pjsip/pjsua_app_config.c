/* $Id: pjsua_app_config.c 5788 2018-05-09 06:58:48Z ming $ */
/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "pjsua_app_common.h"
#include "pjsua_app_config.h"
#include "pjsip_app_api.h"

#define THIS_FILE	"pjsua_app_config.c"

#define MAX_APP_OPTIONS 128

//char   *stdout_refresh_text = "STDOUT_REFRESH";

#if 0
#define str(s) #s
#define xstr(s) str(s)

char *stdout_refresh_text = "STDOUT_REFRESH";

/* Show usage */
static void usage(void)
{
	puts("Usage:");
	puts("  pjsua [options] [SIP URL to call]");
	puts("");
	puts("General options:");
	puts("  --config-file=file  Read the config/arguments from file.");
	puts("  --help              Display this help screen");
	puts("  --version           Display version info");
	puts("");
	puts("Logging options:");
	puts("  --log-file=fname    Log to filename (default stderr)");
	puts(
			"  --log-level=N       Set log max level to N (0(none) to 6(trace)) (default=5)");
	puts(
			"  --app-log-level=N   Set log max level for stdout display (default=4)");
	puts(
			"  --log-append        Append instead of overwrite existing log file.\n");
	puts("  --color             Use colorful logging (default yes on Win32)");
	puts("  --no-color          Disable colorful logging");
	puts(
			"  --light-bg          Use dark colors for light background (default is dark bg)");
	puts("  --no-stderr         Disable stderr");

	puts("");
	puts("SIP Account options:");
	puts("  --registrar=url     Set the URL of registrar server");
	puts("  --id=url            Set the URL of local ID (used in From header)");
	puts("  --realm=string      Set realm");
	puts("  --username=string   Set authentication username");
	puts("  --password=string   Set authentication password");
	puts("  --contact=url       Optionally override the Contact information");
	puts(
			"  --contact-params=S  Append the specified parameters S in Contact header");
	puts(
			"  --contact-uri-params=S  Append the specified parameters S in Contact URI");
	puts("  --proxy=url         Optional URL of proxy server to visit");
	puts("                      May be specified multiple times");
	printf(
			"  --reg-timeout=SEC   Optional registration interval (default %d)\n",
			PJSUA_REG_INTERVAL);
	printf(
			"  --rereg-delay=SEC   Optional auto retry registration interval (default %d)\n",
			PJSUA_REG_RETRY_INTERVAL);
	puts(
			"  --reg-use-proxy=N   Control the use of proxy settings in REGISTER.");
	puts(
			"                      0=no proxy, 1=outbound only, 2=acc only, 3=all (default)");
	puts("  --publish           Send presence PUBLISH for this account");
	puts(
			"  --mwi               Subscribe to message summary/waiting indication");
	puts(
			"  --use-ims           Enable 3GPP/IMS related settings on this account");
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	puts(
			"  --use-srtp=N        Use SRTP?  0:disabled, 1:optional, 2:mandatory,");
	puts("                      3:optional by duplicating media offer (def:0)");
	puts(
			"  --srtp-secure=N     SRTP require secure SIP? 0:no, 1:tls, 2:sips (def:1)");
#endif
	puts(
			"  --use-100rel        Require reliable provisional response (100rel)");
	puts("  --use-timer=N       Use SIP session timers? (default=1)");
	puts("                      0:inactive, 1:optional, 2:mandatory, 3:always");
	printf(
			"  --timer-se=N        Session timers expiration period, in secs (def:%d)\n",
			PJSIP_SESS_TIMER_DEF_SE);
	puts(
			"  --timer-min-se=N    Session timers minimum expiration period, in secs (def:90)");
	puts("  --outb-rid=string   Set SIP outbound reg-id (default:1)");
	puts(
			"  --auto-update-nat=N Where N is 0 or 1 to enable/disable SIP traversal behind");
	puts("                      symmetric NAT (default 1)");
	puts("  --disable-stun      Disable STUN for this account");
	puts("  --next-cred         Add another credentials");
	puts("");
	puts("SIP Account Control:");
	puts("  --next-account      Add more account");
	puts("");
	puts("Transport Options:");
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	puts ("  --ipv6              Use IPv6 instead for SIP and media.");
#endif
	puts("  --set-qos           Enable QoS tagging for SIP and media.");
	puts(
			"  --local-port=port   Set TCP/UDP port. This implicitly enables both ");
	puts(
			"                      TCP and UDP transports on the specified port, unless");
	puts("                      if TCP or UDP is disabled.");
	puts(
			"  --ip-addr=IP        Use the specifed address as SIP and RTP addresses.");
	puts(
			"                      (Hint: the IP may be the public IP of the NAT/router)");
	puts("  --bound-addr=IP     Bind transports to this IP interface");
	puts("  --no-tcp            Disable TCP transport.");
	puts("  --no-udp            Disable UDP transport.");
	puts(
			"  --nameserver=NS     Add the specified nameserver to enable SRV resolution");
	puts("                      This option can be specified multiple times.");
	puts("  --outbound=url      Set the URL of global outbound proxy server");
	puts("                      May be specified multiple times");
	puts(
			"  --stun-srv=FORMAT   Set STUN server host or domain. This option may be");
	puts(
			"                      specified more than once. FORMAT is hostdom[:PORT]");

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	puts ("");
	puts ("TLS Options:");
	puts ("  --use-tls           Enable TLS transport (default=no)");
	puts ("  --tls-ca-file       Specify TLS CA file (default=none)");
	puts ("  --tls-cert-file     Specify TLS certificate file (default=none)");
	puts ("  --tls-privkey-file  Specify TLS private key file (default=none)");
	puts ("  --tls-password      Specify TLS password to private key file (default=none)");
	puts ("  --tls-verify-server Verify server's certificate (default=no)");
	puts ("  --tls-verify-client Verify client's certificate (default=no)");
	puts ("  --tls-neg-timeout   Specify TLS negotiation timeout (default=no)");
	puts ("  --tls-cipher        Specify prefered TLS cipher (optional).");
	puts ("                      May be specified multiple times");
#endif

	puts("");
	puts("Audio Options:");
	puts("  --add-codec=name    Manually add codec (default is to enable all)");
	puts(
			"  --dis-codec=name    Disable codec (can be specified multiple times)");
	puts("  --clock-rate=N      Override conference bridge clock rate");
	puts("  --snd-clock-rate=N  Override sound device clock rate");
	puts(
			"  --stereo            Audio device and conference bridge opened in stereo mode");
	puts("  --null-audio        Use NULL audio device");
	puts("  --play-file=file    Register WAV file in conference bridge.");
	puts("                      This can be specified multiple times.");
	puts("  --play-tone=FORMAT  Register tone to the conference bridge.");
	puts("                      FORMAT is 'F1,F2,ON,OFF', where F1,F2 are");
	puts(
			"                      frequencies, and ON,OFF=on/off duration in msec.");
	puts("                      This can be specified multiple times.");
	puts(
			"  --auto-play         Automatically play the file (to incoming calls only)");
	puts(
			"  --auto-loop         Automatically loop incoming RTP to outgoing RTP");
	puts(
			"  --auto-conf         Automatically put calls in conference with others");
	puts(
			"  --rec-file=file     Open file recorder (extension can be .wav or .mp3");
	puts("  --auto-rec          Automatically record conversation");
	puts("  --quality=N         Specify media quality (0-10, default="
	xstr(PJSUA_DEFAULT_CODEC_QUALITY) ")");
	puts(
			"  --ptime=MSEC        Override codec ptime to MSEC (default=specific)");
	puts(
			"  --no-vad            Disable VAD/silence detector (default=vad enabled)");
	puts("  --ec-tail=MSEC      Set echo canceller tail length (default="
	xstr(PJSUA_DEFAULT_EC_TAIL_LEN) ")");
	puts("  --ec-opt=OPT        Select echo canceller algorithm (0=default, ");
	puts("                        1=speex, 2=suppressor, 3=WebRtc)");
	puts("  --ilbc-mode=MODE    Set iLBC codec mode (20 or 30, default is "
	xstr(PJSUA_DEFAULT_ILBC_MODE) ")");
	puts("  --capture-dev=id    Audio capture device ID (default=-1)");
	puts("  --playback-dev=id   Audio playback device ID (default=-1)");
	puts("  --capture-lat=N     Audio capture latency, in ms (default="
	xstr(PJMEDIA_SND_DEFAULT_REC_LATENCY) ")");
	puts("  --playback-lat=N    Audio playback latency, in ms (default="
	xstr(PJMEDIA_SND_DEFAULT_PLAY_LATENCY) ")");
	puts(
			"  --snd-auto-close=N  Auto close audio device when idle for N secs (default=1)");
	puts("                      Specify N=-1 to disable this feature.");
	puts("                      Specify N=0 for instant close when unused.");
	puts("  --no-tones          Disable audible tones");
	puts(
			"  --jb-max-size       Specify jitter buffer maximum size, in frames (default=-1)");
	puts("  --extra-audio       Add one more audio stream");

#if PJSUA_HAS_VIDEO
	puts ("");
	puts ("Video Options:");
	puts ("  --video             Enable video");
	puts ("  --vcapture-dev=id   Video capture device ID (default=-1)");
	puts ("  --vrender-dev=id    Video render device ID (default=-2)");
	puts ("  --play-avi=FILE     Load this AVI as virtual capture device");
	puts ("  --auto-play-avi     Automatically play the AVI media to call");
#endif

	puts("");
	puts("Media Transport Options:");
	puts("  --use-ice           Enable ICE (default:no)");
	puts(
			"  --ice-regular       Use ICE regular nomination (default: aggressive)");
	puts("  --ice-max-hosts=N   Set maximum number of ICE host candidates");
	puts("  --ice-no-rtcp       Disable RTCP component in ICE (default: no)");
	puts("  --rtp-port=N        Base port to try for RTP (default=4000)");
	puts(
			"  --rx-drop-pct=PCT   Drop PCT percent of RX RTP (for pkt lost sim, default: 0)");
	puts(
			"  --tx-drop-pct=PCT   Drop PCT percent of TX RTP (for pkt lost sim, default: 0)");
	puts("  --use-turn          Enable TURN relay with ICE (default:no)");
	puts(
			"  --turn-srv          Domain or host name of TURN server (\"NAME:PORT\" format)");
	puts(
			"  --turn-tcp          Use TCP connection to TURN server (default no)");
	puts("  --turn-user         TURN username");
	puts("  --turn-passwd       TURN password");
	puts("  --rtcp-mux          Enable RTP & RTCP multiplexing (default: no)");
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	puts("  --srtp-keying       SRTP keying method for outgoing SDP offer.");
	puts("                      0=SDES (default), 1=DTLS");
#endif

	puts("");
	puts("Buddy List (can be more than one):");
	puts("  --add-buddy url     Add the specified URL to the buddy list.");
	puts("");
	puts("User Agent options:");
	puts(
			"  --auto-answer=code  Automatically answer incoming calls with code (e.g. 200)");
	puts(
			"  --max-calls=N       Maximum number of concurrent calls (default:4, max:255)");
	puts("  --thread-cnt=N      Number of worker threads (default:1)");
	puts("  --duration=SEC      Set maximum call duration (default:no limit)");
	puts(
			"  --norefersub        Suppress event subscription when transferring calls");
	puts("  --use-compact-form  Minimize SIP message size");
	puts(
			"  --no-force-lr       Allow strict-route to be used (i.e. do not force lr)");
	puts(
			"  --accept-redirect=N Specify how to handle call redirect (3xx) response.");
	puts("                      0: reject, 1: follow automatically,");
	puts(
			"                      2: follow + replace To header (default), 3: ask");

	puts("");
	puts("CLI options:");
	puts("  --use-cli           Use CLI as user interface");
	puts("  --cli-telnet-port=N CLI telnet port");
	puts("  --no-cli-console    Disable CLI console");
	puts("");

	puts("");
	puts(
			"When URL is specified, pjsua will immediately initiate call to that URL");
	puts("");

	fflush(stdout);
}

static void log_writer_nobuf(int level, const char *buffer, int len)
{
	pj_log_write(level, buffer, len);
	fflush(stdout);
}

/*
 * Read command arguments from config file.
 */
static int read_config_file(pj_pool_t *pool, const char *filename,
		int *app_argc, char ***app_argv)
{
	int i;
	FILE *fhnd;
	char line[200];
	int argc = 0;
	char **argv;
	enum
	{
		MAX_ARGS = 128
	};

	/* Allocate MAX_ARGS+1 (argv needs to be terminated with NULL argument) */
	argv = pj_pool_calloc(pool, MAX_ARGS + 1, sizeof(char*));
	argv[argc++] = *app_argv[0];

	/* Open config file. */
	fhnd = fopen(filename, "rt");
	if (!fhnd)
	{
		PJ_LOG(1, (THIS_FILE, "Unable to open config file %s", filename));
		fflush(stdout);
		return -1;
	}

	/* Scan tokens in the file. */
	while (argc < MAX_ARGS && !feof(fhnd))
	{
		char *token;
		char *p;
		const char *whitespace = " \t\r\n";
		char cDelimiter;
		pj_size_t len;
		int token_len;

		pj_bzero(line, sizeof(line));
		if (fgets(line, sizeof(line), fhnd) == NULL)
			break;

		// Trim ending newlines
		len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[--len] = '\0';
		if (len > 0 && line[len - 1] == '\r')
			line[--len] = '\0';

		if (len == 0)
			continue;

		for (p = line; *p != '\0' && argc < MAX_ARGS; p++)
		{
			// first, scan whitespaces
			while (*p != '\0' && strchr(whitespace, *p) != NULL)
				p++;

			if (*p == '\0')		    // are we done yet?
				break;

			if (*p == '"' || *p == '\'')
			{    // is token a quoted string
				cDelimiter = *p++;	    // save quote delimiter
				token = p;

				while (*p != '\0' && *p != cDelimiter)
					p++;

				if (*p == '\0')		// found end of the line, but,
					cDelimiter = '\0';	// didn't find a matching quote

			}
			else
			{			// token's not a quoted string
				token = p;

				while (*p != '\0' && strchr(whitespace, *p) == NULL)
					p++;

				cDelimiter = *p;
			}

			*p = '\0';
			token_len = (int) (p - token);

			if (token_len > 0)
			{
				if (*token == '#')
					break;  // ignore remainder of line

				argv[argc] = pj_pool_alloc(pool, token_len + 1);
				pj_memcpy(argv[argc], token, token_len + 1);
				++argc;
			}

			*p = cDelimiter;
		}
	}

	/* Copy arguments from command line */
	for (i = 1; i < *app_argc && argc < MAX_ARGS; ++i)
		argv[argc++] = (*app_argv)[i];

	if (argc == MAX_ARGS && (i != *app_argc || !feof(fhnd)))
	{
		PJ_LOG(1,
				(THIS_FILE, "Too many arguments specified in cmd line/config file"));
		fflush(stdout);
		fclose(fhnd);
		return -1;
	}

	fclose(fhnd);

	/* Assign the new command line back to the original command line. */
	*app_argc = argc;
	*app_argv = argv;
	return 0;
}

/* Parse arguments. */
static pj_status_t parse_args(int argc, char *argv[], pj_str_t *uri_to_call)
{
	int c;
	int option_index;
	pjsua_app_config *cfg = &app_config;

	struct pj_getopt_option long_options[] =
	{
	{ "config-file", 1, 0, OPT_CONFIG_FILE },
	{ "log-file", 1, 0, OPT_LOG_FILE },
	{ "log-level", 1, 0, OPT_LOG_LEVEL },
	{ "app-log-level", 1, 0, OPT_APP_LOG_LEVEL },
	{ "log-append", 0, 0, OPT_LOG_APPEND },
	{ "color", 0, 0, OPT_COLOR },
	{ "no-color", 0, 0, OPT_NO_COLOR },
	{ "light-bg", 0, 0, OPT_LIGHT_BG },
	{ "no-stderr", 0, 0, OPT_NO_STDERR },
	{ "help", 0, 0, OPT_HELP },
	{ "version", 0, 0, OPT_VERSION },
	{ "clock-rate", 1, 0, OPT_CLOCK_RATE },
	{ "snd-clock-rate", 1, 0, OPT_SND_CLOCK_RATE },
	{ "stereo", 0, 0, OPT_STEREO },
	{ "null-audio", 0, 0, OPT_NULL_AUDIO },
	{ "local-port", 1, 0, OPT_LOCAL_PORT },
	{ "ip-addr", 1, 0, OPT_IP_ADDR },
	{ "bound-addr", 1, 0, OPT_BOUND_ADDR },
	{ "no-tcp", 0, 0, OPT_NO_TCP },
	{ "no-udp", 0, 0, OPT_NO_UDP },
	{ "norefersub", 0, 0, OPT_NOREFERSUB },
	{ "proxy", 1, 0, OPT_PROXY },
	{ "outbound", 1, 0, OPT_OUTBOUND_PROXY },
	{ "registrar", 1, 0, OPT_REGISTRAR },
	{ "reg-timeout", 1, 0, OPT_REG_TIMEOUT },
	{ "publish", 0, 0, OPT_PUBLISH },
	{ "mwi", 0, 0, OPT_MWI },
	{ "use-100rel", 0, 0, OPT_100REL },
	{ "use-ims", 0, 0, OPT_USE_IMS },
	{ "id", 1, 0, OPT_ID },
	{ "contact", 1, 0, OPT_CONTACT },
	{ "contact-params", 1, 0, OPT_CONTACT_PARAMS },
	{ "contact-uri-params", 1, 0, OPT_CONTACT_URI_PARAMS },
	{ "auto-update-nat", 1, 0, OPT_AUTO_UPDATE_NAT },
	{ "disable-stun", 0, 0, OPT_DISABLE_STUN },
	{ "use-compact-form", 0, 0, OPT_USE_COMPACT_FORM },
	{ "accept-redirect", 1, 0, OPT_ACCEPT_REDIRECT },
	{ "no-force-lr", 0, 0, OPT_NO_FORCE_LR },
	{ "realm", 1, 0, OPT_REALM },
	{ "username", 1, 0, OPT_USERNAME },
	{ "password", 1, 0, OPT_PASSWORD },
	{ "rereg-delay", 1, 0, OPT_REG_RETRY_INTERVAL },
	{ "reg-use-proxy", 1, 0, OPT_REG_USE_PROXY },
	{ "nameserver", 1, 0, OPT_NAMESERVER },
	{ "stun-srv", 1, 0, OPT_STUN_SRV },
	{ "add-buddy", 1, 0, OPT_ADD_BUDDY },
	{ "offer-x-ms-msg", 0, 0, OPT_OFFER_X_MS_MSG },
	{ "no-presence", 0, 0, OPT_NO_PRESENCE },
	{ "auto-answer", 1, 0, OPT_AUTO_ANSWER },
	{ "auto-play", 0, 0, OPT_AUTO_PLAY },
	{ "auto-play-hangup", 0, 0, OPT_AUTO_PLAY_HANGUP },
	{ "auto-rec", 0, 0, OPT_AUTO_REC },
	{ "auto-loop", 0, 0, OPT_AUTO_LOOP },
	{ "auto-conf", 0, 0, OPT_AUTO_CONF },
	{ "play-file", 1, 0, OPT_PLAY_FILE },
	{ "play-tone", 1, 0, OPT_PLAY_TONE },
	{ "rec-file", 1, 0, OPT_REC_FILE },
	{ "rtp-port", 1, 0, OPT_RTP_PORT },

	{ "use-ice", 0, 0, OPT_USE_ICE },
	{ "ice-regular", 0, 0, OPT_ICE_REGULAR },
	{ "use-turn", 0, 0, OPT_USE_TURN },
	{ "ice-max-hosts", 1, 0, OPT_ICE_MAX_HOSTS },
	{ "ice-no-rtcp", 0, 0, OPT_ICE_NO_RTCP },
	{ "turn-srv", 1, 0, OPT_TURN_SRV },
	{ "turn-tcp", 0, 0, OPT_TURN_TCP },
	{ "turn-user", 1, 0, OPT_TURN_USER },
	{ "turn-passwd", 1, 0, OPT_TURN_PASSWD },
	{ "rtcp-mux", 0, 0, OPT_RTCP_MUX },

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
			{ "use-srtp", 1, 0, OPT_USE_SRTP },
			{ "srtp-secure", 1, 0, OPT_SRTP_SECURE },
			{ "srtp-keying", 1, 0, OPT_SRTP_KEYING },
#endif
			{ "add-codec", 1, 0, OPT_ADD_CODEC },
			{ "dis-codec", 1, 0, OPT_DIS_CODEC },
			{ "complexity", 1, 0, OPT_COMPLEXITY },
			{ "quality", 1, 0, OPT_QUALITY },
			{ "ptime", 1, 0, OPT_PTIME },
			{ "no-vad", 0, 0, OPT_NO_VAD },
			{ "ec-tail", 1, 0, OPT_EC_TAIL },
			{ "ec-opt", 1, 0, OPT_EC_OPT },
			{ "ilbc-mode", 1, 0, OPT_ILBC_MODE },
			{ "rx-drop-pct", 1, 0, OPT_RX_DROP_PCT },
			{ "tx-drop-pct", 1, 0, OPT_TX_DROP_PCT },
			{ "next-account", 0, 0, OPT_NEXT_ACCOUNT },
			{ "next-cred", 0, 0, OPT_NEXT_CRED },
			{ "max-calls", 1, 0, OPT_MAX_CALLS },
			{ "duration", 1, 0, OPT_DURATION },
			{ "thread-cnt", 1, 0, OPT_THREAD_CNT },
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
			{	"use-tls", 0, 0, OPT_USE_TLS},
			{	"tls-ca-file",1, 0, OPT_TLS_CA_FILE},
			{	"tls-cert-file",1,0, OPT_TLS_CERT_FILE},
			{	"tls-privkey-file",1,0, OPT_TLS_PRIV_FILE},
			{	"tls-password",1,0, OPT_TLS_PASSWORD},
			{	"tls-verify-server", 0, 0, OPT_TLS_VERIFY_SERVER},
			{	"tls-verify-client", 0, 0, OPT_TLS_VERIFY_CLIENT},
			{	"tls-neg-timeout", 1, 0, OPT_TLS_NEG_TIMEOUT},
			{	"tls-cipher", 1, 0, OPT_TLS_CIPHER},
#endif
			{ "capture-dev", 1, 0, OPT_CAPTURE_DEV },
			{ "playback-dev", 1, 0, OPT_PLAYBACK_DEV },
			{ "capture-lat", 1, 0, OPT_CAPTURE_LAT },
			{ "playback-lat", 1, 0, OPT_PLAYBACK_LAT },
			{ "stdout-refresh", 1, 0, OPT_STDOUT_REFRESH },
			{ "stdout-refresh-text", 1, 0, OPT_STDOUT_REFRESH_TEXT },
#ifdef _IONBF
			{	"stdout-no-buf", 0, 0, OPT_STDOUT_NO_BUF},
#endif
			{ "snd-auto-close", 1, 0, OPT_SND_AUTO_CLOSE },
			{ "no-tones", 0, 0, OPT_NO_TONES },
			{ "jb-max-size", 1, 0, OPT_JB_MAX_SIZE },
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
			{	"ipv6", 0, 0, OPT_IPV6},
#endif
			{ "set-qos", 0, 0, OPT_QOS },
			{ "use-timer", 1, 0, OPT_TIMER },
			{ "timer-se", 1, 0, OPT_TIMER_SE },
			{ "timer-min-se", 1, 0, OPT_TIMER_MIN_SE },
			{ "outb-rid", 1, 0, OPT_OUTB_RID },
			{ "video", 0, 0, OPT_VIDEO },
			{ "extra-audio", 0, 0, OPT_EXTRA_AUDIO },
			{ "vcapture-dev", 1, 0, OPT_VCAPTURE_DEV },
			{ "vrender-dev", 1, 0, OPT_VRENDER_DEV },
			{ "play-avi", 1, 0, OPT_PLAY_AVI },
			{ "auto-play-avi", 0, 0, OPT_AUTO_PLAY_AVI },
			{ "use-cli", 0, 0, OPT_USE_CLI },
			{ "cli-telnet-port", 1, 0, OPT_CLI_TELNET_PORT },
			{ "no-cli-console", 0, 0, OPT_DISABLE_CLI_CONSOLE },
			{ NULL, 0, 0, 0 } };
	pj_status_t status;
	pjsua_acc_config *cur_acc;
	char *config_file = NULL;
	unsigned i;

	/* Run pj_getopt once to see if user specifies config file to read. */
	pj_optind = 0;
	while ((c = pj_getopt_long(argc, argv, "", long_options, &option_index))
			!= -1)
	{
		switch (c)
		{
		case OPT_CONFIG_FILE:
			config_file = pj_optarg;
			break;
		}
		if (config_file)
			break;
	}

	if (config_file)
	{
		status = read_config_file(cfg->pool, config_file, &argc, &argv);
		if (status != 0)
			return status;
	}

	cfg->acc_cnt = 0;
	cur_acc = cfg->acc_cfg;

	/* Reinitialize and re-run pj_getopt again, possibly with new arguments
	 * read from config file.
	 */
	pj_optind = 0;
	while ((c = pj_getopt_long(argc, argv, "", long_options, &option_index))
			!= -1)
	{
		pj_str_t tmp;
		long lval;

		switch (c)
		{

		case OPT_CONFIG_FILE:
			/* Ignore as this has been processed before */
			break;

		case OPT_LOG_FILE:
			cfg->log_cfg.log_filename = pj_str(pj_optarg);
			break;

		case OPT_LOG_LEVEL:
			c = (int) pj_strtoul(pj_cstr(&tmp, pj_optarg));
			if (c < 0 || c > 6)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting integer value 0-6 " "for --log-level"));
				return PJ_EINVAL;
			}
			cfg->log_cfg.level = c;
			pj_log_set_level(c);
			break;

		case OPT_APP_LOG_LEVEL:
			cfg->log_cfg.console_level = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			if (cfg->log_cfg.console_level > 6)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting integer value 0-6 " "for --app-log-level"));
				return PJ_EINVAL;
			}
			break;

		case OPT_LOG_APPEND:
			cfg->log_cfg.log_file_flags |= PJ_O_APPEND;
			break;

		case OPT_COLOR:
			cfg->log_cfg.decor |= PJ_LOG_HAS_COLOR;
			break;

		case OPT_NO_COLOR:
			cfg->log_cfg.decor &= ~PJ_LOG_HAS_COLOR;
			break;

		case OPT_LIGHT_BG:
			pj_log_set_color(1, PJ_TERM_COLOR_R);
			pj_log_set_color(2, PJ_TERM_COLOR_R | PJ_TERM_COLOR_G);
			pj_log_set_color(3, PJ_TERM_COLOR_B | PJ_TERM_COLOR_G);
			pj_log_set_color(4, 0);
			pj_log_set_color(5, 0);
			pj_log_set_color(77, 0);
			break;

		case OPT_NO_STDERR:
#if !defined(PJ_WIN32_WINCE) || PJ_WIN32_WINCE==0
			freopen("/dev/null", "w", stderr);
#endif
			break;

		case OPT_HELP:
			usage();
			return PJ_EINVAL;

		case OPT_VERSION: /* version */
			pj_dump_config();
			return PJ_EINVAL;

		case OPT_NULL_AUDIO:
			cfg->null_audio = PJ_TRUE;
			break;

		case OPT_CLOCK_RATE:
			lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
			if (lval < 8000 || lval > 192000)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting value between " "8000-192000 for conference clock rate"));
				return PJ_EINVAL;
			}
			cfg->media_cfg.clock_rate = (unsigned) lval;
			break;

		case OPT_SND_CLOCK_RATE:
			lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
			if (lval < 8000 || lval > 192000)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting value between " "8000-192000 for sound device clock rate"));
				return PJ_EINVAL;
			}
			cfg->media_cfg.snd_clock_rate = (unsigned) lval;
			break;

		case OPT_STEREO:
			cfg->media_cfg.channel_count = 2;
			break;

		case OPT_LOCAL_PORT: /* local-port */
			lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
			if (lval < 0 || lval > 65535)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting integer value for " "--local-port"));
				return PJ_EINVAL;
			}
			cfg->udp_cfg.port = (pj_uint16_t) lval;
			break;

		case OPT_IP_ADDR: /* ip-addr */
			cfg->udp_cfg.public_addr = pj_str(pj_optarg);
			cfg->rtp_cfg.public_addr = pj_str(pj_optarg);
			break;

		case OPT_BOUND_ADDR: /* bound-addr */
			cfg->udp_cfg.bound_addr = pj_str(pj_optarg);
			cfg->rtp_cfg.bound_addr = pj_str(pj_optarg);
			break;

		case OPT_NO_UDP: /* no-udp */
			if (cfg->no_tcp && !cfg->use_tls)
			{
				PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
				return PJ_EINVAL;
			}

			cfg->no_udp = PJ_TRUE;
			break;

		case OPT_NOREFERSUB: /* norefersub */
			cfg->no_refersub = PJ_TRUE;
			break;

		case OPT_NO_TCP: /* no-tcp */
			if (cfg->no_udp && !cfg->use_tls)
			{
				PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
				return PJ_EINVAL;
			}

			cfg->no_tcp = PJ_TRUE;
			break;

		case OPT_PROXY: /* proxy */
			if (pjsua_verify_sip_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", pj_optarg));
				return PJ_EINVAL;
			}
			cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
			break;

		case OPT_OUTBOUND_PROXY: /* outbound proxy */
			if (pjsua_verify_sip_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid SIP URL '%s' " "in outbound proxy argument", pj_optarg));
				return PJ_EINVAL;
			}
			cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt++] = pj_str(
					pj_optarg);
			break;

		case OPT_REGISTRAR: /* registrar */
			if (pjsua_verify_sip_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid SIP URL '%s' in " "registrar argument", pj_optarg));
				return PJ_EINVAL;
			}
			cur_acc->reg_uri = pj_str(pj_optarg);
			break;

		case OPT_REG_TIMEOUT: /* reg-timeout */
			cur_acc->reg_timeout = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			if (cur_acc->reg_timeout < 1 || cur_acc->reg_timeout > 3600)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid value for --reg-timeout " "(expecting 1-3600)"));
				return PJ_EINVAL;
			}
			break;

		case OPT_PUBLISH: /* publish */
			cur_acc->publish_enabled = PJ_TRUE;
			break;

		case OPT_MWI: /* mwi */
			cur_acc->mwi_enabled = PJ_TRUE;
			break;

		case OPT_100REL: /** 100rel */
			cur_acc->require_100rel = PJSUA_100REL_MANDATORY;
			cfg->cfg.require_100rel = PJSUA_100REL_MANDATORY;
			break;

		case OPT_TIMER: /** session timer */
			lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
			if (lval < 0 || lval > 3)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: expecting integer value 0-3 for --use-timer"));
				return PJ_EINVAL;
			}
			cur_acc->use_timer = (pjsua_sip_timer_use) lval;
			cfg->cfg.use_timer = (pjsua_sip_timer_use) lval;
			break;

		case OPT_TIMER_SE: /** session timer session expiration */
			cur_acc->timer_setting.sess_expires = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			if (cur_acc->timer_setting.sess_expires < 90)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid value for --timer-se " "(expecting higher than 90)"));
				return PJ_EINVAL;
			}
			cfg->cfg.timer_setting.sess_expires =
					cur_acc->timer_setting.sess_expires;
			break;

		case OPT_TIMER_MIN_SE: /** session timer minimum session expiration */
			cur_acc->timer_setting.min_se = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			if (cur_acc->timer_setting.min_se < 90)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid value for --timer-min-se " "(expecting higher than 90)"));
				return PJ_EINVAL;
			}
			cfg->cfg.timer_setting.min_se = cur_acc->timer_setting.min_se;
			break;

		case OPT_OUTB_RID: /* Outbound reg-id */
			cur_acc->rfc5626_reg_id = pj_str(pj_optarg);
			break;

		case OPT_USE_IMS: /* Activate IMS settings */
			cur_acc->auth_pref.initial_auth = PJ_TRUE;
			break;

		case OPT_ID: /* id */
			if (pjsua_verify_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid SIP URL '%s' " "in local id argument", pj_optarg));
				return PJ_EINVAL;
			}
			cur_acc->id = pj_str(pj_optarg);
			break;

		case OPT_CONTACT: /* contact */
			if (pjsua_verify_sip_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact argument", pj_optarg));
				return PJ_EINVAL;
			}
			cur_acc->force_contact = pj_str(pj_optarg);
			break;

		case OPT_CONTACT_PARAMS:
			cur_acc->contact_params = pj_str(pj_optarg);
			break;

		case OPT_CONTACT_URI_PARAMS:
			cur_acc->contact_uri_params = pj_str(pj_optarg);
			break;

		case OPT_AUTO_UPDATE_NAT: /* OPT_AUTO_UPDATE_NAT */
			cur_acc->allow_contact_rewrite = (pj_bool_t) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			break;

		case OPT_DISABLE_STUN:
			cur_acc->sip_stun_use = PJSUA_STUN_USE_DISABLED;
			cur_acc->media_stun_use = PJSUA_STUN_USE_DISABLED;
			break;

		case OPT_USE_COMPACT_FORM:
			/* enable compact form - from Ticket #342 */
		{
			extern pj_bool_t pjsip_include_allow_hdr_in_dlg;
			extern pj_bool_t pjmedia_add_rtpmap_for_static_pt;

			pjsip_cfg()->endpt.use_compact_form = PJ_TRUE;
			/* do not transmit Allow header */
			pjsip_include_allow_hdr_in_dlg = PJ_FALSE;
			/* Do not include rtpmap for static payload types (<96) */
			pjmedia_add_rtpmap_for_static_pt = PJ_FALSE;
		}
			break;

		case OPT_ACCEPT_REDIRECT:
			cfg->redir_op = my_atoi(pj_optarg);
			if (cfg->redir_op > PJSIP_REDIRECT_STOP)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: accept-redirect value '%s' ", pj_optarg));
				return PJ_EINVAL;
			}
			break;

		case OPT_NO_FORCE_LR:
			cfg->cfg.force_lr = PJ_FALSE;
			break;

		case OPT_NEXT_ACCOUNT: /* Add more account. */
			cfg->acc_cnt++;
			cur_acc = &cfg->acc_cfg[cfg->acc_cnt];
			break;

		case OPT_USERNAME: /* Default authentication user */
			cur_acc->cred_info[cur_acc->cred_count].username = pj_str(
					pj_optarg);
			cur_acc->cred_info[cur_acc->cred_count].scheme = pj_str("Digest");
			break;

		case OPT_REALM: /* Default authentication realm. */
			cur_acc->cred_info[cur_acc->cred_count].realm = pj_str(pj_optarg);
			break;

		case OPT_PASSWORD: /* authentication password */
			cur_acc->cred_info[cur_acc->cred_count].data_type =
					PJSIP_CRED_DATA_PLAIN_PASSWD;
			cur_acc->cred_info[cur_acc->cred_count].data = pj_str(pj_optarg);
#if PJSIP_HAS_DIGEST_AKA_AUTH
			cur_acc->cred_info[cur_acc->cred_count].data_type |= PJSIP_CRED_DATA_EXT_AKA;
			cur_acc->cred_info[cur_acc->cred_count].ext.aka.k = pj_str(pj_optarg);
			cur_acc->cred_info[cur_acc->cred_count].ext.aka.cb = &pjsip_auth_create_aka_response;
#endif
			break;

		case OPT_REG_RETRY_INTERVAL:
			cur_acc->reg_retry_interval = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			break;

		case OPT_REG_USE_PROXY:
			cur_acc->reg_use_proxy = (unsigned) pj_strtoul(
					pj_cstr(&tmp, pj_optarg));
			if (cur_acc->reg_use_proxy > 3)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid --reg-use-proxy value '%s'", pj_optarg));
				return PJ_EINVAL;
			}
			break;

		case OPT_NEXT_CRED: /* next credential */
			cur_acc->cred_count++;
			break;

		case OPT_NAMESERVER: /* nameserver */
			cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(
					pj_optarg);
			if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver))
			{
				PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
				return PJ_ETOOMANY;
			}
			break;

		case OPT_STUN_SRV: /* STUN server */
			cfg->cfg.stun_host = pj_str(pj_optarg);
			if (cfg->cfg.stun_srv_cnt == PJ_ARRAY_SIZE(cfg->cfg.stun_srv))
			{
				PJ_LOG(1, (THIS_FILE, "Error: too many STUN servers"));
				return PJ_ETOOMANY;
			}
			cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt++] = pj_str(pj_optarg);
			break;

		case OPT_ADD_BUDDY: /* Add to buddy list. */
			if (pjsua_verify_url(pj_optarg) != 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid URL '%s' in " "--add-buddy option", pj_optarg));
				return -1;
			}
			if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg))
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: too many buddies in buddy list."));
				return -1;
			}
			cfg->buddy_cfg[cfg->buddy_cnt].uri = pj_str(pj_optarg);
			cfg->buddy_cnt++;
			break;

		case OPT_AUTO_PLAY:
			cfg->auto_play = 1;
			break;

		case OPT_AUTO_PLAY_HANGUP:
			cfg->auto_play_hangup = 1;
			break;

		case OPT_AUTO_REC:
			cfg->auto_rec = 1;
			break;

		case OPT_AUTO_LOOP:
			cfg->auto_loop = 1;
			break;

		case OPT_AUTO_CONF:
			cfg->auto_conf = 1;
			break;

		case OPT_PLAY_FILE:
			cfg->wav_files[cfg->wav_count++] = pj_str(pj_optarg);
			break;

		case OPT_PLAY_TONE:
		{
			int f1, f2, on, off;
			int n;

			n = sscanf(pj_optarg, "%d,%d,%d,%d", &f1, &f2, &on, &off);
			if (n != 4)
			{
				puts("Expecting f1,f2,on,off in --play-tone");
				return -1;
			}

			cfg->tones[cfg->tone_count].freq1 = (short) f1;
			cfg->tones[cfg->tone_count].freq2 = (short) f2;
			cfg->tones[cfg->tone_count].on_msec = (short) on;
			cfg->tones[cfg->tone_count].off_msec = (short) off;
			++cfg->tone_count;
		}
			break;

		case OPT_REC_FILE:
			cfg->rec_file = pj_str(pj_optarg);
			break;

		case OPT_USE_ICE:
			cfg->media_cfg.enable_ice = cur_acc->ice_cfg.enable_ice = PJ_TRUE;
			break;

		case OPT_ICE_REGULAR:
			cfg->media_cfg.ice_opt.aggressive =
					cur_acc->ice_cfg.ice_opt.aggressive = PJ_FALSE;
			break;

		case OPT_USE_TURN:
			cfg->media_cfg.enable_turn = cur_acc->turn_cfg.enable_turn =
					PJ_TRUE;
			break;

		case OPT_ICE_MAX_HOSTS:
			cfg->media_cfg.ice_max_host_cands =
					cur_acc->ice_cfg.ice_max_host_cands = my_atoi(pj_optarg);
			break;

		case OPT_ICE_NO_RTCP:
			cfg->media_cfg.ice_no_rtcp = cur_acc->ice_cfg.ice_no_rtcp = PJ_TRUE;
			break;

		case OPT_TURN_SRV:
			cfg->media_cfg.turn_server = cur_acc->turn_cfg.turn_server = pj_str(
					pj_optarg);
			break;

		case OPT_TURN_TCP:
			cfg->media_cfg.turn_conn_type = cur_acc->turn_cfg.turn_conn_type =
					PJ_TURN_TP_TCP;
			break;

		case OPT_TURN_USER:
			cfg->media_cfg.turn_auth_cred.type =
					cur_acc->turn_cfg.turn_auth_cred.type =
							PJ_STUN_AUTH_CRED_STATIC;
			cfg->media_cfg.turn_auth_cred.data.static_cred.realm =
					cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm =
							pj_str("*");
			cfg->media_cfg.turn_auth_cred.data.static_cred.username =
					cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username =
							pj_str(pj_optarg);
			break;

		case OPT_TURN_PASSWD:
			cfg->media_cfg.turn_auth_cred.data.static_cred.data_type =
					cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data_type =
							PJ_STUN_PASSWD_PLAIN;
			cfg->media_cfg.turn_auth_cred.data.static_cred.data =
					cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data =
							pj_str(pj_optarg);
			break;

		case OPT_RTCP_MUX:
			cur_acc->enable_rtcp_mux = PJ_TRUE;
			break;

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
		case OPT_USE_SRTP:
			app_config.cfg.use_srtp = my_atoi(pj_optarg);
			if (!pj_isdigit(*pj_optarg) || app_config.cfg.use_srtp > 3)
			{
				PJ_LOG(1, (THIS_FILE, "Invalid value for --use-srtp option"));
				return -1;
			}
			if ((int) app_config.cfg.use_srtp == 3)
			{
				/* SRTP optional mode with duplicated media offer */
				app_config.cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
				app_config.cfg.srtp_optional_dup_offer = PJ_TRUE;
				cur_acc->srtp_optional_dup_offer = PJ_TRUE;
			}
			cur_acc->use_srtp = app_config.cfg.use_srtp;
			break;
		case OPT_SRTP_SECURE:
			app_config.cfg.srtp_secure_signaling = my_atoi(pj_optarg);
			if (!pj_isdigit(*pj_optarg)
					|| app_config.cfg.srtp_secure_signaling > 2)
			{
				PJ_LOG(1,
						(THIS_FILE, "Invalid value for --srtp-secure option"));
				return -1;
			}
			cur_acc->srtp_secure_signaling =
					app_config.cfg.srtp_secure_signaling;
			break;
		case OPT_SRTP_KEYING:
			app_config.srtp_keying = my_atoi(pj_optarg);
			if (!pj_isdigit(*pj_optarg) || app_config.srtp_keying < 0
					|| app_config.srtp_keying > 1)
			{
				PJ_LOG(1,
						(THIS_FILE, "Invalid value for --srtp-keying option"));
				return -1;
			}
			/* Set SRTP keying to use DTLS over SDES */
			if (app_config.srtp_keying == 1)
			{
				pjsua_srtp_opt *srtp_opt = &app_config.cfg.srtp_opt;
				srtp_opt->keying_count = 2;
				srtp_opt->keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
				srtp_opt->keying[1] = PJMEDIA_SRTP_KEYING_SDES;

				cur_acc->srtp_opt.keying_count = 2;
				cur_acc->srtp_opt.keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
				cur_acc->srtp_opt.keying[1] = PJMEDIA_SRTP_KEYING_SDES;
			}
			break;
#endif

		case OPT_RTP_PORT:
			cfg->rtp_cfg.port = my_atoi(pj_optarg);
			if (cfg->rtp_cfg.port == 0)
			{
				enum
				{
					START_PORT = 4000
				};
				unsigned range;

				range = (65535 - START_PORT - PJSUA_MAX_CALLS * 2);
				cfg->rtp_cfg.port = START_PORT + ((pj_rand() % range) & 0xFFFE);
			}

			if (cfg->rtp_cfg.port < 1 || cfg->rtp_cfg.port > 65535)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: rtp-port argument value " "(expecting 1-65535"));
				return -1;
			}
			break;

		case OPT_DIS_CODEC:
			cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str(pj_optarg);
			break;

		case OPT_ADD_CODEC:
			cfg->codec_arg[cfg->codec_cnt++] = pj_str(pj_optarg);
			break;

			/* These options were no longer valid after new pjsua */
			/*
			 case OPT_COMPLEXITY:
			 cfg->complexity = my_atoi(pj_optarg);
			 if (cfg->complexity < 0 || cfg->complexity > 10) {
			 PJ_LOG(1,(THIS_FILE,
			 "Error: invalid --complexity (expecting 0-10"));
			 return -1;
			 }
			 break;
			 */

		case OPT_DURATION:
			cfg->duration = my_atoi(pj_optarg);
			break;

		case OPT_THREAD_CNT:
			cfg->cfg.thread_cnt = my_atoi(pj_optarg);
			if (cfg->cfg.thread_cnt > 128)
			{
				PJ_LOG(1, (THIS_FILE, "Error: invalid --thread-cnt option"));
				return -1;
			}
			break;

		case OPT_PTIME:
			cfg->media_cfg.ptime = my_atoi(pj_optarg);
			if (cfg->media_cfg.ptime < 10 || cfg->media_cfg.ptime > 1000)
			{
				PJ_LOG(1, (THIS_FILE, "Error: invalid --ptime option"));
				return -1;
			}
			break;

		case OPT_NO_VAD:
			cfg->media_cfg.no_vad = PJ_TRUE;
			break;

		case OPT_EC_TAIL:
			cfg->media_cfg.ec_tail_len = my_atoi(pj_optarg);
			if (cfg->media_cfg.ec_tail_len > 1000)
			{
				PJ_LOG(1,
						(THIS_FILE, "I think the ec-tail length setting " "is too big"));
				return -1;
			}
			break;

		case OPT_EC_OPT:
			cfg->media_cfg.ec_options = my_atoi(pj_optarg);
			break;

		case OPT_QUALITY:
			cfg->media_cfg.quality = my_atoi(pj_optarg);
			if (cfg->media_cfg.quality > 10)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid --quality (expecting 0-10"));
				return -1;
			}
			break;

		case OPT_ILBC_MODE:
			cfg->media_cfg.ilbc_mode = my_atoi(pj_optarg);
			if (cfg->media_cfg.ilbc_mode != 20
					&& cfg->media_cfg.ilbc_mode != 30)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid --ilbc-mode (expecting 20 or 30"));
				return -1;
			}
			break;

		case OPT_RX_DROP_PCT:
			cfg->media_cfg.rx_drop_pct = my_atoi(pj_optarg);
			if (cfg->media_cfg.rx_drop_pct > 100)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid --rx-drop-pct (expecting <= 100"));
				return -1;
			}
			break;

		case OPT_TX_DROP_PCT:
			cfg->media_cfg.tx_drop_pct = my_atoi(pj_optarg);
			if (cfg->media_cfg.tx_drop_pct > 100)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid --tx-drop-pct (expecting <= 100"));
				return -1;
			}
			break;

		case OPT_AUTO_ANSWER:
			cfg->auto_answer = my_atoi(pj_optarg);
			if (cfg->auto_answer < 100 || cfg->auto_answer > 699)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: invalid code in --auto-answer " "(expecting 100-699"));
				return -1;
			}
			break;

		case OPT_MAX_CALLS:
			cfg->cfg.max_calls = my_atoi(pj_optarg);
			if (cfg->cfg.max_calls < 1 || cfg->cfg.max_calls > PJSUA_MAX_CALLS)
			{
				PJ_LOG(1,
						(THIS_FILE,"Error: maximum call setting exceeds " "compile time limit (PJSUA_MAX_CALLS=%d)", PJSUA_MAX_CALLS));
				return -1;
			}
			break;

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
			case OPT_USE_TLS:
			cfg->use_tls = PJ_TRUE;
			break;

			case OPT_TLS_CA_FILE:
			cfg->udp_cfg.tls_setting.ca_list_file = pj_str(pj_optarg);
			break;

			case OPT_TLS_CERT_FILE:
			cfg->udp_cfg.tls_setting.cert_file = pj_str(pj_optarg);
			break;

			case OPT_TLS_PRIV_FILE:
			cfg->udp_cfg.tls_setting.privkey_file = pj_str(pj_optarg);
			break;

			case OPT_TLS_PASSWORD:
			cfg->udp_cfg.tls_setting.password = pj_str(pj_optarg);
			break;

			case OPT_TLS_VERIFY_SERVER:
			cfg->udp_cfg.tls_setting.verify_server = PJ_TRUE;
			break;

			case OPT_TLS_VERIFY_CLIENT:
			cfg->udp_cfg.tls_setting.verify_client = PJ_TRUE;
			cfg->udp_cfg.tls_setting.require_client_cert = PJ_TRUE;
			break;

			case OPT_TLS_NEG_TIMEOUT:
			cfg->udp_cfg.tls_setting.timeout.sec = atoi(pj_optarg);
			break;

			case OPT_TLS_CIPHER:
			{
				pj_ssl_cipher cipher;

				if (pj_ansi_strnicmp(pj_optarg, "0x", 2) == 0)
				{
					pj_str_t cipher_st = pj_str(pj_optarg + 2);
					cipher = pj_strtoul2(&cipher_st, NULL, 16);
				}
				else
				{
					cipher = atoi(pj_optarg);
				}

				if (pj_ssl_cipher_is_supported(cipher))
				{
					static pj_ssl_cipher tls_ciphers[PJ_SSL_SOCK_MAX_CIPHERS];

					tls_ciphers[cfg->udp_cfg.tls_setting.ciphers_num++] = cipher;
					cfg->udp_cfg.tls_setting.ciphers = tls_ciphers;
				}
				else
				{
					pj_ssl_cipher ciphers[PJ_SSL_SOCK_MAX_CIPHERS];
					unsigned j, ciphers_cnt;

					ciphers_cnt = PJ_ARRAY_SIZE(ciphers);
					pj_ssl_cipher_get_availables(ciphers, &ciphers_cnt);

					PJ_LOG(1,(THIS_FILE, "Cipher \"%s\" is not supported by "
									"TLS/SSL backend.", pj_optarg));
					printf("Available TLS/SSL ciphers (%d):\n", ciphers_cnt);
					for (j=0; j<ciphers_cnt; ++j)
					printf("- 0x%06X: %s\n", ciphers[j], pj_ssl_cipher_name(ciphers[j]));
					return -1;
				}
			}
			break;
#endif /* PJSIP_HAS_TLS_TRANSPORT */

		case OPT_CAPTURE_DEV:
			cfg->capture_dev = atoi(pj_optarg);
			break;

		case OPT_PLAYBACK_DEV:
			cfg->playback_dev = atoi(pj_optarg);
			break;

		case OPT_STDOUT_REFRESH:
			stdout_refresh = atoi(pj_optarg);
			break;

		case OPT_STDOUT_REFRESH_TEXT:
			stdout_refresh_text = pj_optarg;
			break;

#ifdef _IONBF
			case OPT_STDOUT_NO_BUF:
			setvbuf(stdout, NULL, _IONBF, 0);
			cfg->log_cfg.cb = &log_writer_nobuf;
			break;
#endif

		case OPT_CAPTURE_LAT:
			cfg->capture_lat = atoi(pj_optarg);
			break;

		case OPT_PLAYBACK_LAT:
			cfg->playback_lat = atoi(pj_optarg);
			break;

		case OPT_SND_AUTO_CLOSE:
			cfg->media_cfg.snd_auto_close_time = atoi(pj_optarg);
			break;

		case OPT_NO_TONES:
			cfg->no_tones = PJ_TRUE;
			break;

		case OPT_JB_MAX_SIZE:
			cfg->media_cfg.jb_max = atoi(pj_optarg);
			break;

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
			case OPT_IPV6:
			cfg->ipv6 = PJ_TRUE;
			break;
#endif
		case OPT_QOS:
			cfg->enable_qos = PJ_TRUE;
			/* Set RTP traffic type to Voice */
			cfg->rtp_cfg.qos_type = PJ_QOS_TYPE_VOICE;
			/* Directly apply DSCP value to SIP traffic. Say lets
			 * set it to CS3 (DSCP 011000). Note that this will not
			 * work on all platforms.
			 */
			cfg->udp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
			cfg->udp_cfg.qos_params.dscp_val = 0x18;
			break;
		case OPT_VIDEO:
			cfg->vid.vid_cnt = 1;
			cfg->vid.in_auto_show = PJ_TRUE;
			cfg->vid.out_auto_transmit = PJ_TRUE;
			break;
		case OPT_EXTRA_AUDIO:
			cfg->aud_cnt++;
			break;

		case OPT_VCAPTURE_DEV:
			cfg->vid.vcapture_dev = atoi(pj_optarg);
			cur_acc->vid_cap_dev = cfg->vid.vcapture_dev;
			break;

		case OPT_VRENDER_DEV:
			cfg->vid.vrender_dev = atoi(pj_optarg);
			cur_acc->vid_rend_dev = cfg->vid.vrender_dev;
			break;

		case OPT_PLAY_AVI:
			if (app_config.avi_cnt >= PJSUA_APP_MAX_AVI)
			{
				PJ_LOG(1, (THIS_FILE, "Too many AVIs"));
				return -1;
			}
			app_config.avi[app_config.avi_cnt++].path = pj_str(pj_optarg);
			break;

		case OPT_AUTO_PLAY_AVI:
			app_config.avi_auto_play = PJ_TRUE;
			break;

		case OPT_USE_CLI:
			cfg->use_cli = PJ_TRUE;
			break;

		case OPT_CLI_TELNET_PORT:
			cfg->cli_cfg.telnet_cfg.port = (pj_uint16_t) atoi(pj_optarg);
			cfg->cli_cfg.cli_fe |= CLI_FE_TELNET;
			break;

		case OPT_DISABLE_CLI_CONSOLE:
			cfg->cli_cfg.cli_fe &= (~CLI_FE_CONSOLE);
			break;

		default:
			PJ_LOG(1,
					(THIS_FILE, "Argument \"%s\" is not valid. Use --help to see help", argv[pj_optind-1]));
			return -1;
		}
	}

	if (pj_optind != argc)
	{
		pj_str_t uri_arg;

		if (pjsua_verify_url(argv[pj_optind]) != PJ_SUCCESS)
		{
			PJ_LOG(1, (THIS_FILE, "Invalid SIP URI %s", argv[pj_optind]));
			return -1;
		}
		uri_arg = pj_str(argv[pj_optind]);
		if (uri_to_call)
			*uri_to_call = uri_arg;
		pj_optind++;

		/* Add URI to call to buddy list if it's not already there */
		for (i = 0; i < cfg->buddy_cnt; ++i)
		{
			if (pj_stricmp(&cfg->buddy_cfg[i].uri, &uri_arg) == 0)
				break;
		}
		if (i == cfg->buddy_cnt && cfg->buddy_cnt < PJSUA_MAX_BUDDIES)
		{
			cfg->buddy_cfg[cfg->buddy_cnt++].uri = uri_arg;
		}

	}
	else
	{
		if (uri_to_call)
			uri_to_call->slen = 0;
	}

	if (pj_optind != argc)
	{
		PJ_LOG(1, (THIS_FILE, "Error: unknown options %s", argv[pj_optind]));
		return PJ_EINVAL;
	}

	if (cfg->acc_cfg[cfg->acc_cnt].id.slen)
		cfg->acc_cnt++;

	for (i = 0; i < cfg->acc_cnt; ++i)
	{
		pjsua_acc_config *acfg = &cfg->acc_cfg[i];

		if (acfg->cred_info[acfg->cred_count].username.slen)
		{
			acfg->cred_count++;
		}

		if (acfg->ice_cfg.enable_ice)
		{
			acfg->ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
		}
		if (acfg->turn_cfg.enable_turn)
		{
			acfg->turn_cfg_use = PJSUA_TURN_CONFIG_USE_CUSTOM;
		}

		/* When IMS mode is enabled for the account, verify that settings
		 * are okay.
		 */
		/* For now we check if IMS mode is activated by looking if
		 * initial_auth is set.
		 */
		if (acfg->auth_pref.initial_auth && acfg->cred_count)
		{
			/* Realm must point to the real domain */
			if (*acfg->cred_info[0].realm.ptr == '*')
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: cannot use '*' as realm with IMS"));
				return PJ_EINVAL;
			}

			/* Username for authentication must be in a@b format */
			if (strchr(acfg->cred_info[0].username.ptr, '@') == 0)
			{
				PJ_LOG(1,
						(THIS_FILE, "Error: Username for authentication must " "be in user@domain format with IMS"));
				return PJ_EINVAL;
			}
		}
	}
	return PJ_SUCCESS;
}
#endif


static PJ_DEF(void) pj_pjsip_log_cb(int level, const char *buffer, int len)
{
	zassert(buffer != NULL);
	if(!len)
		return;
/*	static const char *ltexts[] = { "FATAL:", "ERROR:", " WARN:",
			      " INFO:", "DEBUG:", "TRACE:", "DETRC:"};*/
	/* Copy to terminal/file. */
	if (pj_log_get_decor() & PJ_LOG_HAS_COLOR)
	{
#if defined(PJ_TERM_HAS_COLOR) && PJ_TERM_HAS_COLOR != 0
		pj_term_set_color(pj_log_get_color(level));
#else
		PJ_UNUSED_ARG(level);
#endif
		switch (level)
		{
		case (6):
			zlog_other(ZLOG_SIP, LOG_TRAP, "%s", buffer);
			break;
		case 4:
			zlog_other(ZLOG_SIP, LOG_DEBUG, "%s", buffer);
			break;
		case 3:
			zlog_other(ZLOG_SIP, LOG_INFO, "%s", buffer);
			break;
		case 5:
			zlog_other(ZLOG_SIP, LOG_NOTICE, "%s", buffer);
			break;
		case 2:
			zlog_other(ZLOG_SIP, LOG_WARNING, "%s", buffer);
			break;
		case 1:
			zlog_other(ZLOG_SIP, LOG_ERR, "%s", buffer);
			break;
		case 0:
			zlog_other(ZLOG_SIP, LOG_CRIT, "%s", buffer);
			break;
/*		case LOG_ALERT:
			zlog_other(ZLOG_SIP, LOG_ALERT + 1, "%s", buffer);
			break;
		case LOG_EMERG:
			zlog_other(ZLOG_SIP, LOG_EMERG + 1, "%s", buffer);
			break;*/
		default:
			break;
		}
		//printf("%s", buffer);
#if defined(PJ_TERM_HAS_COLOR) && PJ_TERM_HAS_COLOR != 0
		/* Set terminal to its default color */
		pj_term_set_color(pj_log_get_color(77));
#endif
	}
	else
	{
		switch (level)
		{
		case (6):
			zlog_other(ZLOG_SIP, LOG_TRAP, "%s", buffer);
			break;
		case 4:
			zlog_other(ZLOG_SIP, LOG_DEBUG, "%s", buffer);
			break;
		case 3:
			zlog_other(ZLOG_SIP, LOG_INFO, "%s", buffer);
			break;
		case 5:
			zlog_other(ZLOG_SIP, LOG_NOTICE, "%s", buffer);
			break;
		case 2:
			zlog_other(ZLOG_SIP, LOG_WARNING, "%s", buffer);
			break;
		case 1:
			zlog_other(ZLOG_SIP, LOG_ERR, "%s", buffer);
			break;
		case 0:
			zlog_other(ZLOG_SIP, LOG_CRIT, "%s", buffer);
			break;
		default:
			break;
		}
	}
}

int pl_pjsip_log_file(pjsua_app_config *cfg, char *logfile)
{
	zassert(cfg != NULL);
	zassert(logfile != NULL);
	if (cfg->log_cfg.decor)
	{
		close(cfg->log_cfg.decor);
		cfg->log_cfg.decor = -1;
	}
	if (cfg->log_cfg.log_filename.slen)
	{
		free(cfg->log_cfg.log_filename.ptr);
		cfg->log_cfg.log_filename.ptr = NULL;
		cfg->log_cfg.log_filename.slen = 0;
	}
	cfg->log_cfg.log_filename.ptr = strdup(logfile);
	cfg->log_cfg.log_filename.slen = strlen(logfile);
	return OK;
}

int pl_pjsip_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-6 " "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.level = level;
	pj_log_set_level(level);
	return OK;
}

int pl_pjsip_app_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-6 " "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.console_level = level;
	return OK;
}

int pl_pjsip_log_option(pjsua_app_config *cfg, int option, BOOL enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.log_file_flags |= option;
	else
		cfg->log_cfg.log_file_flags &= ~option;
	return OK;
}

int pl_pjsip_log_color(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.decor |= PJ_LOG_HAS_COLOR;
	else
		cfg->log_cfg.decor &= ~PJ_LOG_HAS_COLOR;
	return OK;
}

int pl_pjsip_log_light_bg(pjsua_app_config *cfg, BOOL enable)
{
	pj_log_set_color(1, PJ_TERM_COLOR_R);
	pj_log_set_color(2, PJ_TERM_COLOR_R | PJ_TERM_COLOR_G);
	pj_log_set_color(3, PJ_TERM_COLOR_B | PJ_TERM_COLOR_G);
	pj_log_set_color(4, 0);
	pj_log_set_color(5, 0);
	pj_log_set_color(77, 0);
	return OK;
}

int pl_pjsip_null_audio(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->null_audio = enable;
	return OK;
}

int pl_pjsip_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting value between " "8000-192000 for conference clock rate"));
		return ERROR;
	}
	cfg->media_cfg.clock_rate = (unsigned) lval;
	return OK;
}

int pl_pjsip_snd_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting value between " "8000-192000 for sound device clock rate"));
		return ERROR;
	}
	cfg->media_cfg.snd_clock_rate = (unsigned) lval;
	return OK;
}

int pl_pjsip_stereo(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->media_cfg.channel_count = 2;
	return OK;
}

int pl_pjsip_local_port(pjsua_app_config *cfg, u_int16 lval)
{
	zassert(cfg != NULL);
	if (lval < 0 || lval > 65535)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value for " "--local-port"));
		return ERROR;
	}
	cfg->udp_cfg.port = (pj_uint16_t) lval;
	return OK;
}

int pl_pjsip_public_address(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->udp_cfg.public_addr.slen)
	{
		free(cfg->udp_cfg.public_addr.ptr);
		cfg->udp_cfg.public_addr.ptr = NULL;
		cfg->udp_cfg.public_addr.slen = 0;
	}
	cfg->udp_cfg.public_addr.ptr = strdup(lval);
	cfg->udp_cfg.public_addr.slen = strlen(lval);

	if (cfg->rtp_cfg.public_addr.slen)
	{
		free(cfg->rtp_cfg.public_addr.ptr);
		cfg->rtp_cfg.public_addr.ptr = NULL;
		cfg->rtp_cfg.public_addr.slen = 0;
	}
	cfg->rtp_cfg.public_addr.ptr = strdup(lval);
	cfg->rtp_cfg.public_addr.slen = strlen(lval);

	return OK;
}

int pl_pjsip_bound_address(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	if (cfg->udp_cfg.bound_addr.slen)
	{
		free(cfg->udp_cfg.bound_addr.ptr);
		cfg->udp_cfg.bound_addr.ptr = NULL;
		cfg->udp_cfg.bound_addr.slen = 0;
	}
	if(lval)
	{
		cfg->udp_cfg.bound_addr.ptr = strdup(lval);
		cfg->udp_cfg.bound_addr.slen = strlen(lval);
	}
	if (cfg->rtp_cfg.bound_addr.slen)
	{
		free(cfg->rtp_cfg.bound_addr.ptr);
		cfg->rtp_cfg.bound_addr.ptr = NULL;
		cfg->rtp_cfg.bound_addr.slen = 0;
	}
	if(lval)
	{
		cfg->rtp_cfg.bound_addr.ptr = strdup(lval);
		cfg->rtp_cfg.bound_addr.slen = strlen(lval);
	}
	return OK;
}

int pl_pjsip_no_udp(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	if (cfg->no_tcp && !cfg->use_tls)
	{
		PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
		return ERROR;
	}

	cfg->no_udp = PJ_TRUE;
	return OK;
}

int pl_pjsip_no_tcp(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	if (cfg->no_udp && !cfg->use_tls)
	{
		PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
		return ERROR;
	}

	cfg->no_tcp = PJ_TRUE;
	return OK;
}

int pl_pjsip_norefersub(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->no_refersub = PJ_TRUE;
	return OK;
}

int pl_pjsip_proxy(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->proxy[cur_acc->proxy_cnt].slen)
	{
		free(cur_acc->proxy[cur_acc->proxy_cnt].ptr);
		cur_acc->proxy[cur_acc->proxy_cnt].ptr = NULL;
		cur_acc->proxy[cur_acc->proxy_cnt].slen = 0;
	}
	cur_acc->proxy[cur_acc->proxy_cnt].ptr = strdup(lval);
	cur_acc->proxy[cur_acc->proxy_cnt].slen = strlen(lval);
	cur_acc->proxy_cnt++;
	return OK;
}

int pl_pjsip_outbound_proxy(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen)
	{
		free(cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr);
		cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr = NULL;
		cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen = 0;
	}
	cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr = strdup(lval);
	cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen = strlen(lval);
	cfg->cfg.outbound_proxy_cnt++;
	return OK;
}

int pl_pjsip_registrar(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->reg_uri.slen)
	{
		free(cur_acc->reg_uri.ptr);
		cur_acc->reg_uri.ptr = NULL;
		cur_acc->reg_uri.slen = 0;
	}
	cur_acc->reg_uri.ptr = strdup(lval);
	cur_acc->reg_uri.slen = strlen(lval);
	return OK;
}

int pl_pjsip_register_timeout(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 1 || lval > 3600)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --reg-timeout " "(expecting 1-3600)"));
		return ERROR;
	}
	cur_acc->reg_timeout = (unsigned) lval;
	return OK;
}

int pl_pjsip_publish(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->publish_enabled = enable;
	return OK;
}

int pl_pjsip_mwi(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->mwi_enabled = enable;
	return OK;
}

int pl_pjsip_100rel(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (enable)
	{
		cur_acc->require_100rel = PJSUA_100REL_MANDATORY;
		cfg->cfg.require_100rel = PJSUA_100REL_MANDATORY;
	}
	else
	{
		cur_acc->require_100rel = PJSUA_100REL_NOT_USED;
		cfg->cfg.require_100rel = PJSUA_100REL_NOT_USED;
	}
	return OK;
}

int pl_pjsip_session_timer(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 0 || lval > 3)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-3 for --use-timer"));
		return ERROR;
	}
	cur_acc->use_timer = (pjsua_sip_timer_use) lval;
	cfg->cfg.use_timer = (pjsua_sip_timer_use) lval;
	return OK;
}

int pl_pjsip_session_timer_expiration(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 90)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --timer-se " "(expecting higher than 90)"));
		return ERROR;
	}
	cur_acc->timer_setting.sess_expires = (pjsua_sip_timer_use) lval;
	cfg->cfg.timer_setting.sess_expires = (pjsua_sip_timer_use) lval;
	return OK;
}

int pl_pjsip_session_timer_expiration_min(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 90)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --timer-se " "(expecting higher than 90)"));
		return ERROR;
	}
	cur_acc->timer_setting.min_se = (pjsua_sip_timer_use) lval;
	cfg->cfg.timer_setting.min_se = (pjsua_sip_timer_use) lval;
	return OK;
}

int pl_pjsip_outbound_reg_id(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->rfc5626_reg_id.slen)
	{
		free(cur_acc->rfc5626_reg_id.ptr);
		cur_acc->rfc5626_reg_id.ptr = NULL;
		cur_acc->rfc5626_reg_id.slen = 0;
	}
	cur_acc->rfc5626_reg_id.ptr = strdup(lval);
	cur_acc->rfc5626_reg_id.slen = strlen(lval);
	return OK;
}

int pl_pjsip_ims(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->auth_pref.initial_auth = enable;
	return OK;
}

int pl_pjsip_url_id(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in local id argument", lval));
		return ERROR;
	}
	if (cur_acc->id.slen)
	{
		free(cur_acc->id.ptr);
		cur_acc->id.ptr = NULL;
		cur_acc->id.slen = 0;
	}
	cur_acc->id.ptr = strdup(lval);
	cur_acc->id.slen = strlen(lval);
	return OK;
}

int pl_pjsip_contact(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact argument", lval));
		return ERROR;
	}
	if (cur_acc->force_contact.slen)
	{
		free(cur_acc->force_contact.ptr);
		cur_acc->force_contact.ptr = NULL;
		cur_acc->force_contact.slen = 0;
	}
	cur_acc->force_contact.ptr = strdup(lval);
	cur_acc->force_contact.slen = strlen(lval);
	return OK;
}

int pl_pjsip_contact_params(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact_params argument", lval));
		return ERROR;
	}
	if (cur_acc->contact_params.slen)
	{
		free(cur_acc->contact_params.ptr);
		cur_acc->contact_params.ptr = NULL;
		cur_acc->contact_params.slen = 0;
	}
	cur_acc->contact_params.ptr = strdup(lval);
	cur_acc->contact_params.slen = strlen(lval);
	return OK;
}

int pl_pjsip_contact_uri_params(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact_uri_params argument", lval));
		return ERROR;
	}
	if (cur_acc->contact_uri_params.slen)
	{
		free(cur_acc->contact_uri_params.ptr);
		cur_acc->contact_uri_params.ptr = NULL;
		cur_acc->contact_uri_params.slen = 0;
	}
	cur_acc->contact_uri_params.ptr = strdup(lval);
	cur_acc->contact_uri_params.slen = strlen(lval);
	return OK;
}

int pl_pjsip_update_nat(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->allow_contact_rewrite = enable;
	return OK;
}

int pl_pjsip_stun(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (enable)
	{
		cur_acc->sip_stun_use = PJSUA_STUN_USE_DEFAULT;
		cur_acc->media_stun_use = PJSUA_STUN_USE_DEFAULT;
	}
	else
	{
		cur_acc->sip_stun_use = PJSUA_STUN_USE_DISABLED;
		cur_acc->media_stun_use = PJSUA_STUN_USE_DISABLED;
	}
	return OK;
}

int pl_pjsip_compact_form(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	extern pj_bool_t pjsip_include_allow_hdr_in_dlg;
	extern pj_bool_t pjmedia_add_rtpmap_for_static_pt;
	pjsip_cfg()->endpt.use_compact_form = enable;
	/* do not transmit Allow header */
	pjsip_include_allow_hdr_in_dlg = PJ_FALSE;
	/* Do not include rtpmap for static payload types (<96) */
	pjmedia_add_rtpmap_for_static_pt = PJ_FALSE;
	return OK;
}

int pl_pjsip_accept_redirect(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	cfg->redir_op = lval;
	return OK;
}

int pl_pjsip_no_force_lr(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->cfg.force_lr = enable;
	return OK;
}

int pl_pjsip_next_account(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->acc_cnt++;
	return OK;
}

int pl_pjsip_username(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].username.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].username.ptr);
		cur_acc->cred_info[cur_acc->cred_count].username.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].username.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].username.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].username.slen = strlen(lval);

/*	if (cur_acc->cred_info[cur_acc->cred_count].scheme.slen == 0)
	{
		cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = strdup("digest");
		cur_acc->cred_info[cur_acc->cred_count].scheme.slen = strlen("digest");
	}*/
	return OK;
}

int pl_pjsip_scheme(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].scheme.slen == 0)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].scheme.ptr);
		cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].scheme.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = strdup("digest");
	cur_acc->cred_info[cur_acc->cred_count].scheme.slen = strlen("digest");
	return OK;
}

int pl_pjsip_realm(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].realm.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].realm.ptr);
		cur_acc->cred_info[cur_acc->cred_count].realm.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].realm.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].realm.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].realm.slen = strlen(lval);
	return OK;
}

int pl_pjsip_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].data.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].data.ptr);
		cur_acc->cred_info[cur_acc->cred_count].data.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].data.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].data_type =
			PJSIP_CRED_DATA_PLAIN_PASSWD;
	cur_acc->cred_info[cur_acc->cred_count].data.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].data.slen = strlen(lval);
#if 0//PJSIP_HAS_DIGEST_AKA_AUTH
	cur_acc->cred_info[cur_acc->cred_count].data_type |= PJSIP_CRED_DATA_EXT_AKA;
	if(cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr);
		cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen = strlen(lval);
	//cur_acc->cred_info[cur_acc->cred_count].ext.aka.k = pj_str(pj_optarg);
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.cb = &pjsip_auth_create_aka_response;
#endif
	return OK;
}

int pl_pjsip_reg_retry_interval(pjsua_app_config *cfg, int interval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->reg_retry_interval = (unsigned) interval;
	return OK;
}

int pl_pjsip_reg_use_proxy(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (value > 4)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --reg-use-proxy value '%d'",value));
		return ERROR;
	}
	cur_acc->reg_use_proxy = (unsigned) value;
	return OK;
}

int pl_pjsip_credential(pjsua_app_config *cfg, int credential)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->cred_count++;
	return OK;
}

int pl_pjsip_nameserver(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);

	//pjsua_acc_config *cur_acc;
	if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	//cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
	if (cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen)
	{
		free(cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr);
		cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr = NULL;
		cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen = 0;
	}
	cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr = strdup(lval);
	cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen = strlen(lval);
	cfg->cfg.nameserver_count++;
	return OK;
}

int pl_pjsip_stunserver(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	//pjsua_acc_config *cur_acc;
	if (cfg->cfg.stun_srv_cnt == PJ_ARRAY_SIZE(cfg->cfg.stun_srv))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many STUN servers"));
		return ERROR;
	}

	//cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
	if (cfg->cfg.stun_host.slen)
	{
		free(cfg->cfg.stun_host.ptr);
		cfg->cfg.stun_host.ptr = NULL;
		cfg->cfg.stun_host.slen = 0;
	}
	cfg->cfg.stun_host.ptr = strdup(lval);
	cfg->cfg.stun_host.slen = strlen(lval);
	if (cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen)
	{
		free(cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr);
		cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr = NULL;
		cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen = 0;
	}
	cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr = strdup(lval);
	cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen = strlen(lval);
	cfg->cfg.stun_srv_cnt++;
	return OK;
}

int pl_pjsip_buddy_list(pjsua_app_config *cfg, char * lval)
{
	//pjsua_acc_config *cur_acc;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid URL '%s' in " "--add-buddy option", lval));
		return ERROR;
	}
	if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	if (cfg->buddy_cfg[cfg->buddy_cnt].uri.slen)
	{
		free(cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr);
		cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr = NULL;
		cfg->buddy_cfg[cfg->buddy_cnt].uri.slen = 0;
	}
	cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr = strdup(lval);
	cfg->buddy_cfg[cfg->buddy_cnt].uri.slen = strlen(lval);
	//cfg->buddy_cfg[cfg->buddy_cnt].uri = pj_str(pj_optarg);
	cfg->buddy_cnt++;
	return OK;
}

int pl_pjsip_auto_play(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	cfg->auto_play = enable;
	return OK;
}

int pl_pjsip_auto_play_hangup(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	cfg->auto_play_hangup = enable;
	return OK;
}

int pl_pjsip_auto_rec(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->auto_rec = enable;
	return OK;
}

int pl_pjsip_auto_loop(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->auto_loop = enable;
	return OK;
}

int pl_pjsip_auto_config(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->auto_conf = enable;
	return OK;
}

int pl_pjsip_play_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->wav_files[cfg->wav_count].slen)
	{
		free(cfg->wav_files[cfg->wav_count].ptr);
		cfg->wav_files[cfg->wav_count].ptr = NULL;
		cfg->wav_files[cfg->wav_count].slen = 0;
	}
	cfg->wav_files[cfg->wav_count].ptr = strdup(lval);
	cfg->wav_files[cfg->wav_count].slen = strlen(lval);
	cfg->wav_count++;
	return OK;
}

int pl_pjsip_play_tone(pjsua_app_config *cfg, int f1, int f2, int on, int off)
{
	zassert(cfg != NULL);
	cfg->tones[cfg->tone_count].freq1 = (short) f1;
	cfg->tones[cfg->tone_count].freq2 = (short) f2;
	cfg->tones[cfg->tone_count].on_msec = (short) on;
	cfg->tones[cfg->tone_count].off_msec = (short) off;
	++cfg->tone_count;
	return OK;
}

int pl_pjsip_rec_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->rec_file.slen)
	{
		free(cfg->rec_file.ptr);
		cfg->rec_file.ptr = NULL;
		cfg->rec_file.slen = 0;
	}
	cfg->auto_rec = PJ_TRUE;
	cfg->rec_file.ptr = strdup(lval);
	cfg->rec_file.slen = strlen(lval);
	return OK;
}

int pl_pjsip_ice_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	//pjsua_acc_config *acfg = &cfg->acc_cfg[i];
	cfg->media_cfg.enable_ice = cur_acc->ice_cfg.enable_ice = enable;
	if(enable)
	{
		cur_acc->ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
	}
	return OK;
}

int pl_pjsip_regular_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_opt.aggressive = cur_acc->ice_cfg.ice_opt.aggressive =
			enable;
	return OK;
}

int pl_pjsip_turn_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.enable_turn = cur_acc->turn_cfg.enable_turn = enable;
	if(enable)
	{
		cur_acc->turn_cfg_use = PJSUA_TURN_CONFIG_USE_CUSTOM;
	}
	return OK;
}

int pl_pjsip_ice_max_hosts(pjsua_app_config *cfg, int maxnum)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_max_host_cands = cur_acc->ice_cfg.ice_max_host_cands =
			maxnum;
	return OK;
}

int pl_pjsip_ice_nortcp_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_no_rtcp = cur_acc->ice_cfg.ice_no_rtcp = enable;
	return OK;
}

int pl_pjsip_turn_srv(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cfg->media_cfg.turn_server.slen)
	{
		free(cfg->media_cfg.turn_server.ptr);
		cfg->media_cfg.turn_server.ptr = NULL;
		cfg->media_cfg.turn_server.slen = 0;
	}
	cfg->media_cfg.turn_server.ptr = strdup(lval);
	cfg->media_cfg.turn_server.slen = strlen(lval);

	if (cur_acc->turn_cfg.turn_server.slen)
	{
		free(cur_acc->turn_cfg.turn_server.ptr);
		cur_acc->turn_cfg.turn_server.ptr = NULL;
		cur_acc->turn_cfg.turn_server.slen = 0;
	}
	cur_acc->turn_cfg.turn_server.ptr = strdup(lval);
	cur_acc->turn_cfg.turn_server.slen = strlen(lval);
	return OK;
}

int pl_pjsip_turn_tcp(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.turn_conn_type = cur_acc->turn_cfg.turn_conn_type =
			PJ_TURN_TP_TCP;
	return OK;
}

int pl_pjsip_turn_user(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm, "*");
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm, "*");

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.username);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.username, lval);
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username,
			lval);

	cfg->media_cfg.turn_auth_cred.type = cur_acc->turn_cfg.turn_auth_cred.type =
			PJ_STUN_AUTH_CRED_STATIC;
	return OK;
}

int pl_pjsip_turn_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	/*    pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm);
	 pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm);

	 pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm, "*");
	 pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm, "*");*/

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.data);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.data, lval);
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data, lval);

	cfg->media_cfg.turn_auth_cred.data.static_cred.data_type =
			cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data_type =
					PJ_STUN_PASSWD_PLAIN;
	return OK;
}

int pl_pjsip_rtcp_mux(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	cur_acc->enable_rtcp_mux = PJ_TRUE;
	return OK;
}

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
int pl_pjsip_srtp_enable(pjsua_app_config *cfg, int type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->cfg.use_srtp = type;    //my_atoi(pj_optarg);
	if (cfg->cfg.use_srtp > 3)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --use-srtp option"));
		return ERROR;
	}
	if ((int) cfg->cfg.use_srtp == 3)
	{
		/* SRTP optional mode with duplicated media offer */
		cfg->cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
		cfg->cfg.srtp_optional_dup_offer = PJ_TRUE;
		cur_acc->srtp_optional_dup_offer = PJ_TRUE;
	}
	cur_acc->use_srtp = cfg->cfg.use_srtp;
	return OK;
}

int pl_pjsip_srtp_secure(pjsua_app_config *cfg, int type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->cfg.srtp_secure_signaling = type;    //my_atoi(pj_optarg);
	if (/*!pj_isdigit(*pj_optarg) ||*/
	cfg->cfg.srtp_secure_signaling > 2)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --srtp-secure option"));
		return ERROR;
	}
	cur_acc->srtp_secure_signaling = cfg->cfg.srtp_secure_signaling;
	return OK;
}

int pl_pjsip_srtp_keying(pjsua_app_config *cfg, int type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->srtp_keying = type;    //my_atoi(pj_optarg);
	if (/*!pj_isdigit(*pj_optarg) || */cfg->srtp_keying < 0
			|| cfg->srtp_keying > 1)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --srtp-keying option"));
		return ERROR;
	}
	/* Set SRTP keying to use DTLS over SDES */
	if (cfg->srtp_keying == 1)
	{
		pjsua_srtp_opt *srtp_opt = &cfg->cfg.srtp_opt;
		srtp_opt->keying_count = 2;
		srtp_opt->keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
		srtp_opt->keying[1] = PJMEDIA_SRTP_KEYING_SDES;

		cur_acc->srtp_opt.keying_count = 2;
		cur_acc->srtp_opt.keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
		cur_acc->srtp_opt.keying[1] = PJMEDIA_SRTP_KEYING_SDES;
	}
	return OK;
}
#endif

int pl_pjsip_rtp_port(pjsua_app_config *cfg, int port)
{
	zassert(cfg != NULL);
	if (port == 0)
	{
		enum
		{
			START_PORT = 4000
		};
		unsigned range;

		range = (65535 - START_PORT - PJSUA_MAX_CALLS * 2);
		cfg->rtp_cfg.port = START_PORT + ((pj_rand() % range) & 0xFFFE);
	}
	else
	{
		if (port < 1 || port > 65535)
		{
			PJ_LOG(1,
					(THIS_FILE, "Error: rtp-port argument value " "(expecting 1-65535"));
			return ERROR;
		}
		cfg->rtp_cfg.port = port;
	}
	return OK;
}

int pl_pjsip_dis_codec(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
/*	pj_str_clr(&cfg->codec_dis[cfg->codec_dis_cnt]);
	pj_str_set(&cfg->codec_dis[cfg->codec_dis_cnt], lval);
	cfg->codec_dis_cnt++;*/
	cfg->codec_dis[cfg->codec_dis_cnt] = pj_str(lval);
	cfg->codec_dis_cnt++;
	//cfg->codec_dis[cfg->codec_dis_cnt++] = inde;
	return OK;
}

int pl_pjsip_add_codec(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
/*	pj_str_clr(&cfg->codec_arg[cfg->codec_cnt]);
	pj_str_set(&cfg->codec_arg[cfg->codec_cnt], lval);
	cfg->codec_cnt++;
*/
	cfg->codec_arg[cfg->codec_cnt] = pj_str(lval);
	cfg->codec_cnt++;
	//cfg->codec_arg[cfg->codec_cnt++] = inde;
	return OK;
}

int pl_pjsip_duration(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->duration = value;
	return OK;
}

int pl_pjsip_thread_cnt(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 128)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --thread-cnt option"));
		return ERROR;
	}
	cfg->cfg.thread_cnt = value;
	return OK;
}

int pl_pjsip_ptime(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 10 || value > 1000)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --ptime option"));
		return ERROR;
	}
	cfg->media_cfg.ptime = value;
	return OK;
}

int pl_pjsip_novad(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.no_vad = enable;
	return OK;
}

int pl_pjsip_ec_tial(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 1000)
	{
		PJ_LOG(1,
				(THIS_FILE, "I think the ec-tail length setting " "is too big"));
		return ERROR;
	}
	cfg->media_cfg.ec_tail_len = value;
	return OK;
}

int pl_pjsip_ec_options(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.ec_options = value;
	return OK;
}

int pl_pjsip_quality(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 10)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --quality (expecting 0-10"));
		return ERROR;
	}
	cfg->media_cfg.quality = value;
	return OK;
}

int pl_pjsip_ilbc_mode(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value != 20 && value != 30)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --ilbc-mode (expecting 20 or 30"));
		return ERROR;
	}
	cfg->media_cfg.ilbc_mode = value;
	return OK;
}

int pl_pjsip_rx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.rx_drop_pct > 100)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --rx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.rx_drop_pct = value;
	return OK;
}

int pl_pjsip_tx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.tx_drop_pct > 100)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --tx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.tx_drop_pct = value;
	return OK;
}

int pl_pjsip_auto_answer(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 100 || value > 699)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid code in --auto-answer " "(expecting 100-699"));
		return ERROR;
	}
	cfg->auto_answer = value;
	return OK;
}

int pl_pjsip_max_calls(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 1 || value > PJSUA_MAX_CALLS)
	{
		PJ_LOG(1,
				(THIS_FILE,"Error: maximum call setting exceeds " "compile time limit (PJSUA_MAX_CALLS=%d)", PJSUA_MAX_CALLS));
		return ERROR;
	}
	cfg->cfg.max_calls = value;
	return OK;
}

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
int pl_pjsip_tls_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->use_tls = enable;
	return OK;
}

int pl_pjsip_tls_ca_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.ca_list_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.ca_list_file, lval);
	return OK;
}

int pl_pjsip_tls_cert_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.cert_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.cert_file, lval);
	return OK;
}

int pl_pjsip_tls_priv_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.privkey_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.privkey_file, lval);
	return OK;
}

int pl_pjsip_tls_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.password);
	pj_str_set(&cfg->udp_cfg.tls_setting.password, lval);
	return OK;
}

int pl_pjsip_tls_verify_server(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_server = enable;
	return OK;
}

int pl_pjsip_tls_verify_client(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_client = enable;
	cfg->udp_cfg.tls_setting.require_client_cert = enable;
	return OK;
}

int pl_pjsip_tls_neg_timeout(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.timeout.sec = value;
	return OK;
}

int pl_pjsip_tls_cipher(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_ssl_cipher cipher;

	if (pj_ansi_strnicmp(lval, "0x", 2) == 0)
	{
		pj_str_t cipher_st = pj_str(lval + 2);
		cipher = pj_strtoul2(&cipher_st, NULL, 16);
	}
	else
	{
		cipher = atoi(lval);
	}

	if (pj_ssl_cipher_is_supported(cipher))
	{
		static pj_ssl_cipher tls_ciphers[PJ_SSL_SOCK_MAX_CIPHERS];

		tls_ciphers[cfg->udp_cfg.tls_setting.ciphers_num++] = cipher;
		cfg->udp_cfg.tls_setting.ciphers = tls_ciphers;
	}
	else
	{
		pj_ssl_cipher ciphers[PJ_SSL_SOCK_MAX_CIPHERS];
		unsigned int j = 0, ciphers_cnt = 0;

		ciphers_cnt = PJ_ARRAY_SIZE(ciphers);
		pj_ssl_cipher_get_availables(ciphers, &ciphers_cnt);

		PJ_LOG(1,
				(THIS_FILE, "Cipher \"%s\" is not supported by " "TLS/SSL backend.", lval));
		printf("Available TLS/SSL ciphers (%d):\n", ciphers_cnt);
		for (j = 0; j < ciphers_cnt; ++j)
		printf("- 0x%06X: %s\n", ciphers[j],
				pj_ssl_cipher_name(ciphers[j]));
		return -1;
	}
	return OK;
}
#endif

int pl_pjsip_capture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	return OK;
}

int pl_pjsip_capture_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_lat = value;
	return OK;
}

int pl_pjsip_playback_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	return OK;
}

int pl_pjsip_playback_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->playback_lat = value;
	return OK;
}

int pl_pjsip_snd_auto_close(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.snd_auto_close_time = value;
	return OK;
}

int pl_pjsip_no_tones(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->no_tones = enable;
	return OK;
}

int pl_pjsip_jb_max_size(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.jb_max = value;
	return OK;
}

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
int pl_pjsip_ipv6_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->ipv6 = enable;
	return OK;
}
#endif

int pl_pjsip_qos_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->enable_qos = enable;
	/* Set RTP traffic type to Voice */
	cfg->rtp_cfg.qos_type = PJ_QOS_TYPE_VOICE;
	/* Directly apply DSCP value to SIP traffic. Say lets
	 * set it to CS3 (DSCP 011000). Note that this will not
	 * work on all platforms.
	 */
	cfg->udp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	cfg->udp_cfg.qos_params.dscp_val = 0x18;
	return OK;
}

int pl_pjsip_video_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->vid.vid_cnt = 1;
	cfg->vid.in_auto_show = PJ_TRUE;
	cfg->vid.out_auto_transmit = PJ_TRUE;
	return OK;
}

int pl_pjsip_extra_audio(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->aud_cnt++;
	return OK;
}

int pl_pjsip_vcapture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;
	cur_acc->vid_cap_dev = cfg->vid.vcapture_dev = value;
	return OK;
}

int pl_pjsip_vrender_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;
	cur_acc->vid_rend_dev = cfg->vid.vrender_dev = value;
	return OK;
}

int pl_pjsip_play_avi(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;

	if (cfg->avi_cnt >= PJSUA_APP_MAX_AVI)
	{
		PJ_LOG(1, (THIS_FILE, "Too many AVIs"));
		return ERROR;
	}

	//cur_acc->vid_rend_dev = cfg->vid.vrender_dev = value;

	pj_str_clr(&cfg->avi[cfg->avi_cnt].path);
	pj_str_set(&cfg->avi[cfg->avi_cnt].path, lval);
	cfg->avi_cnt++;
	return OK;
}

int pl_pjsip_auto_play_avi(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->avi_auto_play = enable;
	return OK;
}

int pl_pjsip_cli_enable(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	cfg->use_cli = enable;
	return OK;
}

int pl_pjsip_cli_telnet_port(pjsua_app_config *cfg, int port)
{
	zassert(cfg != NULL);
	cfg->cli_cfg.telnet_cfg.port = port;
	cfg->cli_cfg.cli_fe |= CLI_FE_TELNET;
	return OK;
}

int pl_pjsip_cli_console(pjsua_app_config *cfg, BOOL enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->cli_cfg.cli_fe &= (~CLI_FE_CONSOLE);
	else
		cfg->cli_cfg.cli_fe |= (CLI_FE_CONSOLE);
	return OK;
}








/*
 #ifdef _IONBF
 case OPT_STDOUT_NO_BUF:
 setvbuf(stdout, NULL, _IONBF, 0);
 cfg->log_cfg.cb = &log_writer_nobuf;
 break;
 #endif
 */

/* Set default config. */
void pjsip_default_config()
{
	char tmp[80];
	unsigned i;
	pjsua_app_config *cfg = &app_config;
	zassert(cfg != NULL);
	pjsua_config_default(&cfg->cfg);
	pj_ansi_sprintf(tmp, "PJSUA v%s %s", pj_get_version(),
			pj_get_sys_info()->info.ptr);
	pj_strdup2_with_null(app_config.pool, &cfg->cfg.user_agent, tmp);

	pjsua_logging_config_default(&cfg->log_cfg);
	cfg->log_cfg.cb = pj_pjsip_log_cb;
	pj_log_set_decor(PJ_LOG_HAS_NEWLINE|PJ_LOG_HAS_CR|PJ_LOG_HAS_INDENT|PJ_LOG_HAS_THREAD_SWC|
			PJ_LOG_HAS_SENDER|PJ_LOG_HAS_THREAD_ID);

	pjsua_media_config_default(&cfg->media_cfg);
	cfg->media_cfg.clock_rate = PJSIP_DEFAULT_CLOCK_RATE;
    cfg->media_cfg.snd_clock_rate = PJSIP_DEFAULT_CLOCK_RATE;


	pjsua_transport_config_default(&cfg->udp_cfg);
	cfg->udp_cfg.port = 5060;
	pjsua_transport_config_default(&cfg->rtp_cfg);
	cfg->rtp_cfg.port = 40000;
	cfg->redir_op = PJSIP_REDIRECT_ACCEPT_REPLACE;
	cfg->duration = PJSUA_APP_NO_LIMIT_DURATION;
	cfg->wav_id = PJSUA_INVALID_ID;
	cfg->rec_id = PJSUA_INVALID_ID;
	cfg->wav_port = PJSUA_INVALID_ID;
	cfg->rec_port = PJSUA_INVALID_ID;
	cfg->speaker_level = 1.0;
	cfg->mic_level = 1.5;
	cfg->capture_dev = PJSUA_INVALID_ID;
	cfg->playback_dev = PJSUA_INVALID_ID;
	cfg->capture_lat = PJMEDIA_SND_DEFAULT_REC_LATENCY;
	cfg->playback_lat = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
	cfg->ringback_slot = PJSUA_INVALID_ID;
	cfg->ring_slot = PJSUA_INVALID_ID;

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->acc_cfg); ++i)
		pjsua_acc_config_default(&cfg->acc_cfg[i]);

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->buddy_cfg); ++i)
		pjsua_buddy_config_default(&cfg->buddy_cfg[i]);

	cfg->vid.vcapture_dev = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
	cfg->vid.vrender_dev = PJMEDIA_VID_DEFAULT_RENDER_DEV;
	cfg->aud_cnt = 0;

	cfg->avi_def_idx = PJSUA_INVALID_ID;

#ifdef PL_PJSIP_CLI_SHELL
	cfg->use_cli = FALSE;
	//cfg->cli_cfg.cli_fe = CLI_FE_CONSOLE;
	//cfg->cli_cfg.telnet_cfg.port = 0;
	cfg->cli_cfg.cli_fe = CLI_FE_TELNET;
	cfg->cli_cfg.telnet_cfg.port = 2323;

#ifdef PL_PJSIP_CALL_SHELL
	cfg->cli_cfg.cli_fe == CLI_FE_SOCKET;
	cfg->cli_cfg.socket_cfg.tcp = FALSE;
#endif
#endif
    /* Add UDP transport. */
	cfg->codec_dis_cnt = 0;
	cfg->codec_cnt = 0;
/*
	cfg->codec_dis_cnt = 0;
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/8000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/16000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/32000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("iLBC/8000");

	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("PCMA");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("GSM");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("G722");


	cfg->codec_cnt = 0;
	cfg->codec_arg[cfg->codec_cnt++] = pj_str("PCMU");
	//cfg->codec_arg[cfg->codec_cnt++] = pj_str("PCMA");
*/
}

#if 1
int pjsip_load_config(void)
{
	int i = 0;
	char buf[128];
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = app_config.acc_cfg;
	zassert(pl_pjsip != NULL);

	if(pl_pjsip->mutex)
		os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);

	pjsip_default_config();

	pl_pjsip_log_level(&app_config, 3);
	pl_pjsip_app_log_level(&app_config, 3);
	pj_log_set_log_func(pj_pjsip_log_cb);


	pl_pjsip_debug_level_set_api(pl_pjsip->debug_level);
	pl_pjsip_debug_detail_set_api(pl_pjsip->debug_detail);


	cur_acc->cred_count = 0;
	if(strlen(pl_pjsip->sip_user.sip_user))
	{
		pl_pjsip_username(&app_config, pl_pjsip->sip_user.sip_user);//Set authentication username
		if(strlen(pl_pjsip->sip_user.sip_password))
		{
			pl_pjsip_password(&app_config, pl_pjsip->sip_user.sip_password);//Set authentication password
		}
		app_config.acc_cnt = 1;
	}
	if(strlen(pl_pjsip->sip_user_sec.sip_user))
	{
		pl_pjsip_username(&app_config, pl_pjsip->sip_user_sec.sip_user);//Set authentication username

		if(strlen(pl_pjsip->sip_user_sec.sip_password))
		{
			pl_pjsip_password(&app_config, pl_pjsip->sip_user_sec.sip_password);//Set authentication password
		}
		app_config.acc_cnt = 2;
	}

	pl_pjsip_scheme(&app_config, "digest");

	if(strlen(pl_pjsip->sip_realm))
	{
		pl_pjsip_realm(&app_config, pl_pjsip->sip_realm);//Set authentication password
	}
	else
		pl_pjsip_realm(&app_config, "*");

	if(strlen(pl_pjsip->sip_server.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_server.sip_address, pl_pjsip->sip_server.sip_port);
		pl_pjsip_registrar(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server

		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s@%s:%d", pl_pjsip->sip_user.sip_user,
				pl_pjsip->sip_server.sip_address, pl_pjsip->sip_server.sip_port);
		pl_pjsip_url_id(&app_config, buf /*"sip:100@192.168.0.103"*/);//Set the URL of local ID (used in From header)

		cur_acc->cred_count = 1;
	}

	if(strlen(pl_pjsip->sip_server_sec.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_server_sec.sip_address, pl_pjsip->sip_server_sec.sip_port);
		pl_pjsip_registrar(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server

		memset(buf, 0, sizeof(buf));
		if(pl_pjsip->sip_user_cnt == 1 && strlen(pl_pjsip->sip_user.sip_user))
			snprintf(buf, sizeof(buf), "sip:%s@%s:%d", pl_pjsip->sip_user.sip_user,
				pl_pjsip->sip_server_sec.sip_address, pl_pjsip->sip_server_sec.sip_port);

		if(pl_pjsip->sip_user_cnt == 2 && strlen(pl_pjsip->sip_user_sec.sip_user))
			snprintf(buf, sizeof(buf), "sip:%s@%s:%d", pl_pjsip->sip_user_sec.sip_user,
				pl_pjsip->sip_server_sec.sip_address, pl_pjsip->sip_server_sec.sip_port);


		pl_pjsip_url_id(&app_config, buf /*"sip:100@192.168.0.103"*/);//Set the URL of local ID (used in From header)

		cur_acc->cred_count = 2;
	}
	//pl_pjsip_credential(&app_config, 0);
	//pl_pjsip_next_account(&app_config);
	if(pl_pjsip->sip_local.sip_port)
		pl_pjsip_local_port(&app_config, pl_pjsip->sip_local.sip_port);

	if(strlen(pl_pjsip->sip_proxy.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d",
				pl_pjsip->sip_proxy.sip_address, pl_pjsip->sip_proxy.sip_port);
		pl_pjsip_proxy(&app_config, buf);
	}

	app_config.no_tcp = PJ_TRUE;
	app_config.no_udp = PJ_FALSE;
	switch(pl_pjsip->proto)
	{
	case PJSIP_PROTO_UDP:
		app_config.no_tcp = PJ_TRUE;
		//pl_pjsip_ice_nortcp_enable(&app_config, TRUE);
		app_config.no_udp = PJ_FALSE;
		break;
	case PJSIP_PROTO_TCP:
	case PJSIP_PROTO_TLS:
	case PJSIP_PROTO_DTLS:
		app_config.no_udp = PJ_TRUE;
		app_config.no_tcp = PJ_FALSE;
		break;
	default:
		break;
	}
	pl_pjsip_100rel(&app_config, pl_pjsip->sip_100_rel);
	//"SIP Account options:"
	//
	if(pl_pjsip->sip_reg_proxy != PJSIP_REGISTER_NONE)
		pl_pjsip_reg_use_proxy(&app_config, pl_pjsip->sip_reg_proxy);
	if(pl_pjsip->sip_rereg_delay)
		pl_pjsip_reg_retry_interval(&app_config, pl_pjsip->sip_rereg_delay);
	pl_pjsip_register_timeout(&app_config, pl_pjsip->sip_expires/*pl_pjsip->sip_reg_timeout*/);

	pl_pjsip_publish(&app_config, pl_pjsip->sip_publish);			//Send presence PUBLISH for this account
	pl_pjsip_mwi(&app_config, pl_pjsip->sip_mwi);
	pl_pjsip_ims(&app_config, pl_pjsip->sip_ims_enable);
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	if(pl_pjsip->sip_srtp_mode != PJSIP_SRTP_DISABLE)
		pl_pjsip_srtp_enable(&app_config, pl_pjsip->sip_srtp_mode);
	if(pl_pjsip->sip_srtp_secure != PJSIP_SRTP_SEC_NO)
	{
		pl_pjsip_srtp_secure(&app_config, pl_pjsip->sip_srtp_secure);
		pl_pjsip_srtp_keying(&app_config, pl_pjsip->sip_srtp_keying);
	}
	//pjsip_srtp_t		sip_srtp_mode;			//Use SRTP?  0:disabled, 1:optional, 2:mandatory,3:optional by duplicating media offer (def:0)
	//pjsip_srtp_sec_t	sip_srtp_secure;		//SRTP require secure SIP? 0:no, 1:tls, 2:sips (def:1)
#endif
	if(pl_pjsip->sip_timer != PJSIP_TIMER_INACTIVE)
		pl_pjsip_session_timer(&app_config, pl_pjsip->sip_timer);
	if(pl_pjsip->sip_timer_sec)
		pl_pjsip_session_timer_expiration(&app_config, pl_pjsip->sip_timer_sec);

	//pl_pjsip_outbound_reg_id(&app_config, pl_pjsip->sip_outb_rid);

	pl_pjsip_update_nat(&app_config, pl_pjsip->sip_auto_update_nat);
	//u_int16				sip_auto_update_nat;	//Where N is 0 or 1 to enable/disable SIP traversal behind symmetric NAT (default 1)
	pl_pjsip_stun(&app_config, pl_pjsip->sip_stun_disable);
	//BOOL				sip_stun_disable;		//Disable STUN for this account

	//Transport Options:
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	pl_pjsip_ipv6_enable(&app_config, pl_pjsip->sip_ipv6_enable);
#endif
	pl_pjsip_qos_enable(&app_config, pl_pjsip->sip_set_qos);
	if(pl_pjsip->sip_noudp)
		pl_pjsip_no_udp(&app_config);
	if(pl_pjsip->sip_notcp)
		pl_pjsip_no_tcp(&app_config);

	if(strlen(pl_pjsip->sip_nameserver.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_nameserver.sip_address, pl_pjsip->sip_nameserver.sip_port);
		pl_pjsip_nameserver(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(pl_pjsip->sip_outbound.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_outbound.sip_address, pl_pjsip->sip_outbound.sip_port);
		pl_pjsip_outbound_proxy(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(pl_pjsip->sip_stun_server.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_stun_server.sip_address, pl_pjsip->sip_stun_server.sip_port);
		pl_pjsip_stunserver(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(pl_pjsip->sip_local.sip_address))
		pl_pjsip_bound_address(&app_config, pl_pjsip->sip_local.sip_address);
	//TLS Options:
/*
	BOOL				sip_tls_enable;
	char				sip_tls_ca_file[PJSIP_FILE_MAX];		//Specify TLS CA file (default=none)
	char				sip_tls_cert_file[PJSIP_FILE_MAX];		//Specify TLS certificate file (default=none)
	char				sip_tls_privkey_file[PJSIP_FILE_MAX];	//Specify TLS private key file (default=none)
	char				sip_tls_password[PJSIP_PASSWORD_MAX];	//Specify TLS password to private key file (default=none)
	pjsip_server_t		sip_tls_verify_server;					//Verify server's certificate (default=no)
	pjsip_server_t		sip_tls_verify_client;					//Verify client's certificate (default=no)
	u_int16				sip_neg_timeout;						//Specify TLS negotiation timeout (default=no)
	char				sip_tls_cipher[PJSIP_DATA_MAX];			//Specify prefered TLS cipher (optional).May be specified multiple times
*/

	//Audio Options:
	pl_pjsip_clock_rate(&app_config, pl_pjsip->sip_clock_rate);
	pl_pjsip_snd_clock_rate(&app_config, pl_pjsip->sip_snd_clock_rate);

	//char				sip_codec[PJSIP_DATA_MAX];
	//char				sip_discodec[PJSIP_DATA_MAX];

	if(pl_pjsip->sip_stereo)
		pl_pjsip_stereo(&app_config);

	if(strlen(pl_pjsip->sip_play_file))
		pl_pjsip_play_file(&app_config, pl_pjsip->sip_play_file);
	//if(strlen(pl_pjsip->sip_play_tone))
	//	pl_pjsip_play_file(&app_config, pl_pjsip->sip_play_tone);
	if(pl_pjsip->sip_auto_play)
		pl_pjsip_auto_play(&app_config, pl_pjsip->sip_auto_play);
	if(pl_pjsip->sip_auto_loop)
		pl_pjsip_auto_loop(&app_config, pl_pjsip->sip_auto_loop);
	if(pl_pjsip->sip_auto_conf)
		pl_pjsip_auto_config(&app_config, pl_pjsip->sip_auto_conf);

	if(strlen(pl_pjsip->sip_rec_file))
	{
		pl_pjsip_rec_file(&app_config, pl_pjsip->sip_rec_file);
		pl_pjsip_auto_rec(&app_config, PJ_TRUE);
	}
	pl_pjsip_quality(&app_config, pl_pjsip->sip_quality);
	pl_pjsip_ptime(&app_config, pl_pjsip->sip_ptime);

	if(pl_pjsip->sip_no_vad)
		pl_pjsip_novad(&app_config, pl_pjsip->sip_no_vad);

	if(pl_pjsip->sip_echo_mode != PJSIP_ECHO_DISABLE)
	{
		if(pl_pjsip->sip_echo_tail)
			pl_pjsip_ec_tial(&app_config, pl_pjsip->sip_echo_tail);
		pl_pjsip_ec_options(&app_config, pl_pjsip->sip_echo_mode - PJSIP_ECHO_DISABLE);
	}
	if(pl_pjsip->sip_ilbc_mode)
		pl_pjsip_ilbc_mode(&app_config, pl_pjsip->sip_ilbc_mode);

	if(pl_pjsip->sip_capture_lat)
		pl_pjsip_capture_lat(&app_config, pl_pjsip->sip_capture_lat);

	if(pl_pjsip->sip_playback_lat)
		pl_pjsip_playback_lat(&app_config, pl_pjsip->sip_playback_lat);

	pl_pjsip_snd_auto_close(&app_config, pl_pjsip->sip_snd_auto_close);
	pl_pjsip_no_tones(&app_config, pl_pjsip->sip_notones);
	if(pl_pjsip->sip_jb_max_size)
		pl_pjsip_jb_max_size(&app_config, pl_pjsip->sip_jb_max_size);

#if PJSUA_HAS_VIDEO
	//Video Options:
	if(pl_pjsip->sip_video)
	{
		pl_pjsip_video_enable(&app_config, pl_pjsip->sip_video);
		if(strlen(pl_pjsip->sip_play_avi))
			pl_pjsip_play_avi(&app_config, pl_pjsip->sip_play_avi);
		pl_pjsip_auto_play_avi(&app_config, pl_pjsip->sip_auto_play_avi);
	}
#endif
	//Media Transport Options:
	pl_pjsip_ice_enable(&app_config, pl_pjsip->sip_ice);
	pl_pjsip_regular_enable(&app_config, pl_pjsip->sip_ice_regular);
	pl_pjsip_ice_max_hosts(&app_config, pl_pjsip->sip_ice_regular);
	pl_pjsip_ice_nortcp_enable(&app_config, pl_pjsip->sip_ice_nortcp);

	if(pl_pjsip->sip_rtp_port)
		pl_pjsip_rtp_port(&app_config, pl_pjsip->sip_rtp_port);

	if(pl_pjsip->sip_rx_drop_pct)
		pl_pjsip_rx_drop_pct(&app_config, pl_pjsip->sip_rx_drop_pct);
	if(pl_pjsip->sip_tx_drop_pct)
		pl_pjsip_tx_drop_pct(&app_config, pl_pjsip->sip_tx_drop_pct);

	pl_pjsip_turn_enable(&app_config, pl_pjsip->sip_turn);
	if(strlen(pl_pjsip->sip_turn_srv.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", pl_pjsip->sip_turn_srv.sip_address, pl_pjsip->sip_turn_srv.sip_port);
		pl_pjsip_turn_srv(&app_config, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	pl_pjsip_turn_tcp(&app_config, pl_pjsip->sip_turn_tcp);

	if(strlen(pl_pjsip->sip_turn_user))
		pl_pjsip_play_avi(&app_config, pl_pjsip->sip_turn_user);
	if(strlen(pl_pjsip->sip_turn_password))
		pl_pjsip_play_avi(&app_config, pl_pjsip->sip_turn_password);

	pl_pjsip_rtcp_mux(&app_config, pl_pjsip->sip_rtcp_mux);
	//Buddy List (can be more than one):
	//void				*buddy_list;
	//User Agent options:
	if(pl_pjsip->sip_auto_answer_code)
		pl_pjsip_auto_answer(&app_config, pl_pjsip->sip_auto_answer_code);
	pl_pjsip_max_calls(&app_config, pl_pjsip->sip_max_calls);
	pl_pjsip_thread_cnt(&app_config, pl_pjsip->sip_thread_max);
	pl_pjsip_duration(&app_config, pl_pjsip->sip_duration);
	if(pl_pjsip->sip_norefersub)
		pl_pjsip_norefersub(&app_config);
	pl_pjsip_compact_form(&app_config, pl_pjsip->sip_use_compact_form);
	pl_pjsip_no_force_lr(&app_config, pl_pjsip->sip_no_force_lr);
	pl_pjsip_accept_redirect(&app_config, pl_pjsip->sip_accept_redirect);

	if(pl_pjsip->sip_codec.is_active)
	{
		pl_pjsip_add_codec(&app_config, pl_pjsip->sip_codec.payload_name);
	}

	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(pl_pjsip->codec[i].is_active)
		{
			pl_pjsip_add_codec(&app_config, pl_pjsip->codec[i].payload_name);
		}
	}
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(pl_pjsip->dicodec[i].is_active)
		{
			pl_pjsip_dis_codec(&app_config, pl_pjsip->dicodec[i].payload_name);
		}
	}

	if(pl_pjsip->mutex)
		os_mutex_unlock(pl_pjsip->mutex);

	return PJ_SUCCESS;
}
#else
int pjsip_load_config(void)
{
	pjsip_default_config();

	pl_pjsip_log_level(&app_config, 6);
	pl_pjsip_app_log_level(&app_config, 6);
	pj_log_set_log_func(pj_pjsip_log_cb);


	pl_pjsip_clock_rate(&app_config, 8000);
	pl_pjsip_snd_clock_rate(&app_config, 8000);
	pl_pjsip_no_tcp(&app_config);
/*
#define SIP_DOMAIN	"myvoipapp.com"
#define SIP_USER	"alice"
#define SIP_PASSWD	"secret"
*/
	pjsua_acc_config *cur_acc;
	cur_acc = app_config.acc_cfg;
	cur_acc->cred_count = 0;
	pl_pjsip_username(&app_config, "100");//Set authentication username
	pl_pjsip_password(&app_config, "100");//Set authentication password
	pl_pjsip_scheme(&app_config, "digest");
	pl_pjsip_realm(&app_config, "*");


	pl_pjsip_registrar(&app_config, "sip:192.168.224.1");//Set the URL of registrar server
	pl_pjsip_url_id(&app_config, "sip:100@192.168.224.1");//Set the URL of local ID (used in From header)


	cur_acc->cred_count = 1;
	//pl_pjsip_credential(&app_config, 0);

	app_config.acc_cnt = 1;
	//pl_pjsip_next_account(&app_config);

	pl_pjsip_reg_retry_interval(&app_config, 2000);
	//pl_pjsip_expires_timer(&app_config, 4000);
	pl_pjsip_rtp_port(&app_config, 50000);
	//status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);

/*	extern int pl_pjsip_realm(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_ims(pjsua_app_config *cfg, BOOL enable);
	extern int pl_pjsip_url_id(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_contact(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_contact_params(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_contact_uri_params(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_proxy(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_outbound_proxy(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_stereo(pjsua_app_config *cfg);
	extern int pl_pjsip_local_port(pjsua_app_config *cfg, u_int16 lval);
	extern int pl_pjsip_public_address(pjsua_app_config *cfg, char * lval);
	extern int pl_pjsip_bound_address(pjsua_app_config *cfg, char * lval);*/
	return PJ_SUCCESS;
}
#endif

#if 0
static pj_status_t parse_config(int argc, char *argv[], pj_str_t *uri_arg)
{
	pj_status_t status;

	/* Initialize default config */
	pjsip_default_config();

	/* Parse the arguments */
	status = parse_args(argc, argv, uri_arg);
	return status;
}

pj_status_t load_config(int argc, char **argv, pj_str_t *uri_arg)
{
	pj_status_t status;
	pj_bool_t use_cli = PJ_FALSE;
	int cli_fe = 0;
	pj_uint16_t cli_telnet_port = 0;

	/** CLI options are not changable **/
	if (app_running)
	{
		use_cli = app_config.use_cli;
		cli_fe = app_config.cli_cfg.cli_fe;
		cli_telnet_port = app_config.cli_cfg.telnet_cfg.port;
	}

	status = parse_config(argc, argv, uri_arg);
	if (status != PJ_SUCCESS)
		return status;

	if (app_running)
	{
		app_config.use_cli = use_cli;
		app_config.cli_cfg.cli_fe = cli_fe;
		app_config.cli_cfg.telnet_cfg.port = cli_telnet_port;
	}

	return status;
}


/*
 * Save account settings
 */
static void write_account_settings(int acc_index, pj_str_t *result)
{
	unsigned i;
	char line[128];
	pjsua_acc_config *acc_cfg = &app_config.acc_cfg[acc_index];

	pj_ansi_sprintf(line, "\n#\n# Account %d:\n#\n", acc_index);
	pj_strcat2(result, line);

	/* Identity */
	if (acc_cfg->id.slen)
	{
		pj_ansi_sprintf(line, "--id %.*s\n", (int) acc_cfg->id.slen,
				acc_cfg->id.ptr);
		pj_strcat2(result, line);
	}

	/* Registrar server */
	if (acc_cfg->reg_uri.slen)
	{
		pj_ansi_sprintf(line, "--registrar %.*s\n", (int) acc_cfg->reg_uri.slen,
				acc_cfg->reg_uri.ptr);
		pj_strcat2(result, line);

		pj_ansi_sprintf(line, "--reg-timeout %u\n", acc_cfg->reg_timeout);
		pj_strcat2(result, line);
	}

	/* Contact */
	if (acc_cfg->force_contact.slen)
	{
		pj_ansi_sprintf(line, "--contact %.*s\n",
				(int) acc_cfg->force_contact.slen, acc_cfg->force_contact.ptr);
		pj_strcat2(result, line);
	}

	/* Contact header parameters */
	if (acc_cfg->contact_params.slen)
	{
		pj_ansi_sprintf(line, "--contact-params %.*s\n",
				(int) acc_cfg->contact_params.slen,
				acc_cfg->contact_params.ptr);
		pj_strcat2(result, line);
	}

	/* Contact URI parameters */
	if (acc_cfg->contact_uri_params.slen)
	{
		pj_ansi_sprintf(line, "--contact-uri-params %.*s\n",
				(int) acc_cfg->contact_uri_params.slen,
				acc_cfg->contact_uri_params.ptr);
		pj_strcat2(result, line);
	}

	/*  */
	if (acc_cfg->allow_contact_rewrite != 1)
	{
		pj_ansi_sprintf(line, "--auto-update-nat %i\n",
				(int) acc_cfg->allow_contact_rewrite);
		pj_strcat2(result, line);
	}

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	/* SRTP */
	if (acc_cfg->use_srtp)
	{
		int use_srtp = (int) acc_cfg->use_srtp;
		if (use_srtp == PJMEDIA_SRTP_OPTIONAL
				&& acc_cfg->srtp_optional_dup_offer)
		{
			use_srtp = 3;
		}
		pj_ansi_sprintf(line, "--use-srtp %i\n", use_srtp);
		pj_strcat2(result, line);
	}
	if (acc_cfg->srtp_secure_signaling !=
	PJSUA_DEFAULT_SRTP_SECURE_SIGNALING)
	{
		pj_ansi_sprintf(line, "--srtp-secure %d\n",
				acc_cfg->srtp_secure_signaling);
		pj_strcat2(result, line);
	}
#endif

	/* Proxy */
	for (i = 0; i < acc_cfg->proxy_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--proxy %.*s\n", (int) acc_cfg->proxy[i].slen,
				acc_cfg->proxy[i].ptr);
		pj_strcat2(result, line);
	}

	/* Credentials */
	for (i = 0; i < acc_cfg->cred_count; ++i)
	{
		if (acc_cfg->cred_info[i].realm.slen)
		{
			pj_ansi_sprintf(line, "--realm %.*s\n",
					(int) acc_cfg->cred_info[i].realm.slen,
					acc_cfg->cred_info[i].realm.ptr);
			pj_strcat2(result, line);
		}

		if (acc_cfg->cred_info[i].username.slen)
		{
			pj_ansi_sprintf(line, "--username %.*s\n",
					(int) acc_cfg->cred_info[i].username.slen,
					acc_cfg->cred_info[i].username.ptr);
			pj_strcat2(result, line);
		}

		if (acc_cfg->cred_info[i].data.slen)
		{
			pj_ansi_sprintf(line, "--password %.*s\n",
					(int) acc_cfg->cred_info[i].data.slen,
					acc_cfg->cred_info[i].data.ptr);
			pj_strcat2(result, line);
		}

		if (i != acc_cfg->cred_count - 1)
			pj_strcat2(result, "--next-cred\n");
	}

	/* reg-use-proxy */
	if (acc_cfg->reg_use_proxy != 3)
	{
		pj_ansi_sprintf(line, "--reg-use-proxy %d\n", acc_cfg->reg_use_proxy);
		pj_strcat2(result, line);
	}

	/* rereg-delay */
	if (acc_cfg->reg_retry_interval != PJSUA_REG_RETRY_INTERVAL)
	{
		pj_ansi_sprintf(line, "--rereg-delay %d\n",
				acc_cfg->reg_retry_interval);
		pj_strcat2(result, line);
	}

	/* 100rel extension */
	if (acc_cfg->require_100rel == PJSUA_100REL_MANDATORY)
	{
		pj_strcat2(result, "--use-100rel\n");
	}

	/* Session Timer extension */
	if (acc_cfg->use_timer)
	{
		pj_ansi_sprintf(line, "--use-timer %d\n", acc_cfg->use_timer);
		pj_strcat2(result, line);
	}
	if (acc_cfg->timer_setting.min_se != 90)
	{
		pj_ansi_sprintf(line, "--timer-min-se %d\n",
				acc_cfg->timer_setting.min_se);
		pj_strcat2(result, line);
	}
	if (acc_cfg->timer_setting.sess_expires != PJSIP_SESS_TIMER_DEF_SE)
	{
		pj_ansi_sprintf(line, "--timer-se %d\n",
				acc_cfg->timer_setting.sess_expires);
		pj_strcat2(result, line);
	}

	/* Publish */
	if (acc_cfg->publish_enabled)
		pj_strcat2(result, "--publish\n");

	/* MWI */
	if (acc_cfg->mwi_enabled)
		pj_strcat2(result, "--mwi\n");

	if (acc_cfg->sip_stun_use != PJSUA_STUN_USE_DEFAULT
			|| acc_cfg->media_stun_use != PJSUA_STUN_USE_DEFAULT)
	{
		pj_strcat2(result, "--disable-stun\n");
	}

	/* Media Transport*/
	if (acc_cfg->ice_cfg.enable_ice)
		pj_strcat2(result, "--use-ice\n");

	if (acc_cfg->ice_cfg.ice_opt.aggressive == PJ_FALSE)
		pj_strcat2(result, "--ice-regular\n");

	if (acc_cfg->turn_cfg.enable_turn)
		pj_strcat2(result, "--use-turn\n");

	if (acc_cfg->ice_cfg.ice_max_host_cands >= 0)
	{
		pj_ansi_sprintf(line, "--ice_max_host_cands %d\n",
				acc_cfg->ice_cfg.ice_max_host_cands);
		pj_strcat2(result, line);
	}

	if (acc_cfg->ice_cfg.ice_no_rtcp)
		pj_strcat2(result, "--ice-no-rtcp\n");

	if (acc_cfg->turn_cfg.turn_server.slen)
	{
		pj_ansi_sprintf(line, "--turn-srv %.*s\n",
				(int) acc_cfg->turn_cfg.turn_server.slen,
				acc_cfg->turn_cfg.turn_server.ptr);
		pj_strcat2(result, line);
	}

	if (acc_cfg->turn_cfg.turn_conn_type == PJ_TURN_TP_TCP)
		pj_strcat2(result, "--turn-tcp\n");

	if (acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.username.slen)
	{
		pj_ansi_sprintf(line, "--turn-user %.*s\n",
				(int) acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.username.slen,
				acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.username.ptr);
		pj_strcat2(result, line);
	}

	if (acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.data.slen)
	{
		pj_ansi_sprintf(line, "--turn-passwd %.*s\n",
				(int) acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.data.slen,
				acc_cfg->turn_cfg.turn_auth_cred.data.static_cred.data.ptr);
		pj_strcat2(result, line);
	}

	if (acc_cfg->enable_rtcp_mux)
		pj_strcat2(result, "--rtcp-mux\n");
}

/*
 * Write settings.
 */
int write_settings(pjsua_app_config *config, char *buf, pj_size_t max)
{
	unsigned acc_index;
	unsigned i;
	pj_str_t cfg;
	char line[128];

	PJ_UNUSED_ARG(max);

	cfg.ptr = buf;
	cfg.slen = 0;

	/* Logging. */
	pj_strcat2(&cfg, "#\n# Logging options:\n#\n");
	pj_ansi_sprintf(line, "--log-level %d\n", config->log_cfg.level);
	pj_strcat2(&cfg, line);

	pj_ansi_sprintf(line, "--app-log-level %d\n",
			config->log_cfg.console_level);
	pj_strcat2(&cfg, line);

	if (config->log_cfg.log_filename.slen)
	{
		pj_ansi_sprintf(line, "--log-file %.*s\n",
				(int) config->log_cfg.log_filename.slen,
				config->log_cfg.log_filename.ptr);
		pj_strcat2(&cfg, line);
	}

	if (config->log_cfg.log_file_flags & PJ_O_APPEND)
	{
		pj_strcat2(&cfg, "--log-append\n");
	}

	/* Save account settings. */
	for (acc_index = 0; acc_index < config->acc_cnt; ++acc_index)
	{

		write_account_settings(acc_index, &cfg);

		if (acc_index < config->acc_cnt - 1)
			pj_strcat2(&cfg, "--next-account\n");
	}

	pj_strcat2(&cfg, "\n#\n# Network settings:\n#\n");

	/* Nameservers */
	for (i = 0; i < config->cfg.nameserver_count; ++i)
	{
		pj_ansi_sprintf(line, "--nameserver %.*s\n",
				(int) config->cfg.nameserver[i].slen,
				config->cfg.nameserver[i].ptr);
		pj_strcat2(&cfg, line);
	}

	/* Outbound proxy */
	for (i = 0; i < config->cfg.outbound_proxy_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--outbound %.*s\n",
				(int) config->cfg.outbound_proxy[i].slen,
				config->cfg.outbound_proxy[i].ptr);
		pj_strcat2(&cfg, line);
	}

	/* Transport options */
	if (config->ipv6)
	{
		pj_strcat2(&cfg, "--ipv6\n");
	}
	if (config->enable_qos)
	{
		pj_strcat2(&cfg, "--set-qos\n");
	}

	/* UDP Transport. */
	pj_ansi_sprintf(line, "--local-port %d\n", config->udp_cfg.port);
	pj_strcat2(&cfg, line);

	/* IP address, if any. */
	if (config->udp_cfg.public_addr.slen)
	{
		pj_ansi_sprintf(line, "--ip-addr %.*s\n",
				(int) config->udp_cfg.public_addr.slen,
				config->udp_cfg.public_addr.ptr);
		pj_strcat2(&cfg, line);
	}

	/* Bound IP address, if any. */
	if (config->udp_cfg.bound_addr.slen)
	{
		pj_ansi_sprintf(line, "--bound-addr %.*s\n",
				(int) config->udp_cfg.bound_addr.slen,
				config->udp_cfg.bound_addr.ptr);
		pj_strcat2(&cfg, line);
	}

	/* No TCP ? */
	if (config->no_tcp)
	{
		pj_strcat2(&cfg, "--no-tcp\n");
	}

	/* No UDP ? */
	if (config->no_udp)
	{
		pj_strcat2(&cfg, "--no-udp\n");
	}

	/* STUN */
	for (i = 0; i < config->cfg.stun_srv_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--stun-srv %.*s\n",
				(int) config->cfg.stun_srv[i].slen,
				config->cfg.stun_srv[i].ptr);
		pj_strcat2(&cfg, line);
	}

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	/* TLS */
	if (config->use_tls)
	pj_strcat2(&cfg, "--use-tls\n");
	if (config->udp_cfg.tls_setting.ca_list_file.slen)
	{
		pj_ansi_sprintf(line, "--tls-ca-file %.*s\n",
				(int)config->udp_cfg.tls_setting.ca_list_file.slen,
				config->udp_cfg.tls_setting.ca_list_file.ptr);
		pj_strcat2(&cfg, line);
	}
	if (config->udp_cfg.tls_setting.cert_file.slen)
	{
		pj_ansi_sprintf(line, "--tls-cert-file %.*s\n",
				(int)config->udp_cfg.tls_setting.cert_file.slen,
				config->udp_cfg.tls_setting.cert_file.ptr);
		pj_strcat2(&cfg, line);
	}
	if (config->udp_cfg.tls_setting.privkey_file.slen)
	{
		pj_ansi_sprintf(line, "--tls-privkey-file %.*s\n",
				(int)config->udp_cfg.tls_setting.privkey_file.slen,
				config->udp_cfg.tls_setting.privkey_file.ptr);
		pj_strcat2(&cfg, line);
	}

	if (config->udp_cfg.tls_setting.password.slen)
	{
		pj_ansi_sprintf(line, "--tls-password %.*s\n",
				(int)config->udp_cfg.tls_setting.password.slen,
				config->udp_cfg.tls_setting.password.ptr);
		pj_strcat2(&cfg, line);
	}

	if (config->udp_cfg.tls_setting.verify_server)
	pj_strcat2(&cfg, "--tls-verify-server\n");

	if (config->udp_cfg.tls_setting.verify_client)
	pj_strcat2(&cfg, "--tls-verify-client\n");

	if (config->udp_cfg.tls_setting.timeout.sec)
	{
		pj_ansi_sprintf(line, "--tls-neg-timeout %d\n",
				(int)config->udp_cfg.tls_setting.timeout.sec);
		pj_strcat2(&cfg, line);
	}

	for (i=0; i<config->udp_cfg.tls_setting.ciphers_num; ++i)
	{
		pj_ansi_sprintf(line, "--tls-cipher 0x%06X # %s\n",
				config->udp_cfg.tls_setting.ciphers[i],
				pj_ssl_cipher_name(config->udp_cfg.tls_setting.ciphers[i]));
		pj_strcat2(&cfg, line);
	}
#endif

	pj_strcat2(&cfg, "\n#\n# Media settings:\n#\n");

	/* Video & extra audio */
	for (i = 0; i < config->vid.vid_cnt; ++i)
	{
		pj_strcat2(&cfg, "--video\n");
	}
	for (i = 1; i < config->aud_cnt; ++i)
	{
		pj_strcat2(&cfg, "--extra-audio\n");
	}

	/* SRTP */
#if PJMEDIA_HAS_SRTP
	if (app_config.cfg.use_srtp != PJSUA_DEFAULT_USE_SRTP)
	{
		int use_srtp = (int) app_config.cfg.use_srtp;
		if (use_srtp == PJMEDIA_SRTP_OPTIONAL
				&& app_config.cfg.srtp_optional_dup_offer)
		{
			use_srtp = 3;
		}
		pj_ansi_sprintf(line, "--use-srtp %d\n", use_srtp);
		pj_strcat2(&cfg, line);
	}
	if (app_config.cfg.srtp_secure_signaling !=
	PJSUA_DEFAULT_SRTP_SECURE_SIGNALING)
	{
		pj_ansi_sprintf(line, "--srtp-secure %d\n",
				app_config.cfg.srtp_secure_signaling);
		pj_strcat2(&cfg, line);
	}
	if (app_config.srtp_keying >= 0 && app_config.srtp_keying <= 1)
	{
		pj_ansi_sprintf(line, "--srtp-keying %d\n", app_config.srtp_keying);
		pj_strcat2(&cfg, line);
	}
#endif

	/* Media */
	if (config->null_audio)
		pj_strcat2(&cfg, "--null-audio\n");
	if (config->auto_play)
		pj_strcat2(&cfg, "--auto-play\n");
	if (config->auto_loop)
		pj_strcat2(&cfg, "--auto-loop\n");
	if (config->auto_conf)
		pj_strcat2(&cfg, "--auto-conf\n");
	for (i = 0; i < config->wav_count; ++i)
	{
		pj_ansi_sprintf(line, "--play-file %s\n", config->wav_files[i].ptr);
		pj_strcat2(&cfg, line);
	}
	for (i = 0; i < config->tone_count; ++i)
	{
		pj_ansi_sprintf(line, "--play-tone %d,%d,%d,%d\n",
				config->tones[i].freq1, config->tones[i].freq2,
				config->tones[i].on_msec, config->tones[i].off_msec);
		pj_strcat2(&cfg, line);
	}
	if (config->rec_file.slen)
	{
		pj_ansi_sprintf(line, "--rec-file %s\n", config->rec_file.ptr);
		pj_strcat2(&cfg, line);
	}
	if (config->auto_rec)
		pj_strcat2(&cfg, "--auto-rec\n");
	if (config->capture_dev != PJSUA_INVALID_ID)
	{
		pj_ansi_sprintf(line, "--capture-dev %d\n", config->capture_dev);
		pj_strcat2(&cfg, line);
	}
	if (config->playback_dev != PJSUA_INVALID_ID)
	{
		pj_ansi_sprintf(line, "--playback-dev %d\n", config->playback_dev);
		pj_strcat2(&cfg, line);
	}
	if (config->media_cfg.snd_auto_close_time != -1)
	{
		pj_ansi_sprintf(line, "--snd-auto-close %d\n",
				config->media_cfg.snd_auto_close_time);
		pj_strcat2(&cfg, line);
	}
	if (config->no_tones)
	{
		pj_strcat2(&cfg, "--no-tones\n");
	}
	if (config->media_cfg.jb_max != -1)
	{
		pj_ansi_sprintf(line, "--jb-max-size %d\n", config->media_cfg.jb_max);
		pj_strcat2(&cfg, line);
	}

	/* Sound device latency */
	if (config->capture_lat != PJMEDIA_SND_DEFAULT_REC_LATENCY)
	{
		pj_ansi_sprintf(line, "--capture-lat %d\n", config->capture_lat);
		pj_strcat2(&cfg, line);
	}
	if (config->playback_lat != PJMEDIA_SND_DEFAULT_PLAY_LATENCY)
	{
		pj_ansi_sprintf(line, "--playback-lat %d\n", config->playback_lat);
		pj_strcat2(&cfg, line);
	}

	/* Media clock rate. */
	if (config->media_cfg.clock_rate != PJSUA_DEFAULT_CLOCK_RATE)
	{
		pj_ansi_sprintf(line, "--clock-rate %d\n",
				config->media_cfg.clock_rate);
		pj_strcat2(&cfg, line);
	}
	else
	{
		pj_ansi_sprintf(line, "#using default --clock-rate %d\n",
				config->media_cfg.clock_rate);
		pj_strcat2(&cfg, line);
	}

	if (config->media_cfg.snd_clock_rate
			&& config->media_cfg.snd_clock_rate != config->media_cfg.clock_rate)
	{
		pj_ansi_sprintf(line, "--snd-clock-rate %d\n",
				config->media_cfg.snd_clock_rate);
		pj_strcat2(&cfg, line);
	}

	/* Stereo mode. */
	if (config->media_cfg.channel_count == 2)
	{
		pj_ansi_sprintf(line, "--stereo\n");
		pj_strcat2(&cfg, line);
	}

	/* quality */
	if (config->media_cfg.quality != PJSUA_DEFAULT_CODEC_QUALITY)
	{
		pj_ansi_sprintf(line, "--quality %d\n", config->media_cfg.quality);
		pj_strcat2(&cfg, line);
	}
	else
	{
		pj_ansi_sprintf(line, "#using default --quality %d\n",
				config->media_cfg.quality);
		pj_strcat2(&cfg, line);
	}

	if (config->vid.vcapture_dev != PJMEDIA_VID_DEFAULT_CAPTURE_DEV)
	{
		pj_ansi_sprintf(line, "--vcapture-dev %d\n", config->vid.vcapture_dev);
		pj_strcat2(&cfg, line);
	}
	if (config->vid.vrender_dev != PJMEDIA_VID_DEFAULT_RENDER_DEV)
	{
		pj_ansi_sprintf(line, "--vrender-dev %d\n", config->vid.vrender_dev);
		pj_strcat2(&cfg, line);
	}
	for (i = 0; i < config->avi_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--play-avi %s\n", config->avi[i].path.ptr);
		pj_strcat2(&cfg, line);
	}
	if (config->avi_auto_play)
	{
		pj_ansi_sprintf(line, "--auto-play-avi\n");
		pj_strcat2(&cfg, line);
	}

	/* ptime */
	if (config->media_cfg.ptime)
	{
		pj_ansi_sprintf(line, "--ptime %d\n", config->media_cfg.ptime);
		pj_strcat2(&cfg, line);
	}

	/* no-vad */
	if (config->media_cfg.no_vad)
	{
		pj_strcat2(&cfg, "--no-vad\n");
	}

	/* ec-tail */
	if (config->media_cfg.ec_tail_len != PJSUA_DEFAULT_EC_TAIL_LEN)
	{
		pj_ansi_sprintf(line, "--ec-tail %d\n", config->media_cfg.ec_tail_len);
		pj_strcat2(&cfg, line);
	}
	else
	{
		pj_ansi_sprintf(line, "#using default --ec-tail %d\n",
				config->media_cfg.ec_tail_len);
		pj_strcat2(&cfg, line);
	}

	/* ec-opt */
	if (config->media_cfg.ec_options != 0)
	{
		pj_ansi_sprintf(line, "--ec-opt %d\n", config->media_cfg.ec_options);
		pj_strcat2(&cfg, line);
	}

	/* ilbc-mode */
	if (config->media_cfg.ilbc_mode != PJSUA_DEFAULT_ILBC_MODE)
	{
		pj_ansi_sprintf(line, "--ilbc-mode %d\n", config->media_cfg.ilbc_mode);
		pj_strcat2(&cfg, line);
	}
	else
	{
		pj_ansi_sprintf(line, "#using default --ilbc-mode %d\n",
				config->media_cfg.ilbc_mode);
		pj_strcat2(&cfg, line);
	}

	/* RTP drop */
	if (config->media_cfg.tx_drop_pct)
	{
		pj_ansi_sprintf(line, "--tx-drop-pct %d\n",
				config->media_cfg.tx_drop_pct);
		pj_strcat2(&cfg, line);

	}
	if (config->media_cfg.rx_drop_pct)
	{
		pj_ansi_sprintf(line, "--rx-drop-pct %d\n",
				config->media_cfg.rx_drop_pct);
		pj_strcat2(&cfg, line);

	}

	/* Start RTP port. */
	pj_ansi_sprintf(line, "--rtp-port %d\n", config->rtp_cfg.port);
	pj_strcat2(&cfg, line);

	/* Disable codec */
	for (i = 0; i < config->codec_dis_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--dis-codec %s\n", config->codec_dis[i].ptr);
		pj_strcat2(&cfg, line);
	}
	/* Add codec. */
	for (i = 0; i < config->codec_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--add-codec %s\n", config->codec_arg[i].ptr);
		pj_strcat2(&cfg, line);
	}

	pj_strcat2(&cfg, "\n#\n# User agent:\n#\n");

	/* Auto-answer. */
	if (config->auto_answer != 0)
	{
		pj_ansi_sprintf(line, "--auto-answer %d\n", config->auto_answer);
		pj_strcat2(&cfg, line);
	}

	/* accept-redirect */
	if (config->redir_op != PJSIP_REDIRECT_ACCEPT_REPLACE)
	{
		pj_ansi_sprintf(line, "--accept-redirect %d\n", config->redir_op);
		pj_strcat2(&cfg, line);
	}

	/* Max calls. */
	pj_ansi_sprintf(line, "--max-calls %d\n", config->cfg.max_calls);
	pj_strcat2(&cfg, line);

	/* Uas-duration. */
	if (config->duration != PJSUA_APP_NO_LIMIT_DURATION)
	{
		pj_ansi_sprintf(line, "--duration %d\n", config->duration);
		pj_strcat2(&cfg, line);
	}

	/* norefersub ? */
	if (config->no_refersub)
	{
		pj_strcat2(&cfg, "--norefersub\n");
	}

	if (pjsip_cfg()->endpt.use_compact_form)
	{
		pj_strcat2(&cfg, "--use-compact-form\n");
	}

	if (!config->cfg.force_lr)
	{
		pj_strcat2(&cfg, "--no-force-lr\n");
	}

	pj_strcat2(&cfg, "\n#\n# Buddies:\n#\n");

	/* Add buddies. */
	for (i = 0; i < config->buddy_cnt; ++i)
	{
		pj_ansi_sprintf(line, "--add-buddy %.*s\n",
				(int) config->buddy_cfg[i].uri.slen,
				config->buddy_cfg[i].uri.ptr);
		pj_strcat2(&cfg, line);
	}

	/* SIP extensions. */
	pj_strcat2(&cfg, "\n#\n# SIP extensions:\n#\n");
	/* 100rel extension */
	if (config->cfg.require_100rel == PJSUA_100REL_MANDATORY)
	{
		pj_strcat2(&cfg, "--use-100rel\n");
	}
	/* Session Timer extension */
	if (config->cfg.use_timer)
	{
		pj_ansi_sprintf(line, "--use-timer %d\n", config->cfg.use_timer);
		pj_strcat2(&cfg, line);
	}
	if (config->cfg.timer_setting.min_se != 90)
	{
		pj_ansi_sprintf(line, "--timer-min-se %d\n",
				config->cfg.timer_setting.min_se);
		pj_strcat2(&cfg, line);
	}
	if (config->cfg.timer_setting.sess_expires != PJSIP_SESS_TIMER_DEF_SE)
	{
		pj_ansi_sprintf(line, "--timer-se %d\n",
				config->cfg.timer_setting.sess_expires);
		pj_strcat2(&cfg, line);
	}

	*(cfg.ptr + cfg.slen) = '\0';
	return (int) cfg.slen;
}
#endif

/*
 *
[zhurish@localhost x86_64-unknown-linux-gnu]$
[zhurish@localhost x86_64-unknown-linux-gnu]$ ./simple_pjsua sip:101@192.168.224.1:5060
09:56:45.052         os_core_unix.c !pjlib 2.8 for POSIX initialized
09:56:45.053         sip_endpoint.c  .Creating endpoint instance...
09:56:45.054                  pjlib  .select() I/O Queue created (0xfa9480)
09:56:45.054         sip_endpoint.c  .Module "mod-msg-print" registered
09:56:45.054        sip_transport.c  .Transport manager created.
09:56:45.054           pjsua_core.c  .PJSUA state changed: NULL --> CREATED
09:56:45.054                    APP  -----------[argv[1]=sip:101@192.168.224.1:5060]
09:56:45.054         sip_endpoint.c  .Module "mod-pjsua-log" registered
09:56:45.054         sip_endpoint.c  .Module "mod-tsx-layer" registered
09:56:45.054         sip_endpoint.c  .Module "mod-stateful-util" registered
09:56:45.054         sip_endpoint.c  .Module "mod-ua" registered
09:56:45.054         sip_endpoint.c  .Module "mod-100rel" registered
09:56:45.054         sip_endpoint.c  .Module "mod-pjsua" registered
09:56:45.054         sip_endpoint.c  .Module "mod-invite" registered
xcb_connection_has_error() returned true
xcb_connection_has_error() returned true
xcb_connection_has_error() returned true
xcb_connection_has_error() returned true
xcb_connection_has_error() returned true
09:56:50.139             alsa_dev.c  ..ALSA driver found 7 devices
09:56:50.139             alsa_dev.c  ..ALSA initialized
09:56:50.139                  pjlib  ..select() I/O Queue created (0xfd6c18)
09:56:50.143         sip_endpoint.c  .Module "mod-evsub" registered
09:56:50.143         sip_endpoint.c  .Module "mod-presence" registered
09:56:50.143         sip_endpoint.c  .Module "mod-mwi" registered
09:56:50.143         sip_endpoint.c  .Module "mod-refer" registered
09:56:50.143         sip_endpoint.c  .Module "mod-pjsua-pres" registered
09:56:50.143         sip_endpoint.c  .Module "mod-pjsua-im" registered
09:56:50.143         sip_endpoint.c  .Module "mod-pjsua-options" registered
09:56:50.143           pjsua_core.c  .1 SIP worker threads created
09:56:50.143           pjsua_core.c  .pjsua version 2.8 for Linux-3.10.1.71/x86_64/glibc-2.17 initialized
09:56:50.143           pjsua_core.c  .PJSUA state changed: CREATED --> INIT
09:56:50.144           pjsua_core.c  SIP UDP socket reachable at 192.168.224.129:5060
09:56:50.144           udp0x1000a10  SIP UDP transport started, published address is 192.168.224.129:5060
09:56:50.144           pjsua_core.c  PJSUA state changed: INIT --> STARTING
09:56:50.144         sip_endpoint.c  .Module "mod-unsolicited-mwi" registered
09:56:50.144           pjsua_core.c  .PJSUA state changed: STARTING --> RUNNING
09:56:50.144            pjsua_acc.c  Adding account: id=sip:100@192.168.224.1
09:56:50.144            pjsua_acc.c  .Account sip:100@192.168.224.1 added with id 0
09:56:50.144            pjsua_acc.c  .Acc 0: setting registration..
09:56:50.144           pjsua_core.c  ...TX 500 bytes Request msg REGISTER/cseq=21248 (tdta0x1008808) to UDP 192.168.224.1:5060:
REGISTER sip:192.168.224.1 SIP/2.0
Via: SIP/2.0/UDP 192.168.224.129:5060;rport;branch=z9hG4bKPjcded2ad1-3f85-4a98-b5c6-00de523a5b07
Max-Forwards: 70
From: <sip:100@192.168.224.1>;tag=77064136-5582-4e00-98e7-cb36bd6fb3d6
To: <sip:100@192.168.224.1>
Call-ID: 20dbbdae-b362-4ccf-a2fe-de72ae02f3c4
CSeq: 21248 REGISTER
Contact: <sip:100@192.168.224.129:5060;ob>
Expires: 300
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Content-Length:  0


--end msg--
09:56:50.144            pjsua_acc.c  ..Acc 0: Registration sent
09:56:50.144                    APP  ----===============-------[uri=sip:101@192.168.224.1:5060]
09:56:50.144           pjsua_call.c  Making call with acc #0 to sip:101@192.168.224.1:5060
09:56:50.145            pjsua_aud.c  .Set sound device: capture=-1, playback=-2
09:56:50.145            pjsua_aud.c  ..Opening sound device (speaker + mic) PCM@16000/1/20ms
09:56:50.154           pjsua_core.c  .RX 603 bytes Response msg 407/REGISTER/cseq=21248 (rdata0x1002498) from UDP 192.168.224.1:5060:
SIP/2.0 407 Proxy Authentication Required
Via: SIP/2.0/UDP 192.168.224.129:5060;branch=z9hG4bKPjcded2ad1-3f85-4a98-b5c6-00de523a5b07;received=192.168.224.129;rport=5060
From: <sip:100@192.168.224.1>;tag=77064136-5582-4e00-98e7-cb36bd6fb3d6
To: <sip:100@192.168.224.1>;tag=6ce0013e5faf2c29
CSeq: 21248 REGISTER
Call-ID: 20dbbdae-b362-4ccf-a2fe-de72ae02f3c4
Allow: ACK, BYE, CANCEL, INFO, INVITE, MESSAGE, NOTIFY, OPTIONS, PRACK, REFER, REGISTER, SUBSCRIBE
Proxy-Authenticate: Digest realm="myvoipapp.com", nonce="276B329D161C60CA53EA28FF66925ED1", algorithm=MD5, stale=FALSE
Content-Length: 0


--end msg--
xcb_connection_has_error() returned true
xcb_connection_has_error() returned true
09:56:52.315            ec0x101aff0  ...AEC created, clock_rate=16000, channel=1, samples per frame=320, tail length=200 ms, latency=0 ms
09:56:52.316          pjsua_media.c  .Call 0: initializing media..
09:56:52.316          pjsua_media.c  ..RTP socket reachable at 192.168.224.129:4000
09:56:52.316          pjsua_media.c  ..RTCP socket reachable at 192.168.224.129:4001
09:56:52.316          pjsua_media.c  ..Media index 0 selected for audio call 0
09:56:52.317           pjsua_core.c  ....TX 1153 bytes Request msg INVITE/cseq=1565 (tdta0x104dd98) to UDP 192.168.224.1:5060:
INVITE sip:101@192.168.224.1:5060 SIP/2.0
Via: SIP/2.0/UDP 192.168.224.129:5060;rport;branch=z9hG4bKPj96bd1300-e08e-4870-874a-0049a0af3e06
Max-Forwards: 70
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1
Contact: <sip:100@192.168.224.129:5060;ob>
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
CSeq: 1565 INVITE
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Supported: replaces, 100rel, timer, norefersub
Session-Expires: 1800
Min-SE: 90
Content-Type: application/sdp
Content-Length:   550

v=0
o=- 3769466212 3769466212 IN IP4 192.168.224.129
s=pjmedia
b=AS:84
t=0 0
a=X-nat:0
m=audio 4000 RTP/AVP 98 97 99 104 3 0 8 9 105 96
c=IN IP4 192.168.224.129
b=TIAS:64000
a=rtcp:4001 IN IP4 192.168.224.129
a=sendrecv
a=rtpmap:98 speex/16000
a=rtpmap:97 speex/8000
a=rtpmap:99 speex/32000
a=rtpmap:104 iLBC/8000
a=fmtp:104 mode=30
a=rtpmap:3 GSM/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:9 G722/8000
a=rtpmap:105 AMR/8000
a=rtpmap:96 telephone-event/8000
a=fmtp:96 0-16
a=ssrc:828795340 cname:30544df26dff291f

--end msg--
09:56:52.318                    APP  .......Call 0 state=CALLING
Press 'h' to hangup all calls, 'q' to quit
09:56:52.318           pjsua_core.c  ....TX 694 bytes Request msg REGISTER/cseq=21249 (tdta0x1008808) to UDP 192.168.224.1:5060:
REGISTER sip:192.168.224.1 SIP/2.0
Via: SIP/2.0/UDP 192.168.224.129:5060;rport;branch=z9hG4bKPjb02e49fe-edc1-4214-9720-4e73c8a3df79
Max-Forwards: 70
From: <sip:100@192.168.224.1>;tag=77064136-5582-4e00-98e7-cb36bd6fb3d6
To: <sip:100@192.168.224.1>
Call-ID: 20dbbdae-b362-4ccf-a2fe-de72ae02f3c4
CSeq: 21249 REGISTER
Contact: <sip:100@192.168.224.129:5060;ob>
Expires: 300
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Proxy-Authorization: Digest username="100", realm="myvoipapp.com", nonce="276B329D161C60CA53EA28FF66925ED1", uri="sip:192.168.224.1", response="240c2ad2c5091681a015c1643b1e5d96", algorithm=MD5
Content-Length:  0


--end msg--
09:56:52.322           pjsua_core.c  .RX 596 bytes Response msg 407/INVITE/cseq=1565 (rdata0x7f6c98001928) from UDP 192.168.224.1:5060:
SIP/2.0 407 Proxy Authentication Required
Via: SIP/2.0/UDP 192.168.224.129:5060;branch=z9hG4bKPj96bd1300-e08e-4870-874a-0049a0af3e06;received=192.168.224.129;rport=5060
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1;tag=2006467f63145ac5
CSeq: 1565 INVITE
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
Allow: ACK, BYE, CANCEL, INFO, INVITE, MESSAGE, NOTIFY, OPTIONS, PRACK, REFER, REGISTER, SUBSCRIBE
Proxy-Authenticate: Digest realm="myvoipapp.com", nonce="276B329D161C60CA53EA28FF66925ED1", algorithm=MD5, stale=FALSE
Content-Length: 0


--end msg--
09:56:52.322           pjsua_core.c  ..TX 359 bytes Request msg ACK/cseq=1565 (tdta0x7f6c980038f8) to UDP 192.168.224.1:5060:
ACK sip:101@192.168.224.1:5060 SIP/2.0
Via: SIP/2.0/UDP 192.168.224.129:5060;rport;branch=z9hG4bKPj96bd1300-e08e-4870-874a-0049a0af3e06
Max-Forwards: 70
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1;tag=2006467f63145ac5
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
CSeq: 1565 ACK
Content-Length:  0


--end msg--
09:56:52.322      tsx0x7f6c98006c68  .......Temporary failure in sending Request msg INVITE/cseq=1566 (tdta0x104dd98), will try next server: Unsupported transport (PJSIP_EUNSUPTRANSPORT)
09:56:52.322           pjsua_core.c  .......TX 1356 bytes Request msg INVITE/cseq=1566 (tdta0x104dd98) to UDP 192.168.224.1:5060:
INVITE sip:101@192.168.224.1:5060 SIP/2.0
Via: SIP/2.0/UDP 192.168.224.129:5060;rport;branch=z9hG4bKPje8d9be4c-04ff-46c6-8166-636172d2105f
Max-Forwards: 70
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1
Contact: <sip:100@192.168.224.129:5060;ob>
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
CSeq: 1566 INVITE
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Supported: replaces, 100rel, timer, norefersub
Session-Expires: 1800
Min-SE: 90
Proxy-Authorization: Digest username="100", realm="myvoipapp.com", nonce="276B329D161C60CA53EA28FF66925ED1", uri="sip:101@192.168.224.1:5060", response="42ffbf239dff9ae534c4f86fa8b95bf5",algorithm=MD5
Content-Type: application/sdp
Content-Length:   550

v=0
o=- 3769466212 3769466212 IN IP4 192.168.224.129
s=pjmedia
b=AS:84
t=0 0
a=X-nat:0
m=audio 4000 RTP/AVP 98 97 99 104 3 0 8 9 105 96
c=IN IP4 192.168.224.129
b=TIAS:64000
a=rtcp:4001 IN IP4 192.168.224.129
a=sendrecv
a=rtpmap:98 speex/16000
a=rtpmap:97 speex/8000
a=rtpmap:99 speex/32000
a=rtpmap:104 iLBC/8000
a=fmtp:104 mode=30
a=rtpmap:3 GSM/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:9 G722/8000
a=rtpmap:105 AMR/8000
a=rtpmap:96 telephone-event/8000
a=fmtp:96 0-16
a=ssrc:828795340 cname:30544df26dff291f

--end msg--
09:56:52.322           pjsua_core.c  .RX 564 bytes Response msg 200/REGISTER/cseq=21249 (rdata0x7f6c98001928) from UDP 192.168.224.1:5060:
SIP/2.0 200 OK
Via: SIP/2.0/UDP 192.168.224.129:5060;branch=z9hG4bKPjb02e49fe-edc1-4214-9720-4e73c8a3df79;received=192.168.224.129;rport=5060
From: <sip:100@192.168.224.1>;tag=77064136-5582-4e00-98e7-cb36bd6fb3d6
To: <sip:100@192.168.224.1>;tag=531734e134737759
CSeq: 21249 REGISTER
Call-ID: 20dbbdae-b362-4ccf-a2fe-de72ae02f3c4
Allow: ACK, BYE, CANCEL, INFO, INVITE, MESSAGE, NOTIFY, OPTIONS, PRACK, REFER, REGISTER, SUBSCRIBE
Server: miniSIPServer V35 (20 clients) build 20190531
Contact: <sip:100@192.168.224.129;ob>
Expires: 300
Content-Length: 0


--end msg--
09:56:52.322            pjsua_acc.c  ....SIP outbound status for acc 0 is not active
09:56:52.322            pjsua_acc.c  ....sip:100@192.168.224.1: registration success, status=200 (OK), will re-register in 300 seconds
09:56:52.323            pjsua_acc.c  ....Keep-alive timer started for acc 0, destination:192.168.224.1:5060:15, interval:18s
09:56:52.324           pjsua_core.c  .RX 487 bytes Response msg 100/INVITE/cseq=1566 (rdata0x7f6c98001928) from UDP 192.168.224.1:5060:
SIP/2.0 100 Trying
Via: SIP/2.0/UDP 192.168.224.129:5060;branch=z9hG4bKPje8d9be4c-04ff-46c6-8166-636172d2105f;received=192.168.224.129;rport=5060
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1
CSeq: 1566 INVITE
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
Allow: ACK, BYE, CANCEL, INFO, INVITE, MESSAGE, NOTIFY, OPTIONS, PRACK, REFER, REGISTER, SUBSCRIBE
Server: miniSIPServer V35 (20 clients) build 20190531
Content-Length: 0


--end msg--
09:56:52.331             alsa_dev.c  pb_thread_func: underrun!
09:56:52.345             alsa_dev.c  pb_thread_func: underrun!
09:56:52.359             alsa_dev.c  pb_thread_func: underrun!
09:56:52.375             alsa_dev.c  pb_thread_func: underrun!
09:56:52.390             alsa_dev.c  pb_thread_func: underrun!
09:56:52.413             alsa_dev.c  pb_thread_func: underrun!
09:56:52.426             alsa_dev.c  pb_thread_func: underrun!
09:56:52.440             alsa_dev.c  pb_thread_func: underrun!
09:56:52.453             alsa_dev.c  pb_thread_func: underrun!
09:56:52.468             alsa_dev.c  pb_thread_func: underrun!
09:56:52.479             alsa_dev.c  pb_thread_func: underrun!
09:56:52.490             alsa_dev.c  pb_thread_func: underrun!
09:56:52.503             alsa_dev.c  pb_thread_func: underrun!
09:56:52.514             alsa_dev.c  pb_thread_func: underrun!
09:56:52.526             alsa_dev.c  pb_thread_func: underrun!
09:56:52.538             alsa_dev.c  pb_thread_func: underrun!
09:56:52.554             alsa_dev.c  pb_thread_func: underrun!
09:56:52.575             alsa_dev.c  pb_thread_func: underrun!
09:56:52.595             alsa_dev.c  pb_thread_func: underrun!
09:56:52.615             alsa_dev.c  pb_thread_func: underrun!
09:56:52.628             alsa_dev.c  pb_thread_func: underrun!
09:56:52.637             alsa_dev.c  pb_thread_func: underrun!
09:56:52.647             alsa_dev.c  pb_thread_func: underrun!
09:56:52.658             alsa_dev.c  pb_thread_func: underrun!
09:56:52.670             alsa_dev.c  pb_thread_func: underrun!
09:56:52.686             alsa_dev.c  pb_thread_func: underrun!
09:56:52.695             alsa_dev.c  pb_thread_func: underrun!
09:56:52.706             alsa_dev.c  pb_thread_func: underrun!
09:56:52.718             alsa_dev.c  pb_thread_func: underrun!
09:56:52.729             alsa_dev.c  pb_thread_func: underrun!
09:56:52.755             alsa_dev.c  pb_thread_func: underrun!
09:56:52.769             alsa_dev.c  pb_thread_func: underrun!
09:56:52.793             alsa_dev.c  pb_thread_func: underrun!
09:56:52.811             alsa_dev.c  pb_thread_func: underrun!
09:56:52.825           pjsua_core.c  .RX 548 bytes Response msg 180/INVITE/cseq=1566 (rdata0x7f6c98001928) from UDP 192.168.224.1:5060:
SIP/2.0 180 Ringing
Via: SIP/2.0/UDP 192.168.224.129:5060;branch=z9hG4bKPje8d9be4c-04ff-46c6-8166-636172d2105f;received=192.168.224.129;rport=5060
From: sip:100@192.168.224.1;tag=b8e126ba-ea5f-4f37-9918-63abefca1386
To: sip:101@192.168.224.1;tag=2d0d354c237d47d2
CSeq: 1566 INVITE
Call-ID: 7368019d-1db1-4715-b821-ba1b96cc3010
Allow: ACK, BYE, CANCEL, INFO, INVITE, MESSAGE, NOTIFY, OPTIONS, PRACK, REFER, REGISTER, SUBSCRIBE
Server: miniSIPServer V35 (20 clients) build 20190531
Contact: "101"<sip:101@192.168.224.1>
Content-Length: 0


--end msg--
09:56:52.825                    APP  .....Call 0 state=EARLY
09:56:52.830             alsa_dev.c  pb_thread_func: underrun!
09:56:52.855             alsa_dev.c  pb_thread_func: underrun!
09:56:52.867             alsa_dev.c  pb_thread_func: underrun!
09:56:52.880             alsa_dev.c  pb_thread_func: underrun!
09:56:52.896             alsa_dev.c  pb_thread_func: underrun!
09:56:52.915             alsa_dev.c  pb_thread_func: underrun!
09:56:52.929             alsa_dev.c  pb_thread_func: underrun!
09:56:52.945             alsa_dev.c  pb_thread_func: underrun!
09:56:52.968             alsa_dev.c  pb_thread_func: underrun!
09:56:52.988             alsa_dev.c  pb_thread_func: underrun!
09:56:53.004             alsa_dev.c  pb_thread_func: underrun!
09:56:53.016             alsa_dev.c  pb_thread_func: underrun!
09:56:53.033             alsa_dev.c  pb_thread_func: underrun!
09:56:53.047             alsa_dev.c  pb_thread_func: underrun!
09:56:53.062             alsa_dev.c  pb_thread_func: underrun!
09:56:53.076             alsa_dev.c  pb_thread_func: underrun!
09:56:53.090             alsa_dev.c  pb_thread_func: underrun!
09:56:53.105             alsa_dev.c  pb_thread_func: underrun!
09:56:53.116             alsa_dev.c  pb_thread_func: underrun!
09:56:53.127             alsa_dev.c  pb_thread_func: underrun!
09:56:53.138             alsa_dev.c  pb_thread_func: underrun!
09:56:53.151             alsa_dev.c  pb_thread_func: underrun!
09:56:53.162             alsa_dev.c  pb_thread_func: underrun!
09:56:53.174             alsa_dev.c  pb_thread_func: underrun!
09:56:53.186             alsa_dev.c  pb_thread_func: underrun!
09:56:53.198             alsa_dev.c  pb_thread_func: underrun!
09:56:53.210             alsa_dev.c  pb_thread_func: underrun!
09:56:53.223             alsa_dev.c  pb_thread_func: underrun!
09:56:53.235             alsa_dev.c  pb_thread_func: underrun!
09:56:53.247             alsa_dev.c  pb_thread_func: underrun!
09:56:53.260             alsa_dev.c  pb_thread_func: underrun!
09:56:53.268             alsa_dev.c  pb_thread_func: underrun!
09:56:53.278             alsa_dev.c  pb_thread_func: underrun!
09:56:53.288             alsa_dev.c  pb_thread_func: underrun!
09:56:53.296             alsa_dev.c  pb_thread_func: underrun!
09:56:53.308             alsa_dev.c  pb_thread_func: underrun!
09:56:53.318             alsa_dev.c  pb_thread_func: underrun!
09:56:53.328             alsa_dev.c  pb_thread_func: underrun!
09:56:53.339             alsa_dev.c  pb_thread_func: underrun!
09:56:53.352             alsa_dev.c  pb_thread_func: underrun!
09:56:53.364             alsa_dev.c  pb_thread_func: underrun!
09:56:53.377             alsa_dev.c  pb_thread_func: underrun!
09:56:53.389             alsa_dev.c  pb_thread_func: underrun!
09:56:53.397             alsa_dev.c  pb_thread_func: underrun!
09:56:53.408             alsa_dev.c  pb_thread_func: underrun!
09:56:53.419             alsa_dev.c  pb_thread_func: underrun!
09:56:53.429             alsa_dev.c  pb_thread_func: underrun!
09:56:53.443             alsa_dev.c  pb_thread_func: underrun!
09:56:53.456             alsa_dev.c  pb_thread_func: underrun!
09:56:53.468             alsa_dev.c  pb_thread_func: underrun!
09:56:53.481             alsa_dev.c  pb_thread_func: underrun!
09:56:53.490             alsa_dev.c  pb_thread_func: underrun!
09:56:53.499             alsa_dev.c  pb_thread_func: underrun!
09:56:53.510             alsa_dev.c  pb_thread_func: underrun!
09:56:53.519             alsa_dev.c  pb_thread_func: underrun!
09:56:53.530             alsa_dev.c  pb_thread_func: underrun!
09:56:53.539             alsa_dev.c  pb_thread_func: underrun!
09:56:53.548             alsa_dev.c  pb_thread_func: underrun!
09:56:53.558             alsa_dev.c  pb_thread_func: underrun!
09:56:53.569             alsa_dev.c  pb_thread_func: underrun!
09:56:53.579             alsa_dev.c  pb_thread_func: underrun!
09:56:53.591             alsa_dev.c  pb_thread_func: underrun!
09:56:53.599             alsa_dev.c  pb_thread_func: underrun!
09:56:53.610             alsa_dev.c  pb_thread_func: underrun!
09:56:53.620             alsa_dev.c  pb_thread_func: underrun!
09:56:53.629             alsa_dev.c  pb_thread_func: underrun!
09:56:53.641             alsa_dev.c  pb_thread_func: underrun!
09:56:53.652             alsa_dev.c  pb_thread_func: underrun!
09:56:53.659             alsa_dev.c  pb_thread_func: underrun!
09:56:53.667             alsa_dev.c  pb_thread_func: underrun!
09:56:53.676             alsa_dev.c  pb_thread_func: underrun!
09:56:53.686             alsa_dev.c  pb_thread_func: underrun!
09:56:53.697             alsa_dev.c  pb_thread_func: underrun!
09:56:53.707             alsa_dev.c  pb_thread_func: underrun!
09:56:53.717             alsa_dev.c  pb_thread_func: underrun!
09:56:53.725             alsa_dev.c  pb_thread_func: underrun!
09:56:53.733             alsa_dev.c  pb_thread_func: underrun!
09:56:53.742             alsa_dev.c  pb_thread_func: underrun!
09:56:53.753             alsa_dev.c  pb_thread_func: underrun!
09:56:53.763             alsa_dev.c  pb_thread_func: underrun!
09:56:53.773             alsa_dev.c  pb_thread_func: underrun!
09:56:53.782             alsa_dev.c  pb_thread_func: underrun!
09:56:53.793             alsa_dev.c  pb_thread_func: underrun!
09:56:53.804             alsa_dev.c  pb_thread_func: underrun!
09:56:53.812             alsa_dev.c  pb_thread_func: underrun!
09:56:53.821             alsa_dev.c  pb_thread_func: underrun!
09:56:53.831             alsa_dev.c  pb_thread_func: underrun!
09:56:53.841             alsa_dev.c  pb_thread_func: underrun!
09:56:53.888             alsa_dev.c  pb_thread_func: underrun!
09:56:53.898             alsa_dev.c  pb_thread_func: underrun!
09:56:53.907             alsa_dev.c  pb_thread_func: underrun!
09:56:53.911           sound_port.c  EC suspended because of inactivity
09:56:53.919             alsa_dev.c  pb_thread_func: underrun!
09:56:53.929             alsa_dev.c  pb_thread_func: underrun!
09:56:53.941             alsa_dev.c  pb_thread_func: underrun!
09:56:53.948             alsa_dev.c  pb_thread_func: underrun!
09:56:53.960             alsa_dev.c  pb_thread_func: underrun!
09:56:53.972             alsa_dev.c  pb_thread_func: underrun!
09:56:53.983             alsa_dev.c  pb_thread_func: underrun!
09:56:53.996             alsa_dev.c  pb_thread_func: underrun!
09:56:54.008             alsa_dev.c  pb_thread_func: underrun!
^C
[zhurish@localhos
 *
 */
