/*
 * $Id: cint_interpreter.c,v 1.40 Broadcom SDK $
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
 * File:        cint_interpreter.c
 * Purpose:     CINT interpreter
 * 
 */

#include "cint_config.h"
#include "cint_interpreter.h"
#include "cint_porting.h"
#include "cint_variables.h"
#include "cint_error.h"
#include "cint_eval_asts.h"
#include "cint_internal.h"
#include "cint_datatypes.h"
#include "cint_debug.h"
#include "cint_parser.h"

#define CINT_CONFIG_MAX_STACK_PRINT_BUFFER 256

static cint_variable_t* __vinterp = NULL; 
static cint_variable_t* __vargv = NULL; 
static cint_variable_t* __vargc = NULL; 

cint_ast_t* __eval_list = NULL; 

typedef struct cint_iterator_list_s {
    struct cint_iterator_list_s* next; 
    cint_custom_iterator_t* iterators; 
} cint_iterator_list_t; 

typedef struct cint_macro_list_s {
    struct cint_macro_list_s* next; 
    cint_custom_macro_t* macros; 
} cint_macro_list_t; 


static cint_iterator_list_t* __iterator_list = NULL; 
static cint_macro_list_t* __macro_list = NULL; 

typedef struct cint_event_handler_list_s {
    struct cint_event_handler_list_s* next; 
    cint_event_handler_cb handler; 
    void* cookie; 
} cint_event_handler_list_t; 

static cint_event_handler_list_t* __handler_list = NULL; 

static const char* __prompt = NULL; 

int
cint_prompt(void)
{
    if(__prompt) {
        CINT_PRINTF("%s", __prompt); 
    }
    return 0; 
}


int
cint_interpreter_event_register(cint_event_handler_cb handler, void* cookie)
{
    cint_event_handler_list_t* nh = CINT_MALLOC(sizeof(*nh)); 
    
    if(nh == NULL) {
        return CINT_E_MEMORY; 
    }

    nh->next = __handler_list; 
    __handler_list = nh; 
    
    nh->handler = handler; 
    nh->cookie = cookie; 

    return CINT_E_NONE; 
}

int
cint_interpreter_event_unregister(cint_event_handler_cb handler)
{
    cint_event_handler_list_t* p; 
    for(p = __handler_list; p; p = p->next) {
        if(p->handler == handler) {
            p->handler = NULL; 
        }               
    }
    return CINT_E_NONE; 
}

cint_error_t 
cint_interpreter_event(cint_interpreter_event_t event)
{
    cint_event_handler_list_t* hp; 
    for(hp = __handler_list; hp; hp = hp->next) {
        if(hp->handler) {
            hp->handler(hp->cookie, event); 
        }
    }
    return CINT_E_NONE; 
}


int 
cint_interpreter_init(void)
{
    static char* _argv0 = NULL; 

    extern cint_data_t cint_builtin_data; 

#if CINT_CONFIG_INCLUDE_TEST_DATA == 1
    extern cint_data_t cint_test_data; 
#endif

    cint_datatype_t dt; 

    /* clear out any previous allocations */
    cint_variable_clear();

    /*
     * Initialize Variable Management
     */
    cint_variable_init(); 

    /*
     * Add our builtins
     */
    cint_datatype_clear(); 
    cint_interpreter_add_atomics(cint_builtin_data.atomics); 
    cint_interpreter_add_data(&cint_builtin_data, NULL); 

    /*
     * Add a variable pointing to the interpreter state. 
     * This allows the state to be manipulated during interpretation. 
     * Use with care. 
     */
    if(cint_datatype_find("cint_interpreter_state_t", &dt) == CINT_E_NONE) {
        cint_variable_create(&__vinterp, "interp", &dt.desc,
                             CINT_VARIABLE_F_SDATA|CINT_VARIABLE_F_NODESTROY,
                             &interp); 
    }
    interp.debug.dtrace = 0;
    interp.debug.atrace = 0;
    interp.debug.ftrace = 0;

#if CINT_CONFIG_INCLUDE_TEST_DATA == 1
    cint_interpreter_add_atomics(cint_test_data.atomics); 
    cint_interpreter_add_data(&cint_test_data, NULL); 
#endif

#if CINT_CONFIG_INCLUDE_PARSER == 1

#endif
    
    cint_interpreter_create_pointer("void", "NULL", NULL); 


    /* 
     * Create the argc/argv global variables 
     */
    dt.desc.basetype = "int"; 
    dt.desc.pcount = 0; 
    dt.desc.array = 0; 
    cint_variable_create(&__vargc, "ARGC", &dt.desc,
                         CINT_VARIABLE_F_NODESTROY, NULL); 
    
    dt.desc.basetype = "char"; 
    dt.desc.pcount = 2; 
    dt.desc.array = 0; 
    cint_variable_create(&__vargv, "ARGV", &dt.desc,
                         CINT_VARIABLE_F_SDATA|CINT_VARIABLE_F_NODESTROY,
                         &_argv0);

    __eval_list = NULL;

    /*
     * Initialize default include path
     */
 #if CINT_CONFIG_INCLUDE_XINCLUDE == 1
    cint_interpreter_include_set(NULL); 
 #endif
 
    /*
     * Changes to the default interpreter state go here
     */
    
    return 0; 
}       

int
cint_interpreter_add_atomics(cint_atomic_type_t* types)
{
    return cint_datatype_add_atomics(types);
}

int
cint_interpreter_add_data(cint_data_t* data, void *dlhandle)
{
    int rc = cint_datatype_add_data(data, dlhandle); 

    if(data->iterators) {
        cint_iterator_list_t* n = CINT_MALLOC(sizeof(*n));
        if (n == NULL) {
            return CINT_E_MEMORY;
        }
        n->next = __iterator_list;      
        n->iterators = data->iterators; 
        __iterator_list = n; 
    }
    if(data->macros) {
        cint_macro_list_t* n = CINT_MALLOC(sizeof(*n)); 
        if (n == NULL) {
            return CINT_E_MEMORY;
        }
        n->next = __macro_list; 
        n->macros = data->macros; 
        __macro_list = n; 
    }
    return rc; 
}

int 
cint_interpreter_add_function(cint_function_t* f)
{
    return cint_datatype_add_function(f); 
}
    

static int
__cint_is_exit(cint_ast_t* ast)
{
    return (ast &&
            (ast->ntype == cintAstIdentifier) &&
            (ast->utype.identifier.s) &&
            !CINT_STRCMP(ast->utype.identifier.s, "exit")); 
}

static void
cint_interpreter_enable_prompt(void)
{
    extern int cint_scanner_prompt; 

    cint_scanner_prompt = 1; 
}

cint_error_t
cint_interpreter_evaluate(cint_ast_t* ast)
{    
    cint_variable_t* rv = NULL; 
    /*
     * Add to global translation unit evaluation list 
     */

    if(__eval_list == NULL) {
        __eval_list = ast; 
    }
    else {
        cint_ast_append(__eval_list, ast); 
    }

    if(interp.parse_only == 0) {
        /*
         * Evaluate new unit
         */
        cint_errno = CINT_E_NONE; 
        rv = cint_eval_asts(ast);
        if (rv && interp.print_expr) {
            cint_variable_print(rv, 0, "");
        }
    }   
    else {
        /* 
         * Check for parsing exit condition
         */
        if(__cint_is_exit(ast)) {
            cint_errno = CINT_E_EXIT; 
        }       
    }   

    /* free all ASTs that were used */
    cint_ast_free_all();
    __eval_list = NULL;

    /* Reprompt after translation unit is complete */
    cint_interpreter_enable_prompt();

    return cint_errno; 
}      

cint_ast_t*
cint_interpreter_tree(void)
{
    return __eval_list; 
}

volatile cint_error_t cint_errno; 


int cint_interpreter_is_type(const char* str)
{
    cint_datatype_t dt; 
    if(cint_datatype_find(str, &dt) == CINT_E_NONE) {        
        return (dt.flags & CINT_DATATYPE_FLAGS_TYPE); 
    }
    return 0; 
}

static void
_indent(int count)
{
    while(count--) {
        CINT_PRINTF(" "); 
    }   
} 

static int
__pointer_print(void** p)
{
    void* vp = *p; 
    if(vp == NULL) {
        CINT_PRINTF("NULL"); 
    }
    else {
        CINT_PRINTF("%p", vp); 
    }
    return 0; 
}


static cint_enum_map_t*
_enum_find(void* p, cint_enum_map_t* maps)
{
    int* e = (int *)p; 
    cint_enum_map_t* em; 

    for(em = maps; em->name; em++) {
        if(em->value == *e) {
            return em; 
        }       
    }   

    return NULL; 
}

int
_str2enum(void* p, int pcount, const char* name, cint_enum_map_t* map)
{       
    int* e = (int *)p; 
    cint_enum_map_t* em; 
    
    while(pcount--) {
        e = * (int**) e; 
    }   
    
    for(em = map; em->name; em++) {
        if(!CINT_STRCMP(name, em->name)) {
            *e = em->value;
            return 0; 
        }       
    }   
    return CINT_E_NOT_FOUND; 
}
                    

/*
 * Used when a variable is referenced from within the printf() function as "$var"
 */
int
cint_variable_printf(cint_variable_t* v)
{
    char fbuffer[CINT_CONFIG_MAX_STACK_PRINT_BUFFER]; 

    if(v == NULL) {
        return CINT_E_BAD_VARIABLE; 
    }   

    if(v->dt.desc.array && v->dt.cap == NULL) {
        CINT_PRINTF("<array variable>"); 
    }   
    else if(v->dt.desc.pcount || (v->dt.flags & CINT_DATATYPE_F_FUNC_POINTER)) {
        /* 
         * This is a pointer 
         */
        __pointer_print((void**)v->data); 
    }
    else if(v->dt.cap && v->dt.cap->format) {
        v->dt.cap->format(v->data, fbuffer, sizeof(fbuffer), cintAtomicFormatCintPrintf); 
        CINT_PRINTF("%s", fbuffer); 
    }
    else if(v->dt.flags & CINT_DATATYPE_F_ATOMIC) {
        v->dt.basetype.ap->format(v->data, fbuffer, sizeof(fbuffer), cintAtomicFormatCintPrintf); 
        CINT_PRINTF("%s", fbuffer); 
    }       
    else if(v->dt.flags & CINT_DATATYPE_F_ENUM) {
        cint_enum_map_t* em = _enum_find(v->data, v->dt.basetype.ep->enum_map); 
        CINT_PRINTF("%s", (em) ? em->name : "<invalid enum>"); 
    }       
    else if(v->dt.flags & CINT_DATATYPE_F_STRUCT) {
        CINT_PRINTF("<structure variable>"); 
    }
    else {
        CINT_PRINTF("<unhandled variable print condition>"); 
    }   
    return 0; 
}
 
/*
 * Used for interactive interpreter "print" command
 */
static int
__cint_variable_print(cint_variable_t* v, int indent, const char* vname)
{
    char fbuffer[CINT_CONFIG_MAX_STACK_PRINT_BUFFER]; 

    const char* datatype; 
    
    if(v == NULL) {
        return CINT_E_BAD_VARIABLE; 
    }

    if(vname == NULL) {
        vname = v->name; 
    }   

    if(vname[0] == '_' &&
       vname[1] == '_' &&
       vname[2] == 'a' &&
       vname[3] == 'u' &&
       vname[4] == 't' &&
       vname[5] == 'o') {
        vname = "$$"; 
    }   
    
    datatype = cint_datatype_format(&v->dt, 0); 

    if(v->dt.desc.pcount || (v->dt.flags & CINT_DATATYPE_F_FUNC_POINTER)) {
        /* 
         * This is a pointer
         */
        _indent(indent); 
        CINT_PRINTF("%s %s = ", datatype, vname); 
        __pointer_print((void**)v->data); 
        CINT_PRINTF("\n"); 
        return 0; 
    }   
   
    /* 
     * Atomic or structure types
     */

    if(v->dt.cap && v->dt.cap->format) {
        _indent(indent); 
        CINT_PRINTF("%s %s = ", datatype, vname); 
        v->dt.cap->format(v->data, fbuffer, sizeof(fbuffer), cintAtomicFormatCintPrintVariable); 
        CINT_PRINTF("%s\n", fbuffer); 
    }
    else if(v->dt.flags & CINT_DATATYPE_F_ATOMIC) {
        _indent(indent); 
        CINT_PRINTF("%s %s = ", datatype, vname); 
        v->dt.basetype.ap->format(v->data, fbuffer, sizeof(fbuffer), cintAtomicFormatCintPrintVariable); 
        CINT_PRINTF("%s\n", fbuffer); 
    }       
    else if(v->dt.flags & CINT_DATATYPE_F_ENUM) {
        cint_enum_map_t* em = _enum_find(v->data, v->dt.basetype.ep->enum_map); 
        _indent(indent); 
        CINT_PRINTF("%s %s = %s (%d)\n", datatype, vname, 
                    (em) ? em->name : "<invalid enum>", (em) ? em->value : *(int*)v->data); 
    }       
    else if(v->dt.flags & CINT_DATATYPE_F_STRUCT) {
        const cint_parameter_desc_t* sm; 
        int i = 0; 
        _indent(indent); 

        CINT_PRINTF("%s %s = {\n", datatype, vname);         
        
        for(sm = v->dt.basetype.sp->struct_members; sm->basetype; sm++) {
            /*
             * Get the address of this member
             */     
            cint_variable_t* av; 
            void* maddr; 
            maddr = v->dt.basetype.sp->maddr(v->data, i, v->dt.basetype.sp); 
            
            /*
             * Create an autovariable of the same type pointing to the address of this member
             */
            {
                cint_datatype_t sdt; 
                int rc; 
                sdt.desc = *sm;
                cint_strlcpy(sdt.type, sm->basetype, sizeof(sdt.type)); 

                if((rc = cint_variable_create(&av, NULL, &sdt.desc,
                                              CINT_VARIABLE_F_SDATA,
                                              maddr)) < 0) {
                    
                    if(rc == CINT_E_TYPE_NOT_FOUND) {
                        /* 
                         * A description for this datatype is missing. 
                         */
                        _indent(indent+4); 
                        cint_warn(NULL, 0, "unknown type '%s'", cint_datatype_format(&sdt, 0)); 
                        
                        /* Keep going */
                        i++; 
                        continue; 
                    }   
                    else {
                        return rc; 
                    }   
                }       
            }   
            
            /* 
             * Print the autovar with this member's name
             */
            cint_variable_print(av, indent+4, sm->name); 
            i++; 
        }   
        _indent(indent); 
        CINT_PRINTF("}\n");     
    }       
    else if(v->dt.flags & (CINT_DATATYPE_F_FUNC | CINT_DATATYPE_F_FUNC_DYNAMIC)) {
        /* Function pointer variable */
        _indent(indent); 
        CINT_PRINTF("%s = %s function '%s %s", 
                    vname, 
                    (v->dt.flags & CINT_DATATYPE_F_FUNC) ? "compiled" : "dynamic", 
                    cint_datatype_format_pd(v->dt.basetype.fp->params, 0), 
                    v->dt.basetype.fp->name); 
        cint_fparams_print(v->dt.basetype.fp->params+1); 
        CINT_PRINTF("'\n"); 
    }

    return CINT_E_NONE; 
}


int
cint_variable_print(cint_variable_t* v, int indent, const char* vname)
{    
    if(v->dt.cap || v->dt.desc.array == 0) {
        return __cint_variable_print(v, indent, vname); 
    }
    else {
        int i; 
        cint_variable_t pv = *v; 
        char name[64]; 
        char* p; 

        _indent(indent); 

        CINT_PRINTF("%s %s = {\n", cint_datatype_format(&v->dt, 0), vname); 
        
        pv.dt.desc.array = 0; 
        p = (char*)pv.data; 

        for(i = 0; i < v->dt.desc.array; i++) {
            CINT_SPRINTF(name, "%s[%d]", vname, i); 
            __cint_variable_print(&pv, indent+4, name); 
            p += (v->size/v->dt.desc.array); 
            pv.data = p;
        }       
        _indent(indent); 
        CINT_PRINTF("}\n"); 
    }
    return 0; 
}

int
cint_constant_find(const char* name, int* c)
{
    return cint_datatype_constant_find(name, c); 
}
     
int
cint_interpreter_run(const char* fname)
{
    cint_ast_t* ast;

    if(fname == NULL) {
        fname = "main"; 
    }

    ast = cint_ast_function(NULL, NULL);     
    ast->utype.function.name = fname; 
    ast->utype.function.parameters = NULL; 
    cint_eval_ast(ast); 
    return cint_errno; 
}

cint_error_t 
cint_interpreter_create_pointer(const char* type, const char* name, void* ptr)
{       
    cint_datatype_t dt; 
    cint_variable_t* v;
    cint_error_t rc; 

    rc = cint_datatype_find((type) ? type : "void", &dt); 
    dt.desc.pcount=1; 
    if((rc = cint_variable_create(&v, name, &dt.desc,
                                  CINT_VARIABLE_F_CONST|
                                  CINT_VARIABLE_F_NODESTROY,
                                  NULL)) == CINT_E_NONE) {
        *(void**)v->data = ptr;
    }   
    
    return rc; 
}


#if CINT_CONFIG_INCLUDE_XINCLUDE == 1

typedef struct cint_directory_list_s {
    struct cint_directory_list_s* next; 
    char* directory; 
} cint_directory_list_t; 

static cint_directory_list_t* __directory_list = NULL; 

int
cint_interpreter_include_set(const char* path)
{
    /*
     * Free existing list
     */
    cint_directory_list_t* list; 
    for(list = __directory_list; list;) {
        cint_directory_list_t* next = list->next; 
        CINT_FREE(list->directory); 
        CINT_FREE(list); 
        list = next; 
    }
    
    /* The head of the list is set to the local directory */
    __directory_list = CINT_MALLOC(sizeof(*__directory_list)); 
    __directory_list->next = NULL; 
    __directory_list->directory = CINT_MALLOC(1); 
    __directory_list->directory[0] = 0; 
    
    return cint_interpreter_include_add(path);
}

int
cint_interpreter_include_add(const char* path)
{
    const char* s = path; 
    int count = 0; 
    /*
     * Directory paths are semicolon or colon separated
     */
    if(path && path[0]) {
        
        int count = 0; 
        cint_directory_list_t* tail; 
        cint_directory_list_t* l = NULL; 
        
        int alloc_size = CINT_STRLEN(path)+1; 

        /* 
         * Get the tail of the current directory list
         */
        for(tail = __directory_list; tail && tail->next; tail = tail->next); 


        /* 
         * Parse and append new paths to the tail of the current list
         */
        do {            
            char* d; 

            /* Allocate new list entry */
            l = CINT_MALLOC(sizeof(*l)); 
            if(l == NULL) {
                return CINT_E_MEMORY; 
            }
            l->next = NULL; 
            
            l->directory = CINT_MALLOC(alloc_size); 
            if(l->directory == NULL) {
                CINT_FREE(l); 
                return CINT_E_MEMORY; 
            }

            /* Copy the current path */
            d = l->directory; 
            while(*s && ((*s != ';') && (*s != ':'))) {
                *d++ = *s++; 
            }
            *d = 0; 

            /* Skip semicolons */
            while(*s && ((*s == ';') || (*s == ':'))) {
                s++;
            }
            
            /* Push this new path on the list */
            tail->next = l;
            tail = l; 
            count++; 
        } while(*s); 
    }
    return count; 
}

/*
 * Search include paths for requested name
 */
static int
_cint_interpreter_include_search(const char* name, char** path_rv)
{
    cint_directory_list_t* l; 
    FILE* fp; 
    char* p;
        
    for (l = __directory_list; l ; l = l->next) {

        if (!l->directory) {
            continue;
        }

        /* directory + '/' + name + \0 */
        p = CINT_MALLOC(CINT_STRLEN(l->directory)+1+CINT_STRLEN(name)+1);
        if(p == NULL) {
            if(path_rv) {
                *path_rv = NULL; 
            }
            return CINT_E_MEMORY; 
        }
        *p = 0;

        if(l->directory[0]) {
            CINT_SPRINTF(p, "%s/", l->directory); 
        }
        CINT_STRCAT(p, name); 
        /* Only (currently) portable way to check existence is to
           open it with CINT_FOPEN() */
        if((fp = CINT_FOPEN(p, "r"))) {
            CINT_FCLOSE(fp); 
            if(path_rv) {
                *path_rv = p; 
            }
            else {
                CINT_FREE(p); 
            }
            return CINT_E_NONE; 
        }
        CINT_FREE(p); 
    }
    if(path_rv) {
        *path_rv = NULL; 
    }
    return CINT_E_NOT_FOUND; 
}

static int
_cint_interpreter_include_path(char** path_rv)
{
    int size = 1; 
    cint_directory_list_t* l; 
    char* p, *s;

    for(l = __directory_list; l ; l = l->next) {
        size += CINT_STRLEN(l->directory)+1; 
    }       
    p = CINT_MALLOC(size); 

    if(p == NULL) {
        return CINT_E_MEMORY; 
    }

    for(l = __directory_list, s = p; l ; l = l->next) {
        s += CINT_SPRINTF(s, "%s;", l->directory); 
    }

    if(path_rv) {
        *path_rv = p; 
    }
    else {
        CINT_FREE(p); 
    }

    return CINT_E_NONE; 
}

int 
cint_interpreter_include_get(const char* name, char** path_rv)
{
    int rv;

    if (name) {
        rv = _cint_interpreter_include_search(name, path_rv);
    } else {
        /*
         * If name is NULL then return the entire current search path 
         */
        rv = _cint_interpreter_include_path(path_rv);
    }

    return rv;
}

#endif /* CINT_CONFIG_INCLUDE_XINCLUDE */

#if CINT_CONFIG_INCLUDE_PARSER == 1

int
cint_interpreter_parse(void* handle, const char* prompt, int argc, char** argv)
{
    cint_ast_t* ast;    
    cint_cparser_t* cp = cint_cparser_create(); 
    int parse_ok;
    int is_ident;

    cint_errno = CINT_E_NONE; 
    __prompt = prompt; 

    cint_cparser_start_handle(cp, handle); 

    /*
     * Initialize ARGC/ARGV  with passed arguments
     */
    *((int*)__vargc->data) = argc; 
    *((char***)__vargv->data) = argv; 

    cint_interpreter_enable_prompt();

    parse_ok = 1;
    while ( cint_errno != CINT_E_EXIT ) {
        ast = cint_cparser_parse(cp);
        is_ident = (ast && ast->ntype == cintAstIdentifier);

        if (handle != NULL && parse_ok && cint_cparser_error(cp)) {
            /* If parsing a file, once there's a parser error,
               continue to parse, but do not evaluate.

               This is to avoid infinite loops from code like this:

               f()
               {
               ^ syntax error, unexpected '{', expecting ',' or ';'
                 while(1) {
                 }
               }

               which, if allowed to evaluate, would execute an
               infinite loop.
            */
            parse_ok = !parse_ok;
            /* It is possible to break here to avoid a cascade of
               syntax errors, continuing on may display more
               legitimate syntax errors. */
        }
        if (parse_ok && ast) {
            /* Evaluate AST */
            cint_interpreter_evaluate(ast);
        }
    }   

    cint_cparser_destroy(cp); 

    return cint_errno; 
}

int
cint_interpreter_is_iterator(const char* str, cint_ast_t** handle)
{
    cint_iterator_list_t* list; 
    cint_custom_iterator_t* iter; 
    for(list = __iterator_list; list; list = list->next) {
        for(iter = list->iterators; iter && iter->name; iter++) {
            if(!CINT_STRCMP(iter->name, str)) {
                if(handle) {
                    *handle = (cint_ast_t*)iter; 
                }
                return 1; 
            }
        }
    }   
    return 0; 
}

int
cint_interpreter_is_macro(const char* str, cint_ast_t** handle)
{       
    cint_macro_list_t* list; 
    cint_custom_macro_t* macro; 
    for(list = __macro_list; list; list = list->next) {
        for(macro = list->macros; macro && macro->name; macro++) {
            if(!CINT_STRCMP(macro->name, str)) {
                if(handle) {
                    *handle = (cint_ast_t*)macro; 
                }
                return 1; 
            }
        }
    }   
    return 0; 
}

cint_ast_t* 
cint_interpreter_iterator(cint_ast_t* handle, cint_ast_t* arguments, cint_ast_t* statements)
{
    cint_ast_t* ast; 
    cint_custom_iterator_t* iter = (cint_custom_iterator_t*)handle; 
    ast = iter->handler(iter->name, arguments, statements); 
    if(ast == NULL) {
        /* Error */
        ast = cint_ast_empty(); 
    }
    return ast; 
}

cint_ast_t*
cint_interpreter_macro(cint_ast_t* handle, cint_ast_t* arguments)
{
    cint_ast_t* ast; 
    cint_custom_macro_t* macro = (cint_custom_macro_t*)handle; 
    ast = macro->handler(macro->name, arguments); 
    if(ast == NULL) {
        /* Error */
        ast = cint_ast_empty(); 
    }
    return ast; 
}       
    

#endif /* CINT_CONFIG_INCLUDE_PARSER */
