/*
 * $Id: lplist.h,v 1.11 Broadcom SDK $
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
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __BCMX_LPLIST_H__
#define __BCMX_LPLIST_H__

#include <bcm/types.h>
#include <bcmx/types.h>

/* 
 * Deprecated flags to indicate groups of ports.
 * 
 * Use BCMX_PORT_LP_* instead.
 */
#define BCMX_LP_ALL             (1 << 0)   /* 0x00000001 */
#define BCMX_LP_FE              (1 << 1)   /* 0x00000002 */
#define BCMX_LP_GE              (1 << 2)   /* 0x00000004 */
#define BCMX_LP_XE              (1 << 3)   /* 0x00000008 */
#define BCMX_LP_HG              (1 << 4)   /* 0x00000010 */

/* 
 * Deprecated flags.
 * 
 * BCMX_LP_UNIQ:  Elements appear on the list at most once.
 * BCMX_LP_SORT:  Keep the list sorted by bcmx_lport value
 * BCMX_LP_USER is a flag that can be used at the application level
 */
#define BCMX_LP_UNIQ            (1 << 5)   /* 0x00000020 */
#define BCMX_LP_SORT            (1 << 6)   /* 0x00000040 */
#define BCMX_LP_USER            (1 << 8)   /* 0x00000100 */

/* BCMX Logical Port List */
typedef struct bcmx_lplist_s {
    int lp_last;            /* Index of last elt on list. -1 --> empty (only
                               significant if list is not null). */
    int lp_alloc;           /* How many spaces currently allocated. */
    bcmx_lport_t *lp_ports; /* Pointer to the ports. */
} bcmx_lplist_t;

/* 
 * Logical Port List macros.
 * 
 *      BCMX_LPLIST_ADD(list, lport)
 *           Add an element to the end of the list; doesn't check
 *           for sorted or uniquness.
 *           Alias for function w/ same (lower case) name
 *      BCMX_LPLIST_REMOVE(list, lport)
 *           Search for the first occurrance of lport on the list
 *           and remove it if found.
 *      BCMX_LPLIST_IDX_REMOVE(list, idx)
 *           Remove the lport at idx
 *      BCMX_LPLIST_ITER(list, lport, count)
 *           Iterate through the list (passed by value)
 *           in order of the array
 *      BCMX_LPLIST_IDX_ITER(list, lport, count)
 *           Iterate through the list (passed by reference)
 *           in order of the array
 *      BCMX_LPLIST_IS_NULL
 *           Boolean: Is list invalid?
 *      BCMX_LPLIST_IS_EMPTY
 *           Boolean:  Is list invalid or has no elements?
 *      BCMX_LPLIST_COUNT
 *           Returns the number of elements on the list
 */
#define BCMX_LPLIST_IS_NULL(list)  \
    (((list) == NULL) || ((list)->lp_ports == NULL)) 
#define BCMX_LPLIST_IS_EMPTY(list)  \
    (BCMX_LPLIST_IS_NULL(list) || ((list)->lp_last < 0)) 
#define BCMX_LPLIST_COUNT(list)  \
    (BCMX_LPLIST_IS_EMPTY(list) ? 0 : ((list)->lp_last + 1)) 
#define BCMX_LPLIST_IDX_REMOVE(list, idx)  do {                 \
    int _i;                                                     \
                                                                \
    if (((idx) >= 0) && ((idx) <= (list)->lp_last)) {           \
        for (_i = (idx); _i < (list)->lp_last; _i++) {          \
            (list)->lp_ports[_i] = (list)->lp_ports[_i + 1];    \
        }                                                       \
        ((list)->lp_last)--;                                    \
    }                                                           \
} while (0) 
#define BCMX_LPLIST_REMOVE(list, lport)  do {                   \
    int _idx;                                                   \
                                                                \
    _idx = bcmx_lplist_index_get(list, lport);                  \
    if (_idx >= 0) {                                            \
        BCMX_LPLIST_IDX_REMOVE(list, _idx);                     \
    }                                                           \
} while (0) 
#define BCMX_LPLIST_ADD(list, lport)  \
    bcmx_lplist_add(list, lport) /* Alias bcmx_lplist_add as a macro. */
#define BCMX_LPLIST_IDX_ITER(list_p, lport, count)               \
    for ((lport) = (list_p)->lp_ports[0], (count) = 0;           \
         (count) <= (list_p)->lp_last;                           \
         (lport) = (list_p)->lp_ports[++(count)]) /* NOTE:  List is a pointer here; not
                                              a pointer below.  Ick. */

/* 
 * Iterate across a port list.
 *    Notes:  If a port is on the list twice, it will
 *    occur twice in the loop.
 *    Does not support flags in the port list (ALL, FE, GE....)
 * NOTE:  List is NOT a pointer here; it is a pointer above.  Ick.
 */
#define BCMX_LPLIST_ITER(list, lport, count)  \
    BCMX_LPLIST_IDX_ITER(&(list), lport, count) 

/* 
 * Logical Port List functions.
 * 
 *   int  bcmx_lplist_init(list, init_count, flags)
 *   void bcmx_lplist_t_init(list)
 *      Initialize the lplist.
 *   int  bcmx_lplist_free(list)
 *   void bcmx_lplist_t_free(list)
 *      Free an initialized lplist.
 *   int bcmx_lplist_clear(list)
 *      Clear an lplist.
 *   int bcmx_lplist_index_get(list, lport)
 *      Return the index of lport
 *   int bcmx_lplist_index_get_from(list, position, port)
 *      Return the index of lport starting at position
 *   bcmx_lport_t bcmx_lplist_index(list, position)
 *      Return the lport at position
 *   int bcmx_lplist_add(list, lport)
 *      Add lport to the end of list
 *   int bcmx_lplist_port_remove(list, lport, all)
 *      Remove first or all lport from list
 *   int bcmx_lplist_eq(list1, list2)
 *      Return 1 if lists have the same members (O(N^2))
 *   int bcmx_lplist_append(list1, list2)
 *      Append list2 to list2
 *   int bcmx_lplist_copy(dest, src)
 *      Copy src to dest
 *   int bcmx_lplist_check(list)
 *      Check list consistency
 *   int bcmx_lplist_range(list, start, end)
 *      Add range of lports to list
 * 
 * Compatibility
 * 
 *   bcmx_lplist_first(list)
 *      Return first lport in list
 *   bcmx_lplist_last_insert(list, port)
 *      Same as bcmx_lplist_add()
 * 
 * Removed for SDK 5.3.0
 * 
 *   BCMX_LP_ALL
 *   BCMX_LP_FE
 *   BCMX_LP_GE
 *   BCMX_LP_XE
 *   BCMX_LP_HG
 * 
 *     BCMX_PORT_LP_* and bcmx_port_lplist_populate provide equivalent
 *     functionality.
 * 
 * 
 *   BCMX_LP_SORT
 *   BCMX_LP_UNIQ
 * 
 *     Use bcmx_lplist_sort() and bcmx_lplist_uniq() to sort and
 *     make lists contail unique members.
 * 
 * 
 *   BCMX_LP_USER
 * 
 *     No equivalent functionality provided.
 * 
 *   bcmx_lplist_current_insert()
 *   bcmx_lplist_current_delete()
 *   bcmx_lplist_last_insert()
 *   bcmx_lplist_insert()
 *   bcmx_lplist_next()
 *   bcmx_lplist_current()
 *   bcmx_lplist_merge()
 *   BCMX_LPL_DEF_LEN
 * 
 *   No equivalent functionality provided.
 */

extern int bcmx_lplist_init(
    bcmx_lplist_t *list, 
    int init_count, 
    uint32 flags);

extern int bcmx_lplist_free(
    bcmx_lplist_t *list);

extern int bcmx_lplist_clear(
    bcmx_lplist_t *list);

extern void bcmx_lplist_t_init(
    bcmx_lplist_t *list);

extern void bcmx_lplist_t_free(
    bcmx_lplist_t *list);

extern int bcmx_lplist_index_get(
    bcmx_lplist_t *list, 
    bcmx_lport_t port);

extern int bcmx_lplist_index_get_from(
    bcmx_lplist_t *list, 
    int position, 
    bcmx_lport_t port);

extern bcmx_lport_t bcmx_lplist_index(
    bcmx_lplist_t *list, 
    int position);

/* Compatibility. */
#define bcmx_lplist_first(list)  bcmx_lplist_index((list), 0) 
#define bcmx_lplist_last_insert(list, port)  bcmx_lplist_add((list), (port)) 

extern int bcmx_lplist_add(
    bcmx_lplist_t *list, 
    bcmx_lport_t lport);

extern int bcmx_lplist_port_remove(
    bcmx_lplist_t *list, 
    bcmx_lport_t lport, 
    int all);

extern int bcmx_lplist_eq(
    bcmx_lplist_t *list1, 
    bcmx_lplist_t *list2);

extern int bcmx_lplist_append(
    bcmx_lplist_t *list1, 
    bcmx_lplist_t *list2);

extern int bcmx_lplist_copy(
    bcmx_lplist_t *dest, 
    bcmx_lplist_t *src);

extern int bcmx_lplist_check(
    bcmx_lplist_t *list);

extern int bcmx_lplist_range(
    bcmx_lplist_t *list, 
    bcmx_lport_t start, 
    bcmx_lport_t end);

extern int bcmx_lplist_is_null(
    bcmx_lplist_t *list);

extern int bcmx_lplist_is_empty(
    bcmx_lplist_t *list);

extern int bcmx_lplist_count(
    bcmx_lplist_t *list);

extern void bcmx_lplist_remove(
    bcmx_lplist_t *list, 
    int lport);

extern void bcmx_lplist_idx_remove(
    bcmx_lplist_t *list, 
    int idx);

extern int bcmx_lplist_pbmp_add(
    bcmx_lplist_t *list, 
    int unit, 
    bcm_pbmp_t *pbm);

extern void bcmx_lplist_to_pbmp(
    bcmx_lplist_t *list, 
    int unit, 
    bcm_pbmp_t *pbm);

extern int bcmx_lplist_sort(
    bcmx_lplist_t *list);

extern int bcmx_lplist_uniq(
    bcmx_lplist_t *list);

extern int _bcmx_lplist_pbmp_add(
    bcmx_lplist_t *list, 
    int unit, 
    bcm_pbmp_t pbm);

/* 
 * LP List to port bitmap function/macros.
 * 
 *     BCMX_LPLIST_TO_PBMP   -- Extract ports from an lplist to a bitmap
 *                              Procedural (no return value)
 *     BCMX_LPLIST_PBMP_ADD  -- Add ports from a bitmap to an lplist
 *                              Returns BCM_E_XXX.
 * 
 * Note:  These macros are inefficient and may be replaced with a
 * better implementation (with lplist changes) in the future.
 */
#define BCMX_LPLISTPTR_TO_PBMP(_lplist, _unit, _pbm)  do {               \
    bcmx_lport_t _lport;                                                 \
    int _count;                                                          \
    int _cpu_port;                                                       \
    int _bcm_unit;                                                       \
    bcm_port_t _bcm_port;                                                \
    BCM_PBMP_CLEAR(_pbm);                                                \
    BCMX_LPLIST_IDX_ITER(((_lplist)), _lport, _count) {                  \
        if (_lport == BCMX_LPORT_LOCAL_CPU) {                            \
            if (bcmx_lport_local_cpu_get(_unit, &_cpu_port) >= 0) {      \
                BCM_PBMP_PORT_ADD(_pbm, BCMX_LPORT_BCM_PORT(_cpu_port)); \
            }                                                            \
        } else if (BCMX_LPORT_VALID(_lport)) {                           \
            bcmx_lport_to_unit_port(_lport, &_bcm_unit, &_bcm_port);     \
            if (_bcm_unit == _unit) {                                    \
                BCM_PBMP_PORT_ADD(_pbm, _bcm_port);                      \
            }                                                            \
        }                                                                \
    }                                                                    \
} while (0) 
#define BCMX_LPLIST_TO_PBMP(_lplist, _unit, _pbm)  \
    BCMX_LPLISTPTR_TO_PBMP(&_lplist, _unit, _pbm) 
#define BCMX_LPLIST_PBMP_ADD(_lplist_p, _unit, _pbm)  \
    _bcmx_lplist_pbmp_add(_lplist_p, _unit, _pbm) 

#endif /* __BCMX_LPLIST_H__ */
