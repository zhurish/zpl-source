/*
 * $Id: eth_drv.c,v 1.10 Broadcom SDK $
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
 */
#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "sdk_netpkt.h"

#define ETH_DRV_MAX_BUF_LEN     2048

#include <sys/socket.h>
#include <sys/types.h>
#include <features.h>    /* for the glibc version number */
#include <net/ethernet.h>     /* the L2 protocols */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <net/if.h>


typedef struct eth_unit_info_s {
    int         sock_fd;

    int         inited;
    rx_cb       rx_cb;
    int         started;
#ifdef NETLINK_TEST
    struct sockaddr_nl nl; 
    int         pid;
#else
   struct      sockaddr_ll sl;  
   struct ifreq ifr;
#endif
} eth_unit_info_t;

#define ETH_MAX_UNITS   1
static eth_unit_info_t  *eth_control[ETH_MAX_UNITS];
static void _eth_drv_handle_rx(int unit, void *pkt, int pkt_len);
static int _eth_drv_tx(int unit, unsigned char *tx_pkt, int pkt_len);


int brcm_ingress_hdr_getandshow(unsigned char *brcm_tag, struct brcm_tag_header *tagheader)
{
    tagheader->ingress = 1;
    tagheader->opcode = ((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK);
    if(BRCM_OPCODE_TYPE1 == tagheader->opcode)
    {
        uint32_t *timestamp = ( uint32_t *)&brcm_tag[4];
        tagheader->header.ingress.TR = ((brcm_tag[3]>>5) & 0x01);
        tagheader->header.ingress.TR_PID = (brcm_tag[3] & BRCM_INGRESS_PORT_MASK);
        tagheader->header.ingress.timestamp = ntohl(*timestamp);
    }
    else if(BRCM_OPCODE_TYPE0 == tagheader->opcode)
    {
        tagheader->header.ingress.reason = (brcm_tag[2]);
        tagheader->header.ingress.TC = ((brcm_tag[3]>>5) & 0x07);
        tagheader->header.ingress.port = (brcm_tag[3] & BRCM_INGRESS_PORT_MASK);
    }
    zlog_debug(MODULE_HAL, "brcm hdr brcm_tag:0x%02x 0x%02x 0x%02x 0x%02x", 
            brcm_tag[0], brcm_tag[1], brcm_tag[2], brcm_tag[3]);

    if(tagheader->opcode == BRCM_OPCODE_TYPE0)
    {
        zlog_debug(MODULE_HAL, "brcm hdr opcode=%d TC=%d port=%d reason=%d", 
            tagheader->opcode,  tagheader->header.ingress.TC, 
            tagheader->header.ingress.port, tagheader->header.ingress.reason);
    }
    else if(tagheader->opcode == BRCM_OPCODE_TYPE1)
    {
        zlog_debug(MODULE_HAL, "brcm hdr opcode=%d TR=%d TR_PID=%d timestamp=%d", 
            tagheader->opcode,  tagheader->header.ingress.TR, 
            tagheader->header.ingress.TR_PID, tagheader->header.ingress.timestamp);
    } 
    return OK;
}

int brcm_ingress_hdr_set(unsigned char *brcm_tag, struct brcm_tag_header *tagheader)
{
    brcm_tag[0] = (tagheader->opcode << BRCM_OPCODE_SHIFT);
    brcm_tag[0] |= ((tagheader->header.egress.TC & BRCM_EGRESS_TC_MASK) << BRCM_EGRESS_TC_SHIFT);
    brcm_tag[0] |= ((tagheader->header.egress.TE & BRCM_EGRESS_TE_MASK) << BRCM_EGRESS_TE_SHIFT);
    if(tagheader->header.egress.TS)
        brcm_tag[1] |= 0x80;
    if(tagheader->header.egress.dstport == 8)
        brcm_tag[2] = 1;
    brcm_tag[3] = (1 << tagheader->header.egress.dstport) & 0xff;
    /*
    brcm_tag[0] = (1 << BRCM_OPCODE_SHIFT) |
		       ((queue & BRCM_IG_TC_MASK) << BRCM_IG_TC_SHIFT);
	brcm_tag[1] = 0;
	brcm_tag[2] = 0;
	if (dp->index == 8)
		brcm_tag[2] = BRCM_IG_DSTMAP2_MASK;
	brcm_tag[3] = (1 << dp->index) & BRCM_IG_DSTMAP1_MASK;
    */
    return 0;
}



#ifndef NETLINK_TEST
static void
_eth_drv_rx_thread(void *p)
{
    struct sockaddr sl;
    int    pkt_len = 0;
    uint32_t fromlen = sizeof(struct sockaddr);
    char * rx_pkt = NULL;
    eth_unit_info_t  *unitp = (eth_unit_info_t  *)p;

    if ((rx_pkt = malloc(ETH_DRV_MAX_BUF_LEN)) == NULL) {
        return;
    }
    while (unitp->started) {
        pkt_len = recvfrom(unitp->sock_fd, rx_pkt, 
           ETH_DRV_MAX_BUF_LEN, 0, &sl, &fromlen);
        zlog_debug(MODULE_HAL, "_eth_drv_rx_thread recv: %d bytes :%s", pkt_len, strerror(errno));
        _eth_drv_handle_rx(0, rx_pkt, pkt_len);
    }
}

static int _eth_drv_init(int unit)
{
    eth_unit_info_t     *channel;
    int                  result;

    if (eth_control[unit]) {
        return 0;
    }

    eth_control[unit] = malloc(sizeof(eth_unit_info_t));
    memset(eth_control[unit], 0, sizeof(eth_unit_info_t));
    channel = eth_control[unit];

    channel->sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (channel->sock_fd == 0) {
        goto error;
    }

    sprintf(channel->ifr.ifr_name, "eth%d", unit);
    result = ioctl(channel->sock_fd, SIOCGIFINDEX, &channel->ifr);

    channel->sl.sll_family = AF_PACKET;
    channel->sl.sll_ifindex = channel->ifr.ifr_ifindex;
    channel->sl.sll_protocol = htons(ETH_P_ALL);
    channel->sl.sll_halen = 6;

    if (bind(channel->sock_fd, (struct sockaddr *)&channel->sl, 
         sizeof(struct sockaddr_ll))) {
        goto error;
    }

    eth_control[unit]->inited++;
    return 0;
error:
    free(eth_control[unit]);
    eth_control[unit] = NULL;
    return -1;
}


static int
_eth_drv_tx(int unit, unsigned char *tx_pkt, int pkt_len)
{
    eth_unit_info_t * p_chan = eth_control[unit];

    return (send(p_chan->sock_fd, tx_pkt, pkt_len, 0)
            == pkt_len) ? 0 : -1;
 
}

#else


static int _eth_drv_init(int unit)
{
    eth_unit_info_t     *channel;
    int                  result;

    if (eth_control[unit]) {
        return 0;
    }

    eth_control[unit] = malloc(sizeof(eth_unit_info_t));
    memset(eth_control[unit], 0, sizeof(eth_unit_info_t));
    channel = eth_control[unit];

    channel->sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (channel->sock_fd <= 0) {
        zlog_err(MODULE_PAL, "==================socket(NETLINK_TEST): %s", strerror(errno));
        goto error;
    }

    channel->nl.nl_family = AF_NETLINK;  
    channel->nl.nl_pid = channel->pid = getpid(); // self pid  
    channel->nl.nl_groups = 0; // multi cast  
   
    if (bind(channel->sock_fd, (struct sockaddr *)&channel->nl, 
         sizeof(struct sockaddr_nl))) {
        zlog_err(MODULE_PAL, "===========================bind: %s", strerror(errno));
        goto error;
    }

    eth_control[unit]->inited++;
    return 0;
error:
    free(eth_control[unit]);
    eth_control[unit] = NULL;
    return -1;
}

static void
_eth_drv_rx_thread(void *p)
{
    struct sockaddr dnl;
    struct nlmsghdr *nlh = NULL;  
    eth_unit_info_t  *unitp = (eth_unit_info_t  *)p;
    struct msghdr msg;
    struct iovec iov;
    int    pkt_len = 0;
    char * rx_pkt = NULL, *npkt;
    zpl_uint32 status;
    if ((rx_pkt = malloc(ETH_DRV_MAX_BUF_LEN)) == NULL) {
        return;
    }

	iov.iov_base = rx_pkt,
	iov.iov_len = ETH_DRV_MAX_BUF_LEN,
	msg.msg_name = (void *) &dnl,
	msg.msg_namelen = sizeof dnl,
	msg.msg_iov = &iov,
	msg.msg_iovlen = 1;

    while (unitp->started) {
        pkt_len = recvmsg(unitp->sock_fd, &msg, 0);

		if (msg.msg_namelen != sizeof dnl)
		{
			zlog_err(MODULE_PAL, "sender address length error: length %d",msg.msg_namelen);
			return -1;
		}
		for (nlh = (struct nlmsghdr *) rx_pkt;
				NLMSG_OK(nlh, (zpl_uint32) status); nlh = NLMSG_NEXT(nlh, status))
        {
            zlog_err(MODULE_PAL, "nlh->nlmsg_type %d",nlh->nlmsg_type);
			/* Finish of reading. */
			if (nlh->nlmsg_type == NLMSG_DONE)
				return -1;

			/* Error handling. */
			if (nlh->nlmsg_type == NLMSG_ERROR)
			{
				//struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(nlh);
				//int errnum = err->error;
				//int msg_type = err->msg.nlmsg_type;
            }
            npkt = NLMSG_DATA(nlh);
            if(npkt && nlh->nlmsg_len)
            {
                zlog_debug(MODULE_HAL, "_eth_drv_rx_thread recv: %d bytes :%s", pkt_len, strerror(errno));
                _eth_drv_handle_rx(0, npkt, nlh->nlmsg_len);
            }
        }
    }
}


static int
_eth_drv_tx_hw(int unit,int type, unsigned char *tx_pkt, int pkt_len)
{
    eth_unit_info_t * p_chan = eth_control[unit];
    struct sockaddr_nl dnl;
 	struct msghdr msg;
	struct iovec iov[3];
    struct nlmsghdr *nlh = NULL;
    int iovlen = 2,ret = 0;
    unsigned char buf[256];
    nlh = (struct nlmsghdr *)buf;
    memset(&dnl, 0, sizeof(struct sockaddr_nl));
    dnl.nl_family = AF_NETLINK;
    dnl.nl_pid = 0;
    dnl.nl_groups = 0;

    nlh->nlmsg_len = NLMSG_SPACE(pkt_len);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = type;//NK_PKT_DATA;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = p_chan->pid;

    //memcpy(NLMSG_DATA(nlh), tx_pkt, pkt_len);
	iov[0].iov_base = nlh;
	iov[0].iov_len = sizeof(struct nlmsghdr);//NLMSG_SPACE(pkt_len);

	iov[1].iov_base = tx_pkt;
	iov[1].iov_len = (pkt_len);

    memset(&msg, 0, sizeof(msg));  
    msg.msg_name = (void *)&dnl;  
    msg.msg_namelen = sizeof(struct sockaddr_nl);  
    msg.msg_iov = &iov;  
    msg.msg_iovlen = iovlen;  

    ret = sendmsg(p_chan->sock_fd, &msg, 0); 
    if(ret <= 0)
        zlog_debug(MODULE_HAL, "================sendmsg : :%s", strerror(errno));  
    return ret;
}

static int _eth_drv_start_nl(int unit, int start)
{
    struct nk_pkt_start nlpkt;
    return _eth_drv_tx_hw(unit, start?NK_PKT_START:NK_PKT_STOP, (unsigned char *)&nlpkt, sizeof(nlpkt));
}

static int _eth_drv_setup(int unit)
{
    struct nk_pkt_start nlpkt;
    nlpkt.ifindex = if_nametoindex("eth0");
    nlpkt.value = eth_control[unit]->pid;
    return _eth_drv_tx_hw(unit, NK_PKT_SETUP, (unsigned char *)&nlpkt, sizeof(nlpkt));
}
static int
_eth_drv_tx(int unit, unsigned char *tx_pkt, int pkt_len)
{
    return _eth_drv_tx_hw( unit, NK_PKT_DATA, tx_pkt,  pkt_len);
}

#endif


static int
_eth_drv_start(int unit)
{
    if ((eth_control[unit] == NULL) || (!eth_control[unit]->rx_cb)) {
        return -1;
    }

    if (eth_control[unit]->started == 0) {

        /*
         * Start an RX thread to poll for packets over socket interface.
         */
        eth_control[unit]->started++;
        os_task_create("drv_rx_thread", OS_TASK_DEFAULT_PRIORITY,
                                    0, _eth_drv_rx_thread, eth_control[unit], OS_TASK_DEFAULT_STACK * 4);       
#ifdef NETLINK_TEST
        _eth_drv_setup(unit);
        _eth_drv_start_nl( unit, 1);
#endif
    } else {
        eth_control[unit]->started++;
    }
    return 0;
}

static int
_eth_drv_stop(int unit)
{
    if ((eth_control[unit] == NULL) || (!eth_control[unit]->started)) {
        return 0;
    }
    #ifdef NETLINK_TEST
    _eth_drv_start_nl( unit, 0);
    #endif
    --eth_control[unit]->started;
    return 0;
}
/********* APIs **********/
static void 
_eth_drv_handle_rx(int unit, void *pkt, int pkt_len)
{
    int rv = 0;
 
    if (eth_control[unit] && eth_control[unit]->rx_cb) {
        rv = (eth_control[unit]->rx_cb(unit, 
             (unsigned char*) pkt, pkt_len) == 0) ?  1 : 0;
    }
    return;
}

int eth_drv_init(int unit)
{
    if (unit >= ETH_MAX_UNITS) {
        return -1;
    }
    return _eth_drv_init(unit);
}
static int kernel_packet_hexmsg(int unit, unsigned char *buf, int len)
{
    int ret = 0;
    zpl_char format[4096];
    struct brcm_tag_header tagheader;
    memset(format, 0, sizeof(format));
    memset(&tagheader, 0, sizeof(tagheader));
    //snprintf(format, sizeof(format), "%s","hello");
    brcm_ingress_hdr_getandshow(buf + 12, &tagheader);
    ret = os_loghex(format, sizeof(format), (const zpl_uchar *)buf,  len);
    zlog_debug(MODULE_HAL, "dev recv: %d bytes:\r\n  hex:%s", len, format);
    return OK;
}
int eth_drv_register(int unit, rx_cb _rx)
{
    if (unit >= ETH_MAX_UNITS) {
        return -1;
    }
    if (eth_control[unit] == NULL) {
        printf("ETH_DRV : error (driver not initialized !!)\n");
        return -1;
    }
    eth_control[unit]->rx_cb = _rx;

    return 0;
}

int eth_drv_start(int unit)
{
    eth_drv_register(0,kernel_packet_hexmsg);
    return _eth_drv_start(unit);
}

int eth_drv_stop(int unit)
{
    return _eth_drv_stop(unit);
}


int eth_drv_tx(int unit, unsigned char * pkt, int len)
{
    return _eth_drv_tx(unit, pkt, len);
}


int eth_drv_unicast(int unit, zpl_phyport_t phyid, int pri, zpl_void *pkt, int len)
{
    unsigned char brcm_tag[4];
    struct brcm_tag_header tagheader;
    tagheader.opcode = BRCM_OPCODE_TYPE1;
    tagheader.header.egress.TC = pri;
    tagheader.header.egress.TE = BRCM_TE_NOT_CARE;
    tagheader.header.egress.TS = 0;
    tagheader.header.egress.dstport = phyid;
    brcm_ingress_hdr_set(brcm_tag, &tagheader);
    return eth_drv_tx( unit, pkt,  len);
}

int eth_drv_vlan_flood(int unit, vlan_t vlan, int pri, zpl_void *pkt, int len)
{
    unsigned char brcm_tag[4];
    struct brcm_tag_header tagheader;
    tagheader.opcode = BRCM_OPCODE_TYPE1;
    tagheader.header.egress.TC = pri;
    tagheader.header.egress.TE = BRCM_TE_NOT_CARE;
    tagheader.header.egress.TS = 0;
    tagheader.header.egress.dstport = 0;
    brcm_ingress_hdr_set(brcm_tag, &tagheader);
    return eth_drv_tx( unit, pkt,  len);
}