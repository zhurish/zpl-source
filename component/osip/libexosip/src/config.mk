#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/osip/libexosip/src
#
PLINCLUDE += -I$(OSIP_ROOT)/libexosip/include
#
#
#OS
OBJS += eXcall_api.o
OBJS += eXconf.o
OBJS += eXinsubscription_api.o
OBJS += eXmessage_api.o
OBJS += eXosip.o         
OBJS += eXregister_api.o 
OBJS += eXmessage_api.o  
OBJS += eXtransport.o    
OBJS += jrequest.o       
OBJS += jresponse.o      
OBJS += jcallback.o      
OBJS += jdialog.o        
OBJS += udp.o            
OBJS += jcall.o          
OBJS += jreg.o           
OBJS += eXutils.o        
OBJS += jevents.o        
OBJS += misc.o           
OBJS += jpipe.o          
OBJS += jauth.o 

OBJS += eXtl_udp.o 
OBJS += eXtl_tcp.o 
OBJS += eXtl_dtls.o 
OBJS += eXtl_tls.o     

OBJS += milenage.o 
OBJS += rijndael.o  

#if BUILD_MAXSIZE
OBJS += eXsubscription_api.o    
OBJS += eXoptions_api.o    
OBJS += eXinsubscription_api.o  
OBJS += eXpublish_api.o    
OBJS += jnotify.o               
OBJS += jsubscribe.o       
OBJS += inet_ntop.o             
#OBJS += inet_ntop.h        
OBJS += jpublish.o              
OBJS += sdp_offans.o
#endif

#############################################################################
# LIB
###########################################################################
LIBS = libeXosip2.a
