/*
 * $Id: et_soc.c,v 1.43 Broadcom SDK $
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
 * Broadcom SOC layer device driver for 
 * Broadcom BCM44XX and BCM47XX 10/100 Mbps Ethernet Controller
 *
 * NOTE: This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 */

#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmdevs.h>
#include <shared/et/bcmenetmib.h>
#include <shared/et/bcmenetrxh.h>
#include <shared/et/bcmgmacmib.h>
#include <shared/et/bcmgmacrxh.h>
#include <shared/et/epivers.h>
#include <shared/et/et_dbg.h>
#include <soc/etc.h>
#include <soc/drv_if.h>
#include <soc/drv.h>

#include <sal/core/dpc.h>
#include <sal/core/time.h>

#define SOC_ET_PROMISC      1
#define SOC_WATCHDOG_ENABLE     1


et_soc_info_t *et_soc = NULL;

/* prototypes called by etc.c */

void et_soc_init(et_soc_info_t *et, bool full);
void et_soc_reset(et_soc_info_t *et);
void et_soc_link_up(et_soc_info_t *et);
void et_soc_link_down(et_soc_info_t *et);
void et_soc_up(et_soc_info_t *et);
void et_soc_down(et_soc_info_t *et, int reset);
int et_soc_dump(et_soc_info_t *et, uchar *buf, uint len);

/* local prototypes */
static void et_soc_free(et_soc_info_t *et);
static void et_soc_sendnext(et_soc_info_t *et);

int et_set_mac_address(int unit, sal_mac_addr_t addr);
#if ET_SOC_WATCHDOG_ENABLE
static void et_watchdog(void *p_et, void *p2, void *p3, void *p4, void *p5);
#endif
static void et_soc_intr(void *_unit);
void et_soc_sendup(et_soc_info_t *et, robo_dv_t *dv_chain);
#ifdef BCMDBG
static void et_dumpet(et_soc_info_t *et, uchar *buf, uint len);
#endif

#if defined(BCMDBG)
static int msglevel = 0xdeadbeef;
#endif

#ifdef IMP_SW_PROTECT

#define IMP_PROT_PULL_INTERVAL (1 * SECOND_USEC)

/* IMP port states */
typedef enum imp_port_state_e {
    IMP_PORT_STATE_RATE_MAX,
    IMP_PORT_STATE_RATE_MID,
    IMP_PORT_STATE_RATE_MIN
}imp_port_state_t;

void soc_imp_prot_init(int unit);

/* Configurate rate value */ 
#define IMP_PORT_MAX_RATE   640
#define IMP_PORT_MID_RATE   512
#define IMP_PORT_MIN_RATE   256
#define IMP_PORT_BURST_SIZE 48

static sal_thread_t imp_prot_thread_id = NULL;
static int imp_state = IMP_PORT_STATE_RATE_MIN;
static sal_time_t imp_last_time;
static uint64 imp_last_count;
static int imp_pull_interval = 0;
static sal_mutex_t imp_prot_lock = NULL;
static int imp_unit = 0;
static int imp_min_rate = IMP_PORT_MIN_RATE;
static int imp_mid_rate = IMP_PORT_MID_RATE;
static int imp_max_rate = IMP_PORT_MAX_RATE;

#define IMP_PROT_LOCK sal_mutex_take(imp_prot_lock, sal_mutex_FOREVER)
#define IMP_PROT_UNLOCK sal_mutex_give(imp_prot_lock)

#endif /* IMP_SW_PROTECT */


int 
et_soc_attach(int unit)
{
    et_soc_info_t *et;
    char name[128];

#if defined(BCMDBG)
    if (msglevel != 0xdeadbeef) {
        et_msg_level = msglevel;
        soc_cm_print("et%d: et_soc_attach: et_msg_level set to 0x%x\n",
                 unit, msglevel);
    }
#endif

    ET_TRACE(("et%d: et_soc_attach\n", unit));

    if (!etc_soc_chipmatch(BROADCOM_VENDOR_ID, CMDEV(unit).dev.dev_id))
        return SOC_E_NOT_FOUND;

    /* allocate private info */
    if ((et = (et_soc_info_t*) ET_MALLOC(sizeof (et_soc_info_t))) == NULL) {
        ET_ERROR(("et%d: kmalloc() failed\n", unit));
        return SOC_E_MEMORY;
    }
    sal_memset(et, 0, sizeof (et_soc_info_t));
    et->dev = unit;

    et->txq_head = et->txq_tail = NULL;
    et->txq_cnt = 0;
    et->rxq_head = et->rxq_tail = NULL;
    et->rxq_cnt = 0;
    et->et_soc_intr_pend = FALSE;

    if ((et->soc_robo_dma_lock = sal_mutex_create("ET_SOC_DMA")) == NULL) {
        ET_ERROR(("et%d: ET_SOC_DMA mutex create failed\n", unit));
        goto fail;
    }
    if ((et->tx_dma_lock = sal_mutex_create("ET_TX_DMA")) == NULL) {
        ET_ERROR(("et%d: ET_TX_DMA mutex create failed\n", unit));
        goto fail;
    }
    if ((et->rx_dma_lock = sal_mutex_create("ET_RX_DMA")) == NULL) {
        ET_ERROR(("et%d: ET_RX_DMA mutex create failed\n", unit));
        goto fail;
    }

    /* common load-time initialization */
    if (!(et->etc = etc_soc_attach((void*)et, BROADCOM_VENDOR_ID,
                  CMDEV(unit).dev.dev_id, (uint)unit,
                  (void *)unit, (void *)NULL))) {
                  
        ET_ERROR(("et%d: etc_attach() failed\n", unit));
        goto fail;
    }

#if ET_SOC_WATCHDOG_ENABLE
    /* init 1 second watchdog timer */
    et->timer = SOC_WD_TIMER;
#endif

    et_soc = et;

    /* register our interrupt handler */
    if (soc_cm_interrupt_connect(unit, et_soc_intr, INT_TO_PTR(unit)) < 0) {
        ET_ERROR(("et%d: could not connect interrupt line\n",
             unit));
        goto fail;
    }

    /* print hello string */
    (*et->etc->chops->longname)(et->etc->ch, name, sizeof (name));
    soc_cm_print("%s %s\n", name, EPI_VERSION_STR);

    return SOC_E_NONE;

fail:
    et_soc_free(et);
    return (SOC_E_INTERNAL);
}

static void
et_soc_free(et_soc_info_t *et)
{
    if (et == NULL)
        return;

    ET_TRACE(("et: et_soc_free\n"));

    /* Detach interrupt handler, if we installed one */
    /* unit # is ISR arg */
    if (soc_cm_interrupt_disconnect(et->dev) < 0) {
        soc_cm_print("soc_detach: could not disconnect interrupt.\n");
        return;
    }

    /* free common resources */
    if (et->etc) {
        etc_soc_detach(et->etc);
        et->etc = NULL;
    }

    if (et->soc_robo_dma_lock) {
        sal_mutex_destroy(et->soc_robo_dma_lock);
    }
    if (et->tx_dma_lock) {
        sal_mutex_destroy(et->tx_dma_lock);
    }
    if (et->rx_dma_lock) {
        sal_mutex_destroy(et->rx_dma_lock);
    }

    ET_MFREE(et, sizeof (et_soc_info_t));
}

int
et_soc_open(int unit)
{
    ASSERT(unit == et_soc->dev);

    ET_TRACE(("et%d: et_soc_open\n", et_soc->etc->unit));

    et_soc->etc->promisc = (SOC_ET_PROMISC) ? TRUE: FALSE;

    et_soc_up(et_soc);

    return SOC_E_NONE;
}

int
et_soc_close(int unit)
{
    ASSERT(unit == et_soc->dev);

    ET_TRACE(("et%d: et_soc_open\n", et_soc->etc->unit));

    et_soc->etc->promisc = FALSE;

    et_soc_down(et_soc, 1);

    return SOC_E_NONE;
}
void
et_soc_debug(int unit)
{
    etc_soc_info_t *etc;

    etc = et_soc->etc;
    etc_soc_debug(etc);
}


/*
 * Yeah, queueing the packets on a tx queue instead of throwing them
 * directly into the descriptor ring in the case of dma is kinda lame,
 * but this results in a unified transmit path for both dma and pio
 * and localizes/simplifies the netif_*_queue semantics, too.
 */
int
et_soc_start(int unit, robo_dv_t *dv_chain)
{
    int cnt = 0;
    robo_dv_t *cur_dv;
    robo_dv_t *last_valid_dv;

    ET_TRACE(("et%d: et_soc_start: len %d\n",
         et_soc->etc->unit, dv_chain->dv_length));
        ET_LOG("et%d: et_soc_start: len %d",
           et_soc->etc->unit,dv_chain->dv_length);

    ET_TX_DMA_LOCK(et_soc);
    ET_SOC_DMA_LOCK(et_soc);

    /* put it on the tx queue and call sendnext */
    if (!et_soc->txq_head) {
        et_soc->txq_head = dv_chain;
    } else {
        et_soc->txq_tail->dv_next = dv_chain;
    }

    /* Get the total number of the chain, start from dv_chain */
    cur_dv = dv_chain;
    last_valid_dv = cur_dv;
    while (cur_dv) {
        cnt ++;
        last_valid_dv = cur_dv;
        cur_dv = cur_dv->dv_next;
    }
    /* Assign txq_tail */
    et_soc->txq_tail = last_valid_dv;

    /* Add the number of dv(s) to txq_cnt */
    et_soc->txq_cnt += cnt;

    et_soc_sendnext(et_soc);
    ET_SOC_DMA_UNLOCK(et_soc);
    ET_TX_DMA_UNLOCK(et_soc);

    ET_LOG("et%d: et_soc_start ret\n", et->etc->unit, 0);

    return (0);
}

static void
et_soc_sendnext(et_soc_info_t *et)
{
    etc_soc_info_t *etc;
    robo_dv_t *dv;

    etc = et->etc;

    ET_TRACE(("et%d: et_sendnext\n", etc->unit));
    ET_LOG("et%d: et_sendnext", etc->unit, 0);

    /* dequeue and send each packet */
#ifdef DMA
    while (*etc->txavail[TX_Q0] > 0) {
#else
    while (etc->pioactive == NULL) {
#endif
#if defined(BCMDBG)
        int i;
#endif

        if ((dv = et->txq_head) == NULL) {
            break;
        }

        if (dv->dv_vcnt > *etc->txavail[TX_Q0]) {
            /* If available tx descriptor is not enough for the next dv */
            break;
        }

        if (et->txq_head == et->txq_tail) {
            et->txq_head = et->txq_tail = NULL;
        } else {
                    et->txq_head = dv->dv_next;
        }

        et->txq_cnt--;

        ET_PRHDR("tx", (struct ether_header *)(dv->dv_dcb[0].dcb_vaddr),
             dv->dv_dcb[0].len);
#if defined(BCMDBG)
        for (i=0; i<dv->dv_vcnt; i++) {
            ET_TRACE(("txpkt dv %d len=%d\n", i, dv->dv_dcb[i].len));
            ET_PRPKT("txpkt", (void *)(dv->dv_dcb[i].dcb_vaddr),
                 dv->dv_dcb[i].len);
        }
#endif

        (*etc->chops->tx)(etc->ch, (void*)dv);

        etc->txframe++;
        etc->txbyte += dv->dv_length;
    }
}

void
et_soc_init(et_soc_info_t *et, bool full)
{
    ASSERT(et == et_soc);

    ET_TRACE(("et%d: et_soc_init\n", et->etc->unit));
    ET_LOG("et%d: et_soc_init", et->etc->unit, 0);

    et->et_soc_init = TRUE;

    et_soc_reset(et);

    etc_soc_init(et->etc, full);

    et->et_soc_init = FALSE;
}


void
et_soc_reset(et_soc_info_t *et)
{
    ASSERT(et == et_soc);

    ET_TRACE(("et%d: et_soc_reset\n", et->etc->unit));

    etc_soc_reset(et->etc);
}

void
et_soc_up(et_soc_info_t *et)
{
    etc_soc_info_t *etc;

    etc = et->etc;

    if (etc->up)
        return;

    ET_TRACE(("et%d: et_soc_up\n", etc->unit));

    etc_soc_up(etc);

#if ET_SOC_WATCHDOG_ENABLE
    /* schedule one second watchdog timer */
    sal_dpc_time(et->timer, et_watchdog, (void *)et, 0, 0, 0, 0);
#endif
}

void
et_soc_down(et_soc_info_t *et, int reset)
{
    etc_soc_info_t *etc;

    etc = et->etc;

    ET_TRACE(("et%d: et_soc_down\n", etc->unit));

#if ET_SOC_WATCHDOG_ENABLE
    /* stop watchdog timer */
    sal_dpc_cancel((void *)et);
#endif

    etc_soc_down(etc, reset);
}

#if ET_SOC_WATCHDOG_ENABLE
static void
et_watchdog(void *p_et, void *p2, void *p3, void *p4, void *p5)
{
    et_soc_info_t *et = (et_soc_info_t *)p_et;

#if !defined(VXWORKS) && !defined(__KERNEL__) && !defined(__ECOS)
    ET_SOC_DMA_LOCK(et);
#endif

    etc_soc_watchdog(et->etc);

    /* reschedule one second watchdog timer */
    sal_dpc_time(et->timer, et_watchdog, (void *)et, 0, 0, 0, 0);

#if !defined(VXWORKS) && !defined(__KERNEL__) && !defined(__ECOS)
    ET_SOC_DMA_UNLOCK(et);
#endif
}
#endif

int
et_set_mac_address(int unit, sal_mac_addr_t addr)
{
    ASSERT(unit == et_soc->dev);

    if (et_soc->etc->up)
        return SOC_E_BUSY;

    ET_TRACE(("et%d: et_set_mac_address\n", et_soc->etc->unit));

    sal_memcpy(&et_soc->etc->cur_etheraddr, addr, ETHER_ADDR_LEN);

    return SOC_E_NONE;
}

static void
et_socintr(void *_unit, void *_events)
{
    int unit = PTR_TO_INT(_unit);
    uint32 events = (uint32)_events;
#ifdef IMP_SW_PROTECT    
    uint32 imp_port;
    soc_pbmp_t t_pbm;
    int no_que = 0;
#endif    
    struct chops *chops;
    void *ch;
    int sendup;
    robo_dv_t *dv;
    int hwrxoff = 0;

    ASSERT(unit == et_soc->dev);
    ASSERT(events);

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;

    hwrxoff = et_soc->etc->hwrxoff; /* HW RX offset value */

    ET_SOC_DMA_LOCK(et_soc);

    if (events & INTR_RX) {
        ET_TRACE(("et%d: et_intr: RX events\n", et_soc->etc->unit));
        sendup = 0;
        while ((dv = (robo_dv_t *)(*chops->rx)(ch))) {
#ifdef BCMDBG
            uint8 *buf = (uint8 *)dv->dv_dcb->dcb_vaddr + hwrxoff;
            ET_TRACE(("Receive packet:\n"));
            ET_TRACE(("%02x %02x %02x %02x %02x %02x %02x %02x "
                  "%02x %02x %02x %02x %02x %02x %02x %02x\n", 
                  buf[0],  buf[1],  buf[2],  buf[3],
                  buf[4],  buf[5],  buf[6],  buf[7],
                  buf[8],  buf[9],  buf[10], buf[11],
                  buf[12], buf[13], buf[14], buf[15]));
            buf = (uint8 *)dv->dv_dcb->dcb_vaddr + hwrxoff + 16;
            ET_TRACE(("%02x %02x %02x %02x %02x %02x %02x %02x "
                  "%02x %02x %02x %02x %02x %02x %02x %02x\n", 
                  buf[0],  buf[1],  buf[2],  buf[3],
                  buf[4],  buf[5],  buf[6],  buf[7],
                  buf[8],  buf[9],  buf[10], buf[11],
                  buf[12], buf[13], buf[14], buf[15]));
#endif
            et_soc_sendup(et_soc, dv);

            if (++sendup > NRXBUFPOST) {
                break;
            }
        }

        /* post more rx bufs */
        (*chops->rxfill)(ch);
    }

    if (events & INTR_TX) {
        ET_TRACE(("et%d: et_soc_intr: TX events\n", et_soc->etc->unit));
        (*chops->txreclaim)(ch, FALSE);
    }

    if (events & INTR_ERROR) {
        ET_TRACE(("et%d: et_soc_intr: ERROR events\n", et_soc->etc->unit));
        if ((*chops->errors)(ch)) {
            /*
             * Try recovering the error
             */
#ifdef IMP_SW_PROTECT 
                /* Constrain to Min egress rate of IMP */
                imp_port = CMIC_PORT(imp_unit);
                SOC_PBMP_CLEAR(t_pbm);
                SOC_PBMP_PORT_ADD(t_pbm, imp_port);
                DRV_RATE_SET
                     (imp_unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                      0, imp_min_rate, IMP_PORT_BURST_SIZE);            
                imp_state = IMP_PORT_STATE_RATE_MIN;
                
#endif /* IMP_SW_PROTECT */
            et_soc_init(et_soc, TRUE);
        }
    }

    /* run the tx queue */
    if (et_soc->txq_cnt > 0) {
        et_soc_sendnext(et_soc);
    }

    et_soc->et_soc_intr_pend = FALSE;

    /* re-enable interrupts */
    (*chops->intrson)(ch);
    ET_LOG("et%d: et_soc_intr ret\n", et_soc->etc->unit, 0);

    ET_SOC_DMA_UNLOCK(et_soc);
    return;
}

static void
et_soc_intr(void *_unit)
{
    uint32 events;
    struct chops *chops;
    void *ch;

#if !defined(VXWORKS) && !defined(__KERNEL__) && !defined(__ECOS)
    ET_SOC_DMA_LOCK(et_soc);
#endif

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;

    if (!et_soc->etc->up)
        goto done;

    /* get interrupt condition bits */
    events = (*chops->getintrevents)(ch);

    if (!(events & INTR_NEW) || et_soc->et_soc_intr_pend)
        goto done;

    ET_TRACE(("et%d: et_intr: events 0x%x\n", et_soc->etc->unit, events));

    /* disable device interrupts */
    (*chops->intrsoff)(ch);
    
    et_soc->et_soc_intr_pend = TRUE;

    sal_dpc((sal_dpc_fn_t)et_socintr, (void *)_unit, (void *)events, 0, 0, 0);
done:
#if !defined(VXWORKS) && !defined(__KERNEL__) && !defined(__ECOS)
    ET_SOC_DMA_UNLOCK(et_soc);
#endif
    return;
}

void
et_soc_sendup(et_soc_info_t *et, robo_dv_t *dv)
{
    etc_soc_info_t *etc;
    bcmenetrxh_t *rxh = NULL;
    uint16 flags, len = 0, timestamp_size = 0;
    uchar eabuf[32];
    uchar rxhdr[MAX_HWRXOFF];
    uint8 *payload = (uint8 *)dv->dv_dcb->dcb_vaddr + ETC47XX_HWRXOFF;
    uint16 dev_id;
    uint8 rev_id;
    uint16 tmp_len = 0;

    etc = et->etc;
    dv->dv_flags = 0; /* RX succeed */
    rxh = (bcmenetrxh_t*) rxhdr; /* Initialized */

    if (et->etc->deviceid == BCM47XX_ENET_ID) {
        payload = (uint8 *)dv->dv_dcb->dcb_vaddr + ETC47XX_HWRXOFF;
        sal_memcpy((void *)rxhdr, (void *)dv->dv_dcb->dcb_vaddr, ETC47XX_HWRXOFF);
        /* packet buffer starts with rxhdr */
        rxh = (bcmenetrxh_t*) rxhdr;
        len = rxh->len;
#ifdef BE_HOST
        len = ((len & 0xff00) >> 8) | ((len & 0xff) << 8);
#endif
    } else if (et->etc->deviceid == BCM53000_GMAC_ID) {
        payload = (uint8 *)dv->dv_dcb->dcb_vaddr + ETCGMAC_HWRXOFF;
        sal_memcpy((void *)rxhdr, (void *)dv->dv_dcb->dcb_vaddr, ETCGMAC_HWRXOFF);
        /* packet buffer starts with rxhdr */
        rxh = (bcmenetrxh_t*) rxhdr;
        len = rxh->len;
#ifdef BE_HOST
        len = ((len & 0xff00) >> 8) | ((len & 0xff) << 8);
#endif
    }

    /* Check if BRCM tag and VLAN are present */
    if ((payload[2*ENET_MAC_SIZE] == (ENET_DEFAULT_BRCMID >> 8)) &&
        (payload[2*ENET_MAC_SIZE+1] == (ENET_DEFAULT_BRCMID & 0xff))) {
        /* Strip off and save the BRCM Tag */
        sal_memcpy((void *)&dv->dv_brcm_tag,
                       (void *)(payload + 2*ENET_MAC_SIZE + 2),
               sizeof(uint32));
        dv->dv_brcm_tag = soc_ntohl(dv->dv_brcm_tag);
        /* The port id is 24 of phsical port 0 in BCM5347 */
        if (SOC_IS_ROBO5347(soc_mii_unit)) {
            dv->dv_src_portid -= 24;
        } else if (SOC_IS_ROBO53242(soc_mii_unit) ||SOC_IS_ROBO53262(soc_mii_unit)) {
            dv->dv_src_portid_53242 -= 24;
        }
        /* Handle the time stamped packets */ 
        timestamp_size = 0;
        if (SOC_IS_ROBO5395(soc_mii_unit)) {
            if (dv->dv_opcode == BRCM_OP_EGR_TS) {
                timestamp_size = 4;
                sal_memcpy((void *)&dv->rx_timestamp,
                    (void *)(payload + 2*ENET_MAC_SIZE + 
                    ENET_BRCM_TAG_SIZE), sizeof(uint32));
            }
        }
        len = rxh->len;
#ifdef BE_HOST
        len = ((len & 0xff00) >> 8) | ((len & 0xff) << 8);
#endif

        /*
         * The frame count field of 5324 is offset bit 16 
         * while the same field of 5396, 5389 and 5398 is offset bit 15.
         * However, the defined dv_len is offset bit 15. Thus we need
         * to shift right one bit if underlying chip is 5324
         */

        tmp_len = dv->dv_len;
        if (SOC_IS_ROBO5324(soc_mii_unit)) {
            tmp_len >>= 1;              
        }
        /* The frame count field of 5348 is offset bit 17 */
        if( SOC_IS_ROBO5348(soc_mii_unit) || SOC_IS_ROBO5347(soc_mii_unit)||
            SOC_IS_ROBO53242(soc_mii_unit) || SOC_IS_ROBO53262(soc_mii_unit)) {
            tmp_len >>= 2;
        }


        assert(ENET_MAC_SIZE == ENET_BRCM_TAG_SIZE);

        /* Make sure the len in BRCM TAG matches with DMA length */
        if (len == (uint16)(tmp_len + ENET_BRCM_TAG_SIZE + 4
                     + timestamp_size)) {
            /* Copy SA to the location of BRCM Tag */
            sal_memcpy((void *)(payload + 2*ENET_MAC_SIZE + timestamp_size),
                       (void *)(payload + ENET_MAC_SIZE),
                       ENET_MAC_SIZE);

            /* Copy DA to the location of BRCM Tag */
            sal_memcpy((void *)(payload + ENET_MAC_SIZE + timestamp_size),
                       (void *)(payload),
                       ENET_MAC_SIZE);

            /*
             * Workaround for BCM53242/53262A0 IMP port CRC error.
             */
            dv->dv_public4.u8 = 0;
            if (SOC_IS_ROBO53242(soc_mii_unit)||SOC_IS_ROBO53262(soc_mii_unit)) {
                soc_cm_get_id(soc_mii_unit, &dev_id, &rev_id);
                if ((rev_id == BCM53242_A0_REV_ID) || 
                        (rev_id == BCM53262_A0_REV_ID)) {
                    int tpid_offset_1 = 2*ENET_MAC_SIZE+ENET_BRCM_TAG_SIZE;
                    int tpid_offset_2 = tpid_offset_1+ENET_TAG_SIZE;
                    int da_offset = 0, bcast_pkt = FALSE;
                    int x = 0, tmp = 0;

                    /* check the frame type for this A0 WAR.
                     *  - Broadcast(DA=0xffffffffffff) + 2*SP_tag
                     */
                    for (x = 0; x < ENET_MAC_SIZE; x++){
                        if (payload[da_offset + x] != 0xff) {
                            break;
                        }
                    }
                    bcast_pkt = (x == ENET_MAC_SIZE) ? TRUE : FALSE;
                    
                    if (bcast_pkt && 
                        (payload[tpid_offset_1] == payload[tpid_offset_2]) &&
                        (payload[tpid_offset_1 + 1] == 
                            payload[tpid_offset_2 + 1])) {

                        tmp = 2*ENET_MAC_SIZE+ ENET_BRCM_TAG_SIZE;
                        for (x = (tmp-1); x >= 0; x--) {
                            sal_memcpy((void *)(payload + x + ENET_TAG_SIZE),
                                       (void *)(payload) + x,
                                       sizeof(uint8));
                        }
                        dv->dv_public4.u8 = ENET_TAG_SIZE;
                    }
                }
            }

            dv->dv_length = len - (ENET_BRCM_TAG_SIZE + 
                timestamp_size + sizeof(uint32));
            dv->dv_dcb->len = dv->dv_length;
        } else {
            dv->dv_flags |= RXF_RXER;
        }
    } else if (SOC_IS_ROBO53115(soc_mii_unit) || 
        SOC_IS_ROBO53118(soc_mii_unit) || SOC_IS_ROBO53101(soc_mii_unit) ||
        SOC_IS_ROBO53125(soc_mii_unit) || SOC_IS_ROBO53128(soc_mii_unit)) {

        /* 
         * There is no BRCM type (2 bytes) for BCM53115, BCM53118.
         * BRCM Tag size = 4 bytes(ENET_BRCM_TAG_SIZE - 2) for BCM53115, BCM53118.
         */
        /* Strip off and save the BRCM Tag */
        sal_memcpy((void *)&dv->dv_brcm_tag,
                       (void *)(payload + 2*ENET_MAC_SIZE),
               sizeof(uint32));
        dv->dv_brcm_tag = soc_ntohl(dv->dv_brcm_tag);

        /* Handle the time stamped packets */ 
        timestamp_size = 0;
        if (dv->dv_opcode == BRCM_OP_EGR_TS) {
            timestamp_size = 4;
            sal_memcpy((void *)&dv->rx_timestamp,
                (void *)(payload + 2*ENET_MAC_SIZE + 
                ENET_BRCM_SHORT_TAG_SIZE), sizeof(uint32));
        }

        len = rxh->len;
#ifdef BE_HOST
        len = ((len & 0xff00) >> 8) | ((len & 0xff) << 8);
#endif

        assert(ENET_MAC_SIZE == ENET_BRCM_TAG_SIZE);

        /* Replace the frame_len in BRCM TAG with ClassificationID for BCM53115, BCM53118 */
        /* 
          * Copy SA + DA  to the location of BRCM Tag
          * No BRCM type(2 bytes) : shift 4 bytes (Remove BRCM Tag) 
          * We do three times memcpy(DA + SA) to avoid overlap (each time shift 4 bytes)
          */

        /* SA : 2 ~ 5 bytes(4 bytes) */
        sal_memcpy((void *)(payload + (2*ENET_MAC_SIZE) + timestamp_size),
                   (void *)(payload + (2*ENET_MAC_SIZE - 4)), 4);

        /* DA : 4 ~ 5 bytes + SA : 0 ~ 1 bytes (4 bytes) */
        sal_memcpy((void *)(payload + (2*ENET_MAC_SIZE - 4 + timestamp_size)),
                   (void *)(payload + (ENET_MAC_SIZE - 2)), 4);

        /* DA : 0 ~ 3 bytes(4 bytes) */
        sal_memcpy((void *)(payload + (2*ENET_MAC_SIZE - 8 + timestamp_size)),
           (void *)(payload), 4);

        dv->dv_length = len - (ENET_BRCM_SHORT_TAG_SIZE + 
            timestamp_size);
        dv->dv_dcb->len = dv->dv_length;
    } else if (SOC_IS_TB(soc_mii_unit)) {
        /* 
         * There is no BRCM type (2 bytes) for BCM53280.
         * BRCM Tag size = (4 * 2)bytes for BCM53280.
         */
        /* Strip off and save the BRCM Tag */
        sal_memcpy((void *)&dv->dv_brcm_tag,
                       (void *)(payload), sizeof(uint32));
        sal_memcpy((void *)&dv->dv_brcm_tag2,
                       (void *)(payload + sizeof(uint32)), sizeof(uint32));
        dv->dv_brcm_tag = soc_ntohl(dv->dv_brcm_tag);
        dv->dv_brcm_tag2 = soc_ntohl(dv->dv_brcm_tag2);

        assert(ENET_MAC_SIZE == ENET_BRCM_TAG_SIZE);

        dv->dv_length = len - (uint16)(ENET_BRCM_SHORT_TAG_SIZE * 2);
        dv->dv_dcb->len = dv->dv_length;
    } else {
        ET_ERROR(("ERROR received pkts: without BRCM TAG\n"));
        dv->dv_len = 0;
        dv->dv_length = 0;
        dv->dv_flags |= RXF_RXER;
        dv->dv_dcb->len = 0;
    }    

    ET_TRACE(("et%d: et_sendup: %d bytes\n", et->etc->unit, dv->dv_length));
    ET_LOG("et%d: et_sendup: len %d", et->etc->unit, dv->dv_length);

    etc->rxframe++;
    etc->rxbyte += len;

    ET_PRHDR("rx", (struct ether_header*) dv->dv_dcb->dcb_vaddr, dv->dv_length);
    ET_PRPKT("rxpkt", (void *)dv->dv_dcb->dcb_vaddr, dv->dv_length);

    /* check for reported frame errors */
    flags = ltoh16(rxh->flags);
    if (et->etc->deviceid == BCM47XX_ENET_ID) {
        flags &= (RXF_NO | RXF_RXER | RXF_CRC | RXF_OV);
    } else if (et->etc->deviceid == BCM53000_GMAC_ID) {
        flags &= (GRXF_OVF | GRXF_OVERSIZE | GRXF_CRC);
    }
    
    if (flags) {
        bcm_ether_ntoa((char *) ((struct ether_header*)
                            dv->dv_dcb->dcb_vaddr)->ether_shost,
                       (char *) eabuf);
        if (RXH_OVERSIZE(et->etc, rxh)) {
            ET_ERROR(("et%d: rx: over size packet from %s\n", et->etc->unit, eabuf));
        }
        if (RXH_CRC(et->etc, rxh)) {
            ET_ERROR(("et%d: rx: crc error from %s\n", et->etc->unit, eabuf));
        }
        if (RXH_OVF(et->etc, rxh)) {
            ET_ERROR(("et%d: rx: fifo overflow\n", et->etc->unit));
        }
        if (RXH_NO(et->etc, rxh)) {
            ET_ERROR(("et%d: rx: crc error (odd nibbles) from %s\n",
                      et->etc->unit, eabuf));
        }
        if (RXH_RXER(et->etc, rxh)) {
            ET_ERROR(("et%d: rx: symbol error from %s\n", et->etc->unit, eabuf));
        }
        dv->dv_flags |= flags;
    }
    if (!et_soc->et_soc_rxmon) {
        dv->dv_flags = RXF_OV;
        ET_ERROR(("et%d: rx: rxmon turn off\n", etc->unit));
        /*
         * Mark this packet as fifo overflow due to monitor packets function.
         */
    }

    /* send it up */
    if (dv->dv_done_chain) {
        dv->dv_done_chain(dv->dv_unit, dv);
    } else if (dv->dv_done_packet) {
        dv->dv_done_packet(dv->dv_unit, dv, dv->dv_dcb);
    } else {
            ET_ERROR(("et%d: rx: SEVERE!!! no callback function.\n", 
                etc->unit));
        soc_cm_sfree(dv->dv_unit, (void *)dv->dv_dcb->dcb_vaddr);
        soc_robo_dma_dv_free(dv->dv_unit, (robo_dv_t *)dv);
        return;
    }
       

    ET_LOG("et%d: et_sendup ret", et->etc->unit, 0);

    return;
}

int
et_soc_rx_chain(int unit, robo_dv_t *dv_chain)
{
    struct chops *chops;
    void *ch;

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;

    ET_RX_DMA_LOCK(et_soc);
    ET_SOC_DMA_LOCK(et_soc);

    /* put it on the tx queue and call sendnext */
    if (!et_soc->rxq_head) {
        et_soc->rxq_head = dv_chain;
    } else {
        et_soc->rxq_tail->dv_next = dv_chain;
    }
    et_soc->rxq_tail = dv_chain;
    et_soc->rxq_cnt++;


    if (!et_soc->et_soc_init) {
        /* post more rx bufs */
        (*chops->rxfill)(ch);
    }
    ET_SOC_DMA_UNLOCK(et_soc);
    ET_RX_DMA_UNLOCK(et_soc);

    ET_LOG("et%d: et_soc_rx_chain ret\n", et->etc->unit, 0);

    return (0);
}

robo_dv_t *
et_soc_rx_chain_get(int unit)
{
    robo_dv_t *dv = NULL;

    if ((dv = et_soc->rxq_head) == NULL) {
            return dv;
    }

    if (et_soc->rxq_head == et_soc->rxq_tail) {
        et_soc->rxq_head = et_soc->rxq_tail = NULL;
    } else {
        et_soc->rxq_head = dv->dv_next;
    }
    et_soc->rxq_cnt--;

    return dv;
}

int
et_dumpbuf(et_soc_info_t *et, uchar *buf, uint len)
{
    /* big enough? */
    if (len < 4096)
        return (4096);

    sprintf((char *) buf, "et%d: %s %s version %s\n", et->etc->unit,
        __DATE__, __TIME__, EPI_VERSION_STR);

#ifdef BCMDBG
    et_dumpet(et, &buf[strlen(buf)], len - strlen(buf));
    etc_soc_dump(et->etc, &buf[strlen(buf)], len - strlen(buf));
#endif

    return (strlen((char *) buf));
}

#ifdef BCMDBG
static void
et_dumpet(et_soc_info_t *et, uchar *buf, uint len)
{
    sprintf(&buf[strlen(buf)], "et %p dev %d\n",
        et, et->dev);
}
#endif  /* BCMDBG */

void
et_soc_link_up(et_soc_info_t *et)
{
    ET_ERROR(("et%d: link up\n", et->etc->unit));
}

void
et_soc_link_down(et_soc_info_t *et)
{
    ET_ERROR(("et%d: link down\n", et->etc->unit));
}

void
et_soc_rxmon_off(int unit)
{
    ET_SOC_DMA_LOCK(et_soc);
    et_soc->et_soc_rxmon = FALSE;
    (*et_soc->etc->chops->rxreset)(et_soc->etc->ch); 
    ET_SOC_DMA_UNLOCK(et_soc);
}

void
et_soc_rxmon_on(int unit)
{
    ET_SOC_DMA_LOCK(et_soc);
    et_soc->et_soc_rxmon = TRUE;
    (*et_soc->etc->chops->rxinit)(et_soc->etc->ch); 
    ET_SOC_DMA_UNLOCK(et_soc);
}

#ifdef BCM47XX_CHOPS
/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void*
et_soc_phyfind(et_soc_info_t *et, uint coreunit)
{
    ASSERT(et == et_soc);

    if (et->etc == NULL)
        return NULL;
    if (et_soc->etc->coreunit != coreunit)
        return NULL;
    return (et_soc);
}

/* shared phy read entry point */
uint16
et_soc_phyrd(et_soc_info_t *et, uint phyaddr, uint reg)
{
    uint16 val;

    ASSERT(et == et_soc);

#ifdef MDC_MDIO_SUPPORT
    MDC_DATA_LOCK;
#endif /* MDC_MDIO_SUPPORT */
    val = et->etc->chops->phyrd(et->etc->ch, phyaddr, reg);
#ifdef MDC_MDIO_SUPPORT
    MDC_DATA_UNLOCK;
#endif /* MDC_MDIO_SUPPORT */

    return (val);
}

/* shared phy write entry point */
void
et_soc_phywr(et_soc_info_t *et, uint phyaddr, uint reg, uint16 val)
{
    ASSERT(et == et_soc);

#ifdef MDC_MDIO_SUPPORT
    MDC_DATA_LOCK;
#endif /* MDC_MDIO_SUPPORT */
    et->etc->chops->phywr(et->etc->ch, phyaddr, reg, val);
#ifdef MDC_MDIO_SUPPORT
    MDC_DATA_UNLOCK;
#endif /* MDC_MDIO_SUPPORT */
}
#endif  /* BCM47XX_CHOPS */


#ifdef IMP_SW_PROTECT


static void
soc_imp_prot_thread(void *param)
{
    uint32 reg_addr, reg_value;
    int imp_port, reg_len;
    int unit = PTR_TO_INT(param);
    uint64  cur_count, temp64;
    sal_time_t    cur_time;
    uint32 delta;
    pbmp_t t_pbm;
    int no_que = 0;

    /* Initialization */
    imp_port = CMIC_PORT(unit);
    imp_last_time = sal_time();
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, TXOCTETSr, imp_port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, TXOCTETSr);
    ET_TRACE(("imp_prot_thread : addr = 0x%x\n", reg_addr));
    ((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&imp_last_count, reg_len));

    
    sal_usleep(imp_pull_interval);
    SOC_PBMP_CLEAR(t_pbm);
    SOC_PBMP_PORT_ADD(t_pbm, imp_port);
    DRV_RATE_SET
        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
        0, imp_max_rate, IMP_PORT_BURST_SIZE);
    
    /* Main thread */
    while (imp_pull_interval) {
        /* 1. Get the IMP port output counter */
        ((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&cur_count, reg_len));
        cur_time = sal_time();

        sal_memcpy(&temp64, &cur_count, sizeof(uint64));
        COMPILER_64_SUB_64(temp64, imp_last_count);
        delta = COMPILER_64_LO(temp64);
        ET_TRACE(("imp_prot_thread : state = %d, delta = %d\n", 
            imp_state, delta));

        /* 
         * Adjust the egress rate of IMP port 
         * according to the output counter value 
         */ 
        IMP_PROT_LOCK;
        switch(imp_state) {
            case IMP_PORT_STATE_RATE_MAX:
                if (( delta / (cur_time - imp_last_time)) > 
                    (IMP_PORT_MID_RATE * 128)) {
                    DRV_RATE_SET
                        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                        0, imp_min_rate, IMP_PORT_BURST_SIZE);
                    imp_state = IMP_PORT_STATE_RATE_MIN;
                    ET_TRACE(("STATE ==> RATE MIN\n"));
                } else if (( delta / (cur_time - imp_last_time)) > 
                    (IMP_PORT_MIN_RATE * 128)) {
                    DRV_RATE_SET
                        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                        0, imp_mid_rate, IMP_PORT_BURST_SIZE);
                    imp_state = IMP_PORT_STATE_RATE_MID;
                    ET_TRACE(("STATE ==> MID RATE\n"));
                }
                break;
            case IMP_PORT_STATE_RATE_MID:
                if ((delta/(cur_time - imp_last_time)) < 
                    (IMP_PORT_MIN_RATE * 128)) {
                    DRV_RATE_SET
                        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                        0, imp_max_rate, IMP_PORT_BURST_SIZE);
                    imp_state = IMP_PORT_STATE_RATE_MAX;
                    ET_TRACE(("STATE ==> RATE MAX\n"));
                } else  if (( delta / (cur_time - imp_last_time)) > 
                    (IMP_PORT_MID_RATE * 128)) {
                    DRV_RATE_SET
                        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                        0, imp_min_rate, IMP_PORT_BURST_SIZE);
                    imp_state = IMP_PORT_STATE_RATE_MIN;
                    ET_TRACE(("STATE ==> RATE MIN\n"));
                }
                break;
            case IMP_PORT_STATE_RATE_MIN:
                if ((delta/(cur_time - imp_last_time)) < 
                    (IMP_PORT_MIN_RATE * 128)) {         
                    DRV_RATE_SET
                        (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                        0, imp_mid_rate, IMP_PORT_BURST_SIZE);
                    imp_state = IMP_PORT_STATE_RATE_MID;
                    ET_TRACE(("STATE ==> RATE MID\n"));
                }
                break;
            default:
                break;
        }
        IMP_PROT_UNLOCK;
        
        /* Update time and counter */ 
        sal_memcpy(&imp_last_count, &cur_count, sizeof(uint64));
        sal_memcpy(&imp_last_time, &cur_time, sizeof(sal_time_t));
        sal_usleep(imp_pull_interval);
                
    }

    sal_thread_exit(0);
}

int
soc_imp_prot_set(int unit, int min_rate, int mid_rate, int max_rate)
{
    if ((min_rate > max_rate) || (min_rate > mid_rate)) {
        return SOC_E_PARAM;
    }
    if (mid_rate > max_rate) {
        return SOC_E_PARAM;
    }
    imp_min_rate = min_rate;
    imp_mid_rate = mid_rate;
    imp_max_rate = max_rate;

    return SOC_E_NONE;
}

void
soc_imp_prot_dump(int unit)
{
    soc_cm_print("IMP Protect :\n");
    soc_cm_print("Thread pull interval = %d ns\n", imp_pull_interval);
    soc_cm_print(" Min rate = %d kbits/sec, Middle rate = %d kbits/sec, ",
        imp_min_rate, imp_mid_rate);
    soc_cm_print("Max rate = %d kbits/sec\n ", imp_max_rate);
}


void
soc_imp_prot_init(int unit)
{

    /* Create semaphore */
    if ((imp_prot_lock = sal_mutex_create("IMP_PROT_LOCK")) == NULL) {
        ET_ERROR(("et%d: IMP Protect mutex create failed\n", unit));
        goto imp_prot_fail;
    }
    imp_pull_interval = IMP_PROT_PULL_INTERVAL;
    imp_state = IMP_PORT_STATE_RATE_MAX;
    imp_unit = unit;
    COMPILER_64_ZERO(imp_last_count);
    ET_TRACE(("soc_imp_prot_init : STATE = NONE\n"));
    
     /* Create main thread */
    imp_prot_thread_id = sal_thread_create(
            "impprot",
            SAL_THREAD_STKSZ,
            8, 
            soc_imp_prot_thread,
            INT_TO_PTR(unit)
        );
    ET_TRACE(("imp_prot_thread_id = %p\n", imp_prot_thread_id));
    if (imp_prot_thread_id == NULL){
        ET_ERROR(("imp protect init: create imp_prot_thread failed!"));
        goto imp_prot_fail;
    }
    return;
    
imp_prot_fail:
    if (imp_prot_lock != NULL) {
        sal_mutex_destroy(imp_prot_lock);
    }
    

}

#endif  /* IMP_SW_PROTECT */
