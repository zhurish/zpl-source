#ifndef __OSPF_POLICY_H__
#define __OSPF_POLICY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define OSPF_POLICY_NAME_LEN            20   /*·�ɲ������Ƴ���*/
/* policy index */
enum
{
	OSPF_EX_POLICY = 1,
	OSPF_IM_POLICY = 2,
	OSPF_PRE_POLICY = 3,
	OSPF_ASE_PRE_POLICY = 4,
};

#if 0
typedef struct ospfPolicyEntry
{
	/*dest*/
	u_int uiDest;
	/*mask len*/
	u_int uiDMask;
	/*next hop*/
	u_int uiNextHop;
	/*metric*/
	u_int uiCost;
	/*preference*/
	u_char ucPref;

}tOSPF_POLICY_ENTRY;
#endif



struct ospfPolicyEntry_node
{
	u_int8 proto;
	u_int8 family;
	u_int8 prefixLen;
	u_int8 dest[4];
	u_int8 nexthop[4];
	u_int32 metric;
	u_int32 tag;
};
struct ospfPolicyEntry
{
	struct ospfPolicyEntry_node ospf_com;
	u_int type;
	u_int translate;
};

/* type */
enum
{
	OSPF_INTERNAL = 0,
	OSPF_EXTERNAL_TYPE1 = 1,
	OSPF_EXTERNAL_TYPE2 = 2,

	APPLY_OSPF_TYPE1,
	APPLY_OSPF_TYPE2,
};

/* translate */
enum
{
	APPLY_OSPF_TRANSLATE = 1,
	APPLY_OSPF_NOTRANSLATE = 2,

	RPOLICY_OSPF = 1,
};

enum
{
	OSPF_POLICY_PERMIT = 1,
	OSPF_POLICY_DENY,
	OSPF_POLICY_NOMATCH,
	OSPF_POLICY_NOENTRY,
};

/* policy index */
enum
{
	OSPF_TYPE_RMAP = 1,
	OSPF_TYPE_PLIST,
	OSPF_TYPE_CLIST,
	OSPF_TYPE_ALIST,
	OSPF_TYPE_FILTER,
};// OSPF_TYPE_CMD_E;

#ifndef HAVE_ROUTEPOLICY
int ospf_filter_policy_im_func(void *pRoute);
int ospf_filter_policy_ex_func(void *pRoute);
int ospf_filter_policy_pre_func(void *pRoute);
int ospf_filter_policy_ase_pre_func(void *pRoute);
int ospf_import_policy_func(void *pRoute);
#endif
void ospf_route_map_update(char *PolicyName);
void ospf_route_prefix_update(char *PolicyName);

#ifdef __cplusplus
}
#endif
#endif
