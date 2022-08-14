#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "host.h"
#include "route_types.h"
#include "nsm_event.h"
#include "vty.h"
#ifndef SUNOS_5
#include <sys/un.h>
#endif


static const struct nsm_event_desc_table unknown = { 0, "unknown" };

#define DESC_ENTRY(T) [(T)] = { (T), (#T) }
static const struct nsm_event_desc_table command_types[] = {
  DESC_ENTRY (NSM_EVENT_HELLO),
  DESC_ENTRY (NSM_EVENT_INTERFACE_ADD),
  DESC_ENTRY (NSM_EVENT_INTERFACE_DELETE),
  DESC_ENTRY (NSM_EVENT_INTERFACE_ADDRESS_ADD),
  DESC_ENTRY (NSM_EVENT_INTERFACE_ADDRESS_DELETE),
  DESC_ENTRY (NSM_EVENT_INTERFACE_UP),
  DESC_ENTRY (NSM_EVENT_INTERFACE_DOWN),
  DESC_ENTRY (NSM_EVENT_INTERFACE_MODE),
  DESC_ENTRY (NSM_EVENT_INTERFACE_RENAME),

  DESC_ENTRY (NSM_EVENT_INTERFACE_VRF_BIND),
  DESC_ENTRY (NSM_EVENT_INTERFACE_VRF_UNBIND),

  DESC_ENTRY (NSM_EVENT_ROUTER_ID_ADD),
  DESC_ENTRY (NSM_EVENT_ROUTER_ID_DELETE),
  DESC_ENTRY (NSM_EVENT_ROUTER_ID_UPDATE),

  DESC_ENTRY (NSM_EVENT_IPV4_ROUTE_ADD),
  DESC_ENTRY (NSM_EVENT_IPV4_ROUTE_DELETE),
  DESC_ENTRY (NSM_EVENT_IPV6_ROUTE_ADD),
  DESC_ENTRY (NSM_EVENT_IPV6_ROUTE_DELETE),

  DESC_ENTRY (NSM_EVENT_REDISTRIBUTE_ADD),
  DESC_ENTRY (NSM_EVENT_REDISTRIBUTE_DELETE),
  DESC_ENTRY (NSM_EVENT_REDISTRIBUTE_DEFAULT_ADD),
  DESC_ENTRY (NSM_EVENT_REDISTRIBUTE_DEFAULT_DELETE),

  DESC_ENTRY (NSM_EVENT_IPV4_NEXTHOP_LOOKUP),
  DESC_ENTRY (NSM_EVENT_IPV6_NEXTHOP_LOOKUP),
  DESC_ENTRY (NSM_EVENT_IPV4_IMPORT_LOOKUP),
  DESC_ENTRY (NSM_EVENT_IPV6_IMPORT_LOOKUP),
  DESC_ENTRY (NSM_EVENT_IPV4_NEXTHOP_LOOKUP_MRIB),

  DESC_ENTRY (NSM_EVENT_NEXTHOP_REGISTER),
  DESC_ENTRY (NSM_EVENT_NEXTHOP_UNREGISTER),
  DESC_ENTRY (NSM_EVENT_NEXTHOP_UPDATE),

  DESC_ENTRY (NSM_EVENT_VRF_REGISTER),
  DESC_ENTRY (NSM_EVENT_VRF_UNREGISTER),

  DESC_ENTRY (NSM_EVENT_MESSAGE_MAX),	
};
#undef DESC_ENTRY

const char *
zserv_command_string(zpl_uint32  command) {
	if (command >= array_size(command_types)) {
		zlog_err(MODULE_DEFAULT, "unknown zserv command type: %u", command);
		return unknown.string;
	}
	return command_types[command].string;
}
