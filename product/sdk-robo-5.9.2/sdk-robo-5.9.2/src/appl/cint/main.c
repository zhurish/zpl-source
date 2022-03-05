/*
 * $Id: main.c,v 1.13 Broadcom SDK $
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
 * File:        main.c
 * Purpose:     Standalone CINT application
 *
 */

#include "cint_config.h"

#if CINT_CONFIG_INCLUDE_MAIN == 1

#include "cint_interpreter.h"
#include "cint_ast.h"
#include "cint_parser.h"
#include "cint_variables.h"
#include <stdio.h>
#include <limits.h>
#include <getopt.h>


/*
 * Program Options
 */

static int _opt_ast = 0; 
static int _opt_prompt = 1; 

#ifdef YYDEBUG
extern int cint_c_debug; 
#endif

static struct option long_options[] = 
    {
        { "atrace", no_argument, &interp.debug.atrace, 1 }, 
        { "ftrace", no_argument, &interp.debug.ftrace, 1 }, 
        { "dtrace", no_argument, &interp.debug.dtrace, 1 }, 
        { "print",  no_argument, &interp.print_expr, 1 }, 
        { "ast", no_argument, &_opt_ast, 1 }, 
        { "noprompt", no_argument, &_opt_prompt, 0 }, 
#ifdef YYDEBUG
        { "yydebug", no_argument, &cint_c_debug, 1 }, 
#endif
        { 0, 0, 0, 0 }, 
    }; 
       

static char *
santize_filename(char *s)
{
    return s ? ((strlen(s) > PATH_MAX) ? NULL : s) : NULL;
}

#if CINT_CONFIG_INCLUDE_XINCLUDE == 1
static void
init_include_path(void)
{
    char *path;

    path = getenv("CINT_INCLUDE_PATH");
    if (path) {
        cint_interpreter_include_set(path); 
    }
}
#endif

int 
main(int argc, char* argv[])
{
    FILE* fp = NULL; 
    char *file_arg;

    while(1) {
        
        int opt_index; 
        int c; 
        
        c = getopt_long(argc, argv, "", long_options, &opt_index); 
        
        if(c == -1) {
            break; 
        }
    }   
    file_arg = santize_filename(argv[optind]);

    /*
     * What actions?
     */
    if(_opt_ast) {

        /* Do not interpret. Just parse and dump the ASTs */
        cint_cparser_t* cp = cint_cparser_create(); 
        cint_ast_t* ast; 

        if(cp == NULL) {
            printf("** error creating parser object\n"); 
            return -1; 
        }

        if(file_arg) {

            /* Assume file argument */
            fp = fopen(file_arg, "r"); 
            
            if(fp) {
                cint_cparser_start_handle(cp, fp); 
            }
            else {
                /* Assume immediate string argument */
                cint_cparser_start_string(cp, file_arg); 
            }
        }
        else {
            /* Assume stdin */
            cint_cparser_start_handle(cp, NULL); 
        }       

        /*
         * Parse translation units until EOF or error
         */
        while((ast = cint_cparser_parse(cp))) {
            cint_ast_dump(ast, 0);            
        }       
        
        cint_cparser_destroy(cp); 

        if(fp) {
            fclose(fp); 
        }

        return 0; 
    }   
    
    cint_interpreter_init(); 
#if CINT_CONFIG_INCLUDE_XINCLUDE == 1
    init_include_path();
#endif

    if(file_arg) {
        fp = fopen(file_arg, "r"); 
    }   

    cint_interpreter_parse(fp, (_opt_prompt) ? "cint> " : NULL, 0, NULL); 
    
    if(fp) {
        fclose(fp); 
    }  

    cint_datatype_clear();
    cint_variable_clear();

    return 0;
}

#else /* CINT_CONFIG_INCLUDE_MAIN */
int cint_main_c_not_empty; 
#endif
