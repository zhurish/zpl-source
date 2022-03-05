/*
 * soc_cint_data.c
 *
 * Hand-coded support for a few SAL core routines. 
 *
 */
int soc_core_cint_data_not_empty; 
#include <sdk_config.h>

#if defined(INCLUDE_LIB_CINT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <soc/property.h>
#include <soc/drv.h>


CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         soc_init, \
                         int,int,unit,0,0);

CINT_FWRAPPER_CREATE_RP2(char*,char,1,0,
                         soc_property_get_str,
                         int,int,unit,0,0,
                         char*,char,name,1,0); 

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         soc_property_get,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 
CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         int,int,defneg,0,0); 

CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp_default,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         pbmp_t,pbmp_t,def,0,0); 

CINT_FWRAPPER_CREATE_RP3(char*,char,1,0,
                         soc_property_port_get_str,                         
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0);

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_port_get,
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         soc_property_suffix_num_get,
                         int,int,unit,0,0,
                         int,int,tc,0,0,
                         char*,char,name,1,0,
                         char*,char,suffix,1,0,
                         uint32,uint32,def,0,0); 

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_cos_get,
                         int,int,unit,0,0,
                         soc_cos_t,soc_cos_t,cos,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 

static cint_function_t __cint_soc_functions[] = 
    {
        CINT_FWRAPPER_ENTRY(soc_init), 
        CINT_FWRAPPER_ENTRY(soc_property_get_str), 
        CINT_FWRAPPER_ENTRY(soc_property_get), 
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp), 
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp_default), 
        CINT_FWRAPPER_ENTRY(soc_property_port_get_str),
        CINT_FWRAPPER_ENTRY(soc_property_port_get),
        CINT_FWRAPPER_ENTRY(soc_property_suffix_num_get),
        CINT_FWRAPPER_ENTRY(soc_property_cos_get),
        CINT_ENTRY_LAST
        
    }; 
    


cint_data_t soc_cint_data = 
    {
        NULL,
        __cint_soc_functions,
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL
    }; 

#endif /* INCLUDE_LIB_CINT */

    
