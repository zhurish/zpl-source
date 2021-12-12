/* onvif_util.c
*/




#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "soapH.h"
//#include "wsdd.nsmap"
//#include "RemoteDiscoveryBinding.nsmap"

#include "wsddapi.h"
#include "onvif_util.h"
#include "uuid/uuid.h"

#ifdef ZPL_ONVIF_SSL
zpl_bool soap_access_control(struct soap* soap) {
	const char *username = soap_wsse_get_Username(soap);
      const char *password;
      if (!username)
        return soap->error; // no username: return FailedAuthentication

      password = "123456"; // lookup password of username
      if (soap_wsse_verify_Password(soap, password))
        return soap->error; // password verification failed: return FailedAuthentic

      return SOAP_OK;
}
#endif

int _hw_onvif_get_uuid(zpl_uint8 *buf)
{
	//uuid_t	uuid;  
 	//uuid_generate(uuid);
	//uuid_unparse(uuid, buf);
  strcpy(buf, "5d645de6-bc78-11eb-a90f-000c29bf00a4");
  return 0;
}

int _hw_onvif_get_IPport(zpl_uint8 *buf, zpl_uint16 *port)
{
	strcpy(buf, "192.168.182.129");
  if(port)
    port = 5000;
  return 0;
}
