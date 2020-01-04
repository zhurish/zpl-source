/*
 * cmd_x5b_app.c
 *
 *  Created on: 2019年5月21日
 *      Author: DELL
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "template.h"

#include "application.h"

#ifdef APP_X5BA_MODULE

#ifdef PL_OPENWRT_UCI
extern int x5_b_ubus_uci_update_cb(char *buf, int len);
#endif
/*
 *
 * X5-B MGT module
 *
 */
static int app_write_config(struct vty *vty, void *pVoid);



DEFUN (app_template,
		app_template_cmd,
		"template app esp",
		"Template configure\n"
		"APP configure\n"
		"ESP configure\n")
{
	//int ret = ERROR;
	template_t * temp = nsm_template_lookup_name ("app esp");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		return CMD_SUCCESS;
	}
	else
	{
		temp = nsm_template_new ();
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "app esp");
			strcpy(temp->prompt, "app-esp"); /* (config-app-esp)# */
			temp->write_template = app_write_config;
			temp->pVoid = x5b_app_tmp();
			nsm_template_install(temp, 0);

			vty->node = TEMPLATE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_app_template,
		no_app_template_cmd,
		"no template app esp ",
		NO_STR
		"Template configure\n"
		"APP configure\n"
		"ESP configure\n")
{
	template_t * temp = nsm_template_lookup_name ("app esp");
	if(temp)
	{
		x5b_app_free();
		nsm_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

// remote
DEFUN (x5_esp_address_port,
		x5_esp_address_port_cmd,
		"ip esp (toc|toa) port <1024-65536>",
		IP_STR
		"ESP configure\n"
		"TO C-module\n"
		"TO A-module\n"
		"Port Configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "toc"))
		ret = x5b_app_port_set_api(E_CMD_TO_C, atoi(argv[1]));
	else if(strstr(argv[0], "toa"))
		ret = x5b_app_port_set_api(E_CMD_TO_A, atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN(no_x5_esp_address_port,
		no_x5_esp_address_port_cmd,
		"no ip esp (toc|toa) port",
		NO_STR
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "toc"))
		ret = x5b_app_port_set_api(E_CMD_TO_C, 0);
	else if(strstr(argv[0], "toa"))
		ret = x5b_app_port_set_api(E_CMD_TO_A, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (x5_esp_keepalive_interval,
		x5_esp_keepalive_interval_cmd,
		"ip esp (toc|toa) keepalive-interval <1-256>",
		IP_STR
		"ESP configure\n"
		"TO C-module\n"
		"TO A-module\n"
		"Port Configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "toc"))
		ret = x5b_app_interval_set_api(E_CMD_TO_C, atoi(argv[1]));
	else if(strstr(argv[0], "toa"))
		ret = x5b_app_interval_set_api(E_CMD_TO_A, atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN(no_x5_esp_keepalive_interval,
		no_x5_esp_keepalive_interval_cmd,
		"no ip esp (toc|toa) keepalive-interval",
		NO_STR
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "toc"))
		ret = x5b_app_interval_set_api(E_CMD_TO_C, 0);
	else if(strstr(argv[0], "toa"))
		ret = x5b_app_interval_set_api(E_CMD_TO_A, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//local
DEFUN (x5_esp_local_address,
		x5_esp_local_address_cmd,
		"ip esp local address " CMD_KEY_IPV4,
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	ret = x5b_app_local_address_set_api((argv[0]));
	if(argc == 2)
		ret = x5b_app_local_port_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(x5_esp_local_address,
		x5_esp_local_address_port_cmd,
		"ip esp local address " CMD_KEY_IPV4 " port <1-65535>",
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n"
		"port value");


DEFUN (no_x5_esp_local_address,
		no_x5_esp_local_address_cmd,
		"no ip esp local address",
		IP_STR
		"ESP configure\n"
		"IP Address\n")
{
	int ret = ERROR;
	ret = x5b_app_local_address_set_api("192.168.1.111");
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN(no_x5_esp_local_address_port,
		no_x5_esp_local_address_port_cmd,
		"no ip esp local address " CMD_KEY_IPV4 " port",
		NO_STR
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n")
{
	int ret = ERROR;
	ret = x5b_app_local_port_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//debug
DEFUN (debug_x5_esp,
		debug_x5_esp_cmd,
		"debug esp (event|recv|send|hex|upgrade)",
		DEBUG_STR
		"ESP configure\n"
		"Event info\n"
		"Recv info\n"
		"Send info\n"
		"HEX info\n"
		"Upgrade info\n")
{
	if(strstr(argv[0], "event"))
		X5_B_ESP32_DEBUG_ON(EVENT);
	else if(strstr(argv[0], "recv"))
		X5_B_ESP32_DEBUG_ON(RECV);
	else if(strstr(argv[0], "send"))
		X5_B_ESP32_DEBUG_ON(SEND);
	else if(strstr(argv[0], "hex"))
		X5_B_ESP32_DEBUG_ON(HEX);
	else if(strstr(argv[0], "upgrade"))
		X5_B_ESP32_DEBUG_ON(UPDATE);
	return  CMD_SUCCESS;
}

DEFUN (no_debug_x5_esp,
		no_debug_x5_esp_cmd,
		"no debug esp (event|recv|send|hex|upgrade)",
		NO_STR
		DEBUG_STR
		"ESP configure\n"
		"Event info\n"
		"Recv info\n"
		"Send info\n"
		"HEX info\n"
		"Upgrade info\n")
{
	if(strstr(argv[0], "event"))
		X5_B_ESP32_DEBUG_OFF(EVENT);
	else if(strstr(argv[0], "recv"))
		X5_B_ESP32_DEBUG_OFF(RECV);
	else if(strstr(argv[0], "send"))
		X5_B_ESP32_DEBUG_OFF(SEND);
	else if(strstr(argv[0], "hex"))
		X5_B_ESP32_DEBUG_OFF(HEX);
	else if(strstr(argv[0], "upgrade"))
		X5_B_ESP32_DEBUG_OFF(UPDATE);
	return  CMD_SUCCESS;
}

DEFUN (show_x5b_app_config,
		show_x5b_app_config_cmd,
		"show ip esp (config|state)",
		SHOW_STR
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	if(strstr(argv[0],"config"))
		x5b_app_show_config(vty);
	else
		x5b_app_show_state(vty);
	return CMD_SUCCESS;
}


DEFUN (show_x5b_app_param,
		show_x5b_app_param_cmd,
		"show ip esp param (global|open|face)",
		SHOW_STR
		IP_STR
		"X5-B Mgt A configure\n"
		"Param Information\n"
		"Global Information\n"
		"Open Information\n"
		"Face Information\n")
{
	if(strstr(argv[0], "global"))
		x5b_app_show_param(vty, 0);
	else if(strstr(argv[0], "open"))
		x5b_app_show_param(vty, 1);
	else if(strstr(argv[0], "face"))
		x5b_app_show_param(vty, 2);
	return CMD_SUCCESS;
}


#ifdef X5B_APP_TEST_DEBUG
DEFUN (register_ok_test_cmd,
		register_ok_test_cmd_cmd,
		"result register (toa|toc)",
		IP_STR
		"SIP configure\n"
		"RTP port\n"
		"port number\n")
{
	int pl = 0;
	if(argc == 2)
		pl = 1;
	if(strstr(argv[0], "toa"))
		x5b_app_test_register(E_CMD_TO_A, pl);
	else if(strstr(argv[0], "toc"))
		x5b_app_test_register(E_CMD_TO_C, pl);
	return  CMD_SUCCESS;
}

ALIAS(register_ok_test_cmd,
		register_ok_test1_cmd_cmd,
		"result register (toa|toc) (payload|)",
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n"
		"port value");


DEFUN (x5b_app_call_test,
	   x5b_app_call_test_cmd,
		"x5b-call <1000-9999>",
		"x5b Call Configure\n"
		"Stop Configure\n")
{
	int ret = ERROR;
	{
		ret = x5b_app_test_call(atoi(argv[0]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#endif
/*
DEFUN (x5b_app_update_test,
		x5b_app_update_test_cmd,
		"ip esp update FILENAME",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	x5b_app_upgrade_handle(WEB_UPLOAD_BASE,argv[0]);
	return CMD_SUCCESS;
}*/

#ifdef X5B_APP_DATABASE
DEFUN (x5b_app_reset,
	   x5b_app_reset_cmd,
		"x5b app clear-all",
		"X5-B configure\n"
		"App Information\n"
		"Clear All Configure\n")
{
/*	int c = 0;
	vty_sync_out(vty, "Please enter for confirmation.(y/n)");
	c = vty_getc_input(vty);
	if(c == 'y')
	{*/
#ifdef PL_OPENWRT_UCI
		remove("/app/etc/dbase");
		remove("/app/etc/card");
		remove("/app/etc/thlog.log");
#endif
		sync();
/*		vty_sync_out(vty, "If you need to restart system, Please enter for confirmation.(y/n)");
		c = vty_getc_input(vty);
		if(c == 'y')*/
		{
#ifdef PL_OPENWRT_UCI
			x5_b_ubus_uci_update_cb("reboot -c f", strlen("reboot -c f"));
#endif
			//super_system("lua-sync -m reboot -c f");
			super_system("reboot -d 1 -f");
		}
	//}
	return CMD_SUCCESS;
}
#endif

DEFUN (x5b_app_wiggins_test,
	   x5b_app_wiggins_test_cmd,
		"ip esp wiggins (26|34|66)",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	x5b_app_wiggins_setting(NULL, atoi(argv[0]), E_CMD_TO_A);
	return CMD_SUCCESS;
}

#ifdef X5B_APP_DATABASE
DEFUN (x5b_app_cardid_test,
	   x5b_app_cardid_test_cmd,
		"ip esp cardid ID",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	u_int8     ID[8];
	card_id_string_to_hex(argv[0], strlen(((char *)argv[0])), ID);
	x5b_app_open_door_by_cardid(NULL, ID, strlen(((char *)argv[0]))/2, E_CMD_TO_A);
	return CMD_SUCCESS;
}

DEFUN (x5b_app_del_cardid_test,
	   x5b_app_del_cardid_test_cmd,
		"ip esp delete-cardid ID",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	x5b_app_delete_card(NULL, argv[0], E_CMD_TO_A);
	return CMD_SUCCESS;
}
#endif


DEFUN (x5b_app_face_show_test,
	   x5b_app_face_show_test_cmd,
		"ip esp face-show USERID",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	;//x5b_app_face_img_show(NULL, argv[0]);
	return CMD_SUCCESS;
}


DEFUN (x5b_app_start_test,
		x5b_app_start_test_cmd,
		"ip esp unit-test (start|stop)",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	if(strstr(argv[0], "stop"))
		x5b_app_test_stop();
	else
		x5b_app_test_start();
	return CMD_SUCCESS;
}

DEFUN (x5b_app_start_test_call,
		x5b_app_start_test_call_cmd,
		"ip esp unit-test call NUM",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	x5_b_app_test_call_phonenum(argv[0]);
	//x5_b_app_test_call_phone(argv[0]);
	return CMD_SUCCESS;
}

DEFUN (x5b_app_start_test_call_list,
		x5b_app_start_test_call_list_cmd,
		"ip esp unit-test call-list",
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	x5_b_app_test_call_list();
	return CMD_SUCCESS;
}




static int app_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "template app esp%s",VTY_NEWLINE);
		x5b_app_show_config(vty);
		return 1;
	}
	return 0;
}


void cmd_app_x5b_init(void)
{

//	install_default(APP_TEMPLATES_NODE);
//	install_default_basic(APP_TEMPLATES_NODE);
//
//	reinstall_node(APP_TEMPLATES_NODE, app_write_config);
#ifdef PL_VOIP_MODULE
	if(voip_global_enabled())
#endif
	{
		template_t * temp = nsm_template_new ();
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "app esp");
			strcpy(temp->prompt, "app-esp"); /* (config-app-esp)# */
			//temp->prompt[64];

			//temp->id;

			//temp->pVoid;
			temp->pVoid = x5b_app_tmp();
			temp->write_template = app_write_config;
			//temp->show_template = app_write_config;

			nsm_template_install(temp, 0);
		}
	/*
		extern void nsm_template_free (template_t *template);
		extern void nsm_template_install (template_t *template, int module);
		extern template_t* nsm_template_lookup (int module);
		extern template_t* nsm_template_lookup_name (char * name);
	*/
		install_element(CONFIG_NODE, &app_template_cmd);
		install_element(CONFIG_NODE, &no_app_template_cmd);

	/*
		install_element(TEMPLATE_NODE, &x5_esp_address_cmd);
		install_element(TEMPLATE_NODE, &x5_esp_address_port_cmd);

		install_element(TEMPLATE_NODE, &no_x5_esp_address_cmd);
		install_element(TEMPLATE_NODE, &no_x5_esp_address_port_cmd);
	*/
		install_element(TEMPLATE_NODE, &x5_esp_address_port_cmd);
		install_element(TEMPLATE_NODE, &no_x5_esp_address_port_cmd);

		install_element(TEMPLATE_NODE, &x5_esp_keepalive_interval_cmd);
		install_element(TEMPLATE_NODE, &no_x5_esp_keepalive_interval_cmd);

		/*
		 * local
		 */
		install_element(TEMPLATE_NODE, &x5_esp_local_address_cmd);
		install_element(TEMPLATE_NODE, &x5_esp_local_address_port_cmd);

		install_element(TEMPLATE_NODE, &no_x5_esp_local_address_cmd);
		install_element(TEMPLATE_NODE, &no_x5_esp_local_address_port_cmd);


		install_element(ENABLE_NODE, &debug_x5_esp_cmd);
		install_element(ENABLE_NODE, &no_debug_x5_esp_cmd);

		install_element(ENABLE_NODE, &show_x5b_app_config_cmd);
		install_element(CONFIG_NODE, &show_x5b_app_config_cmd);
		install_element(TEMPLATE_NODE, &show_x5b_app_config_cmd);

		install_element(ENABLE_NODE, &show_x5b_app_param_cmd);
		install_element(CONFIG_NODE, &show_x5b_app_param_cmd);
		install_element(TEMPLATE_NODE, &show_x5b_app_param_cmd);

		//install_element(ENABLE_NODE, &x5b_app_update_test_cmd);


		install_element(ENABLE_NODE, &x5b_app_wiggins_test_cmd);
#ifdef X5B_APP_DATABASE
		install_element(ENABLE_NODE, &x5b_app_reset_cmd);
		install_element(ENABLE_NODE, &x5b_app_face_show_test_cmd);
		install_element(ENABLE_NODE, &x5b_app_cardid_test_cmd);
		install_element(ENABLE_NODE, &x5b_app_del_cardid_test_cmd);
#endif

		//unit test cmd
		install_element(ENABLE_NODE, &x5b_app_start_test_cmd);
		install_element(ENABLE_NODE, &x5b_app_start_test_call_cmd);
		install_element(ENABLE_NODE, &x5b_app_start_test_call_list_cmd);
		//debug cmd
#ifdef X5B_APP_TEST_DEBUG
		install_element(ENABLE_NODE, &register_ok_test_cmd_cmd);
		install_element(ENABLE_NODE, &register_ok_test1_cmd_cmd);
		install_element(ENABLE_NODE, &x5b_app_call_test_cmd);
#endif
	}
}
#endif
