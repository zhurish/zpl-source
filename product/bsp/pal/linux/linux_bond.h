
#ifndef __LINUX_BOND_H__
#define __LINUX_BOND_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_TRUNK
extern int _ipkernel_bond_create(struct interface *ifp);
extern int _ipkernel_bond_delete(struct interface *ifp);

extern int _if_bond_add_slave(struct interface *ifp, struct interface *slave);
extern int _if_bond_delete_slave(struct interface *ifp, struct interface *slave);
extern int _if_bond_slave_active(struct interface *ifp, struct interface *slave);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_BOND_H__ */
