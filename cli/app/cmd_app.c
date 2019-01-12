/*
 * cmd_voip.c
 *
 *  Created on: 2018��12��18��
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
			temp->pVoid = x5_b_a_app_tmp();
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
		x5_b_a_app_free();
		nsm_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

// remote
DEFUN (x5_esp_address,
		x5_esp_address_cmd,
		"ip esp address " CMD_KEY_IPV4,
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	ret = x5_b_a_address_set_api((argv[0]));
	if(argc == 2)
		ret = x5_b_a_port_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(x5_esp_address,
		x5_esp_address_port_cmd,
		"ip esp address " CMD_KEY_IPV4 " port <1-65535>",
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n"
		"port value");


DEFUN (no_x5_esp_address,
		no_x5_esp_address_cmd,
		"no ip esp address",
		IP_STR
		"ESP configure\n"
		"IP Address\n")
{
	int ret = ERROR;
	ret = x5_b_a_address_set_api("192.168.1.111");
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN(no_x5_esp_address_port,
		no_x5_esp_address_port_cmd,
		"no ip esp address " CMD_KEY_IPV4 " port",
		NO_STR
		IP_STR
		"ESP configure\n"
		"IP Address\n"
		CMD_KEY_IPV4_HELP
		"Port Configure\n")
{
	int ret = ERROR;
	ret = x5_b_a_port_set_api(0);
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
	ret = x5_b_a_local_address_set_api((argv[0]));
	if(argc == 2)
		ret = x5_b_a_local_port_set_api(atoi(argv[0]));
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
	ret = x5_b_a_local_address_set_api("192.168.1.111");
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
	ret = x5_b_a_local_port_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

//debug
DEFUN (debug_x5_esp,
		debug_x5_esp_cmd,
		"debug esp (event|recv|send|hex)",
		DEBUG_STR
		"ESP configure\n"
		"Event info\n"
		"Recv info\n"
		"Send info\n"
		"HEX info\n")
{
	if(strstr(argv[0], "event"))
		X5_B_ESP32_DEBUG_ON(EVENT);
	else if(strstr(argv[0], "recv"))
		X5_B_ESP32_DEBUG_ON(RECV);
	else if(strstr(argv[0], "send"))
		X5_B_ESP32_DEBUG_ON(SEND);
	else if(strstr(argv[0], "hex"))
		X5_B_ESP32_DEBUG_ON(HEX);
	return  CMD_SUCCESS;
}

DEFUN (no_debug_x5_esp,
		no_debug_x5_esp_cmd,
		"no debug esp (event|recv|send|hex)",
		NO_STR
		DEBUG_STR
		"ESP configure\n"
		"Event info\n"
		"Recv info\n"
		"Send info\n"
		"HEX info\n")
{
	if(strstr(argv[0], "event"))
		X5_B_ESP32_DEBUG_OFF(EVENT);
	else if(strstr(argv[0], "recv"))
		X5_B_ESP32_DEBUG_OFF(RECV);
	else if(strstr(argv[0], "send"))
		X5_B_ESP32_DEBUG_OFF(SEND);
	else if(strstr(argv[0], "hex"))
		X5_B_ESP32_DEBUG_OFF(HEX);
	return  CMD_SUCCESS;
}

DEFUN (show_x5_b_a_config,
		show_x5_b_a_config_cmd,
		"show ip esp (config|state)",
		SHOW_STR
		IP_STR
		"X5-B Mgt A configure\n"
		"Config Information\n")
{
	if(strstr(argv[0],"config"))
		x5_b_a_show_config(vty);
	else
		x5_b_a_show_state(vty);
	return CMD_SUCCESS;
}



#ifdef X5_B_A_DEBUG
DEFUN (result_test_cmd,
		result_test_cmd_cmd,
		"result (open|calling|recv) <0-10>",
		IP_STR
		"SIP configure\n"
		"RTP port\n"
		"port number\n")
{
	int val = atoi(argv[1]);
	if(strstr(argv[0], "open"))
		open_result_test(val);
	else if(strstr(argv[0], "calling"))
		call_result_test(val);
	else
		call_recv_test();
	return  CMD_SUCCESS;
}
#endif


static int app_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "template app esp%s",VTY_NEWLINE);
		x5_b_a_show_config(vty);
		return 1;
	}
	return 0;
}


void cmd_app_init(void)
{
//	install_default(APP_TEMPLATES_NODE);
//	install_default_basic(APP_TEMPLATES_NODE);
//
//	reinstall_node(APP_TEMPLATES_NODE, app_write_config);

	template_t * temp = nsm_template_new ();
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "app esp");
		strcpy(temp->prompt, "app-esp"); /* (config-app-esp)# */
		//temp->prompt[64];

		//temp->id;

		//temp->pVoid;
		temp->pVoid = x5_b_a_app_tmp();
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

	install_element(TEMPLATE_NODE, &x5_esp_address_cmd);
	install_element(TEMPLATE_NODE, &x5_esp_address_port_cmd);

	install_element(TEMPLATE_NODE, &no_x5_esp_address_cmd);
	install_element(TEMPLATE_NODE, &no_x5_esp_address_port_cmd);

	install_element(TEMPLATE_NODE, &x5_esp_local_address_cmd);
	install_element(TEMPLATE_NODE, &x5_esp_local_address_port_cmd);

	install_element(TEMPLATE_NODE, &no_x5_esp_local_address_cmd);
	install_element(TEMPLATE_NODE, &no_x5_esp_local_address_port_cmd);


	install_element(ENABLE_NODE, &debug_x5_esp_cmd);
	install_element(ENABLE_NODE, &no_debug_x5_esp_cmd);

	install_element(ENABLE_NODE, &show_x5_b_a_config_cmd);
	install_element(CONFIG_NODE, &show_x5_b_a_config_cmd);
	install_element(TEMPLATE_NODE, &show_x5_b_a_config_cmd);

#ifdef X5_B_A_DEBUG
	install_element(ENABLE_NODE, &result_test_cmd_cmd);
#endif
}
