/*
 * $Id: cint_sdk_atomics.c,v 1.20 Broadcom SDK $
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
 *
 */
#include "cint_sdk_atomics.h"
#include <cint_porting.h>
#include <cint_internal.h>

int cint_sdk_atomics_not_empty; 

#ifdef INCLUDE_LIB_CINT

#include <sal/core/alloc.h>
#include <sal/core/libc.h>
#include <sal/appl/io.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <bcm/types.h>
#include <shared/util.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <cint_error.h>

#include <bcm/rx.h>
#include <bcm/port.h>

/***********************************************************************
 *
 * These are the atomic hand-coded type handlers for the SDK datatypes
 * 
 * These structures are fed into the core cint library along
 * with the autogenerated API data. 
 *
 **********************************************************************/

static int
__cint_set_pbmp_t(void* p, const char* expr)
{
    char tmp[64];     
    char * s = tmp; 
    int port;
    int c;
    int open_brace, close_brace;
    char *end;
    bcm_pbmp_t* pp = (bcm_pbmp_t*)p;

    BCM_PBMP_CLEAR(*pp);
    s = tmp;
    open_brace = close_brace = 0;
    while((c=*expr++) != 0) {
        switch (c) {
        case '{':
            open_brace++;
            break;
            
        case '}':
            close_brace++;
            /* fall through */
        case ' ':
            /* convert non-empty integer port expression */
            *s = 0;
            if (tmp[0]) {
                port = sal_ctoi(tmp, &end);
                if (*end != 0 || tmp == end ||
                    port < 0 || port >= BCM_PBMP_PORT_MAX) {
                    return 1;
                }
                BCM_PBMP_PORT_ADD(*pp,port); 
                s = tmp;
            }
            break;

        default:
            /* copy */
            if ((s-tmp) < (sizeof(tmp)-1)) {
                *s++ = c;
            } else {
                /* tmp overflow */
                return 1;
            }
            break;
        }
    }
    return !(open_brace == 1 && close_brace == 1); 
}

static int
__cint_format_pbmp_t(void* p, char* dst, int size, cint_atomic_format_t format)
{
    bcm_pbmp_t* pp = (bcm_pbmp_t*)p; 
    int port; 

    cint_snprintf_ex(&dst, &size, "{ "); 
    BCM_PBMP_ITER((*pp), port) {
        cint_snprintf_ex(&dst, &size, "%d ", port); 
    }
    cint_snprintf_ex(&dst, &size, "}"); 
    return 0; 
}

static int
__cint_set_bcm_mac_t(void* p, const char* expr)
{
    bcm_mac_t m = { 0, 0, 0, 0, 0, 0 }; 
    char buffer[16] = "0x"; 
    
    const char* s = expr; 
    char* d = buffer+2; 
    int i = 0; 
    char *end;

    for(s = expr;; s++) {
        if(*s == ':' || *s == 0) {            
            if(i <= 5) {
                m[i++] = sal_ctoi(buffer, &end); 
                if (*end != 0 || buffer == end) {
                    return 1;
                }
            }   
            d = buffer+2; 
            if(*s == 0) {
                break; 
            }   
        }       
        else {
            *d++ = *s; 
        }       
    } 
    
    
    CINT_MEMCPY(p, m, sizeof(m)); 
    return 0; 
}

static int
__cint_format_bcm_mac_t(void* p, char* dst, int size, cint_atomic_format_t format)
{
    unsigned char* m = (unsigned char*) p;     
    cint_snprintf_ex(&dst, &size, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", 
                     m[0], m[1], m[2], m[3], m[4], m[5]); 
    return 0; 
}


static int
__cint_set_uint64(void* p, const char* expr)
{
    uint64* u64 = (uint64*)p; 
    char slo[16] = { 0 }; 
    char shi[16] = { 0 }; 

    const char* s; 
    char* d; 
    char *end;

    /*
     * Format: "number" -- sets low
     * Format: "number:number" -- sets hi and low
     */
    uint32 lo = 0; 
    uint32 hi = 0; 
    
    d = shi;
    for(s = expr; *s; s++) {
        if(*s == ':' || *s == ',' || *s == ' ') {
            *d = 0; 
            d = slo; 
        }
        else {
            *d++ = *s;
        }       
    }
    *d = 0; 

    if(slo[0] && shi[0]) {
        /* Both specifier */
        lo = sal_ctoi(slo, &end);
        if (*end != 0 || slo == end) {
            return 1;
        }
        hi = sal_ctoi(shi, &end); 
        if (*end != 0 || shi == end) {
            return 1;
        }
    }   
    else {
        lo = sal_ctoi(shi, &end); 
        if (*end != 0 || shi == end) {
            return 1;
        }
        hi = 0; 
    }

    COMPILER_64_SET(*u64, hi, lo); 
    return 0;
}

static int
__cint_format_uint64(void* p, char* dst, int size, cint_atomic_format_t format)
{
    uint64* u64 = (uint64*)p; 
    uint32 hi; 
    uint32 lo;
    COMPILER_64_TO_32_LO(lo, *u64); 
    COMPILER_64_TO_32_HI(hi, *u64); 
    cint_snprintf_ex(&dst, &size, "{0x%.8X 0x%.8X}", hi, lo); 
    return 0; 
}


static int
__cint_format_bcm_rx_reasons_t(void* p, char* dst, int size, cint_atomic_format_t format)
{
    bcm_rx_reason_t r; 
    bcm_rx_reasons_t* rs = (bcm_rx_reasons_t*) p;     
    static char* rnames[] = BCM_RX_REASON_NAMES_INITIALIZER; 
    
    if(BCM_RX_REASON_IS_NULL(*rs)) {
        cint_snprintf_ex(&dst, &size, "None"); 
    }   
    else {
        BCM_RX_REASON_ITER((*rs),r) {
            cint_snprintf_ex(&dst, &size, "%s ", rnames[r]); 
        }
    }   
    return 0; 
}

static int
__cint_set_bcm_rx_reasons_t(void* p, const char* expr)
{
    return -1; 
}

/*
 * This is the exported table for our datatypes
 */
cint_atomic_type_t cint_sdk_atomics[] = 
    {
        {
            "uint64", 
            sizeof(uint64), 
            0, 
            __cint_format_uint64,
            __cint_set_uint64
        },
        {
            "int64", 
            sizeof(uint64), 
            0, 
            __cint_format_uint64,
            __cint_set_uint64
        },
        {
            "bcm_pbmp_t", 
            sizeof(bcm_pbmp_t),
            0, 
            __cint_format_pbmp_t, 
            __cint_set_pbmp_t,
        },      
        {
            "bcm_mac_t", 
            sizeof(bcm_mac_t),
            CINT_ATOMIC_TYPE_F_CAP_ONLY, 
            __cint_format_bcm_mac_t, 
            __cint_set_bcm_mac_t,
        },      
        {
            "bcm_rx_reasons_t", 
            sizeof(bcm_rx_reasons_t), 
            0, 
            __cint_format_bcm_rx_reasons_t, 
            __cint_set_bcm_rx_reasons_t, 
        },
        { NULL }, 
    }; 


static char* __macro__bcm_errmsg(int rc) 
{ 
    return bcm_errmsg(rc); 
}
CINT_FWRAPPER_CREATE_RP1(char*,char,1,0,
                         __macro__bcm_errmsg,
                         int,int,rc,0,0); 

static cint_function_t __cint_sdk_functions[] = 
    {
        CINT_FWRAPPER_NENTRY("bcm_errmsg", __macro__bcm_errmsg), 
        CINT_ENTRY_LAST
    }; 



static cint_parameter_desc_t __cint_sdk_typedefs[] = 
    {
        { "int", "bcm_port_mdix_t", 0, 0 }, 
        { "char", "uint8", 0, 0 }, 
        { "char", "int8", 0, 0 }, 
        { "short", "uint16", 0, 0 }, 
        { "int", "uint32", 0, 0 }, 
        { "int", "int32", 0, 0 }, 
        { "uint32", "SHR_BITDCL", 0, 0 }, 
        { "bcm_pbmp_t", "soc_pbmp_t", 0, 0 }, 
        { "bcm_cos_t", "soc_cos_t", 0, 0 }, 
        { "bcm_mac_t", "const bcm_mac_t", 0, 0 }, 

        /* 
         * These are "broken" API definitions. 
         * Enumerations and structures are defined in <shared> and 
         * #defined or typedef'ed to the BCM API equivalents. 
         *
         * The structures are defined here by hand. 
         * The enumerations are just typed as "int". The enumeration names are not available. 
         *
         */           
        { "int", "_shr_module_t", 0, 0 }, 
        { "int", "_shr_port_mode_t", 0, 0 }, 
        { "int", "_shr_pa_encap_t", 0, 0 }, 
        { "int", "_shr_port_mdix_t", 0, 0 }, 
        { "int", "_shr_port_mdix_status_t", 0, 0 }, 
        { "int", "_shr_dma_chan_t", 0, 0 }, 
        { "int", "_shr_port_stp_t", 0, 0 }, 
        { "int", "_shr_port_phy_control_t", 0, 0 }, 
        { "int", "_shr_port_cable_state_t", 0, 0 }, 
        { "int", "_shr_port_encap_t", 0, 0 }, 
        { "int", "_shr_port_mcast_flood_t", 0, 0 }, 
        { "int", "_shr_port_medium_t", 0, 0 }, 
        { "int", "_shr_port_duplex_t", 0, 0 }, 
        { "int", "_shr_port_if_t", 0, 0 }, 
        { "int", "_shr_port_ms_t", 0, 0 }, 


        { NULL }, 
    }; 


/*
 * Custom Language extensions for BCM macros
 */
cint_ast_t* 
__BCM_PBMP_ITER_HANDLER(const char* name, cint_ast_t* arguments, cint_ast_t* statements)
{
    cint_ast_t* _for; 
    cint_ast_t* _if; 
    cint_ast_t* _condition;
    cint_ast_t* arg0;
    cint_ast_t* arg1; 

    /*
     * We only take two arguments
     */
    if(cint_ast_count(arguments) != 2) {
        cint_ast_error(arguments, CINT_E_BAD_AST, "wrong number of arguments to %s() -- expected 2, recieved %d", 
                       name, cint_ast_count(arguments)); 
        return NULL;
    }   
    arg0 = arguments; 
    arg1 = arguments->next; 

    /*
     * This returns the following code tree:
     *
     * for(arg1 = 0; arg1 < BCM_PBMP_PORT_MAX; arg1++) {
     *    if(BCM_PBMP_MEMBER(arg0, arg1) 
     *       [statements]
     *
     * to simulate BCM_PBMP_ITER()
     *
     */
    

    /* Function call "BCM_PBMP_MEMBER(arg0, arg1)" */
    _condition = cint_ast_function(cint_ast_identifier("BCM_PBMP_MEMBER"), arguments); 
              
    /* if(BCM_PBMP_MEMBER(pbmp,port)) { statements } */
    _if = cint_ast_if(_condition, statements, 0); 
              
    /* for(arg1 = 0; arg1 < BCM_PBMP_PORT_MAX; arg1++) (condition) */
    _for = cint_ast_for( /* arg1 = 0 */       
                        cint_ast_operator(cintOpAssign,      
                                          arg1, 
                                          cint_ast_integer(0)), 
                        /* arg1 < BCM_PBMP_PORT_MAX */
                        cint_ast_operator(cintOpLessThan, 
                                          arg1,
                                          cint_ast_identifier("BCM_PBMP_PORT_MAX")), 

                        /* arg1++ */
                        cint_ast_operator(cintOpAssign, 
                                          arg1, 
                                          cint_ast_operator(cintOpAdd, 
                                                            arg1, 
                                                            cint_ast_integer(1))),

                        /* statements */
                        _if); 
    
    return _for; 
}

cint_ast_t* 
__BCM_RX_REASON_ITER_HANDLER(const char* name, cint_ast_t* arguments, cint_ast_t* statements)
{
    cint_ast_t* _for; 
    cint_ast_t* _if; 
    cint_ast_t* _condition;
    cint_ast_t* arg0;
    cint_ast_t* arg1; 

    /*
     * We only take two arguments
     */
    if(cint_ast_count(arguments) != 2) {
        cint_ast_error(arguments, CINT_E_BAD_AST, "wrong number of arguments to %s() -- expected 2, recieved %d", 
                       name, cint_ast_count(arguments)); 
        return NULL;
    }   
    arg0 = arguments; 
    arg1 = arguments->next; 

    /*
     * This returns the following code tree:
     *
     * for(arg1 = bcmRxReasonInvalid; arg1 < bcmRxReasonCount; arg1++) {
     *    if(BCM_RX_REASON_GET(arg0, arg1) 
     *       [statements]
     *
     * to simulate BCM_RX_REASON_ITER()
     *
     */
    

    /* Function call "BCM_PBMP_MEMBER(arg0, arg1)" */
    _condition = cint_ast_function(cint_ast_identifier("BCM_RX_REASON_GET"), arguments); 
              
    /* if(BCM_RX_REASON_GET(arg0,arg1)) { statements } */
    _if = cint_ast_if(_condition, statements, 0); 
              
    /* for(arg1 = bcmRxReasonInvalid; arg1 < bcmRxReasonCount; arg1++) (condition) */
    _for = cint_ast_for( /* arg1 = bcmRxReasonInvalid */       
                        cint_ast_operator(cintOpAssign,      
                                          arg1, 
                                          cint_ast_integer(bcmRxReasonInvalid)), 
                        /* arg1 < bcmRxReasonCount */
                        cint_ast_operator(cintOpLessThan, 
                                          arg1,
                                          cint_ast_integer(bcmRxReasonCount)), 

                        /* arg1++ */
                        cint_ast_operator(cintOpAssign, 
                                          arg1, 
                                          cint_ast_operator(cintOpAdd, 
                                                            arg1, 
                                                            cint_ast_integer(1))),

                        /* statements */
                        _if); 
    
    return _for; 
}

static cint_ast_t*
__BCM_IF_ERROR_RETURN_HANDLER(const char* name, cint_ast_t* arguments)
{
    /* Inserts the following code:
       
    do { int __rv__; if ((__rv__ = (arguments)) < 0) return(__rv__); } while(0)

       This is equivalent to the BCM_IF_ERROR_RETURN() macro 
    */
    cint_ast_t* _while; 
    cint_ast_t* __rv__ = cint_ast_identifier("__rv__"); 
    cint_ast_t* _decl_rv = cint_ast_declaration(); 
    cint_ast_t* _if; 
    cint_ast_t* _statements; 

    /* Declare __rv__ = (arguments) */
    _decl_rv->utype.declaration.type = cint_ast_type("int"); 
    _decl_rv->utype.declaration.pcount = 0; 
    _decl_rv->utype.declaration.array = NULL; 
    _decl_rv->utype.declaration.identifier = __rv__; 
    _decl_rv->utype.declaration.init = arguments; 
    
    _if = cint_ast_if(cint_ast_operator(cintOpLessThan, 
                                        __rv__, 
                                        cint_ast_integer(0)), 
                      cint_ast_return(__rv__), 
                      NULL); 

    /* Make the if statement come after the declaration */
    cint_ast_append(_decl_rv, _if); 
                      
    /* Need braces around statement list */
    _statements = cint_ast_operator(cintOpOpenBrace, 0, 0); 
    _statements->next = _decl_rv; 
    cint_ast_append(_statements, cint_ast_operator(cintOpCloseBrace, 0, 0)); 

    _while = cint_ast_while(/* while(0) */
                            cint_ast_integer(0), 
                            
                            /* statements */
                            _statements, 

                            /* order */
                            1); 
    return _while; 
}

static cint_custom_iterator_t __sdk_custom_iterators[] = 
    {
        { "BCM_PBMP_ITER", __BCM_PBMP_ITER_HANDLER },
        { "BCM_RX_REASON_ITER", __BCM_RX_REASON_ITER_HANDLER }, 
        { NULL }
    }; 

static cint_custom_macro_t __sdk_custom_macros[] = 
    {
        { "BCM_IF_ERROR_RETURN", __BCM_IF_ERROR_RETURN_HANDLER }, 
        { NULL }
    }; 



/*
 * Structures defined in <shared> which are #defined or typedef'ed to BCM datatypes. 
 * These have to be handled manually. 
 */
static void*
__cint_maddr__shr_port_ability_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    _shr_port_ability_t* s = (_shr_port_ability_t*) p;
    
    switch(mnum)
        {
        case 0: rv = &(s->speed_half_duplex); break;
        case 1: rv = &(s->speed_full_duplex); break;
        case 2: rv = &(s->pause); break;
        case 3: rv = &(s->interface); break;
        case 4: rv = &(s->medium); break;
        case 5: rv = &(s->loopback); break;
        case 6: rv = &(s->flags); break;
        case 7: rv = &(s->encap); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_parameter_desc_t __cint_struct_members__shr_port_ability_t[] = 
    {
        { "_shr_port_mode_t", "speed_half_duplex", 0, 0 },
        { "_shr_port_mode_t", "speed_full_duplex", 0, 0 },
        { "_shr_port_mode_t", "pause", 0, 0 },
        { "_shr_port_mode_t", "interface", 0, 0 },
        { "_shr_port_mode_t", "medium", 0, 0 }, 
        { "_shr_port_mode_t", "loopback", 0, 0 },
        { "_shr_port_mode_t", "flags", 0, 0 },
        { "_shr_port_mode_t", "encap", 0, 0 },
        { NULL }
    }; 

static void*
__cint_maddr__shr_port_cable_diag_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    _shr_port_cable_diag_t* s = (_shr_port_cable_diag_t*) p;
    
    switch(mnum)
        {
        case 0: rv = &(s->state); break;
        case 1: rv = &(s->npairs); break;
        case 2: rv = &(s->pair_state); break;
        case 3: rv = &(s->pair_len); break;
        case 4: rv = &(s->fuzz_len); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_parameter_desc_t __cint_struct_members__shr_phy_config_t[] = 
    {
        { "int", "enable", 0, 0 }, 
        { "int", "preferred", 0, 0 }, 
        { "int", "autoneg_enable", 0, 0 }, 
        { "_shr_port_mode_t", "autoneg_advert", 0, 0 }, 
        { "_shr_port_ability_t", "advert_ability", 0, 0 }, 
        { "int", "force_speed", 0, 0 }, 
        { "int", "force_duplex", 0, 0 },
        { "int", "master", 0, 0 }, 
        { "_shr_port_mdix_t", "mdix", 0, 0 },         
        { NULL }
    }; 

static void*
__cint_maddr__shr_phy_config_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    _shr_phy_config_t* s = (_shr_phy_config_t*) p;
    
    switch(mnum)
        {
        case 0: rv = &(s->enable); break;
        case 1: rv = &(s->preferred); break;
        case 2: rv = &(s->autoneg_enable); break;
        case 3: rv = &(s->autoneg_advert); break;
        case 4: rv = &(s->advert_ability); break;
        case 5: rv = &(s->force_speed); break;
        case 6: rv = &(s->force_duplex); break;
        case 7: rv = &(s->master); break;
        case 8: rv = &(s->mdix); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_parameter_desc_t __cint_struct_members__shr_port_cable_diag_t[] = 
    {
        { "_shr_port_cable_state_t", "state", 0, 0 },
        { "int", "npairs", 0, 0 },
        { "_shr_port_cable_state_t", "pair_state", 0, 4 },
        { "int", "pair_len", 0, 4 },
        { "int", "fuzz_len", 0, 0 }, 
        { NULL }
}; 

cint_struct_type_t __cint_sdk_structures[] = 
    {
        {
            "_shr_port_ability_t",
            sizeof(_shr_port_ability_t), 
            __cint_struct_members__shr_port_ability_t, 
            __cint_maddr__shr_port_ability_t
        },
        {
            "_shr_port_cable_diag_t", 
            sizeof(_shr_port_cable_diag_t), 
            __cint_struct_members__shr_port_cable_diag_t,
            __cint_maddr__shr_port_cable_diag_t
        }, 
        {
            "_shr_phy_config_t", 
            sizeof(_shr_phy_config_t), 
            __cint_struct_members__shr_phy_config_t, 
            __cint_maddr__shr_phy_config_t, 
        },
        { NULL }
    }; 


/*
 * Function pointers in <shared>
 */

static cint_function_pointer_t __cint_sdk_function_pointers[3];

static int
__cint_fpointer__shr_port_phy_reset_cb(int unit, _shr_port_t port, void* user_data)
{
    int rc = 0; 
    cint_interpreter_callback(__cint_sdk_function_pointers+0, 3, 1, &unit, &port, &user_data, &rc); 
    return rc; 
}
static cint_parameter_desc_t __cint_parameters__shr_port_phy_reset_cb[] = 
    {
        { "int", "rc", 0, 0 }, 
        { "int", "unit", 0, 0 }, 
        { "int", "port", 0, 0 }, 
        { "void", "user_data", 1, 0 }, 
        { NULL }
    }; 

static void
__cint_fpointer__shr_port_medium_status_cb(int unit, int port, _shr_port_medium_t medium, void* user_arg)
{
    cint_interpreter_callback(__cint_sdk_function_pointers+0, 4, 0, &unit, &port, &medium, &user_arg); 
}
static cint_parameter_desc_t __cint_parameters__shr_port_medium_status_cb[] = 
    {
        { "void", "r", 0, 0 }, 
        { "int", "unit", 0, 0 }, 
        { "int", "port", 0, 0 }, 
        { "_shr_port_medium_t", "medium", 0, 0 }, 
        { "void", "user_arg", 1, 0 }, 
        { NULL }
    }; 

static cint_function_pointer_t __cint_sdk_function_pointers[3] = 
    {
        {
            "_shr_port_phy_reset_cb_t", 
            (cint_fpointer_t) __cint_fpointer__shr_port_phy_reset_cb, 
            __cint_parameters__shr_port_phy_reset_cb
        },
        {
            "_shr_port_medium_status_cb_t", 
            (cint_fpointer_t) __cint_fpointer__shr_port_medium_status_cb, 
            __cint_parameters__shr_port_medium_status_cb
        },
        { NULL }
    };

cint_data_t cint_sdk_data = 
    {
        NULL, 
        __cint_sdk_functions,
        __cint_sdk_structures, 
        NULL, 
        __cint_sdk_typedefs, 
        NULL, 
        __cint_sdk_function_pointers,
        __sdk_custom_iterators, 
        __sdk_custom_macros, 
    }; 


#endif /* INCLUDE_LIB_CINT */
