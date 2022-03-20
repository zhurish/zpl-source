/*
 * nsm_port.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_PORT_H__
#define __NSM_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NSM_PORT_JUMBO_DEFAULT		zpl_false
#define NSM_PORT_LOOPBACK_DEFAULT	zpl_false
#define NSM_PORT_PUASE_TX_DEFAULT	zpl_false
#define NSM_PORT_PUASE_RX_DEFAULT	zpl_false
#define NSM_PORT_LEARNING_DEFAULT	zpl_true
#define NSM_PORT_SWLEARNING_DEFAULT	zpl_false
#define NSM_PORT_PROTECT_DEFAULT	zpl_false
#define NSM_PORT_FLOWCONTROL_TX_DEFAULT	zpl_false
#define NSM_PORT_FLOWCONTROL_RX_DEFAULT	zpl_false

typedef struct nsm_port_s
{
	struct interface *ifp;
	zpl_bool		jumbo_enable;
	zpl_bool		loopback;
	zpl_bool		pause_tx;
	zpl_bool		pause_rx;

	zpl_bool		learning_enable;
	zpl_bool		sw_learning_enable;
	zpl_bool		protect;
	zpl_bool		flowcontrol_tx;
	zpl_bool		flowcontrol_rx;

}nsm_port_t;



extern int nsm_port_init(void);
extern int nsm_port_exit(void);
extern int nsm_port_start(void);

extern int nsm_port_jumbo_set_api(struct interface *ifp, zpl_bool value);
extern int nsm_port_jumbo_get_api(struct interface *ifp, zpl_bool *value);

extern int nsm_port_loopback_set_api(struct interface *ifp, zpl_bool value);
extern int nsm_port_loopback_get_api(struct interface *ifp, zpl_bool *value);

extern int nsm_port_learning_set_api(struct interface *ifp, zpl_bool value);
extern int nsm_port_learning_get_api(struct interface *ifp, zpl_bool *value);

extern int nsm_port_sw_learning_set_api(struct interface *ifp, zpl_bool value);
extern int nsm_port_sw_learning_get_api(struct interface *ifp, zpl_bool *value);

extern int nsm_port_pause_set_api(struct interface *ifp, zpl_bool tx, zpl_bool rx);
extern int nsm_port_pause_get_api(struct interface *ifp, zpl_bool *tx, zpl_bool *rx);

extern int nsm_port_protect_set_api(struct interface *ifp, zpl_bool value);
extern int nsm_port_protect_get_api(struct interface *ifp, zpl_bool *value);

extern int nsm_port_flowcontrol_set_api(struct interface *ifp, zpl_bool tx, zpl_bool rx);
extern int nsm_port_flowcontrol_get_api(struct interface *ifp, zpl_bool *tx, zpl_bool *rx);


#ifdef ZPL_SHELL_MODULE
extern void cmd_port_init (void);

#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_PORT_H__ */
