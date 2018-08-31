#ifdef NEW_BGP_WANTED

#ifndef BGP4COM_H
#define BGP4COM_H

#ifdef __cplusplus
extern "C" {
#endif
#include "plateform.h"
#include "kernel_network.h"
#include "zebra_api.h"
#include "l3vpn.h"
#include "mpls_api.h"
#include "bgp4_api.h"
#include "bgp_relation.h"
#include "bgp4dfs.h"
#include "bgp4msgh.h"
#include "bgp4main.h"
#include "bgp4peer.h"
#include "bgp4util.h"
#include "bgp4path.h"
#include "bgp4rts.h"
#include "bgp4aggr.h"
#include "bgp4tree.h"
#include "bgp4tcph.h"
#include "bgp4debug.h"
#include "bgp4gr.h"
#include "bgp4redistribute.h"
#include "bgp4kernal.h"
#include "bgp4tree.h"
#include "bgp4main.h"
#include "bgp4sync.h"
#include "bgp4policy.h"
#include "bgp4mplsvpn.h"
#ifdef USE_LINUX_OS
#define RTM_VERSION            4
#define RTM_NEW_RTMSG          24
#define RTM_ADD                1
#define RTM_DELETE             2  
#define RTM_MISS               7
#define RTM_LOCK               8
#define RTM_OLDADD             9
#define RTM_OLDDEL             0xa
#define RTM_RESOLVE            0xb
#define RTM_CHANGE             0x3
#define RTM_REDIRECT           0x6
#define RTM_IFINFO             0xe

#define RTM_VPN_ADD            0xfff1  
#define RTM_VPN_DEL            0xfff2
#define RTM_VPN_RT_CHANGE      0xfff3
#define RTM_TCPSYNC_ADD        0xfff4
#define RTM_TCPSYNC_DEL        0xfff5
#define RTM_SUBSYSTEM_UP       0xfff6
#define RTM_SLOT_UP            0xfff7
#define RTM_CARD_UP            0xfff8
#define RTM_CARD_DOWN          0xfff9
#define RTM_CARD_ROLECHANGE    0xfffa
#define RTM_BFD_SESSION        0xfffb
#define RTM_BFD_IFSTATUS       0xfffc
#define RTM_LOSING             0xfffd

#endif
#ifdef __cplusplus
}
#endif
	 
#endif /* BGP4COM_H */
#else
#ifndef BGP4COM_H
#define BGP4COM_H

#ifdef __cplusplus
      extern "C" {
     #endif
     
#include "avl.h"
#include "bitmap.h"

typedef struct avl_tree_t tBGP4_TREE;
typedef struct avl_node_t tBGP4_LISTNODE;
#define bgp4_lstinit(x) INIT_LIST_HEAD(x)
#define bgp4_lstnodeinit(x) INIT_LIST_HEAD(x)

#define bgp4_lstadd(x,y) list_add((y),(x))
#define bgp4_lstadd_tail(x,y) list_add_tail((y),(x))
#define bgp4_lstdelete(x,y) list_del(y)

#define bgp4_lstinsert(x,y,z) list_add(z,y)
#define bgp4_lstconcat(x,y) list_splice_tail((y),(x))

#define bgp4_lstempty(x)  list_empty_careful(x)

#define LST_EACH(lst, f) list_for_each(f,lst)

#if!defined(WIN32)
#define LST_LOOP(lst, f, n, t)  list_for_each_entry(f,lst,n)
#define LST_LOOP_SAFE(lst, f, nxt, n,t) list_for_each_entry_safe(f,nxt,lst,n)
#else
#define LST_LOOP(lst, f, n, t)  list_for_each_entry(f,lst,n,t)
#define LST_LOOP_SAFE(lst, f, nxt, n,t) list_for_each_entry_safe(f,nxt,lst,n,t)
#endif

#include "bgp4_api.h"
#include "bgp4dfs.h"
#include "bgp4msgh.h"
#include "bgp4main.h"
#include "bgp4peer.h"
#include "bgp4util.h"
#include "bgp4path.h"
#include "bgp4rts.h"
#include "bgp4aggr.h"
#include "bgp4tree.h"
#include "bgp4tcph.h"
#include "bgp4debug.h"
#include "bgp4gr.h"
#include "bgp4redistribute.h"
#include "bgp4kernal.h"
#include "bgp4tree.h"
#include "bgp4main.h"
#include "bgp4sync.h"
#include "bgp4policy.h"
#include "bgp4mplsvpn.h"

#ifdef __cplusplus
     }
     #endif   
     
#endif /* BGP4COM_H */

#endif
