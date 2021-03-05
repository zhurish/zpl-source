/*
 *	Wireless Tools
 *
 *		Jean II - HPLB '99 - HPL 99->07
 *
 * This tool can access various piece of information on the card
 * not part of iwconfig...
 * You need to link this code against "iwlist.c" and "-lm".
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2007 Jean Tourrilhes <jt@hpl.hp.com>
 */
#include "iw_config.h"
#include "iwioctl.h"
#include "iwlib.h"		/* Header */
#include <sys/time.h>


static ospl_bool iwlist_detail = ospl_true;

int iwlist_detail_set(ospl_bool value)
{
	iwlist_detail = value;
	return 0;
}

ospl_bool iwlist_detail_get()
{
	return iwlist_detail;
}
/****************************** TYPES ******************************/

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
	/* State */
	int ap_num; /* Access Point number 1->N */
	int val_index; /* Value in table 0->(N-1) */
} iwscan_state;

/*
 * Bit to name mapping
 */
typedef struct iwmask_name
{
	ospl_uint32 mask; /* bit mask for the value */
	const char * name; /* human readable name for the value */
} iwmask_name;

/*
 * Types of authentication parameters
 */
typedef struct iw_auth_descr
{
	int value; /* Type of auth value */
	const char * label; /* User readable version */
	const struct iwmask_name * names; /* Names for this value */
	const int num_names; /* Number of names */
} iw_auth_descr;

/**************************** CONSTANTS ****************************/

#define IW_SCAN_HACK		0x8000

#define IW_EXTKEY_SIZE	(sizeof(struct iw_encode_ext) + IW_ENCODING_TOKEN_MAX)

/* ------------------------ WPA CAPA NAMES ------------------------ */
/*
 * This is the user readable name of a bunch of WPA constants in wireless.h
 * Maybe this should go in iwlib.c ?
 */

#define IW_ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

//static const struct iwmask_name iw_enc_mode_name[] = {
//  { IW_ENCODE_RESTRICTED,	"restricted" },
//  { IW_ENCODE_OPEN,		"open" },
//};
//#define	IW_ENC_MODE_NUM		IW_ARRAY_LEN(iw_enc_mode_name)

static const struct iwmask_name iw_auth_capa_name[] =
{
	{ IW_ENC_CAPA_WPA, "WPA" },
	{ IW_ENC_CAPA_WPA2, "WPA2" },
	{ IW_ENC_CAPA_CIPHER_TKIP, "CIPHER-TKIP" },
	{ IW_ENC_CAPA_CIPHER_CCMP, "CIPHER-CCMP" },
};
#define	IW_AUTH_CAPA_NUM	IW_ARRAY_LEN(iw_auth_capa_name)

static const struct iwmask_name iw_auth_cypher_name[] =
{
	{ IW_AUTH_CIPHER_NONE, "none" },
	{ IW_AUTH_CIPHER_WEP40, "WEP-40" },
	{ IW_AUTH_CIPHER_TKIP, "TKIP" },
	{ IW_AUTH_CIPHER_CCMP, "CCMP" },
	{ IW_AUTH_CIPHER_WEP104, "WEP-104" },
};
#define	IW_AUTH_CYPHER_NUM	IW_ARRAY_LEN(iw_auth_cypher_name)

static const struct iwmask_name iw_wpa_ver_name[] =
{
	{ IW_AUTH_WPA_VERSION_DISABLED, "disabled" },
	{ IW_AUTH_WPA_VERSION_WPA, "WPA" },
	{ IW_AUTH_WPA_VERSION_WPA2, "WPA2" },
};
#define	IW_WPA_VER_NUM		IW_ARRAY_LEN(iw_wpa_ver_name)

static const struct iwmask_name iw_auth_key_mgmt_name[] =
{
	{ IW_AUTH_KEY_MGMT_802_1X, "802.1x" },
	{ IW_AUTH_KEY_MGMT_PSK, "PSK" },
};
#define	IW_AUTH_KEY_MGMT_NUM	IW_ARRAY_LEN(iw_auth_key_mgmt_name)

static const struct iwmask_name iw_auth_alg_name[] =
{
	{ IW_AUTH_ALG_OPEN_SYSTEM, "open" },
	{ IW_AUTH_ALG_SHARED_KEY, "shared-key" },
	{ IW_AUTH_ALG_LEAP, "LEAP" },
};
#define	IW_AUTH_ALG_NUM		IW_ARRAY_LEN(iw_auth_alg_name)

static const struct iw_auth_descr iw_auth_settings[] =
{
	{ IW_AUTH_WPA_VERSION, "WPA version", iw_wpa_ver_name, IW_WPA_VER_NUM },
	{ IW_AUTH_KEY_MGMT, "Key management", iw_auth_key_mgmt_name, IW_AUTH_KEY_MGMT_NUM },
	{ IW_AUTH_CIPHER_PAIRWISE, "Pairwise cipher", iw_auth_cypher_name, IW_AUTH_CYPHER_NUM },
	{ IW_AUTH_CIPHER_GROUP, "Pairwise cipher", iw_auth_cypher_name, IW_AUTH_CYPHER_NUM },
	{ IW_AUTH_TKIP_COUNTERMEASURES, "TKIP countermeasures", NULL, 0 },
	{ IW_AUTH_DROP_UNENCRYPTED, "Drop unencrypted", NULL, 0 },
	{ IW_AUTH_80211_AUTH_ALG, "Authentication algorithm", iw_auth_alg_name, IW_AUTH_ALG_NUM },
	{ IW_AUTH_RX_UNENCRYPTED_EAPOL, "Receive unencrypted EAPOL", NULL, 0 },
	{ IW_AUTH_ROAMING_CONTROL, "Roaming control", NULL, 0 },
	{ IW_AUTH_PRIVACY_INVOKED, "Privacy invoked", NULL, 0 },
};
#define	IW_AUTH_SETTINGS_NUM		IW_ARRAY_LEN(iw_auth_settings)

/* Values for the IW_ENCODE_ALG_* returned by SIOCSIWENCODEEXT */
static const char * iw_encode_alg_name[] = { "none", "WEP", "TKIP", "CCMP", "unknown" };
#define	IW_ENCODE_ALG_NUM		IW_ARRAY_LEN(iw_encode_alg_name)

#ifndef IW_IE_CIPHER_NONE
/* Cypher values in GENIE (pairwise and group) */
#define IW_IE_CIPHER_NONE	0
#define IW_IE_CIPHER_WEP40	1
#define IW_IE_CIPHER_TKIP	2
#define IW_IE_CIPHER_WRAP	3
#define IW_IE_CIPHER_CCMP	4
#define IW_IE_CIPHER_WEP104	5
/* Key management in GENIE */
#define IW_IE_KEY_MGMT_NONE	0
#define IW_IE_KEY_MGMT_802_1X	1
#define IW_IE_KEY_MGMT_PSK	2
#endif	/* IW_IE_CIPHER_NONE */

/* Values for the IW_IE_CIPHER_* in GENIE */
static const char * iw_ie_cypher_name[] = { "none", "WEP-40", "TKIP", "WRAP", "CCMP", "WEP-104", };
#define	IW_IE_CYPHER_NUM	IW_ARRAY_LEN(iw_ie_cypher_name)

/* Values for the IW_IE_KEY_MGMT_* in GENIE */
static const char * iw_ie_key_mgmt_name[] = { "none", "802.1x", "PSK", };
#define	IW_IE_KEY_MGMT_NUM	IW_ARRAY_LEN(iw_ie_key_mgmt_name)

/************************* WPA SUBROUTINES *************************/

/*------------------------------------------------------------------*/
/*
 * Print all names corresponding to a mask.
 * This may want to be used in iw_print_retry_value() ?
 */
static void iw_print_mask_name(ospl_uint32 mask,
		const struct iwmask_name names[], const ospl_uint32 num_names,
		const char * sep)
{
	ospl_uint32 i;

	/* Print out all names for the bitmask */
	for (i = 0; i < num_names; i++)
	{
		if (mask & names[i].mask)
		{
			/* Print out */
			iw_printf("%s%s", sep, names[i].name);
			/* Remove the bit from the mask */
			mask &= ~names[i].mask;
		}
	}
	/* If there is unconsumed bits... */
	if (mask != 0)
		iw_printf("%sUnknown", sep);
}

/*------------------------------------------------------------------*/
/*
 * Print the name corresponding to a value, with overflow check.
 */
static void iw_print_value_name(ospl_uint32 value, const char * names[],
		const ospl_uint32 num_names)
{
	if (value >= num_names)
		iw_printf(" unknown (%d)", value);
	else
		iw_printf(" %s", names[value]);
}

/*------------------------------------------------------------------*/
/*
 * Parse, and display the results of an unknown IE.
 *
 */
static void iw_print_ie_unknown(ospl_uint8 * iebuf, int buflen)
{
	int ielen = iebuf[1] + 2;
	int i;

	if (ielen > buflen)
		ielen = buflen;

	iw_printf("Unknown: ");
	for (i = 0; i < ielen; i++)
		iw_printf("%02X", iebuf[i]);
	iw_printf("\n");
}

/*------------------------------------------------------------------*/
/*
 * Parse, and display the results of a WPA or WPA2 IE.
 *
 */
static inline void iw_print_ie_wpa(ospl_uint8 * iebuf, int buflen)
{
	int ielen = iebuf[1] + 2;
	ospl_uint32 offset = 2; /* Skip the IE id, and the length. */
	ospl_uint8 wpa1_oui[3] =
	{ 0x00, 0x50, 0xf2 };
	ospl_uint8 wpa2_oui[3] =
	{ 0x00, 0x0f, 0xac };
	ospl_uint8 * wpa_oui;
	int i;
	ospl_uint16 ver = 0;
	ospl_uint16 cnt = 0;

	if (ielen > buflen)
		ielen = buflen;

#ifdef DEBUG
	/* Debugging code. In theory useless, because it's debugged ;-) */
	printf("IE raw value %d [%02X", buflen, iebuf[0]);
	for(i = 1; i < buflen; i++)
	printf(":%02X", iebuf[i]);
	printf("]\n");
#endif

	switch (iebuf[0])
	{
	case 0x30: /* WPA2 */
		/* Check if we have enough data */
		if (ielen < 4)
		{
			iw_print_ie_unknown(iebuf, buflen);
			return;
		}

		wpa_oui = wpa2_oui;
		break;

	case 0xdd: /* WPA or else */
		wpa_oui = wpa1_oui;

		/* Not all IEs that start with 0xdd are WPA.
		 * So check that the OUI is valid. Note : offset==2 */
		if ((ielen < 8) || (memcmp(&iebuf[offset], wpa_oui, 3) != 0)
				|| (iebuf[offset + 3] != 0x01))
		{
			iw_print_ie_unknown(iebuf, buflen);
			return;
		}

		/* Skip the OUI type */
		offset += 4;
		break;

	default:
		return;
	}

	/* Pick version number (little endian) */
	ver = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;

	if (iebuf[0] == 0xdd)
		iw_printf("WPA Version %d\n", ver);
	if (iebuf[0] == 0x30)
		iw_printf("IEEE 802.11i/WPA2 Version %d\n", ver);

	/* From here, everything is technically optional. */

	/* Check if we are done */
	if (ielen < (offset + 4))
	{
		/* We have a ospl_int16 IE.  So we should assume TKIP/TKIP. */
		iw_printf("                        Group Cipher : TKIP\n");
		iw_printf("                        Pairwise Cipher : TKIP\n");
		return;
	}

	/* Next we have our group cipher. */
	if (memcmp(&iebuf[offset], wpa_oui, 3) != 0)
	{
		iw_printf("                        Group Cipher : Proprietary\n");
	}
	else
	{
		iw_printf("                        Group Cipher :");
		iw_print_value_name(iebuf[offset + 3], iw_ie_cypher_name,
		IW_IE_CYPHER_NUM);
		iw_printf("\n");
	}
	offset += 4;

	/* Check if we are done */
	if (ielen < (offset + 2))
	{
		/* We don't have a pairwise cipher, or auth method. Assume TKIP. */
		iw_printf("                        Pairwise Ciphers : TKIP\n");
		return;
	}

	/* Otherwise, we have some number of pairwise ciphers. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;
	iw_printf("                        Pairwise Ciphers (%d) :", cnt);

	if (ielen < (offset + 4 * cnt))
		return;

	for (i = 0; i < cnt; i++)
	{
		if (memcmp(&iebuf[offset], wpa_oui, 3) != 0)
		{
			iw_printf(" Proprietary");
		}
		else
		{
			iw_print_value_name(iebuf[offset + 3], iw_ie_cypher_name,
			IW_IE_CYPHER_NUM);
		}
		offset += 4;
	}
	iw_printf("\n");

	/* Check if we are done */
	if (ielen < (offset + 2))
		return;

	/* Now, we have authentication suites. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;
	iw_printf("                        Authentication Suites (%d) :", cnt);

	if (ielen < (offset + 4 * cnt))
		return;

	for (i = 0; i < cnt; i++)
	{
		if (memcmp(&iebuf[offset], wpa_oui, 3) != 0)
		{
			iw_printf(" Proprietary");
		}
		else
		{
			iw_print_value_name(iebuf[offset + 3], iw_ie_key_mgmt_name,
			IW_IE_KEY_MGMT_NUM);
		}
		offset += 4;
	}
	iw_printf("\n");

	/* Check if we are done */
	if (ielen < (offset + 1))
		return;

	/* Otherwise, we have capabilities bytes.
	 * For now, we only care about preauth which is in bit position 1 of the
	 * first byte.  (But, preauth with WPA version 1 isn't supposed to be
	 * allowed.) 8-) */
	if (iebuf[offset] & 0x01)
	{
		iw_printf("                       Preauthentication Supported\n");
	}
}

/*------------------------------------------------------------------*/
/*
 * Process a generic IE and display the info in human readable form
 * for some of the most interesting ones.
 * For now, we only decode the WPA IEs.
 */
static inline void iw_print_gen_ie(ospl_uint8 * buffer, int buflen)
{
	ospl_uint32 offset = 0;

	/* Loop on each IE, each IE is minimum 2 bytes */
	while (offset <= (buflen - 2))
	{
		iw_printf("                    IE: ");

		/* Check IE type */
		switch (buffer[offset])
		{
		case 0xdd: /* WPA1 (and other) */
		case 0x30: /* WPA2 */
			iw_print_ie_wpa(buffer + offset, buflen);
			break;
		default:
			iw_print_ie_unknown(buffer + offset, buflen);
		}
		/* Skip over this IE to the next one in the list. */
		offset += buffer[offset + 1] + 2;
	}
}

/***************************** SCANNING *****************************/
/*
 * This one behave quite differently from the others
 *
 * Note that we don't use the scanning capability of iwlib (functions
 * iw_process_scan() and iw_scan()). The main reason is that
 * iw_process_scan() return only a subset of the scan data to the caller,
 * for example custom elements and bitrates are ommited. Here, we
 * do the complete job...
 */

/*------------------------------------------------------------------*/
/*
 * Print one element from the scanning results
 */
static inline void print_scanning_token(struct stream_descr * stream, /* Stream of events */
struct iw_event * event, /* Extracted token */
struct iwscan_state * state, struct iw_range * iw_range, /* Range info */
int has_range)
{
	char buffer[128]; /* Temporary buffer */
	/* Now, let's decode the event */
	switch (event->cmd)
	{
	case SIOCGIWAP:
		iw_printf("           AP: %02d - Address: %s\n", state->ap_num,
				iw_saether_ntop(&event->u.ap_addr, buffer));
		state->ap_num++;
		break;
	case SIOCGIWNWID:
		if (event->u.nwid.disabled)
			iw_printf("                    NWID:off/any\n");
		else
			iw_printf("                    NWID:%X\n", event->u.nwid.value);
		break;
	case SIOCGIWFREQ:
	{
		double freq; /* Frequency/channel */
		int channel = -1; /* Converted to channel */
		freq = iw_freq2float(&(event->u.freq));
		/* Convert to channel if possible */
		if (has_range)
			channel = iw_freq_to_channel(freq, iw_range);
		iw_print_freq(buffer, sizeof(buffer), freq, channel,
				event->u.freq.flags);
		iw_printf("                    %s\n", buffer);
	}
		break;
	case SIOCGIWMODE:
		/* Note : event->u.mode is unsigned, no need to check <= 0 */
		if (event->u.mode >= IW_NUM_OPER_MODE)
			event->u.mode = IW_NUM_OPER_MODE;
		iw_printf("                    Mode:%s\n",
				iw_operation_mode[event->u.mode]);
		break;
	case SIOCGIWNAME:
		iw_printf("                    Protocol:%-1.16s\n", event->u.name);
		break;
	case SIOCGIWESSID:
	{
		char essid[IW_ESSID_MAX_SIZE + 1];
		memset(essid, '\0', sizeof(essid));
		if ((event->u.essid.pointer) && (event->u.essid.length))
			memcpy(essid, event->u.essid.pointer, event->u.essid.length);
		if (event->u.essid.flags)
		{
			/* Does it have an ESSID index ? */
			if ((event->u.essid.flags & IW_ENCODE_INDEX) > 1)
				iw_printf("                    ESSID:\"%s\" [%d]\n", essid,
						(event->u.essid.flags & IW_ENCODE_INDEX));
			else
				iw_printf("                    ESSID:\"%s\"\n", essid);
		}
		else
			iw_printf("                    ESSID:off/any/hidden\n");
	}
		break;
	case SIOCGIWENCODE:
	{
		ospl_uint8 key[IW_ENCODING_TOKEN_MAX];
		if (event->u.data.pointer)
			memcpy(key, event->u.data.pointer, event->u.data.length);
		else
			event->u.data.flags |= IW_ENCODE_NOKEY;
		iw_printf("                    Encryption key:");
		if (event->u.data.flags & IW_ENCODE_DISABLED)
			iw_printf("off\n");
		else
		{
			/* Display the key */
			iw_print_key(buffer, sizeof(buffer), key, event->u.data.length,
					event->u.data.flags);
			iw_printf("%s", buffer);

			/* Other info... */
			if ((event->u.data.flags & IW_ENCODE_INDEX) > 1)
				iw_printf(" [%d]", event->u.data.flags & IW_ENCODE_INDEX);
			if (event->u.data.flags & IW_ENCODE_RESTRICTED)
				iw_printf("   Security mode:restricted");
			if (event->u.data.flags & IW_ENCODE_OPEN)
				iw_printf("   Security mode:open");
			iw_printf("\n");
		}
	}
		break;
	case SIOCGIWRATE:
		if (state->val_index == 0)
			iw_printf("                    Bit Rates:");
		else if ((state->val_index % 5) == 0)
			iw_printf("\n                              ");
		else
			iw_printf("; ");
		iw_print_bitrate(buffer, sizeof(buffer), event->u.bitrate.value);
		iw_printf("%s", buffer);
		/* Check for termination */
		if (stream->value == NULL)
		{
			iw_printf("\n");
			state->val_index = 0;
		}
		else
			state->val_index++;
		break;
	case SIOCGIWMODUL:
	{
		ospl_uint32 modul = event->u.param.value;
		int i;
		int n = 0;
		iw_printf("                    Modulations :");
		for (i = 0; i < IW_SIZE_MODUL_LIST; i++)
		{
			if ((modul & iw_modul_list[i].mask) == iw_modul_list[i].mask)
			{
				if ((n++ % 8) == 7)
					iw_printf("\n                        ");
				else
					iw_printf(" ; ");
				iw_printf("%s", iw_modul_list[i].cmd);
			}
		}
		iw_printf("\n");
	}
		break;
	case IWEVQUAL:
		iw_print_stats(buffer, sizeof(buffer), &event->u.qual, iw_range,
				has_range);
		iw_printf("                    %s\n", buffer);
		break;
	case IWEVGENIE:
		/* Informations Elements are complex, let's do only some of them */
		iw_print_gen_ie(event->u.data.pointer, event->u.data.length);
		break;
	case IWEVCUSTOM:
	{
		char custom[IW_CUSTOM_MAX + 1];
		if ((event->u.data.pointer) && (event->u.data.length))
			memcpy(custom, event->u.data.pointer, event->u.data.length);
		custom[event->u.data.length] = '\0';
		iw_printf("                    Extra:%s\n", custom);
	}
		break;
	default:
		iw_printf("                    (Unknown Wireless Token 0x%04X)\n",
				event->cmd);
	} /* switch(event->cmd) */
}

static void iw_print_stats_detail(char * buffer, int buflen,
		const iwqual * qual, const iwrange * range, int has_range)
{
	int len;

	/* People are very often confused by the 8 bit arithmetic happening
	 * here.
	 * All the values here are encoded in a 8 bit integer. 8 bit integers
	 * are either unsigned [0 ; 255], signed [-128 ; +127] or
	 * negative [-255 ; 0].
	 * Further, on 8 bits, 0x100 == 256 == 0.
	 *
	 * Relative/percent values are always encoded unsigned, between 0 and 255.
	 * Absolute/dBm values are always encoded between -192 and 63.
	 * (Note that up to version 28 of Wireless Tools, dBm used to be
	 *  encoded always negative, between -256 and -1).
	 *
	 * How do we separate relative from absolute values ?
	 * The old way is to use the range to do that. As of WE-19, we have
	 * an explicit IW_QUAL_DBM flag in updated...
	 * The range allow to specify the real min/max of the value. As the
	 * range struct only specify one bound of the value, we assume that
	 * the other bound is 0 (zero).
	 * For relative values, range is [0 ; range->max].
	 * For absolute values, range is [range->max ; 63].
	 *
	 * Let's take two example :
	 * 1) value is 75%. qual->value = 75 ; range->max_qual.value = 100
	 * 2) value is -54dBm. noise floor of the radio is -104dBm.
	 *    qual->value = -54 = 202 ; range->max_qual.value = -104 = 152
	 *
	 * Jean II
	 */

	/* Just do it...
	 * The old way to detect dBm require both the range and a non-null
	 * level (which confuse the test). The new way can deal with level of 0
	 * because it does an explicit test on the flag. */
	if (has_range
			&& ((qual->level != 0)
					|| (qual->updated & (IW_QUAL_DBM | IW_QUAL_RCPI))))
	{
		/* Deal with quality : always a relative value */
		if (!(qual->updated & IW_QUAL_QUAL_INVALID))
		{
			len = snprintf(buffer, buflen, "%d/%d ", qual->qual,
					range->max_qual.qual);
			buffer += len;
			buflen -= len;
		}

		/* Check if the statistics are in RCPI (IEEE 802.11k) */
		if (qual->updated & IW_QUAL_RCPI)
		{
			/* Deal with signal level in RCPI */
			/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
			if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
			{
				double rcpilevel = (qual->level / 2.0) - 110.0;
				len = snprintf(buffer, buflen, "%g dBm", rcpilevel);
				buffer += len;
				buflen -= len;
			}
		}
		else
		{
			/* Check if the statistics are in dBm */
			if ((qual->updated & IW_QUAL_DBM)
					|| (qual->level > range->max_qual.level))
			{
				/* Deal with signal level in dBm  (absolute power measurement) */
				if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
				{
					int dblevel = qual->level;
					/* Implement a range for dBm [-192; 63] */
					if (qual->level >= 64)
						dblevel -= 0x100;
					len = snprintf(buffer, buflen, "%d dBm", dblevel);
					buffer += len;
					buflen -= len;
				}
			}
			else
			{
				/* Deal with signal level as relative value (0 -> max) */
				if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
				{
					len = snprintf(buffer, buflen, "%d/%d dBm", qual->level,
							range->max_qual.level);
					buffer += len;
					buflen -= len;
				}
			}
		}
	}
	else
	{
		/* We can't read the range, so we don't know... */
		snprintf(buffer, buflen, "%d/0 %d dBm", qual->qual, qual->level);
	}
}

static inline void print_scanning_token_detail(struct stream_descr * stream, /* Stream of events */
struct iw_event * event, /* Extracted token */
struct iwscan_state * state, struct iw_range * iw_range, /* Range info */
int has_range, int finsh)
{
	char buffer[128]; /* Temporary buffer */
	if (finsh)
	{
		iw_printf("\n");
		return;
	}
	memset(buffer, 0, sizeof(buffer));
	/* Now, let's decode the event */
	switch (event->cmd)
	{
	case SIOCGIWAP:
		if (state->ap_num > 1)
			iw_printf("\n");
		iw_printf("%-18s", iw_saether_ntop(&event->u.ap_addr, buffer));
		state->ap_num++;
		break;
	case SIOCGIWNWID:
		break;
	case SIOCGIWFREQ:
	{
		double freq; /* Frequency/channel */
		int channel = -1; /* Converted to channel */
		freq = iw_freq2float(&(event->u.freq));
		/* Convert to channel if possible */
		if (has_range)
			channel = iw_freq_to_channel(freq, iw_range);
		if (freq >= KILO)
		{
			iw_print_freq_value(buffer, sizeof(buffer), freq);
			iw_printf(" %-10s", buffer);
		}
		//channel
	}
		break;
	case SIOCGIWMODE:
		/* Note : event->u.mode is unsigned, no need to check <= 0 */
		if (event->u.mode >= IW_NUM_OPER_MODE)
			event->u.mode = IW_NUM_OPER_MODE;
		iw_printf("%-10s", iw_operation_mode[event->u.mode]);
		break;
	case SIOCGIWNAME:
		//iw_printf("                    Protocol:%-1.16s\n", event->u.name);
		break;
	case SIOCGIWESSID:
	{
		char essid[IW_ESSID_MAX_SIZE + 1];
		memset(essid, '\0', sizeof(essid));
		if ((event->u.essid.pointer) && (event->u.essid.length))
			memcpy(essid, event->u.essid.pointer, event->u.essid.length);
		if (event->u.essid.flags)
		{
			/* Does it have an ESSID index ? */
			if ((event->u.essid.flags & IW_ENCODE_INDEX) > 1)
				iw_printf(" %-20s[%d]", essid,
						(event->u.essid.flags & IW_ENCODE_INDEX));
			else
				iw_printf(" %-20s", essid);
		}
		else
			iw_printf(" %-20s", "off/any/hidden");
	}
		break;
	case SIOCGIWENCODE:
	{
		ospl_uint8 key[IW_ENCODING_TOKEN_MAX];
		if (event->u.data.pointer)
			memcpy(key, event->u.data.pointer, event->u.data.length);
		else
			event->u.data.flags |= IW_ENCODE_NOKEY;
		//iw_printf("                    Encryption key:");
		if (event->u.data.flags & IW_ENCODE_DISABLED)
		{
			snprintf(buffer, sizeof(buffer), "off");
			iw_printf(" %-8s", buffer);
		}
		else
		{
			/* Other info... */
			if (event->u.data.flags & IW_ENCODE_OPEN)
			{
				snprintf(buffer, sizeof(buffer), "open");
				iw_printf(" %-8s", buffer);
			}
			else
			{
				snprintf(buffer, sizeof(buffer), "on");
				iw_printf(" %-8s", buffer);
			}
		}
	}
		break;
	case SIOCGIWRATE:
		break;
	case SIOCGIWMODUL:
		break;
	case IWEVQUAL:
		iw_print_stats_detail(buffer, sizeof(buffer), &event->u.qual, iw_range,
				has_range);
		iw_printf(" %-16s", buffer);
		break;
	case IWEVGENIE:
		/* Informations Elements are complex, let's do only some of them */
		//iw_print_gen_ie(event->u.data.pointer, event->u.data.length);
		break;
	case IWEVCUSTOM:
		break;
	default:
		break;
	} /* switch(event->cmd) */
}

/*------------------------------------------------------------------*/
/*
 * Perform a scanning on one device
 */
static int print_scanning_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_scan_req scanopt; /* Options for 'set' */
	int scanflags = 0; /* Flags for scan */
	ospl_uint8 * buffer = NULL; /* Results */
	int buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
	struct iw_range range;
	int has_range;
	struct timeval tv; /* Select timeout */
	int timeout = 15000000; /* 15s */
	int head = 0;
	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Debugging stuff */
	if ((IW_EV_LCP_PK2_LEN != IW_EV_LCP_PK_LEN)
			|| (IW_EV_POINT_PK2_LEN != IW_EV_POINT_PK_LEN))
	{
		fprintf(stderr,
				"*** Please report to jt@hpl.hp.com your platform details\n");
		fprintf(stderr, "*** and the following line :\n");
		fprintf(stderr,
				"*** IW_EV_LCP_PK2_LEN = %zu ; IW_EV_POINT_PK2_LEN = %zu\n\n",
				IW_EV_LCP_PK2_LEN, IW_EV_POINT_PK2_LEN);
	}

	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

	/* Check if the interface could support scanning. */
	if ((!has_range) || (range.we_version_compiled < 14))
	{
		fprintf(stderr, "%-8.16s  Interface doesn't support scanning.\n\n",
				kname2ifname(ifname));
		return (-1);
	}

	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));

	/* Parse command line arguments and extract options.
	 * Note : when we have enough options, we should use the parser
	 * from iwconfig... */
	while (count > 0)
	{
		/* One arg is consumed (the option name) */
		count--;

		/*
		 * Check for Active Scan (scan with specific essid)
		 */
		if (!strncmp(args[0], "essid", 5))
		{
			if (count < 1)
			{
				fprintf(stderr, "Too few arguments for scanning option [%s]\n",
						args[0]);
				return (-1);
			}
			args++;
			count--;

			/* Store the ESSID in the scan options */
			scanopt.essid_len = strlen(args[0]);
			memcpy(scanopt.essid, args[0], scanopt.essid_len);
			/* Initialise BSSID as needed */
			if (scanopt.bssid.sa_family == 0)
			{
				scanopt.bssid.sa_family = ARPHRD_ETHER;
				memset(scanopt.bssid.sa_data, 0xff, ETH_ALEN);
			}
			/* Scan only this ESSID */
			scanflags |= IW_SCAN_THIS_ESSID;
		}
		else
		/* Check for last scan result (do not trigger scan) */
		if (!strncmp(args[0], "last", 4))
		{
			/* Hack */
			scanflags |= IW_SCAN_HACK;
		}
		else
		{
			fprintf(stderr, "Invalid scanning option [%s]\n", args[0]);
			return (-1);
		}

		/* Next arg */
		args++;
	}

	/* Check if we have scan options */
	if (scanflags)
	{
		wrq.u.data.pointer = (caddr_t) & scanopt;
		wrq.u.data.length = sizeof(scanopt);
		wrq.u.data.flags = scanflags;
	}
	else
	{
		wrq.u.data.pointer = NULL;
		wrq.u.data.flags = 0;
		wrq.u.data.length = 0;
	}

	/* If only 'last' was specified on command line, don't trigger a scan */
	if (scanflags == IW_SCAN_HACK)
	{
		/* Skip waiting */
		tv.tv_usec = 0;
	}
	else
	{
		/* Initiate Scanning */
		if (iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
		{
			if ((errno != EPERM) || (scanflags != 0))
			{
				fprintf(stderr,
						"%-8.16s  Interface doesn't support scanning : %s\n\n",
						ifname, strerror(errno));
				return (-1);
			}
			/* If we don't have the permission to initiate the scan, we may
			 * still have permission to read left-over results.
			 * But, don't wait !!! */
#if 0
			/* Not cool, it display for non wireless interfaces... */
			fprintf(stderr, "%-8.16s  (Could not trigger scanning, just reading left-over results)\n", kname2ifname(ifname));
#endif
			tv.tv_usec = 0;
		}
	}
	timeout -= tv.tv_usec;

	/* Forever */
	while (1)
	{
		fd_set rfds; /* File descriptors for select */
		int last_fd; /* Last fd */
		int ret;

		/* Guess what ? We must re-generate rfds each time */
		FD_ZERO(&rfds);
		last_fd = -1;

		/* In here, add the rtnetlink fd in the list */

		/* Wait until something happens */
		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);

		/* Check if there was an error */
		if (ret < 0)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "Unhandled signal - exiting...\n");
			return (-1);
		}

		/* Check if there was a timeout */
		if (ret == 0)
		{
			ospl_uint8 * newbuf;

			realloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = realloc(buffer, buflen);
			if (newbuf == NULL)
			{
				if (buffer)
					free(buffer);
				fprintf(stderr, "%s: Allocation failed\n", __FUNCTION__);
				return (-1);
			}
			buffer = newbuf;

			/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if (iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
			{
				/* Check if buffer was too small (WE-17 only) */
				if ((errno == E2BIG) && (range.we_version_compiled > 16)
						&& (buflen < 0xFFFF))
				{
					/* Some driver may return very large scan results, either
					 * because there are many cells, or because they have many
					 * large elements in cells (like IWEVCUSTOM). Most will
					 * only need the regular sized buffer. We now use a dynamic
					 * allocation of the buffer to satisfy everybody. Of course,
					 * as we don't know in advance the size of the array, we try
					 * various increasing sizes. Jean II */

					/* Check if the driver gave us any hints. */
					if (wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;

					/* wrq.u.data.length is 16 bits so max size is 65535 */
					if (buflen > 0xFFFF)
						buflen = 0xFFFF;

					/* Try again */
					goto realloc;
				}

				/* Check if results not available yet */
				if (errno == EAGAIN)
				{
					/* Restart timer for only 100ms*/
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if (timeout > 0)
						continue; /* Try again later */
				}

				/* Bad error */
				free(buffer);
				fprintf(stderr, "%-8.16s  Failed to read scan data : %s\n\n",
						ifname, strerror(errno));
				return (-2);
			}
			else
				/* We have the results, go to process them */
				break;
		}

		/* In here, check if event and event type
		 * if scan event, read results. All errors bad & no reset timeout */
	}

	if (wrq.u.data.length)
	{
		struct iw_event iwe;
		struct stream_descr stream;
		struct iwscan_state state =
		{ .ap_num = 1, .val_index = 0 };
		int ret;

#ifdef DEBUG
		/* Debugging code. In theory useless, because it's debugged ;-) */
		int i;
		printf("Scan result %d [%02X", wrq.u.data.length, buffer[0]);
		for(i = 1; i < wrq.u.data.length; i++)
		printf(":%02X", buffer[i]);
		printf("]\n");
#endif
		//if (!iwlist_detail)
		//	iw_printf("%-8.16s  Scan completed :\n", kname2ifname(ifname));
		//else
		{
			if (head == 0 && iwlist_detail)
			{
				head = 1;
				iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
						"------------------", "----------", "----------------",
						"--------", "--------------------", "----------");
				iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
						"      BSSIED      ", "   FREQ   ", "qual/max signal",
						"  KEY", "        NAME", "   MODE");
				iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
						"------------------", "----------", "----------------",
						"--------", "--------------------", "----------");
			}
		}
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		do
		{
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe,
					range.we_version_compiled);
			if (ret > 0)
			{
				if (iwlist_detail)
					print_scanning_token_detail(&stream, &iwe, &state, &range,
							has_range, 0);
				else
					print_scanning_token(&stream, &iwe, &state, &range,
							has_range);
			}
			else
			{
				if (iwlist_detail)
					print_scanning_token_detail(&stream, &iwe, &state, &range,
							has_range, 1);
			}
		} while (ret > 0);
		iw_printf("\n");
	}
	//else
	//	iw_printf("%-8.16s  No scan results\n\n", kname2ifname(ifname));
	//iwlist_detail = 0;
	free(buffer);
	return (0);
}

/*********************** FREQUENCIES/CHANNELS ***********************/

/*------------------------------------------------------------------*/
/*
 * Print the number of channels and available frequency for the device
 */
/* Create a channel to the NET kernel. */
int iw_get_freq_info(char * ifname, iw_dev_t *iwdev)
{
	struct iw_range range;
	double freq;
	int k = 0;//, i = 0;
	//char buffer[128]; /* Temporary buffer */

	int skfd = 0;//, goterr = 0;
	if ((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}
	memset(&iwdev->freq, 0, sizeof(iwdev->freq));
	/* Get list of frequencies / channels */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
	{
		fprintf(stderr, "%-8.16s  no frequency information.\n\n", kname2ifname(ifname));
		return -1;
	}
	else
	{
		if (range.num_frequency > 0)
		{
			iw_printf("total: %d channels, available frequencies :\n",
					range.num_channels);

			for (k = 0; k < range.num_frequency; k++)
			{
				freq = iw_freq2float(&(range.freq[k]));
				iwdev->freq[k].active = ospl_true;
				iwdev->freq[k].channel = range.freq[k].i;
				iwdev->freq[k].freq = freq;

/*				iw_print_freq_value(buffer, sizeof(buffer), freq);

				iw_printf("          Channel %.2d : %s\n", range.freq[k].i,
						buffer);*/
			}
		}
		//else
		//	iw_printf("%-8.16s  %d channels\n", kname2ifname(ifname), range.num_channels);
	}
	iw_sockets_close(skfd);
	return (0);
}


static int print_freq_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	//struct iwreq wrq;
	struct iw_range range;
	double freq;
	int k;
	//int channel;
	char buffer[128]; /* Temporary buffer */

	/* Avoid "Unused parameter" warning */
/*
	args = args;
	count = count;
*/

	/* Get list of frequencies / channels */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
		fprintf(stderr, "%-8.16s  no frequency information.\n\n", kname2ifname(ifname));
	else
	{
		if (range.num_frequency > 0)
		{
			/*			iw_printf(
			 "%-8.16s  %d channels in total; available frequencies :\n",
			 ifname, range.num_channels);*/
			iw_printf("total: %d channels, available frequencies :\n",
					range.num_channels);
			/* Print them all */
			for (k = 0; k < range.num_frequency; k++)
			{
				freq = iw_freq2float(&(range.freq[k]));
				iw_print_freq_value(buffer, sizeof(buffer), freq);

				iw_printf("          Channel %.2d : %s\n", range.freq[k].i,
						buffer);
			}
		}
		else
			iw_printf("%-8.16s  %d channels\n", kname2ifname(ifname), range.num_channels);

		/* Get current frequency / channel and display it */
		/*		if (iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) >= 0)
		 {
		 freq = iw_freq2float(&(wrq.u.freq));
		 channel = iw_freq_to_channel(freq, &range);
		 iw_print_freq(buffer, sizeof(buffer), freq, channel,
		 wrq.u.freq.flags);
		 iw_printf("          Current %s\n\n", buffer);
		 }*/
	}
	return (0);
}

/***************************** BITRATES *****************************/

/*------------------------------------------------------------------*/
/*
 * Print the number of available bitrates for the device
 */
int iw_get_bitrate_info(char * ifname, iw_dev_t *iwdev)
{
	struct iwreq wrq;
	struct iw_range range;
	int k = 0;//, i = 0;
	//char buffer[128]; /* Temporary buffer */

	int skfd = 0;//, goterr = 0;
	if ((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}
	memset(&iwdev->bitrates, 0, sizeof(iwdev->bitrates));
	/* Get list of frequencies / channels */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
	{
		fprintf(stderr, "%-8.16s  no bit-rate information.\n\n", kname2ifname(ifname));
		return -1;
	}
	else
	{
		iwdev->num_bitrates = range.num_bitrates;
		if ((range.num_bitrates > 0) && (range.num_bitrates <= IW_MAX_BITRATES))
		{
			for (k = 0; k < range.num_bitrates; k++)
			{
				iwdev->bitrates[k].active = ospl_true;
				iwdev->bitrates[k].channel = k;
				iwdev->bitrates[k].bitrate = range.bitrate[k];
			}
		}
		//else
		//	iw_printf("%-8.16s  unknown bit-rate information.\n", kname2ifname(ifname));

		/* Get current bit rate */
		if (iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
		{
			iwdev->cu_bitrates.bitrate = wrq.u.bitrate.value;
			iwdev->cu_bitrates.active = ospl_true;
			//iw_print_bitrate(buffer, sizeof(buffer), wrq.u.bitrate.value);
			//iw_printf("          Current Bit Rate%c%s\n",
			//		(wrq.u.bitrate.fixed ? '=' : ':'), buffer);
		}

		/* Try to get the broadcast bitrate if it exist... */
		if (range.bitrate_capa & IW_BITRATE_BROADCAST)
		{
			wrq.u.bitrate.flags = IW_BITRATE_BROADCAST;
			if (iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
			{
				iwdev->broadcast_bitrates.bitrate = wrq.u.bitrate.value;
				iwdev->broadcast_bitrates.active = ospl_true;

				//iw_print_bitrate(buffer, sizeof(buffer), wrq.u.bitrate.value);
				//iw_printf("          Broadcast Bit Rate%c%s\n",
				//		(wrq.u.bitrate.fixed ? '=' : ':'), buffer);
			}
		}
	}
	iw_sockets_close(skfd);
	return (0);
}

static int print_bitrate_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	int k;
	char buffer[128];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
		fprintf(stderr, "%-8.16s  no bit-rate information.\n\n", kname2ifname(ifname));
	else
	{
		if ((range.num_bitrates > 0) && (range.num_bitrates <= IW_MAX_BITRATES))
		{
			iw_printf("%-8.16s  %d available bit-rates :\n", kname2ifname(ifname),
					range.num_bitrates);
			/* Print them all */
			for (k = 0; k < range.num_bitrates; k++)
			{
				iw_print_bitrate(buffer, sizeof(buffer), range.bitrate[k]);
				/* Maybe this should be %10s */
				iw_printf("\t  %s\n", buffer);
			}
		}
		else
			iw_printf("%-8.16s  unknown bit-rate information.\n", kname2ifname(ifname));

		/* Get current bit rate */
		if (iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
		{
			iw_print_bitrate(buffer, sizeof(buffer), wrq.u.bitrate.value);
			iw_printf("          Current Bit Rate%c%s\n",
					(wrq.u.bitrate.fixed ? '=' : ':'), buffer);
		}

		/* Try to get the broadcast bitrate if it exist... */
		if (range.bitrate_capa & IW_BITRATE_BROADCAST)
		{
			wrq.u.bitrate.flags = IW_BITRATE_BROADCAST;
			if (iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
			{
				iw_print_bitrate(buffer, sizeof(buffer), wrq.u.bitrate.value);
				iw_printf("          Broadcast Bit Rate%c%s\n",
						(wrq.u.bitrate.fixed ? '=' : ':'), buffer);
			}
		}

		iw_printf("\n");
	}
	return (0);
}

/************************* ENCRYPTION KEYS *************************/

/*------------------------------------------------------------------*/
/*
 * Print all the available encryption keys for the device
 */
static int print_keys_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	ospl_uint8 key[IW_ENCODING_TOKEN_MAX];
	ospl_uint32 k;
	char buffer[128];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
		fprintf(stderr, "%-8.16s  no encryption keys information.\n\n", kname2ifname(ifname));
	else
	{
		iw_printf("%-8.16s  ", kname2ifname(ifname));
		/* Print key sizes */
		if ((range.num_encoding_sizes > 0)
				&& (range.num_encoding_sizes < IW_MAX_ENCODING_SIZES))
		{
			iw_printf("%d key sizes : %d", range.num_encoding_sizes,
					range.encoding_size[0] * 8);
			/* Print them all */
			for (k = 1; k < range.num_encoding_sizes; k++)
				iw_printf(", %d", range.encoding_size[k] * 8);
			iw_printf("bits\n          ");
		}
		/* Print the keys and associate mode */
		iw_printf("%d keys available :\n", range.max_encoding_tokens);
		for (k = 1; k <= range.max_encoding_tokens; k++)
		{
			wrq.u.data.pointer = (caddr_t) key;
			wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
			wrq.u.data.flags = k;
			if (iw_get_ext(skfd, ifname, SIOCGIWENCODE, &wrq) < 0)
			{
				fprintf(stderr,
						"Error reading wireless keys (SIOCGIWENCODE): %s\n",
						strerror(errno));
				break;
			}
			if ((wrq.u.data.flags & IW_ENCODE_DISABLED)
					|| (wrq.u.data.length == 0))
				iw_printf("\t\t[%d]: off\n", k);
			else
			{
				/* Display the key */
				iw_print_key(buffer, sizeof(buffer), key, wrq.u.data.length,
						wrq.u.data.flags);
				iw_printf("\t\t[%d]: %s", k, buffer);

				/* Other info... */
				iw_printf(" (%d bits)", wrq.u.data.length * 8);
				iw_printf("\n");
			}
		}
		/* Print current key index and mode */
		wrq.u.data.pointer = (caddr_t) key;
		wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
		wrq.u.data.flags = 0; /* Set index to zero to get current */
		if (iw_get_ext(skfd, ifname, SIOCGIWENCODE, &wrq) >= 0)
		{
			/* Note : if above fails, we have already printed an error
			 * message int the loop above */
			iw_printf("          Current Transmit Key: [%d]\n",
					wrq.u.data.flags & IW_ENCODE_INDEX);
			if (wrq.u.data.flags & IW_ENCODE_RESTRICTED)
				iw_printf("          Security mode:restricted\n");
			if (wrq.u.data.flags & IW_ENCODE_OPEN)
				iw_printf("          Security mode:open\n");
		}

		iw_printf("\n\n");
	}
	return (0);
}

/************************* POWER MANAGEMENT *************************/

/*------------------------------------------------------------------*/
/*
 * Print Power Management info for each device
 */
static int get_pm_value(int skfd, char * ifname, struct iwreq * pwrq, ospl_uint32 flags,
		char * buffer, int buflen, int we_version_compiled)
{
	/* Get Another Power Management value */
	pwrq->u.power.flags = flags;
	if (iw_get_ext(skfd, ifname, SIOCGIWPOWER, pwrq) >= 0)
	{
		/* Let's check the value and its type */
		if (pwrq->u.power.flags & IW_POWER_TYPE)
		{
			iw_print_pm_value(buffer, buflen, pwrq->u.power.value,
					pwrq->u.power.flags, we_version_compiled);
			iw_printf("\n                 %s", buffer);
		}
	}
	return (pwrq->u.power.flags);
}

/*------------------------------------------------------------------*/
/*
 * Print Power Management range for each type
 */
static void print_pm_value_range(char * name, int mask, int iwr_flags,
		int iwr_min, int iwr_max, char * buffer, int buflen,
		int we_version_compiled)
{
	if (iwr_flags & mask)
	{
		ospl_uint32 flags = (iwr_flags & ~(IW_POWER_MIN | IW_POWER_MAX));
		/* Display if auto or fixed */
		iw_printf("%s %s ; ", (iwr_flags & IW_POWER_MIN) ? "Auto " : "Fixed",
				name);
		/* Print the range */
		iw_print_pm_value(buffer, buflen, iwr_min, flags | IW_POWER_MIN,
				we_version_compiled);
		iw_printf("%s\n                          ", buffer);
		iw_print_pm_value(buffer, buflen, iwr_max, flags | IW_POWER_MAX,
				we_version_compiled);
		iw_printf("%s\n          ", buffer);
	}
}

/*------------------------------------------------------------------*/
/*
 * Power Management types of values
 */
static const ospl_uint32 pm_type_flags[] =
{
IW_POWER_PERIOD,
IW_POWER_TIMEOUT,
IW_POWER_SAVING, };
static const int pm_type_flags_size = (sizeof(pm_type_flags)
		/ sizeof(pm_type_flags[0]));

/*------------------------------------------------------------------*/
/*
 * Print Power Management info for each device
 */
static int print_pm_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	char buffer[128];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 10))
		fprintf(stderr, "%-8.16s  no power management information.\n\n",
				kname2ifname(ifname));
	else
	{
		iw_printf("%-8.16s  ", kname2ifname(ifname));

		/* Display modes availables */
		if (range.pm_capa & IW_POWER_MODE)
		{
			iw_printf("Supported modes :\n          ");
			if (range.pm_capa & (IW_POWER_UNICAST_R | IW_POWER_MULTICAST_R))
				iw_printf(
						"\t\to Receive all packets (unicast & multicast)\n          ");
			if (range.pm_capa & IW_POWER_UNICAST_R)
				iw_printf(
						"\t\to Receive Unicast only (discard multicast)\n          ");
			if (range.pm_capa & IW_POWER_MULTICAST_R)
				iw_printf(
						"\t\to Receive Multicast only (discard unicast)\n          ");
			if (range.pm_capa & IW_POWER_FORCE_S)
				iw_printf(
						"\t\to Force sending using Power Management\n          ");
			if (range.pm_capa & IW_POWER_REPEATER)
				iw_printf("\t\to Repeat multicast\n          ");
		}
		/* Display min/max period availables */
		print_pm_value_range("period ", IW_POWER_PERIOD, range.pmp_flags,
				range.min_pmp, range.max_pmp, buffer, sizeof(buffer),
				range.we_version_compiled);
		/* Display min/max timeout availables */
		print_pm_value_range("timeout", IW_POWER_TIMEOUT, range.pmt_flags,
				range.min_pmt, range.max_pmt, buffer, sizeof(buffer),
				range.we_version_compiled);
		/* Display min/max saving availables */
		print_pm_value_range("saving ", IW_POWER_SAVING, range.pms_flags,
				range.min_pms, range.max_pms, buffer, sizeof(buffer),
				range.we_version_compiled);

		/* Get current Power Management settings */
		wrq.u.power.flags = 0;
		if (iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) >= 0)
		{
			ospl_uint32 flags = wrq.u.power.flags;

			/* Is it disabled ? */
			if (wrq.u.power.disabled)
				iw_printf("Current mode:off\n");
			else
			{
				ospl_uint32 pm_type = 0;
				ospl_uint32 pm_mask = 0;
				ospl_uint32 remain_mask = range.pm_capa & IW_POWER_TYPE;
				int i = 0;

				/* Let's check the mode */
				iw_print_pm_mode(buffer, sizeof(buffer), flags);
				iw_printf("Current %s", buffer);

				/* Let's check if nothing (simply on) */
				if ((flags & IW_POWER_MODE) == IW_POWER_ON)
					iw_printf("mode:on");

				/* Let's check the value and its type */
				if (wrq.u.power.flags & IW_POWER_TYPE)
				{
					iw_print_pm_value(buffer, sizeof(buffer), wrq.u.power.value,
							wrq.u.power.flags, range.we_version_compiled);
					iw_printf("\n                 %s", buffer);
				}

				while (1)
				{
					/* Deal with min/max for the current value */
					pm_mask = 0;
					/* If we have been returned a MIN value, ask for the MAX */
					if (flags & IW_POWER_MIN)
						pm_mask = IW_POWER_MAX;
					/* If we have been returned a MAX value, ask for the MIN */
					if (flags & IW_POWER_MAX)
						pm_mask = IW_POWER_MIN;
					/* If we have something to ask for... */
					if (pm_mask)
					{
						pm_mask |= pm_type;
						get_pm_value(skfd, ifname, &wrq, pm_mask, buffer,
								sizeof(buffer), range.we_version_compiled);
					}

					/* Remove current type from mask */
					remain_mask &= ~(wrq.u.power.flags);

					/* Check what other types we still have to read */
					while (i < pm_type_flags_size)
					{
						pm_type = remain_mask & pm_type_flags[i];
						if (pm_type)
							break;
						i++;
					}
					/* Nothing anymore : exit the loop */
					if (!pm_type)
						break;

					/* Ask for this other type of value */
					flags = get_pm_value(skfd, ifname, &wrq, pm_type, buffer,
							sizeof(buffer), range.we_version_compiled);
					/* Loop back for min/max */
				}
				iw_printf("\n");
			}
		}
		iw_printf("\n");
	}
	return (0);
}

/************************** TRANSMIT POWER **************************/

/*------------------------------------------------------------------*/
/*
 * Print the number of available transmit powers for the device
 */
static int print_txpower_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	int dbm;
	int mwatt;
	int k;

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 10))
		fprintf(stderr, "%-8.16s  no transmit-power information.\n\n", kname2ifname(ifname));
	else
	{
		if ((range.num_txpower <= 0) || (range.num_txpower > IW_MAX_TXPOWER))
			iw_printf("%-8.16s  unknown transmit-power information.\n\n",
					kname2ifname(ifname));
		else
		{
			iw_printf("%-8.16s  %d available transmit-powers :\n", kname2ifname(ifname),
					range.num_txpower);
			/* Print them all */
			for (k = 0; k < range.num_txpower; k++)
			{
				/* Check for relative values */
				if (range.txpower_capa & IW_TXPOW_RELATIVE)
				{
					iw_printf("\t  %d (no units)\n", range.txpower[k]);
				}
				else
				{
					if (range.txpower_capa & IW_TXPOW_MWATT)
					{
						dbm = iw_mwatt2dbm(range.txpower[k]);
						mwatt = range.txpower[k];
					}
					else
					{
						dbm = range.txpower[k];
						mwatt = iw_dbm2mwatt(range.txpower[k]);
					}
					iw_printf("\t  %d dBm  \t(%d mW)\n", dbm, mwatt);
				}
			}
		}

		/* Get current Transmit Power */
		if (iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) >= 0)
		{
			iw_printf("          Current Tx-Power");
			/* Disabled ? */
			if (wrq.u.txpower.disabled)
				iw_printf(":off\n\n");
			else
			{
				/* Fixed ? */
				if (wrq.u.txpower.fixed)
					iw_printf("=");
				else
					iw_printf(":");
				/* Check for relative values */
				if (wrq.u.txpower.flags & IW_TXPOW_RELATIVE)
				{
					/* I just hate relative value, because they are
					 * driver specific, so not very meaningfull to apps.
					 * But, we have to support that, because
					 * this is the way hardware is... */
					iw_printf("\t  %d (no units)\n", wrq.u.txpower.value);
				}
				else
				{
					if (wrq.u.txpower.flags & IW_TXPOW_MWATT)
					{
						dbm = iw_mwatt2dbm(wrq.u.txpower.value);
						mwatt = wrq.u.txpower.value;
					}
					else
					{
						dbm = wrq.u.txpower.value;
						mwatt = iw_dbm2mwatt(wrq.u.txpower.value);
					}
					iw_printf("%d dBm  \t(%d mW)\n\n", dbm, mwatt);
				}
			}
		}
	}
	return (0);
}

#ifndef WE_ESSENTIAL
/*********************** RETRY LIMIT/LIFETIME ***********************/

/*------------------------------------------------------------------*/
/*
 * Print one retry value
 */
static int get_retry_value(int skfd, char * ifname, struct iwreq * pwrq,
		ospl_uint32 flags, char * buffer, int buflen, int we_version_compiled)
{
	/* Get Another retry value */
	pwrq->u.retry.flags = flags;
	if (iw_get_ext(skfd, ifname, SIOCGIWRETRY, pwrq) >= 0)
	{
		/* Let's check the value and its type */
		if (pwrq->u.retry.flags & IW_RETRY_TYPE)
		{
			iw_print_retry_value(buffer, buflen, pwrq->u.retry.value,
					pwrq->u.retry.flags, we_version_compiled);
			iw_printf("%s\n                 ", buffer);
		}
	}
	return (pwrq->u.retry.flags);
}

/*------------------------------------------------------------------*/
/*
 * Print Power Management range for each type
 */
static void print_retry_value_range(char * name, int mask, int iwr_flags,
		int iwr_min, int iwr_max, char * buffer, int buflen,
		int we_version_compiled)
{
	if (iwr_flags & mask)
	{
		ospl_uint32 flags = (iwr_flags & ~(IW_RETRY_MIN | IW_RETRY_MAX));
		/* Display if auto or fixed */
		iw_printf("%s %s ; ", (iwr_flags & IW_POWER_MIN) ? "Auto " : "Fixed",
				name);
		/* Print the range */
		iw_print_retry_value(buffer, buflen, iwr_min, flags | IW_POWER_MIN,
				we_version_compiled);
		iw_printf("%s\n                           ", buffer);
		iw_print_retry_value(buffer, buflen, iwr_max, flags | IW_POWER_MAX,
				we_version_compiled);
		iw_printf("%s\n          ", buffer);
	}
}

/*------------------------------------------------------------------*/
/*
 * Print Retry info for each device
 */
static int print_retry_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	char buffer[128];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 11))
		fprintf(stderr, "%-8.16s  no retry limit/lifetime information.\n\n",
				kname2ifname(ifname));
	else
	{
		iw_printf("%-8.16s  ", kname2ifname(ifname));

		/* Display min/max limit availables */
		print_retry_value_range("limit   ", IW_RETRY_LIMIT, range.retry_flags,
				range.min_retry, range.max_retry, buffer, sizeof(buffer),
				range.we_version_compiled);
		/* Display min/max lifetime availables */
		print_retry_value_range("lifetime", IW_RETRY_LIFETIME,
				range.r_time_flags, range.min_r_time, range.max_r_time, buffer,
				sizeof(buffer), range.we_version_compiled);

		/* Get current retry settings */
		wrq.u.retry.flags = 0;
		if (iw_get_ext(skfd, ifname, SIOCGIWRETRY, &wrq) >= 0)
		{
			ospl_uint32 flags = wrq.u.retry.flags;

			/* Is it disabled ? */
			if (wrq.u.retry.disabled)
				iw_printf("Current mode:off\n          ");
			else
			{
				ospl_uint32 retry_type = 0;
				ospl_uint32 retry_mask = 0;
				ospl_uint32 remain_mask = range.retry_capa & IW_RETRY_TYPE;

				/* Let's check the mode */
				iw_printf("Current mode:on\n                 ");

				/* Let's check the value and its type */
				if (wrq.u.retry.flags & IW_RETRY_TYPE)
				{
					iw_print_retry_value(buffer, sizeof(buffer),
							wrq.u.retry.value, wrq.u.retry.flags,
							range.we_version_compiled);
					iw_printf("%s\n                 ", buffer);
				}

				while (1)
				{
					/* Deal with min/max/ospl_int16/long for the current value */
					retry_mask = 0;
					/* If we have been returned a MIN value, ask for the MAX */
					if (flags & IW_RETRY_MIN)
						retry_mask = IW_RETRY_MAX;
					/* If we have been returned a MAX value, ask for the MIN */
					if (flags & IW_RETRY_MAX)
						retry_mask = IW_RETRY_MIN;
					/* Same for SHORT and LONG */
					if (flags & IW_RETRY_SHORT)
						retry_mask = IW_RETRY_LONG;
					if (flags & IW_RETRY_LONG)
						retry_mask = IW_RETRY_SHORT;
					/* If we have something to ask for... */
					if (retry_mask)
					{
						retry_mask |= retry_type;
						get_retry_value(skfd, ifname, &wrq, retry_mask, buffer,
								sizeof(buffer), range.we_version_compiled);
					}

					/* And if we have both a limit and a lifetime,
					 * ask the other one */
					remain_mask &= ~(wrq.u.retry.flags);
					retry_type = remain_mask;
					/* Nothing anymore : exit the loop */
					if (!retry_type)
						break;

					/* Ask for this other type of value */
					flags = get_retry_value(skfd, ifname, &wrq, retry_type,
							buffer, sizeof(buffer), range.we_version_compiled);
					/* Loop back for min/max/ospl_int16/long */
				}
			}
		}
		iw_printf("\n");
	}
	return (0);
}

/************************ ACCESS POINT LIST ************************/
/*
 * Note : now that we have scanning support, this is depracted and
 * won't survive long. Actually, next version it's out !
 */

/*------------------------------------------------------------------*/
/*
 * Display the list of ap addresses and the associated stats
 * Exacly the same as the spy list, only with different IOCTL and messages
 */
static int print_ap_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	char buffer[(sizeof(struct iw_quality) + sizeof(struct sockaddr))
			* IW_MAX_AP];
	char temp[128];
	struct sockaddr * hwa;
	struct iw_quality * qual;
	iwrange range;
	int has_range = 0;
	int has_qual = 0;
	int n;
	int i;

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Collect stats */
	wrq.u.data.pointer = (caddr_t) buffer;
	wrq.u.data.length = IW_MAX_AP;
	wrq.u.data.flags = 0;
	if (iw_get_ext(skfd, ifname, SIOCGIWAPLIST, &wrq) < 0)
	{
		fprintf(stderr,
				"%-8.16s  Interface doesn't have a list of Peers/Access-Points\n\n",
				kname2ifname(ifname));
		return (-1);
	}

	/* Number of addresses */
	n = wrq.u.data.length;
	has_qual = wrq.u.data.flags;

	/* The two lists */
	hwa = (struct sockaddr *) buffer;
	qual = (struct iw_quality *) (buffer + (sizeof(struct sockaddr) * n));

	/* Check if we have valid mac address type */
	if (iw_check_mac_addr_type(skfd, ifname) < 0)
	{
		fprintf(stderr, "%-8.16s  Interface doesn't support MAC addresses\n\n",
				kname2ifname(ifname));
		return (-2);
	}

	/* Get range info if we can */
	if (iw_get_range_info(skfd, ifname, &(range)) >= 0)
		has_range = 1;

	/* Display it */
	if (n == 0)
		iw_printf("%-8.16s  No Peers/Access-Point in range\n", kname2ifname(ifname));
	else
		iw_printf("%-8.16s  Peers/Access-Points in range:\n", kname2ifname(ifname));
	for (i = 0; i < n; i++)
	{
		if (has_qual)
		{
			/* Print stats for this address */
			iw_printf("    %s : ", iw_saether_ntop(&hwa[i], temp));
			iw_print_stats(temp, sizeof(buffer), &qual[i], &range, has_range);
			iw_printf("%s\n", temp);
		}
		else
			/* Only print the address */
			iw_printf("    %s\n", iw_saether_ntop(&hwa[i], temp));
	}
	iw_printf("\n");
	return (0);
}

/******************** WIRELESS EVENT CAPABILITY ********************/

static const char * event_capa_req[] =
{ [SIOCSIWNWID - SIOCIWFIRST] = "Set NWID (kernel generated)", [SIOCSIWFREQ
		- SIOCIWFIRST] = "Set Frequency/Channel (kernel generated)",
		[SIOCGIWFREQ - SIOCIWFIRST] = "New Frequency/Channel", [SIOCSIWMODE
				- SIOCIWFIRST] = "Set Mode (kernel generated)", [SIOCGIWTHRSPY
				- SIOCIWFIRST] = "Spy threshold crossed", [SIOCGIWAP
				- SIOCIWFIRST] = "New Access Point/Cell address - roaming",
		[SIOCGIWSCAN - SIOCIWFIRST] = "Scan request completed", [SIOCSIWESSID
				- SIOCIWFIRST] = "Set ESSID (kernel generated)", [SIOCGIWESSID
				- SIOCIWFIRST] = "New ESSID", [SIOCGIWRATE - SIOCIWFIRST
				] = "New bit-rate", [SIOCSIWENCODE - SIOCIWFIRST
				] = "Set Encoding (kernel generated)", [SIOCGIWPOWER
				- SIOCIWFIRST] = NULL, };

static const char * event_capa_evt[] =
{ [IWEVTXDROP - IWEVFIRST] = "Tx packet dropped - retry exceeded", [IWEVCUSTOM
		- IWEVFIRST] = "Custom driver event", [IWEVREGISTERED - IWEVFIRST
		] = "Registered node", [IWEVEXPIRED - IWEVFIRST] = "Expired node", };

/*------------------------------------------------------------------*/
/*
 * Print the event capability for the device
 */
static int print_event_capa_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iw_range range;
	ospl_uint32 cmd;

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 10))
		fprintf(stderr,
				"%-8.16s  no wireless event capability information.\n\n",
				kname2ifname(ifname));
	else
	{
#ifdef DEBUG
		/* Debugging ;-) */
		for(cmd = 0x8B00; cmd < 0x8C0F; cmd++)
		{
			int idx = IW_EVENT_CAPA_INDEX(cmd);
			int mask = IW_EVENT_CAPA_MASK(cmd);
			printf("0x%X - %d - %X\n", cmd, idx, mask);
		}
#endif

		iw_printf("%-8.16s  Wireless Events supported :\n", kname2ifname(ifname));

		for (cmd = SIOCIWFIRST; cmd <= SIOCGIWPOWER; cmd++)
		{
			int idx = IW_EVENT_CAPA_INDEX(cmd);
			int mask = IW_EVENT_CAPA_MASK(cmd);
			if (range.event_capa[idx] & mask)
				iw_printf("          0x%04X : %s\n", cmd,
						event_capa_req[cmd - SIOCIWFIRST]);
		}
		for (cmd = IWEVFIRST; cmd <= IWEVEXPIRED; cmd++)
		{
			int idx = IW_EVENT_CAPA_INDEX(cmd);
			int mask = IW_EVENT_CAPA_MASK(cmd);
			if (range.event_capa[idx] & mask)
				iw_printf("          0x%04X : %s\n", cmd,
						event_capa_evt[cmd - IWEVFIRST]);
		}
		iw_printf("\n");
	}
	return (0);
}

/*************************** WPA SUPPORT ***************************/

/*------------------------------------------------------------------*/
/*
 * Print the authentication parameters for the device
 */
static int print_auth_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	ospl_uint32 k;

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 18))
		fprintf(stderr, "%-8.16s  no authentication information.\n\n", kname2ifname(ifname));
	else
	{
		/* Print WPA/802.1x/802.11i security parameters */
		if (!range.enc_capa)
		{
			iw_printf("%-8.16s  unknown authentication information.\n\n",
					kname2ifname(ifname));
		}
		else
		{
			/* Display advanced encryption capabilities */
			iw_printf("%-8.16s  Authentication capabilities :", kname2ifname(ifname));
			iw_print_mask_name(range.enc_capa, iw_auth_capa_name,
			IW_AUTH_CAPA_NUM, "\n\t\t");
			iw_printf("\n");

			/* Extract all auth settings */
			for (k = 0; k < IW_AUTH_SETTINGS_NUM; k++)
			{
				//wrq.u.param.flags = iw_auth_settings[k].value;
				if (iw_get_ext(skfd, ifname, SIOCGIWAUTH, &wrq) >= 0)
				{
					iw_printf("          Current %s(0x%x) :",
							iw_auth_settings[k].label, wrq.u.param.flags);
					if (iw_auth_settings[k].names != NULL)
						iw_print_mask_name(wrq.u.param.value,
								iw_auth_settings[k].names,
								iw_auth_settings[k].num_names, "\n\t\t");
					else
						iw_printf((wrq.u.param.value) ? " yes" : " no");
					iw_printf("\n");
				}
			}
		}

		iw_printf("\n\n");
	}
	return (0);
}

/*------------------------------------------------------------------*/
/*
 * Print all the available wpa keys for the device
 */
static int print_wpakeys_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;
	ospl_uint8 extbuf[IW_EXTKEY_SIZE];
	struct iw_encode_ext *extinfo;
	ospl_uint32 k;
	char buffer[128];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* This always point to the same place */
	extinfo = (struct iw_encode_ext *) extbuf;

	/* Extract range info */
	if (iw_get_range_info(skfd, ifname, &range) < 0)
		fprintf(stderr, "%-8.16s  no wpa key information.\n\n", kname2ifname(ifname));
	else
	{
		iw_printf("%-8.16s  ", kname2ifname(ifname));
		/* Print key sizes */
		if ((range.num_encoding_sizes > 0)
				&& (range.num_encoding_sizes < IW_MAX_ENCODING_SIZES))
		{
			iw_printf("%d key sizes : %d", range.num_encoding_sizes,
					range.encoding_size[0] * 8);
			/* Print them all */
			for (k = 1; k < range.num_encoding_sizes; k++)
				iw_printf(", %d", range.encoding_size[k] * 8);
			iw_printf("bits\n          ");
		}

		/* Print the keys */
		iw_printf("%d keys available :\n", range.max_encoding_tokens);
		for (k = 1; k <= range.max_encoding_tokens; k++)
		{
			/* Cleanup. Driver may not fill everything */
			memset(extbuf, '\0', IW_EXTKEY_SIZE);

			/* Get whole struct containing one WPA key */
			wrq.u.data.pointer = (caddr_t) extbuf;
			wrq.u.data.length = IW_EXTKEY_SIZE;
			wrq.u.data.flags = k;
			if (iw_get_ext(skfd, ifname, SIOCGIWENCODEEXT, &wrq) < 0)
			{
				fprintf(stderr,
						"Error reading wpa keys (SIOCGIWENCODEEXT): %s\n",
						strerror(errno));
				break;
			}

			/* Sanity check */
			if (wrq.u.data.length
					< (sizeof(struct iw_encode_ext) + extinfo->key_len))
				break;

			/* Check if key is disabled */
			if ((wrq.u.data.flags & IW_ENCODE_DISABLED)
					|| (extinfo->key_len == 0))
				iw_printf("\t\t[%d]: off\n", k);
			else
			{
				/* Display the key */
				iw_print_key(buffer, sizeof(buffer), extinfo->key,
						extinfo->key_len, wrq.u.data.flags);
				iw_printf("\t\t[%d]: %s", k, buffer);

				/* Key size */
				iw_printf(" (%d bits)", extinfo->key_len * 8);
				iw_printf("\n");

				/* Other info... */
				iw_printf("\t\t     Address: %s\n",
						iw_saether_ntop(&extinfo->addr, buffer));

				iw_printf("\t\t     Algorithm:");
				iw_print_value_name(extinfo->alg, iw_encode_alg_name,
				IW_ENCODE_ALG_NUM);

				iw_printf("\n\t\t     Flags: 0x%08x\n", extinfo->ext_flags);
				if (extinfo->ext_flags & IW_ENCODE_EXT_TX_SEQ_VALID)
					iw_printf("\t\t        tx-seq-valid\n");
				if (extinfo->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID)
					iw_printf("\t\t        rx-seq-valid\n");
				if (extinfo->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
					iw_printf("\t\t        group-key\n");
			}
		}
		/* Print current key index and mode */
		wrq.u.data.pointer = (caddr_t) extbuf;
		wrq.u.data.length = IW_EXTKEY_SIZE;
		wrq.u.data.flags = 0; /* Set index to zero to get current */
		if (iw_get_ext(skfd, ifname, SIOCGIWENCODEEXT, &wrq) >= 0)
		{
			/* Note : if above fails, we have already printed an error
			 * message int the loop above */
			iw_printf("          Current Transmit Key: [%d]\n",
					wrq.u.data.flags & IW_ENCODE_INDEX);
			if (wrq.u.data.flags & IW_ENCODE_RESTRICTED)
				iw_printf("          Security mode:restricted\n");
			if (wrq.u.data.flags & IW_ENCODE_OPEN)
				iw_printf("          Security mode:open\n");
		}

		iw_printf("\n\n");
	}
	return (0);
}

/*------------------------------------------------------------------*/
/*
 * Print the Generic IE for the device
 * Note : indentation is broken. We need to fix that.
 */
static int print_gen_ie_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	ospl_uint8 buf[IW_GENERIC_IE_MAX];

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	wrq.u.data.pointer = (caddr_t) buf;
	wrq.u.data.length = IW_GENERIC_IE_MAX;
	wrq.u.data.flags = 0;

	if (iw_get_ext(skfd, ifname, SIOCGIWGENIE, &wrq) < 0)
		fprintf(stderr, "%-8.16s  no generic IE (%s).\n\n", ifname,
				strerror(errno));
	else
	{
		fprintf(stderr, "%-8.16s\n", kname2ifname(ifname));
		if (wrq.u.data.length == 0)
			iw_printf("          empty generic IE\n");
		else
			iw_print_gen_ie(buf, wrq.u.data.length);
		iw_printf("\n");
	}
	return (0);
}

/**************************** MODULATION ****************************/

/*------------------------------------------------------------------*/
/*
 * Print Modulation info for each device
 */
static int print_modul_info(int skfd, char * ifname, char * args[], /* Command line args */
int count, iw_user_cb_t *cb) /* Args count */
{
	struct iwreq wrq;
	struct iw_range range;

	/* Avoid "Unused parameter" warning */
	args = args;
	count = count;

	/* Extract range info */
	if ((iw_get_range_info(skfd, ifname, &range) < 0)
			|| (range.we_version_compiled < 11))
		fprintf(stderr, "%-8.16s  no modulation information.\n\n", kname2ifname(ifname));
	else
	{
		if (range.modul_capa == 0x0)
			iw_printf("%-8.16s  unknown modulation information.\n\n", kname2ifname(ifname));
		else
		{
			int i;
			iw_printf("%-8.16s  Modulations available :\n", kname2ifname(ifname));

			/* Display each modulation available */
			for (i = 0; i < IW_SIZE_MODUL_LIST; i++)
			{
				if ((range.modul_capa & iw_modul_list[i].mask)
						== iw_modul_list[i].mask)
					iw_printf("              %-8s: %s\n", iw_modul_list[i].cmd,
							iw_modul_list[i].verbose);
			}

			/* Get current modulations settings */
			wrq.u.param.flags = 0;
			if (iw_get_ext(skfd, ifname, SIOCGIWMODUL, &wrq) >= 0)
			{
				ospl_uint32 modul = wrq.u.param.value;
				int n = 0;

				iw_printf("          Current modulations %c",
						wrq.u.param.fixed ? '=' : ':');

				/* Display each modulation enabled */
				for (i = 0; i < IW_SIZE_MODUL_LIST; i++)
				{
					if ((modul & iw_modul_list[i].mask)
							== iw_modul_list[i].mask)
					{
						if ((n++ % 8) == 0)
							iw_printf("\n              ");
						else
							iw_printf(" ; ");
						iw_printf("%s", iw_modul_list[i].cmd);
					}
				}

				iw_printf("\n");
			}
			iw_printf("\n");
		}
	}
	return (0);
}
#endif	/* WE_ESSENTIAL */

/************************* COMMON UTILITIES *************************/
/*
 * This section was initially written by Michael Tokarev <mjt@tls.msk.ru>
 * but heavily modified by me ;-)
 */

/*------------------------------------------------------------------*/
/*
 * Map command line arguments to the proper procedure...
 */
typedef struct iwlist_entry
{
	const char * cmd; /* Command line ospl_int16hand */
	iw_enum_handler fn; /* Subroutine */
	int max_count;
	const char * argsname; /* Args as human readable string */
} iwlist_cmd;

static const struct iwlist_entry iwlist_cmds[] =
{
	{ "scanning", print_scanning_info, -1, "[essid NNN] [last]" },
	{ "frequency", print_freq_info, 0, NULL },
	{ "channel", print_freq_info, 0, NULL },
	{ "bitrate", print_bitrate_info, 0, NULL },
	{ "rate", print_bitrate_info, 0, NULL },
	{ "encryption", print_keys_info, 0, NULL },
	{ "keys", print_keys_info, 0, NULL },
	{ "power", print_pm_info, 0, NULL },
	{ "txpower", print_txpower_info, 0, NULL },
#ifndef WE_ESSENTIAL
	{ "retry", print_retry_info, 0, NULL },
	{ "ap", print_ap_info, 0, NULL },
	{ "accesspoints", print_ap_info, 0, NULL },
	{ "peers", print_ap_info, 0, NULL },
	{ "event", print_event_capa_info, 0, NULL },
	{ "auth", print_auth_info, 0, NULL },
	{ "wpakeys", print_wpakeys_info, 0, NULL },
	{ "genie", print_gen_ie_info, 0, NULL },
	{ "modulation", print_modul_info, 0, NULL },
#endif	/* WE_ESSENTIAL */
	{ NULL, NULL, 0, 0 },
};

/*------------------------------------------------------------------*/
/*
 * Find the most appropriate command matching the command line
 */
static inline const iwlist_cmd *
find_command(const char * cmd)
{
	const iwlist_cmd * found = NULL;
	int ambig = 0;
	ospl_uint32 len = strlen(cmd);
	int i;

	/* Go through all commands */
	for (i = 0; iwlist_cmds[i].cmd != NULL; ++i)
	{
		/* No match -> next one */
		if (strncasecmp(iwlist_cmds[i].cmd, cmd, len) != 0)
			continue;

		/* Exact match -> perfect */
		if (len == strlen(iwlist_cmds[i].cmd))
			return &iwlist_cmds[i];

		/* Partial match */
		if (found == NULL)
			/* First time */
			found = &iwlist_cmds[i];
		else
		/* Another time */
		if (iwlist_cmds[i].fn != found->fn)
			ambig = 1;
	}

	if (found == NULL)
	{
		fprintf(stderr,
				"iwlist: unknown command `%s' (check 'iwlist --help').\n", cmd);
		return NULL;
	}

	if (ambig)
	{
		fprintf(stderr,
				"iwlist: command `%s' is ambiguous (check 'iwlist --help').\n",
				cmd);
		return NULL;
	}

	return found;
}

/*------------------------------------------------------------------*/
/*
 * Display help
 */
static void iw_usage(int status)
{
	FILE * f = status ? stderr : stdout;
	int i;

	for (i = 0; iwlist_cmds[i].cmd != NULL; ++i)
	{
		fprintf(f, "%s [interface] %s %s\n",
				(i ? "             " : "Usage: iwlist"), iwlist_cmds[i].cmd,
				iwlist_cmds[i].argsname ? iwlist_cmds[i].argsname : "");
	}

	exit(status);
}

/******************************* MAIN ********************************/

/*------------------------------------------------------------------*/
/*
 * The main !
 */
int iwlist_main(iw_user_cb_t *cb, int argc, char ** argv)
{
	int skfd; /* generic raw socket desc.	*/
	char *dev; /* device name			*/
	char *cmd; /* command			*/
	char **args; /* Command arguments */
	int count; /* Number of arguments */
	const iwlist_cmd *iwcmd;
	int goterr = 0;

	if (argc < 2)
		iw_usage(1);

	/* Those don't apply to all interfaces */
	if ((argc == 2) && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
		iw_usage(0);
	if ((argc == 2)
			&& (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")))
		return (iw_print_version_info("iwlist"));

	if (argc == 2)
	{
		cmd = argv[1];
		dev = NULL;
		args = NULL;
		count = 0;
	}
	else
	{
		cmd = argv[2];
		dev = argv[1];
		args = argv + 3;
		count = argc - 3;
	}

	/* find a command */
	iwcmd = find_command(cmd);
	if (iwcmd == NULL)
		return 1;

	/* Check arg numbers */
	if ((iwcmd->max_count >= 0) && (count > iwcmd->max_count))
	{
		fprintf(stderr, "iwlist: command `%s' needs fewer arguments (max %d)\n",
				iwcmd->cmd, iwcmd->max_count);
		return 1;
	}

	/* Create a channel to the NET kernel. */
	if ((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}

	/* do the actual work */
	if (dev)
		goterr = (*iwcmd->fn)(skfd, dev, args, count, cb);
	else
		iw_enum_devices(skfd, iwcmd->fn, args, count, cb);

	/* Close the socket. */
	iw_sockets_close(skfd);

	return goterr;
}
