#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "host.h"
#include <modbus.h>
#include "modbus-api.h"
#include "os_task.h"

static const uint16_t UT_BITS_ADDRESS = 0;	//线圈状态寄存器起始地址
static const uint16_t UT_BITS_NB = 8;		//线圈状态寄存器数量
static uint8_t UT_BITS_TAB[] = { 0xCD}; //指向线圈状态寄存器值

static const uint16_t UT_INPUT_BITS_ADDRESS = 0;//离散输入寄存器起始地址
static const uint16_t UT_INPUT_BITS_NB = 8;//离散输入寄存器数量
static uint8_t UT_INPUT_BITS_TAB[] = { 0xAC};//指向离散输入寄存器值

static const uint16_t UT_REGISTERS_ADDRESS = 0;//保持寄存器起始地址
static const uint16_t UT_REGISTERS_NB = 3;//保持寄存器数量
static uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };//指向保持寄存器值

static const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0;//输入寄存器起始地址
static const uint16_t UT_INPUT_REGISTERS_NB = 0x3;//输入寄存器数量
static uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A,0x000B,0x000C };//指向输入寄存器值



modbus_config_t *modbus_config = NULL;



MODBUS_API int modbus_mode_set_api(modbus_mode_t mode)
{
	if(modbus_config)
	{
		modbus_config->mode = mode;
		return OK;
	}
	return ERROR;
}

MODBUS_API modbus_mode_t modbus_mode_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->mode;
	}
	return MODBUS_TYPE_NONE;
}

int modbus_type_set_api(modbus_type_t type)
{
	if(modbus_config)
	{
		modbus_config->type = type;
		return OK;
	}
	return ERROR;
}

modbus_type_t modbus_type_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->type;
	}
	return MODBUS_TYPE_NONE;
}

int modbus_id_set_api(zpl_uint32 id)
{
	if(modbus_config)
	{
		modbus_config->slave_id = id;
		return OK;
	}
	return ERROR;
}

zpl_uint32 modbus_id_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->slave_id;
	}
	return 0;
}

int modbus_port_set_api(zpl_uint16 port)
{
	if(modbus_config)
	{
		modbus_config->port = port;
		return OK;
	}
	return ERROR;
}

zpl_uint16 modbus_port_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->port;
	}
	return 0;
}

int modbus_device_set_api(zpl_char *name)
{
	if(modbus_config)
	{
		if(modbus_config->address)
			free(modbus_config->address);
		modbus_config->address = strdup(name);
		return OK;
	}
	return ERROR;
}

zpl_char* modbus_device_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->address;
	}
	return NULL;
}

int modbus_baudrate_set_api(zpl_uint32 baudrate)
{
	if(modbus_config)
	{
		modbus_config->baudrate = baudrate;
		return OK;
	}
	return ERROR;
}

zpl_uint32 modbus_baudrate_get_api(void)
{
	if(modbus_config)
	{
		return modbus_config->baudrate;
	}
	return MODBUS_TYPE_NONE;
}


int modbus_hold_registers_set_api(zpl_double tempAverage, zpl_double tempMax, zpl_double tempMin)
{
	if(modbus_config && modbus_config->tab_registers)
	{
		modbus_config->tab_registers->tab_registers[0] = (uint16_t)(tempAverage * 100);
		modbus_config->tab_registers->tab_registers[1] = (uint16_t)(tempMax * 100);
		modbus_config->tab_registers->tab_registers[2] = (uint16_t)(tempMin * 100);
		return OK;
	}
	return ERROR;
}

int modbus_input_registers_set_api(zpl_double tempAverage, zpl_double tempMax, zpl_double tempMin)
{
	if(modbus_config && modbus_config->tab_registers)
	{
		modbus_config->tab_registers->tab_input_registers[0] = (uint16_t)(tempAverage * 100);
		modbus_config->tab_registers->tab_input_registers[1] = (uint16_t)(tempMax * 100);
		modbus_config->tab_registers->tab_input_registers[2] = (uint16_t)(tempMin * 100);
		return OK;
	}
	return ERROR;
}

int modbus_start_api(void)
{
	if(modbus_config)
	{
		zpl_uint32 i = 0;
		if (modbus_config->type == MODBUS_TYPE_TCP && modbus_config->address) {
			modbus_config->ctx = modbus_new_tcp(modbus_config->address, modbus_config->port ? modbus_config->port:1502);
			modbus_config->query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
		} else if (modbus_config->type == MODBUS_TYPE_TCP_PI && modbus_config->address) {
			modbus_config->ctx = modbus_new_tcp_pi(modbus_config->address, "1502");
			modbus_config->query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
		} else if(modbus_config->address && modbus_config->baudrate){
			modbus_config->ctx = modbus_new_rtu(modbus_config->address/*"/dev/ttyUSB0"*/, modbus_config->baudrate/*115200*/, 'N', 8, 1);
			modbus_set_slave(modbus_config->ctx, modbus_config->slave_id);
			modbus_config->query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
		}
		else
			return ERROR;

		modbus_set_debug(modbus_config->ctx, TRUE);
		/* 0x、1x、3x、4x共4个区 寄存器的 缓存申请 */
		modbus_config->tab_registers = modbus_mapping_new_start_address(
			UT_BITS_ADDRESS, UT_BITS_NB,
			UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
			UT_REGISTERS_ADDRESS, UT_REGISTERS_NB,
			UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB);
		if (modbus_config->tab_registers == NULL) {
			fprintf(stderr, "Failed to allocate the mapping: %s\n",
					modbus_strerror(ipstack_errno));
			modbus_free(modbus_config->ctx);
			return ERROR;
		}

		/* Examples from PI_MODBUS_300.pdf.
		Only the read-only input values are assigned. */

		/* Initialize input values that's can be only done server side. */
		modbus_set_bits_from_bytes(modbus_config->tab_registers->tab_input_bits, 0, UT_INPUT_BITS_NB,
								UT_INPUT_BITS_TAB);

		modbus_set_bits_from_bytes(modbus_config->tab_registers->tab_bits, 0, UT_BITS_NB,
								UT_BITS_TAB);

		/* Initialize values of INPUT REGISTERS */
		for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
			modbus_config->tab_registers->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];;
		}

		for (i=0; i < UT_REGISTERS_NB; i++) {
			modbus_config->tab_registers->tab_registers[i] = UT_REGISTERS_TAB[i];;
		}
		modbus_config->running = zpl_true;
		return OK;
	}
	return ERROR;
}



int modbus_stop_api(void)
{
	if(modbus_config)
	{
		if(modbus_config->running)
		{
			modbus_config->running = zpl_false;
			modbus_config->waiting = zpl_true;
			while(modbus_config->waiting)
			#ifdef OS_WIN32
				Sleep(100);
			#else
				usleep(10000);
			#endif
		}
		if(modbus_config->tab_registers)
		{
			modbus_mapping_free(modbus_config->tab_registers);
			modbus_config->tab_registers = NULL;
		}
		if(modbus_config->query)
		{
			free(modbus_config->query);
			modbus_config->query = NULL;
		}
		/* For RTU */
		if(modbus_config->ctx)
		{
			modbus_close(modbus_config->ctx);
			modbus_free(modbus_config->ctx);
			modbus_config->ctx = NULL;
		}
		return OK;
	}
	return ERROR;
}
#ifdef OS_WIN32
static DWORD WINAPI modbus_main_task (LPVOID *p)
#else
static int modbus_main_task(void *p)
#endif
{
    zpl_socket_t s;
    int rc = 0;
	host_waitting_loadconfig();
	while (modbus_config && modbus_config->running == zpl_false)
#ifdef OS_WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	if(modbus_config && modbus_config->ctx && modbus_config->tab_registers && modbus_config->query)
	{
		while(modbus_config->running == zpl_false)
		#ifdef OS_WIN32
			Sleep(1000);
		#else
			sleep(1);
		#endif
		if(modbus_config->mode == MODBUS_MODE_SERVER || modbus_config->mode == MODBUS_MODE_SLAVE)
		{
			if (modbus_config->type == MODBUS_TYPE_TCP) {
				s = modbus_tcp_listen(modbus_config->ctx, 1);
			} else if (modbus_config->type == MODBUS_TYPE_TCP_PI) {
				s = modbus_tcp_pi_listen(modbus_config->ctx, 1);
			}
		}
		else
		{
			rc = modbus_connect(modbus_config->ctx);
			if (rc == -1) {
				fprintf(stderr, "Unable to connect %s\n", modbus_strerror(ipstack_errno));
				modbus_free(modbus_config->ctx);
				modbus_config->waiting = zpl_false;
				return -1;
			}
		}
		while (modbus_config->running) 
		{
			s = modbus_get_socket(modbus_config->ctx);
			if(ipstack_invalid(s))
			{
				if(modbus_config->mode == MODBUS_MODE_SERVER || modbus_config->mode == MODBUS_MODE_SLAVE)
				{
					if (modbus_config->type == MODBUS_TYPE_TCP) {
						modbus_tcp_accept(modbus_config->ctx, &s);
					} else if (modbus_config->type == MODBUS_TYPE_TCP_PI) {
						modbus_tcp_pi_accept(modbus_config->ctx, &s);
					} 
				}
				else 
				{
					rc = modbus_connect(modbus_config->ctx);
					if (rc == -1) {
						fprintf(stderr, "Unable to connect %s\n", modbus_strerror(ipstack_errno));
						modbus_free(modbus_config->ctx);
						modbus_config->waiting = zpl_false;
						return -1;
					}
				}
			}

			do {
				rc = modbus_receive(modbus_config->ctx, modbus_config->query);
				/* Filtered queries return 0 */
			} while (rc == 0);

			/* The connection is not closed on errors which require on reply such as
			bad CRC in RTU. */
			if (rc == -1 && ipstack_errno != EMBBADCRC) {
				modbus_close(modbus_config->ctx);
				continue;
			}

			rc = modbus_reply(modbus_config->ctx, modbus_config->query, rc, modbus_config->tab_registers);
			if (rc == -1) {
				modbus_close(modbus_config->ctx);
				continue;
			}
		}
		//modbus_close(modbus_config->ctx);
		if (modbus_config->type == MODBUS_TYPE_TCP || modbus_config->type == MODBUS_TYPE_TCP_PI) 
		{
			if (!ipstack_invalid(s)) {
		#ifdef OS_WIN32
				closesocket(s);
		#else
				ipstack_close(s);
		#endif
			}
		}
		modbus_config->waiting = zpl_false;
	}
    return 0;
}



int modbus_module_init(void)
{
	if(modbus_config == NULL)
	{
		modbus_config = malloc(sizeof(modbus_config_t));
		if(modbus_config)
		{
			memset(modbus_config, 0, sizeof(modbus_config_t));
			return OK;
		}
		else
			return ERROR;
	}
	return OK;
}

int modbus_module_exit(void)
{
	return OK;
}

int modbus_module_task_init(void)
{
#ifdef OS_WIN32
	if(modbus_config)
		modbus_config->taskhandle = CreateThread(NULL, 0, modbus_main_task, modbus_config, 0, &modbus_config->taskid);
#else
	if(modbus_config)
		modbus_config->taskid = os_task_create("modbusTask", OS_TASK_DEFAULT_PRIORITY,
	               0, modbus_main_task, modbus_config, OS_TASK_DEFAULT_STACK*4);
#endif
	if(modbus_config->taskid > 0)
	{
		module_setup_task(MODULE_MODBUS, modbus_config->taskid);
		return OK;
	}
	return ERROR;
}

int modbus_module_task_exit(void)
{
	if(modbus_config)
	{
#ifdef OS_WIN32
		if(modbus_config->taskhandle)
			TerminateThread(modbus_config->taskhandle, 0);
#else
		if(modbus_config->taskid > 0)
			os_task_destroy(modbus_config->taskid);
#endif
	}
	return OK;
}

struct module_list module_list_modbus = 
{ 
	.module=MODULE_MODBUS, 
	.name="MODBUS\0", 
	.module_init=modbus_module_init, 
	.module_exit=modbus_module_exit, 
	.module_task_init=modbus_module_task_init, 
	.module_task_exit=modbus_module_task_exit, 
	.module_cmd_init=NULL, 
	.flags = ZPL_MODULE_NEED_INIT,
	.taskid=0,
};