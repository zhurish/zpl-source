/*
 * $Id: dos.c,v 1.5 Broadcom SDK $
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
 * Field Processor related CLI commands
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv_if.h>
#include <soc/drv.h>

 int 
drv_bcm53115_dos_enable_set(int unit, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_DOS_NONE:
            /* Remove all preventions for DOS attack */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = 0;
            /* DRV_DOS_ICMPV6_LONG_PING */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_ICMPV4_LONG_PING */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_ICMPV6_FRAGMENTS */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_ICMPV4_FRAGMENTS */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_TCP_FRAG */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_FRAG_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_MIN_TCP_HDR_SZ */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SHORT_HDR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_SYN_WITH_SP_LT1024 */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYN_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_SYN_FIN_SCAN */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYNFIN_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_XMASS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_NULL_WITH_TCP_SEQ_ZERO */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_UDP_BLAT */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                UDP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_TCP_BLAT */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_LAND */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                IP_LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_DISABLE_LEARN */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_ICMPV6_LONG_PING:
            /* Enable to check dos type MAX_ICMPv6_Size */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_ICMPV4_LONG_PING:
            /* Enable to check dos type MAX_ICMPv4_Size */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_ICMPV6_FRAGMENTS:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_ICMPV4_FRAGMENTS:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_TCP_FRAG:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_FRAG_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SHORT_HDR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_TCP_FRAG_OFFSET:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_FRAG_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYN_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYNFIN_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_XMASS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                UDP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_TCP_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_UDP_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                UDP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_LAND:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                IP_LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 255 */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MINIMUM_TCP_HDR_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MINIMUM_TCP_HDR_SZr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = param;
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, MINIMUM_TCP_HDR_SZr, &reg_value, 
                MIN_TCP_HDR_SZf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_MAX_ICMPV6_SIZE:
            /* Set MAX_ICMPv6_Size */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MAX_ICMPV6_SIZE_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MAX_ICMPV6_SIZE_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = param;
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, MAX_ICMPV6_SIZE_REGr, &reg_value, 
                MAX_ICMPV6_SIZEf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_MAX_ICMPV4_SIZE:
            /* Set MAX_ICMPv4_Size */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MAX_ICMPV4_SIZE_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MAX_ICMPV4_SIZE_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = param;
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, MAX_ICMPV4_SIZE_REGr, &reg_value, 
                MAX_ICMPV4_SIZEf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_DISABLE_LEARN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

int 
drv_bcm53115_dos_enable_get(int unit, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_DOS_ICMPV6_LONG_PING:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_ICMPV4_LONG_PING:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_LONG_PING_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_ICMPV6_FRAGMENTS:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV6_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_ICMPV4_FRAGMENTS:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                ICMPV4_FRAGMENT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_TCP_FRAG:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_FRAG_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to get register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_TCP_FRAG_OFFSET:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_FRAG_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to get register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYN_ERR_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_SYNFIN_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_XMASS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ( (rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_BLAT:
            /* Just need to get one of the cases */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_TCP_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                TCP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_UDP_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                UDP_BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_LAND:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                IP_LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 255 */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MINIMUM_TCP_HDR_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MINIMUM_TCP_HDR_SZr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, MINIMUM_TCP_HDR_SZr, &reg_value, 
                MIN_TCP_HDR_SZf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            *param = temp;
            break;
        case DRV_DOS_MAX_ICMPV6_SIZE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MAX_ICMPV6_SIZE_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MAX_ICMPV6_SIZE_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, MAX_ICMPV6_SIZE_REGr, &reg_value, 
                MAX_ICMPV6_SIZEf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            *param = temp;
            break;
        case DRV_DOS_MAX_ICMPV4_SIZE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MAX_ICMPV4_SIZE_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MAX_ICMPV4_SIZE_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, MAX_ICMPV4_SIZE_REGr, &reg_value, 
                MAX_ICMPV4_SIZEf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            *param = temp;
            break;
        case DRV_DOS_DISABLE_LEARN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}
