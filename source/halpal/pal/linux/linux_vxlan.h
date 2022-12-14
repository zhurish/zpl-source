
#ifndef __LINUX_VXLAN_H__
#define __LINUX_VXLAN_H__

#ifdef __cplusplus
extern "C" {
#endif

struct vxlan_param
{
	int id;
	int group;
	char *address; /* group or remote address */
	int ifindex;
	char *localaddr;
	int ttl;
	int inherit;
	int tos;
	int df;
	int label;
	int dstport;
	int srcport;
	int srcport_max;
	bool learning;
	bool proxy;
	bool rsc;
	bool l2miss;
	bool l3miss;
	int ageing;
	int maxaddress;
	bool udpcsum;
	bool udp6zerocsumtx;
	bool udp6zerocsumrx;
	bool remcsumtx;
	int remcsumrx;
	bool external;
	int external_mode; //] [ gbp ] [ gpe ]\n"
};


int vxlan_parse_opt(int v6, struct vxlan_param *param,
						   struct nlmsghdr *n);
                           
#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VXLAN_H__ */
