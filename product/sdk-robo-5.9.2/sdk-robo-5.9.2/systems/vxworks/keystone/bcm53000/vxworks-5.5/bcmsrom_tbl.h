/*
 * Table that encodes the srom formats for PCI/PCIe NICs.
 *
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * $Id: bcmsrom_tbl.h,v 1.2 Broadcom SDK $
 */

typedef struct {
	const char *name;
	uint32	revmask;
	uint32	flags;
	uint16	off;
	uint16	mask;
} sromvar_t;

#define SRFL_MORE	1		/* value continues as described by the next entry */
#define	SRFL_NOFFS	2		/* value bits can't be all one's */
#define	SRFL_PRHEX	4		/* value is in hexdecimal format */
#define	SRFL_PRSIGN	8		/* value is in signed decimal format */
#define	SRFL_CCODE	0x10		/* value is in country code format */
#define	SRFL_ETHADDR	0x20		/* value is an Ethernet address */
#define SRFL_LEDDC	0x40		/* value is an LED duty cycle */

/* Assumptions:
 * - Ethernet address spans across 3 consective words
 *
 * Table rules:
 * - Add multiple entries next to each other if a value spans across multiple words
 *   (even multiple fields in the same word) with each entry except the last having
 *   it's SRFL_MORE bit set.
 * - Ethernet address entry does not follow above rule and must not have SRFL_MORE
 *   bit set. Its SRFL_ETHADDR bit implies it takes multiple words.
 * - The last entry's name field must be NULL to indicate the end of the table. Other
 *   entries must have non-NULL name.
 */

static const sromvar_t pci_sromvars[] = {
	{"boardrev",	0x0000000e,	SRFL_PRHEX,	SROM_AABREV,		SROM_BR_MASK},
	{"boardrev",	0x000000f0,	SRFL_PRHEX,	SROM4_BREV,		0xffff},
	{"boardrev",	0xffffff00,	SRFL_PRHEX,	SROM8_BREV,		0xffff},
	{"boardflags",	0x00000002,	SRFL_PRHEX,	SROM_BFL,		0xffff},
	{"boardflags",	0x00000004,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL,	0xffff},
	{"",		0,		0,		SROM_BFL2,		0xffff},
	{"boardflags",	0x00000008,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL,	0xffff},
	{"",		0,		0,		SROM3_BFL2,		0xffff},
	{"boardflags",	0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL0,	0xffff},
	{"",		0,		0,		SROM4_BFL1,		0xffff},
	{"boardflags",	0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL0,	0xffff},
	{"",		0,		0,		SROM5_BFL1,		0xffff},
	{"boardflags",	0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL0,	0xffff},
	{"",		0,		0,		SROM8_BFL1,		0xffff},
	{"boardflags2", 0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL2,	0xffff},
	{"",		0,		0,		SROM4_BFL3,		0xffff},
	{"boardflags2", 0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL2,	0xffff},
	{"",		0,		0,		SROM5_BFL3,		0xffff},
	{"boardflags2", 0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL2,	0xffff},
	{"",		0,		0,		SROM8_BFL3,		0xffff},
	{"boardtype",	0xfffffffc,	SRFL_PRHEX,	SROM_SSID,		0xffff},
	{"boardnum",	0x00000006,	0,		SROM_MACLO_IL0,		0xffff},
	{"boardnum",	0x00000008,	0,		SROM3_MACLO,		0xffff},
	{"boardnum",	0x00000010,	0,		SROM4_MACLO,		0xffff},
	{"boardnum",	0x000000e0,	0,		SROM5_MACLO,		0xffff},
	{"boardnum",	0xffffff00,	0,		SROM8_MACLO,		0xffff},
	{"cc",		0x00000002,	0,		SROM_AABREV,		SROM_CC_MASK},
	{"regrev",	0x00000008,	0,		SROM_OPO,		0xff00},
	{"regrev",	0x00000010,	0,		SROM4_REGREV,		0x00ff},
	{"regrev",	0x000000e0,	0,		SROM5_REGREV,		0x00ff},
	{"regrev",	0xffffff00,	0,		SROM8_REGREV,		0x00ff},
	{"ledbh0",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10,		0x00ff},
	{"ledbh1",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10,		0xff00},
	{"ledbh2",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32,		0x00ff},
	{"ledbh3",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32,		0xff00},
	{"ledbh0",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10,		0x00ff},
	{"ledbh1",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10,		0xff00},
	{"ledbh2",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32,		0x00ff},
	{"ledbh3",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32,		0xff00},
	{"ledbh0",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10,		0x00ff},
	{"ledbh1",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10,		0xff00},
	{"ledbh2",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32,		0x00ff},
	{"ledbh3",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32,		0xff00},
	{"ledbh0",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH10,		0x00ff},
	{"ledbh1",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH10,		0xff00},
	{"ledbh2",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH32,		0x00ff},
	{"ledbh3",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH32,		0xff00},
	{"pa0b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB0,		0xffff},
	{"pa0b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB1,		0xffff},
	{"pa0b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB2,		0xffff},
	{"pa0itssit",	0x0000000e,	0,		SROM_ITT,		0x00ff},
	{"pa0maxpwr",	0x0000000e,	0,		SROM_WL10MAXP,		0x00ff},
	{"pa0b0",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB0,		0xffff},
	{"pa0b1",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB1,		0xffff},
	{"pa0b2",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB2,		0xffff},
	{"pa0itssit",	0xffffff00,	0,		SROM8_W0_ITTMAXP,	0xff00},
	{"pa0maxpwr",	0xffffff00,	0,		SROM8_W0_ITTMAXP,	0x00ff},
	{"opo",		0x0000000c,	0,		SROM_OPO,		0x00ff},
	{"opo",		0xffffff00,	0,		SROM8_2G_OFDMPO,	0x00ff},
	{"aa2g",	0x0000000e,	0,		SROM_AABREV,		SROM_AA0_MASK},
	{"aa2g",	0x000000f0,	0,		SROM4_AA,		0x00ff},
	{"aa2g",	0xffffff00,	0,		SROM8_AA,		0x00ff},
	{"aa5g",	0x0000000e,	0,		SROM_AABREV,		SROM_AA1_MASK},
	{"aa5g",	0x000000f0,	0,		SROM4_AA,		0xff00},
	{"aa5g",	0xffffff00,	0,		SROM8_AA,		0xff00},
	{"ag0",		0x0000000e,	0,		SROM_AG10,		0x00ff},
	{"ag1",		0x0000000e,	0,		SROM_AG10,		0xff00},
	{"ag0",		0x000000f0,	0,		SROM4_AG10,		0x00ff},
	{"ag1",		0x000000f0,	0,		SROM4_AG10,		0xff00},
	{"ag2",		0x000000f0,	0,		SROM4_AG32,		0x00ff},
	{"ag3",		0x000000f0,	0,		SROM4_AG32,		0xff00},
	{"ag0",		0xffffff00,	0,		SROM8_AG10,		0x00ff},
	{"ag1",		0xffffff00,	0,		SROM8_AG10,		0xff00},
	{"ag2",		0xffffff00,	0,		SROM8_AG32,		0x00ff},
	{"ag3",		0xffffff00,	0,		SROM8_AG32,		0xff00},
	{"pa1b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB0,		0xffff},
	{"pa1b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB1,		0xffff},
	{"pa1b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB2,		0xffff},
	{"pa1lob0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB0,		0xffff},
	{"pa1lob1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB1,		0xffff},
	{"pa1lob2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB2,		0xffff},
	{"pa1hib0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB0,		0xffff},
	{"pa1hib1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB1,		0xffff},
	{"pa1hib2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB2,		0xffff},
	{"pa1itssit",	0x0000000e,	0,		SROM_ITT,		0xff00},
	{"pa1maxpwr",	0x0000000e,	0,		SROM_WL10MAXP,		0xff00},
	{"pa1lomaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP,		0xff00},
	{"pa1himaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP,		0x00ff},
	{"pa1b0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0,		0xffff},
	{"pa1b1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1,		0xffff},
	{"pa1b2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2,		0xffff},
	{"pa1lob0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0_LC,	0xffff},
	{"pa1lob1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1_LC,	0xffff},
	{"pa1lob2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2_LC,	0xffff},
	{"pa1hib0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0_HC,	0xffff},
	{"pa1hib1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1_HC,	0xffff},
	{"pa1hib2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2_HC,	0xffff},
	{"pa1itssit",	0xffffff00,	0,		SROM8_W1_ITTMAXP,	0xff00},
	{"pa1maxpwr",	0xffffff00,	0,		SROM8_W1_ITTMAXP,	0x00ff},
	{"pa1lomaxpwr",	0xffffff00,	0,		SROM8_W1_MAXP_LCHC,	0xff00},
	{"pa1himaxpwr",	0xffffff00,	0,		SROM8_W1_MAXP_LCHC,	0x00ff},
	{"bxa2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x1800},
	{"rssisav2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x0700},
	{"rssismc2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x00f0},
	{"rssismf2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x000f},
	{"bxa2g",	0xffffff00,	0,		SROM8_BXARSSI2G,	0x1800},
	{"rssisav2g",	0xffffff00,	0,		SROM8_BXARSSI2G,	0x0700},
	{"rssismc2g",	0xffffff00,	0,		SROM8_BXARSSI2G,	0x00f0},
	{"rssismf2g",	0xffffff00,	0,		SROM8_BXARSSI2G,	0x000f},
	{"bxa5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x1800},
	{"rssisav5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x0700},
	{"rssismc5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x00f0},
	{"rssismf5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x000f},
	{"bxa5g",	0xffffff00,	0,		SROM8_BXARSSI5G,	0x1800},
	{"rssisav5g",	0xffffff00,	0,		SROM8_BXARSSI5G,	0x0700},
	{"rssismc5g",	0xffffff00,	0,		SROM8_BXARSSI5G,	0x00f0},
	{"rssismf5g",	0xffffff00,	0,		SROM8_BXARSSI5G,	0x000f},
	{"tri2g",	0x00000008,	0,		SROM_TRI52G,		0x00ff},
	{"tri5g",	0x00000008,	0,		SROM_TRI52G,		0xff00},
	{"tri5gl",	0x00000008,	0,		SROM_TRI5GHL,		0x00ff},
	{"tri5gh",	0x00000008,	0,		SROM_TRI5GHL,		0xff00},
	{"tri2g",	0xffffff00,	0,		SROM8_TRI52G,		0x00ff},
	{"tri5g",	0xffffff00,	0,		SROM8_TRI52G,		0xff00},
	{"tri5gl",	0xffffff00,	0,		SROM8_TRI5GHL,		0x00ff},
	{"tri5gh",	0xffffff00,	0,		SROM8_TRI5GHL,		0xff00},
	{"rxpo2g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G,		0x00ff},
	{"rxpo5g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G,		0xff00},
	{"rxpo2g",	0xffffff00,	SRFL_PRSIGN,	SROM8_RXPO52G,		0x00ff},
	{"rxpo5g",	0xffffff00,	SRFL_PRSIGN,	SROM8_RXPO52G,		0xff00},
	{"txchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_TXCHAIN_MASK},
	{"rxchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_RXCHAIN_MASK},
	{"antswitch",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_SWITCH_MASK},
	{"txchain",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_TXCHAIN_MASK},
	{"rxchain",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_RXCHAIN_MASK},
	{"antswitch",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_SWITCH_MASK},
	{"tssipos2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_ANTSWLUT_MASK},
	{"tssipos5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_ANTSWLUT_MASK},
	{"txpid2ga0",	0x000000f0,	0,		SROM4_TXPID2G,		0x00ff},
	{"txpid2ga1",	0x000000f0,	0,		SROM4_TXPID2G,		0xff00},
	{"txpid2ga2",	0x000000f0,	0,		SROM4_TXPID2G + 1,	0x00ff},
	{"txpid2ga3",	0x000000f0,	0,		SROM4_TXPID2G + 1,	0xff00},
	{"txpid5ga0",	0x000000f0,	0,		SROM4_TXPID5G,		0x00ff},
	{"txpid5ga1",	0x000000f0,	0,		SROM4_TXPID5G,		0xff00},
	{"txpid5ga2",	0x000000f0,	0,		SROM4_TXPID5G + 1,	0x00ff},
	{"txpid5ga3",	0x000000f0,	0,		SROM4_TXPID5G + 1,	0xff00},
	{"txpid5gla0",	0x000000f0,	0,		SROM4_TXPID5GL,		0x00ff},
	{"txpid5gla1",	0x000000f0,	0,		SROM4_TXPID5GL,		0xff00},
	{"txpid5gla2",	0x000000f0,	0,		SROM4_TXPID5GL + 1,	0x00ff},
	{"txpid5gla3",	0x000000f0,	0,		SROM4_TXPID5GL + 1,	0xff00},
	{"txpid5gha0",	0x000000f0,	0,		SROM4_TXPID5GH,		0x00ff},
	{"txpid5gha1",	0x000000f0,	0,		SROM4_TXPID5GH,		0xff00},
	{"txpid5gha2",	0x000000f0,	0,		SROM4_TXPID5GH + 1,	0x00ff},
	{"txpid5gha3",	0x000000f0,	0,		SROM4_TXPID5GH + 1,	0xff00},
	{"cck2gpo",	0x000000f0,	0,		SROM4_2G_CCKPO,		0xffff},
	{"cck2gpo",	0xffffff00,	0,		SROM8_2G_CCKPO,		0xffff},
	{"ofdm2gpo",	0x000000f0,	SRFL_MORE,	SROM4_2G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_2G_OFDMPO + 1,	0xffff},
	{"ofdm5gpo",	0x000000f0,	SRFL_MORE,	SROM4_5G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5G_OFDMPO + 1,	0xffff},
	{"ofdm5glpo",	0x000000f0,	SRFL_MORE,	SROM4_5GL_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5GL_OFDMPO + 1,	0xffff},
	{"ofdm5ghpo",	0x000000f0,	SRFL_MORE,	SROM4_5GH_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5GH_OFDMPO + 1,	0xffff},
	{"ofdm2gpo",	0xffffff00,	SRFL_MORE,	SROM8_2G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_2G_OFDMPO + 1,	0xffff},
	{"ofdm5gpo",	0xffffff00,	SRFL_MORE,	SROM8_5G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5G_OFDMPO + 1,	0xffff},
	{"ofdm5glpo",	0xffffff00,	SRFL_MORE,	SROM8_5GL_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5GL_OFDMPO + 1,	0xffff},
	{"ofdm5ghpo",	0xffffff00,	SRFL_MORE,	SROM8_5GH_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5GH_OFDMPO + 1,	0xffff},
	{"mcs2gpo0",	0x000000f0,	0,		SROM4_2G_MCSPO,		0xffff},
	{"mcs2gpo1",	0x000000f0,	0,		SROM4_2G_MCSPO + 1,	0xffff},
	{"mcs2gpo2",	0x000000f0,	0,		SROM4_2G_MCSPO + 2,	0xffff},
	{"mcs2gpo3",	0x000000f0,	0,		SROM4_2G_MCSPO + 3,	0xffff},
	{"mcs2gpo4",	0x000000f0,	0,		SROM4_2G_MCSPO + 4,	0xffff},
	{"mcs2gpo5",	0x000000f0,	0,		SROM4_2G_MCSPO + 5,	0xffff},
	{"mcs2gpo6",	0x000000f0,	0,		SROM4_2G_MCSPO + 6,	0xffff},
	{"mcs2gpo7",	0x000000f0,	0,		SROM4_2G_MCSPO + 7,	0xffff},
	{"mcs5gpo0",	0x000000f0,	0,		SROM4_5G_MCSPO,		0xffff},
	{"mcs5gpo1",	0x000000f0,	0,		SROM4_5G_MCSPO + 1,	0xffff},
	{"mcs5gpo2",	0x000000f0,	0,		SROM4_5G_MCSPO + 2,	0xffff},
	{"mcs5gpo3",	0x000000f0,	0,		SROM4_5G_MCSPO + 3,	0xffff},
	{"mcs5gpo4",	0x000000f0,	0,		SROM4_5G_MCSPO + 4,	0xffff},
	{"mcs5gpo5",	0x000000f0,	0,		SROM4_5G_MCSPO + 5,	0xffff},
	{"mcs5gpo6",	0x000000f0,	0,		SROM4_5G_MCSPO + 6,	0xffff},
	{"mcs5gpo7",	0x000000f0,	0,		SROM4_5G_MCSPO + 7,	0xffff},
	{"mcs5glpo0",	0x000000f0,	0,		SROM4_5GL_MCSPO,	0xffff},
	{"mcs5glpo1",	0x000000f0,	0,		SROM4_5GL_MCSPO + 1,	0xffff},
	{"mcs5glpo2",	0x000000f0,	0,		SROM4_5GL_MCSPO + 2,	0xffff},
	{"mcs5glpo3",	0x000000f0,	0,		SROM4_5GL_MCSPO + 3,	0xffff},
	{"mcs5glpo4",	0x000000f0,	0,		SROM4_5GL_MCSPO + 4,	0xffff},
	{"mcs5glpo5",	0x000000f0,	0,		SROM4_5GL_MCSPO + 5,	0xffff},
	{"mcs5glpo6",	0x000000f0,	0,		SROM4_5GL_MCSPO + 6,	0xffff},
	{"mcs5glpo7",	0x000000f0,	0,		SROM4_5GL_MCSPO + 7,	0xffff},
	{"mcs5ghpo0",	0x000000f0,	0,		SROM4_5GH_MCSPO,	0xffff},
	{"mcs5ghpo1",	0x000000f0,	0,		SROM4_5GH_MCSPO + 1,	0xffff},
	{"mcs5ghpo2",	0x000000f0,	0,		SROM4_5GH_MCSPO + 2,	0xffff},
	{"mcs5ghpo3",	0x000000f0,	0,		SROM4_5GH_MCSPO + 3,	0xffff},
	{"mcs5ghpo4",	0x000000f0,	0,		SROM4_5GH_MCSPO + 4,	0xffff},
	{"mcs5ghpo5",	0x000000f0,	0,		SROM4_5GH_MCSPO + 5,	0xffff},
	{"mcs5ghpo6",	0x000000f0,	0,		SROM4_5GH_MCSPO + 6,	0xffff},
	{"mcs5ghpo7",	0x000000f0,	0,		SROM4_5GH_MCSPO + 7,	0xffff},
	{"mcs2gpo0",	0xffffff00,	0,		SROM8_2G_MCSPO,		0xffff},
	{"mcs2gpo1",	0xffffff00,	0,		SROM8_2G_MCSPO + 1,	0xffff},
	{"mcs2gpo2",	0xffffff00,	0,		SROM8_2G_MCSPO + 2,	0xffff},
	{"mcs2gpo3",	0xffffff00,	0,		SROM8_2G_MCSPO + 3,	0xffff},
	{"mcs2gpo4",	0xffffff00,	0,		SROM8_2G_MCSPO + 4,	0xffff},
	{"mcs2gpo5",	0xffffff00,	0,		SROM8_2G_MCSPO + 5,	0xffff},
	{"mcs2gpo6",	0xffffff00,	0,		SROM8_2G_MCSPO + 6,	0xffff},
	{"mcs2gpo7",	0xffffff00,	0,		SROM8_2G_MCSPO + 7,	0xffff},
	{"mcs5gpo0",	0xffffff00,	0,		SROM8_5G_MCSPO,		0xffff},
	{"mcs5gpo1",	0xffffff00,	0,		SROM8_5G_MCSPO + 1,	0xffff},
	{"mcs5gpo2",	0xffffff00,	0,		SROM8_5G_MCSPO + 2,	0xffff},
	{"mcs5gpo3",	0xffffff00,	0,		SROM8_5G_MCSPO + 3,	0xffff},
	{"mcs5gpo4",	0xffffff00,	0,		SROM8_5G_MCSPO + 4,	0xffff},
	{"mcs5gpo5",	0xffffff00,	0,		SROM8_5G_MCSPO + 5,	0xffff},
	{"mcs5gpo6",	0xffffff00,	0,		SROM8_5G_MCSPO + 6,	0xffff},
	{"mcs5gpo7",	0xffffff00,	0,		SROM8_5G_MCSPO + 7,	0xffff},
	{"mcs5glpo0",	0xffffff00,	0,		SROM8_5GL_MCSPO,	0xffff},
	{"mcs5glpo1",	0xffffff00,	0,		SROM8_5GL_MCSPO + 1,	0xffff},
	{"mcs5glpo2",	0xffffff00,	0,		SROM8_5GL_MCSPO + 2,	0xffff},
	{"mcs5glpo3",	0xffffff00,	0,		SROM8_5GL_MCSPO + 3,	0xffff},
	{"mcs5glpo4",	0xffffff00,	0,		SROM8_5GL_MCSPO + 4,	0xffff},
	{"mcs5glpo5",	0xffffff00,	0,		SROM8_5GL_MCSPO + 5,	0xffff},
	{"mcs5glpo6",	0xffffff00,	0,		SROM8_5GL_MCSPO + 6,	0xffff},
	{"mcs5glpo7",	0xffffff00,	0,		SROM8_5GL_MCSPO + 7,	0xffff},
	{"mcs5ghpo0",	0xffffff00,	0,		SROM8_5GH_MCSPO,	0xffff},
	{"mcs5ghpo1",	0xffffff00,	0,		SROM8_5GH_MCSPO + 1,	0xffff},
	{"mcs5ghpo2",	0xffffff00,	0,		SROM8_5GH_MCSPO + 2,	0xffff},
	{"mcs5ghpo3",	0xffffff00,	0,		SROM8_5GH_MCSPO + 3,	0xffff},
	{"mcs5ghpo4",	0xffffff00,	0,		SROM8_5GH_MCSPO + 4,	0xffff},
	{"mcs5ghpo5",	0xffffff00,	0,		SROM8_5GH_MCSPO + 5,	0xffff},
	{"mcs5ghpo6",	0xffffff00,	0,		SROM8_5GH_MCSPO + 6,	0xffff},
	{"mcs5ghpo7",	0xffffff00,	0,		SROM8_5GH_MCSPO + 7,	0xffff},
	{"cddpo",	0x000000f0,	0,		SROM4_CDDPO,		0xffff},
	{"stbcpo",	0x000000f0,	0,		SROM4_STBCPO,		0xffff},
	{"bw40po",	0x000000f0,	0,		SROM4_BW40PO,		0xffff},
	{"bwduppo",	0x000000f0,	0,		SROM4_BWDUPPO,		0xffff},
	{"cddpo",	0xffffff00,	0,		SROM8_CDDPO,		0xffff},
	{"stbcpo",	0xffffff00,	0,		SROM8_STBCPO,		0xffff},
	{"bw40po",	0xffffff00,	0,		SROM8_BW40PO,		0xffff},
	{"bwduppo",	0xffffff00,	0,		SROM8_BWDUPPO,		0xffff},
	{"ccode",	0x0000000f,	SRFL_CCODE,	SROM_CCODE,		0xffff},
	{"ccode",	0x00000010,	SRFL_CCODE,	SROM4_CCODE,		0xffff},
	{"ccode",	0x000000e0,	SRFL_CCODE,	SROM5_CCODE,		0xffff},
	{"ccode",	0xffffff00,	SRFL_CCODE,	SROM8_CCODE,		0xffff},
	{"macaddr",	0xffffff00,	SRFL_ETHADDR,	SROM8_MACHI,		0xffff},
	{"macaddr",	0x000000e0,	SRFL_ETHADDR,	SROM5_MACHI,		0xffff},
	{"macaddr",	0x00000010,	SRFL_ETHADDR,	SROM4_MACHI,		0xffff},
	{"macaddr",	0x00000008,	SRFL_ETHADDR,	SROM3_MACHI,		0xffff},
	{"il0macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_IL0,		0xffff},
	{"et1macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_ET1,		0xffff},
	{"leddc",	0xffffff00,	SRFL_NOFFS|SRFL_LEDDC,	SROM8_LEDDC,	0xffff},
	{"leddc",	0x000000e0,	SRFL_NOFFS|SRFL_LEDDC,	SROM5_LEDDC,	0xffff},
	{"leddc",	0x00000010,	SRFL_NOFFS|SRFL_LEDDC,	SROM4_LEDDC,	0xffff},
	{"leddc",	0x00000008,	SRFL_NOFFS|SRFL_LEDDC,	SROM3_LEDDC,	0xffff},
	{NULL,		0,		0,		0,			0}
};

static const sromvar_t perpath_pci_sromvars[] = {
	{"maxp2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP,	0x00ff},
	{"itt2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP,	0xff00},
	{"itt5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP,	0xff00},
	{"pa2gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA,		0xffff},
	{"pa2gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 1,	0xffff},
	{"pa2gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 2,	0xffff},
	{"pa2gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 3,	0xffff},
	{"maxp5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP,	0x00ff},
	{"maxp5gha",	0x000000f0,	0,		SROM4_5GLH_MAXP,	0x00ff},
	{"maxp5gla",	0x000000f0,	0,		SROM4_5GLH_MAXP,	0xff00},
	{"pa5gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA,		0xffff},
	{"pa5gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 1,	0xffff},
	{"pa5gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 2,	0xffff},
	{"pa5gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 3,	0xffff},
	{"pa5glw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA,		0xffff},
	{"pa5glw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 1,	0xffff},
	{"pa5glw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 2,	0xffff},
	{"pa5glw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 3,	0xffff},
	{"pa5ghw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA,		0xffff},
	{"pa5ghw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 1,	0xffff},
	{"pa5ghw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 2,	0xffff},
	{"pa5ghw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 3,	0xffff},
	{"maxp2ga",	0xffffff00,	0,		SROM8_2G_ITT_MAXP,	0x00ff},
	{"itt2ga",	0xffffff00,	0,		SROM8_2G_ITT_MAXP,	0xff00},
	{"itt5ga",	0xffffff00,	0,		SROM8_5G_ITT_MAXP,	0xff00},
	{"pa2gw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA,		0xffff},
	{"pa2gw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA + 1,	0xffff},
	{"pa2gw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA + 2,	0xffff},
	{"maxp5ga",	0xffffff00,	0,		SROM8_5G_ITT_MAXP,	0x00ff},
	{"maxp5gha",	0xffffff00,	0,		SROM8_5GLH_MAXP,	0x00ff},
	{"maxp5gla",	0xffffff00,	0,		SROM8_5GLH_MAXP,	0xff00},
	{"pa5gw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA,		0xffff},
	{"pa5gw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA + 1,	0xffff},
	{"pa5gw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA + 2,	0xffff},
	{"pa5glw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA,		0xffff},
	{"pa5glw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA + 1,	0xffff},
	{"pa5glw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA + 2,	0xffff},
	{"pa5ghw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA,		0xffff},
	{"pa5ghw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA + 1,	0xffff},
	{"pa5ghw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA + 2,	0xffff},
	{NULL,		0,		0,		0, 			0}
};
