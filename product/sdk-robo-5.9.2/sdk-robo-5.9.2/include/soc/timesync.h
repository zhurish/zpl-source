/*
 * $Id: timesync.h,v 1.2 Broadcom SDK $
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
 * This file defines common network port modes.
 *
 * Its contents are not used directly by applications; it is used only
 * by header files of parent APIs which need to define port modes.
 */

#ifndef _SOC_TIMESYNC_H
#define _SOC_TIMESYNC_H

#include <shared/port.h>


#define SOC_PORT_TIMESYNC_ENABLE                 _SHR_PORT_TIMESYNC_ENABLE
#define SOC_PORT_TIMESYNC_CAPTURE_TS_ENABLE      _SHR_PORT_TIMESYNC_CAPTURE_TS_ENABLE
#define SOC_PORT_TIMESYNC_HEARTBEAT_TS_ENABLE    _SHR_PORT_TIMESYNC_HEARTBEAT_TS_ENABLE
#define SOC_PORT_TIMESYNC_ONE_STEP               _SHR_PORT_TIMESYNC_ONE_STEP
#define SOC_PORT_TIMESYNC_RX_CRC_ENABLE          _SHR_PORT_TIMESYNC_RX_CRC_ENABLE
#define SOC_PORT_TIMESYNC_8021AS_ENABLE          _SHR_PORT_TIMESYNC_8021AS_ENABLE
#define SOC_PORT_TIMESYNC_L2_ENABLE              _SHR_PORT_TIMESYNC_L2_ENABLE
#define SOC_PORT_TIMESYNC_IP4_ENABLE             _SHR_PORT_TIMESYNC_IP4_ENABLE
#define SOC_PORT_TIMESYNC_IP6_ENABLE             _SHR_PORT_TIMESYNC_IP6_ENABLE
#define SOC_PORT_TIMESYNC_CLOCK_SRC_EXT          _SHR_PORT_TIMESYNC_CLOCK_SRC_EXT


#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP

#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP
#define SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME \
        _SHR_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME

#define SOC_PORT_TIMESYNC_MODE_FREE \
        _SHR_PORT_TIMESYNC_MODE_FREE
#define SOC_PORT_TIMESYNC_MODE_SYNCIN \
        _SHR_PORT_TIMESYNC_MODE_SYNCIN
#define SOC_PORT_TIMESYNC_MODE_CPU \
        _SHR_PORT_TIMESYNC_MODE_CPU

#define SOC_PORT_CONTROL_TIMESYNC_CAPTURE_TIMESTAMP \
        _SHR_PORT_CONTROL_TIMESYNC_CAPTURE_TIMESTAMP
#define SOC_PORT_CONTROL_TIMESYNC_HEARTBEAT_TIMESTAMP \
        _SHR_PORT_CONTROL_TIMESYNC_HEARTBEAT_TIMESTAMP
#define SOC_PORT_CONTROL_TIMESYNC_NCOADDEND \
        _SHR_PORT_CONTROL_TIMESYNC_NCOADDEND
#define SOC_PORT_CONTROL_TIMESYNC_EXT_PHASEADJUST \
        _SHR_PORT_CONTROL_TIMESYNC_EXT_PHASEADJUST
#define SOC_PORT_CONTROL_TIMESYNC_FRAMESYNC \
        _SHR_PORT_CONTROL_TIMESYNC_FRAMESYNC
#define SOC_PORT_CONTROL_TIMESYNC_LOCAL_TIME \
        _SHR_PORT_CONTROL_TIMESYNC_LOCAL_TIME

/* time_spec type */
typedef _shr_time_spec_t soc_time_spec_t;

/* Actions on Egress event messages */
typedef _shr_port_timesync_event_message_egress_mode_t soc_port_timesync_event_message_egress_mode_t;

/* Actions on ingress event messages */
typedef _shr_port_timesync_event_message_ingress_mode_t soc_port_timesync_event_message_ingress_mode_t;

/* Global mode actions */
typedef _shr_port_timesync_global_mode_t soc_port_timesync_global_mode_t;

/* Fast call actions */
typedef _shr_port_control_timesync_t soc_port_control_timesync_t;

/* Base timesync configuration type. */
typedef struct soc_port_timesync_config_s {
    uint32 flags;                       /* Flags BCM_PORT_TIMESYNC_* */
    uint16 itpid;                       /* 1588 inner tag */
    uint16 otpid;                       /* 1588 outer tag */
    soc_port_timesync_global_mode_t gmode; /* Global mode */
    soc_time_spec_t original_timecode;  /* Original timecode to be inserted */
    uint32 tx_timestamp_offset;         /* TX AFE delay in ns - per port */
    uint32 rx_timestamp_offset;         /* RX AFE delay in ns - per port */
    soc_port_timesync_event_message_egress_mode_t tx_sync_mode; /* sync */
    soc_port_timesync_event_message_egress_mode_t tx_delay_request_mode; /* delay request */
    soc_port_timesync_event_message_egress_mode_t tx_pdelay_request_mode; /* pdelay request */
    soc_port_timesync_event_message_egress_mode_t tx_pdelay_response_mode; /* pdelay response */
    soc_port_timesync_event_message_ingress_mode_t rx_sync_mode; /* sync */
    soc_port_timesync_event_message_ingress_mode_t rx_delay_request_mode; /* delay request */
    soc_port_timesync_event_message_ingress_mode_t rx_pdelay_request_mode; /* pdelay request */
    soc_port_timesync_event_message_ingress_mode_t rx_pdelay_response_mode; /* pdelay response */
} soc_port_timesync_config_t;

/* soc_port_timesync_config_set */
extern int soc_port_timesync_config_set(
    int unit, 
    soc_port_t port, 
    soc_port_timesync_config_t *conf);

/* soc_port_timesync_config_get */
extern int soc_port_timesync_config_get(
    int unit, 
    soc_port_t port, 
    soc_port_timesync_config_t *conf);

/* soc_port_control_timesync_set */
extern int soc_port_control_timesync_set(
    int unit, 
    soc_port_t port, 
    soc_port_control_timesync_t type, 
    uint64 value);

/* soc_port_control_timesync_get */
extern int soc_port_control_timesync_get(
    int unit, 
    soc_port_t port, 
    soc_port_control_timesync_t type, 
    uint64 *value);

#endif  /* !_SOC_TIMESYNC_H */
