/*
 * $Id: i2c_cmds.c,v 1.89 Broadcom SDK $
 *
 * I2C specific commands.
 * These commands are specific to the I2C driver and or I2C slave
 * devices which use that driver. See drv/i2c/ for more details.
 *
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 */
#ifndef NO_SAL_APPL

#ifdef INCLUDE_I2C

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>

#include <soc/cmext.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/i2c.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/bcmi2c.h>
#include <bcm/init.h>
#include <bcm/error.h>

#include <appl/diag/system.h>
#ifdef COMPILER_HAS_DOUBLE
#include <stdlib.h>
#endif

/* Decimal point formatting */
#define INT_FRAC_1PT(x) (x) / 10, (x) % 10
#define INT_FRAC_3PT(x) (x) / 1000, (x) % 1000
#define INT_FRAC_3PT_3(x) (x) / 1000000, ((x) / 1000) % 1000
#define INT_FRAC_2PT_4(x) (x) / 1000000, ((x) / 10000) % 100

#define I2C_MUX_VOLTAGE_CONTROL  0 
STATIC char * _i2c_mux_dev_name_get (int mux_type);
STATIC int dac_devs_init (int unit,int bx, char *mux_dev);


/*
 * Function: cmd_temperature
 * Purpose: Temperature and Temperature monitoring commands.
 */
char cmd_temperature_usage[] =
    "Usages:\n\t"
    "       temperature [show]\n\t"
    "           - show current temperature on all devices.\n\t"
    "       temperature [watch|nowatch] [delay]\n\t"
    "           - monitor temperatures in background, reporting changes.\n\t"
    "\n";

cmd_result_t
cmd_temperature(int unit, args_t *a)
{
    char *function = ARG_GET(a);
    char *interval = ARG_GET(a);
    uint32 delay = 5; /* Report stats every 5 seconds */
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;
    if (! function || ! sal_strncasecmp(function, "show", strlen(function)) ){
	soc_i2c_lm75_temperature_show(unit);
    } else if ( ! sal_strncasecmp(function,"watch",strlen(function)) ){
	if ( interval ) 
	    delay = parse_integer(interval);
	soc_i2c_lm75_monitor(unit, TRUE, delay);
    } else if ( ! sal_strncasecmp(function,"nowatch",strlen(function)) ){
	soc_i2c_lm75_monitor(unit, FALSE, 0);
    } else {
	return CMD_USAGE;
    }
    return CMD_OK;
}

/*
 * Function: cmd_pcie
 * Purpose: Display/Modify pcie config registers via I2C debug BUS
 */
char cmd_pcie_usage[] =
    "Usages:\n\t"
    "       pcie r4 [off] [nbytes]\n\t"
    "           - show specified number of pcie bytes starting at offset.\n\t"
    "       pcie w4 [off] [data] \n\t"
    "           - write data byte to specified pcie offset.\n\t"
    "\n";

cmd_result_t
cmd_pcie(int unit, args_t *a)
{
    char *function = ARG_GET(a);
    char *offset = ARG_GET(a);
    char *dw = ARG_GET(a);
    uint32 addr, data, i ;
    uint8* ptr, *dpb;
    int fd, rv = SOC_E_NONE;
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (!function || !offset || !dw)
	return CMD_USAGE ;
    
    if ( (fd = bcm_i2c_open(unit, I2C_PCIE_0, 0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), I2C_PCIE_0, bcm_errmsg(fd));
	return CMD_FAIL;
    }

    /* NOTE: will use first NVM device found : pcie0*/
    printk("%s: using device %s\n", ARG_CMD(a), soc_i2c_devname(unit, fd));
    addr = parse_integer(offset);
    data = parse_integer(dw);
    if ( ! sal_strncasecmp(function, "r4", 2) ){
	ptr = (uint8*)sal_alloc(4, "cmd_pcie");
	if(!ptr)
	    return CMD_USAGE;
	/* Pretty print data : one 32-byte page at a time*/
	for(i = 0; i < data; i++) {
            unsigned int r_len =4;
            if( (rv = bcm_i2c_read(unit, fd, addr + i * 4, ptr, &r_len)) < 0){
                printk("ERROR: read of %d bytes failed:%s\n",
                       parse_integer(dw), bcm_errmsg(rv));
            }
	    printk("%04x 0x%02x%02x%02x%02x\n",
            addr + i * 4,
            ptr[0],
            ptr[1],
            ptr[2],
            ptr[3]);
	}
	printk("\nRead %d bytes total\n", data); 
        sal_free(ptr);
    } else if ( ! sal_strncasecmp(function,"w4",2) ){
        dpb = (uint8 *)&data;
	if( (rv = bcm_i2c_write(unit, fd, addr, dpb, 4)) < 0){
	    printk("ERROR: write of byte at 0x%x failed:%s\n",
		   addr, bcm_errmsg(rv));
	}
    } else {
	return CMD_USAGE;
    }
    return CMD_OK;
}

/*
 * Function: cmd_nvram
 * Purpose: Display/Modify NVRAM via Atmel 24C64 64K I2C EEPROM chip.
 */
char cmd_nvram_usage[] =
    "Usages:\n\t"
    "       nvram r [off] [nbytes]\n\t"
    "           - show specified number of NVRAM bytes starting at offset.\n\t"
    "       nvram w [off] [data] \n\t"
    "           - write data byte to specified NVRAM offset.\n\t"
    "\n";

cmd_result_t
cmd_nvram(int unit, args_t *a)
{
    char *function = ARG_GET(a);
    char *offset = ARG_GET(a);
    char *dw = ARG_GET(a);
    uint32 addr, data, i ;
    uint8* ptr, db;
    int fd, rv = SOC_E_NONE;
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (!function || !offset || !dw)
	return CMD_USAGE ;
    
    if ( (fd = bcm_i2c_open(unit, I2C_NVRAM_0, 0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), I2C_NVRAM_0, bcm_errmsg(fd));
	return CMD_FAIL;
    }

    /* NOTE: will use first NVM device found : nvram0*/
    printk("%s: using device %s\n", ARG_CMD(a), soc_i2c_devname(unit, fd));
    addr = parse_integer(offset);
    data = parse_integer(dw);
    if ( ! sal_strncasecmp(function, "r", 1) ){
	ptr = (uint8*)sal_alloc(data, "cmd_nvram");
	if(!ptr)
	    return CMD_USAGE;
	printk("Reading %d bytes at address 0x%x\n\t", data,addr);
	if( (rv = bcm_i2c_read(unit, fd, addr, ptr, &data)) < 0){
	    printk("ERROR: read of %d bytes failed:%s\n",
		   parse_integer(dw), bcm_errmsg(rv));
	}
	/* Pretty print data : one 32-byte page at a time*/
	for(i = 0; i < data; i++){
	    printk("0x%02x ", ptr[i]);
	    if( (i < (data-1)) && ((i % 8) == 7))
		printk("\n\t");
	}
	printk("\nRead %d bytes total\n", data); 
        sal_free(ptr);
    } else if ( ! sal_strncasecmp(function,"w",1) ){
	db = (uint8)data;
	if( (rv = bcm_i2c_write(unit, fd, addr, &db, 1)) < 0){
	    printk("ERROR: write of byte at 0x%x failed:%s\n",
		   addr, bcm_errmsg(rv));
	}
    } else {
	return CMD_USAGE;
    }
    return CMD_OK;

}

/*
 * Function: cmd_adc
 * Purpose: Show MAX127 A/D Conversions for boards with the chip.
 */
char cmd_adc_usage[] =
    "Usages:\n\t"
    "       adc show num\n\t"
    "           - show MAX127 A/D Conversions (num = 0,1).\n\t"
    "       adc samples num\n\t"
    "           - set MAX127 A/D Conversions sample count.\n\t"
    "\n";
extern int soc_i2c_max127_iterations; /* Yes, a global, in max127.c, controls
			       * the number of samples taken by ADC.
			       */
cmd_result_t
cmd_adc(int unit, args_t *a)
{
    int fd;    
    uint32 num;
    char dev[25];
    char *function = ARG_GET(a);
    char *chip = ARG_GET(a);
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if ((! function) || (!chip))
	return CMD_USAGE ;

    num = parse_integer(chip);

    if(!sal_strncasecmp(function,"samples",strlen(function))){
	soc_i2c_max127_iterations = num;
	return CMD_OK;
    }

    sprintf(dev,"%s%d","adc",num);

    if ( (fd = bcm_i2c_open(unit, dev,0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), dev, bcm_errmsg(fd));
	return CMD_FAIL;
    }

    if ( (bcm_i2c_ioctl(unit, fd, -1, NULL, 0) < 0)) {
	printk("ERROR: failed to perform A/D conversions.\n");
    }

    return CMD_OK ;

}

/*
 * Function: cmd_xclk
 * Purpose: Set W2239x clock speed for PCI, SDRAM, or core clocks
 */
char cmd_xclk_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "       xclocks dump | r | w | core | sdram | pci [...]\n"
#else
    "       xclocks dump \n\t"
    "           - display all registers on CY22393x.\n\t"
    "       xclocks r <address> \n\t"
    "           - read CY2239x register at address.\n\t"
    "       xclocks w <address> <data> \n\t"
    "           - write CY22393 register data at address.\n\t"
    "       xclocks core [speed] \n\t"
    "           - set BCM56xx core clock speed.\n\t"
    "       xclocks sdram [speed] \n\t"
    "           - set BCM56xx SDRAM clock speed.\n\t"
    "       xclocks pci [speed] \n\t"
    "           - set BCM56xx PCI clock speed.\n"
    "In all cases, if a speed is not specified, the current is shown\n"
#endif
    ;

cmd_result_t
cmd_xclk(int unit, args_t *a)
{
    char *source = ARG_GET(a);
    char *speed = ARG_GET(a);
#ifdef COMPILER_HAS_DOUBLE
    double clockFVal = 0;
#else
    int clockFVal = 0;
#endif
    int cmd = 0;
    int fd ;
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (!source ) {
	printk("Error: no operation or source (core, etc.) specified!\n");
	return CMD_FAIL;
    }
    /* NOTE: on 5625 BB, MUX (lpt0)=1 */
    
    /* Setup ioctl command code */
    if(!strcmp(source,"core")){
	cmd = (speed != NULL) ? I2C_XPLL_SET_CORE : I2C_XPLL_GET_CORE;
    } else if (!strcmp(source, "pci")){
	cmd = (speed != NULL) ? I2C_XPLL_SET_PCI : I2C_XPLL_GET_PCI;
    } else if (!strcmp(source, "sdram")){
	cmd = (speed != NULL) ? I2C_XPLL_SET_SDRAM : I2C_XPLL_GET_SDRAM;
    } else if (!strcmp(source, "r") || !strcmp(source, "dump")){
	cmd = I2C_XPLL_GET_REG;
    } else if (!strcmp(source, "w")){
	cmd = I2C_XPLL_SET_REG;
    } else {
	printk("Invalid subcommand or clock source (%s)\n", source);
	return CMD_USAGE;
    }
    /* Open PLL. This should be a CY22393, but could be an older CY22150 */
    if ( (fd = bcm_i2c_open(unit, I2C_XPLL_0, 0, 0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), I2C_XPLL_0, bcm_errmsg(fd));
	return CMD_FAIL;
    }
    if ( I2C_XPLL_GET_REG == cmd ) {
	/* Read register */
	char* addr = speed;
	uint8 x;
	uint32 nbytes = 0;

	if( addr) { 
	    uint8 off = parse_integer(addr);
	    printk("Read register @%s\n", addr);
	    bcm_i2c_read(unit, fd, off, &x, &nbytes);
	    printk("pll[%x] = 0x%x\n", off, x);
	}
	else {
	    int i;
	    /* Assume the CY22393 driver */
	    printk("Read all registers ...\n");
	    for( i = 0x08; i <= 0x17; i++) { 
		bcm_i2c_read(unit, fd, i, &x, &nbytes);
		printk("pll[%x] = 0x%x\n", i, x);
	    }
	    for( i = 0x40; i <= 0x57; i++) { 
		bcm_i2c_read(unit, fd, i, &x, &nbytes);
		printk("pll[%x] = 0x%x\n", i, x);
	    }
	}

    } else if (I2C_XPLL_SET_REG == cmd ) {
	/* Write register */
	char* addr = speed;
	char* data = ARG_GET(a);
	uint8 off,val;
	printk("Write register\n");

	if ( !addr || !data) {
	    return CMD_USAGE;
	} else {
	    off = parse_integer(addr);
	    val = (uint8)parse_integer(data);
	    bcm_i2c_write(unit, fd, off, &val, 1);
	    printk("0x%x => pll[%x]\n", val, off);
	}
	
    } else { /* IOCTL's */

	/* Set clock speed */
	if ( speed) { 
#ifdef COMPILER_HAS_DOUBLE
	    clockFVal = atof( speed ) ;
#else
            /* Parse input in MHz and convert to Hz */
	    clockFVal = _shr_atof_exp10(speed, 6);
#endif
	    /* Set speed */
	    if ( (bcm_i2c_ioctl(unit, fd, cmd, &clockFVal, 0) < 0)) {
		printk("ERROR: failed to perform "
		       "clock speed configuration.\n");
		return CMD_FAIL;
	    }	    
#ifdef COMPILER_HAS_DOUBLE
	    printk("%s: %s clock set to %2.2fMhz\n",
		   I2C_XPLL_0, source, clockFVal ) ;
#else
	    printk("%s: %s clock set to %d.%02dMhz\n",
		   I2C_XPLL_0, source, 
                   INT_FRAC_2PT_4(clockFVal));
#endif
	} else {
	    /* Get clock speed */
	    if ( (bcm_i2c_ioctl(unit, fd, cmd,&clockFVal, 0) < 0)){
		printk("ERROR: failed to retrieve clock configuration.\n");
	    }
#ifdef COMPILER_HAS_DOUBLE
	    printk("%s: %s clock is %2.2fMhz\n",
		   I2C_XPLL_0, source, clockFVal ) ;	
#else
	    printk("%s: %s clock is %d.%02dMhz\n",
		   I2C_XPLL_0, source, 
                   INT_FRAC_2PT_4(clockFVal));
#endif
	}
    }
    return CMD_OK;
}

/*
 * Function: cmd_poe
 * Purpose: Read and write registers in the LTC4258 chip.
 */
char cmd_poe_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "       poe poe_num dump | status | clear | r | w\n"
#else
    "       poe poe_num dump \n\t"
    "           - display all registers on LTC4258[poe_num].\n\t"
    "       poe poe_num status \n\t"
    "           - display port status on LTC4258[poe_num].\n\t"
    "       poe poe_num clear \n\t"
    "           - clear interrupt bits on LTC4258[poe_num].\n\t"
    "       poe poe_num r <address> \n\t"
    "           - read LTC4258[poe_num] register at address.\n\t"
    "       poe poe_num w <address> <data> \n\t"
    "           - write LTC4258[poe_num]  register data at address.\n"
    "In all cases, poe_num = [1..6]. MuxSel may also be necessary.\n"
#endif
    ;

cmd_result_t
cmd_poe(int unit, args_t *a)
{

#define I2C_POE_GET_REG    0x10
#define I2C_POE_SET_REG    0x20
#define TEXTBUFFER_SIZE 1024

    char *num_text = ARG_GET(a);
    char *op_text  = ARG_GET(a);
    char *addr_text = ARG_GET(a);
    char *device[6] = {
      I2C_POE_1, I2C_POE_2, I2C_POE_3,
      I2C_POE_4, I2C_POE_5, I2C_POE_6
    };
    char* data_text;
    int poe_num, cmd = 0;
    int fd ;

    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (!num_text) {
	printk("ERROR: no PoE index specified!\n");
	return CMD_FAIL;
    }

    if ( ((poe_num = parse_integer(num_text)) < 1) ||
	 (poe_num > 6) ) {
	printk("ERROR: invalid PoE poe_num!\n");
	return CMD_FAIL;
    }

    if (!op_text ) {
	printk("ERROR: no operation specified!\n");
	return CMD_FAIL;
    }
    
    /* Setup ioctl command code */
    if (!strcmp(op_text, "dump") || !strcmp(op_text, "r")) {
	cmd = I2C_POE_GET_REG;
    } else if (!strcmp(op_text, "status")) {
	cmd = I2C_POE_IOC_STATUS;
    } else if (!strcmp(op_text, "clear")) {
	cmd = I2C_POE_IOC_CLEAR;
    } else if (!strcmp(op_text, "rescan")) {
	cmd = I2C_POE_IOC_RESCAN;
    } else if (!strcmp(op_text, "w")) {
	cmd = I2C_POE_SET_REG;
    } else {
	printk("Invalid operation (%s)\n", op_text);
	return CMD_USAGE;
    }

    /* Open the LTC4258. */
    if ( (fd = bcm_i2c_open(unit, device[poe_num-1], 0, 0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), device[poe_num-1], bcm_errmsg(fd));
	return CMD_FAIL;
    }
    if ( I2C_POE_GET_REG == cmd ) {
	/* Read register */
	uint8 x;
	uint32 nbytes = 0;

	if( addr_text) { 
	    uint8 off = parse_integer(addr_text);
	    printk("Read register [0x%02x]\n", off);
	    bcm_i2c_read(unit, fd, off, &x, &nbytes);
	    printk("%s[0x%02x] = 0x%02X\n", device[poe_num-1], off, x);
	}
	else {
	  data_text = (char *)sal_alloc(TEXTBUFFER_SIZE, "cmd_poe");
	  cmd = I2C_POE_IOC_REG_DUMP;
	  if ( (bcm_i2c_ioctl(unit, fd, cmd, data_text, TEXTBUFFER_SIZE) < 0)) {
	    printk("ERROR: failed to dump PoE chip registers.\n");
	    if (data_text)
	      sal_free(data_text);
	    return CMD_FAIL;
	  }	    
	  printk("%s\n", data_text);
	  sal_free(data_text);
	}
    } 
    else if (I2C_POE_SET_REG == cmd ) {
	/* Write register */
	uint8 off,val;
	data_text = ARG_GET(a);
	printk("Write register\n");

	if ( !addr_text || !data_text) {
	    return CMD_USAGE;
	} else {
	    off = parse_integer(addr_text);
	    val = (uint8)parse_integer(data_text);
	    bcm_i2c_write(unit, fd, off, &val, 1);
	    printk("0x%02X => %s[0x%02x]\n", val, device[poe_num-1], off);
	}
    }

    else if (I2C_POE_IOC_CLEAR == cmd ) {
        /* Clear any PoE interrupts */
	if ( (bcm_i2c_ioctl(unit, fd, cmd, NULL, 0) < 0)) {
	  printk("ERROR: %s Clear_Interrupts IOCTL failed.\n",
		 device[poe_num-1]);
	  return CMD_FAIL;
	}	    
    } 

    else if (I2C_POE_IOC_RESCAN == cmd ) {
        /* Re-scan (detection and class) */
	if ( (bcm_i2c_ioctl(unit, fd, cmd, NULL, 0) < 0)) {
	  printk("ERROR: %s ReScan IOCTL failed.\n", device[poe_num-1]);
	  return CMD_FAIL;
	}	    
    } 

    else if (I2C_POE_IOC_STATUS == cmd ) {
        /* Use new IOCTL method to get text information */
	data_text = (char *)sal_alloc(TEXTBUFFER_SIZE, "cmd_poe");
	if ( (bcm_i2c_ioctl(unit, fd, cmd, data_text, TEXTBUFFER_SIZE) < 0)) {
	  printk("ERROR: %s Status IOCTL failed.\n", device[poe_num-1]);
	  if (data_text)
	    sal_free(data_text);
	  return CMD_FAIL;
	}	    
	printk("%s\n", data_text);
	sal_free(data_text);
    } 
	
    else { /* other IOCTL's */

      /* Fancy Shmancy IOCTLs under construction 
       * I2C_POE_IOC_ENABLE_PORT
       * I2C_POE_IOC_AUTO
       * I2C_POE_IOC_DISABLE_PORT
       * I2C_POE_IOC_SHUTDOWN
       */

    }
    return CMD_OK;
} /* end cmd_poe() */

typedef enum {
    /* Legacy PD was detected */
    POE_PD_PSTAT_ON_C = 0,

    /* 802.3af-compliant PD was detected */
    POE_PD_PSTAT_ON_R = 1,

    /* Mains voltage is higher than limits; all ports shut down */
    POE_PD_PSTAT_OFF_HI = 6,

    /* Mains voltage is lower than limits; all ports shut down */
    POE_PD_PSTAT_OFF_LO = 7,

    /* Hardware pin disables all ports  */
    POE_PD_PSTAT_OFF_HW = 8,

    /* There are fewer ports available than set */
    POE_PD_PSTAT_OFF_NULL = 12,

    /* Interim state during power up */
    POE_PD_PSTAT_OFF_POWER_UP = 17,

    /* Port does not respond or hardware is at fault */
    POE_PD_PSTAT_OFF_HW_INT = 18,

    /* User command set port to off */
    POE_PD_PSTAT_OFF_USER_CMD = 26,

    /* Interim state during line detection */
    POE_PD_PSTAT_OFF_DET = 27,

    /* Non-standard PD connected */
    POE_PD_PSTAT_OFF_NON_PD = 28,

    /* Succession of under/over load states caused port shutdown */
    POE_PD_PSTAT_OFF_UNDER_OVER = 29,

    /* Underload state according to 802.3af or Signature capacitor < 22uF */
    POE_PD_PSTAT_OFF_UNDER_CAP = 30,

    /* Overload state accourding to 802.3af or Signature capacitor > 1000uF */
    POE_PD_PSTAT_OFF_OVER_CAP = 31,

    /* Power management fn shut down port due to lack of power */
    POE_PD_PSTAT_OFF_LACK = 32,

    /* Hardware problems preventing port operation */
    POE_PD_PSTAT_OFF_INT_HW = 33,

    /* Port fails capacitor detection due to voltage being applied */
    POE_PD_PSTAT_OFF_VOL_INJ = 36,

    /* Port fails capacitor detection due to out-out-range capacitor value */
    POE_PD_PSTAT_OFF_CAP_OUT = 37,

    /* Port fails capacitor detection due to discharged capacitor */
    POE_PD_PSTAT_OFF_CAP_DET = 38,

    /* Port is forced on, unless systems error occurs */
    POE_PD_PSTAT_ON_FORCE = 43,

    /* Underfined error during force on */
    POE_PD_PSTAT_ON_UNDEF = 44,

    /* Supply voltage higher than settings */
    /*   (error appears only after port is forced on) */
    POE_PD_PSTAT_ON_VOL_HI = 45,

    /* Supply voltage lower than settings */
    POE_PD_PSTAT_ON_VOL_LO = 46,

    /* Disable_PDU flags was raised during force on */
    POE_PD_PSTAT_ON_PDU_FLAG = 47,

    /* Disabling is done by "Set on/off" command */
    POE_PD_PSTAT_ON_OFF_CMD = 48,

    /* Overload condition according to 802.3af */
    POE_PD_PSTAT_OFF_OVERLOAD = 49,

    /* Power management mechanism detects out of power budget */
    POE_PD_PSTAT_OFF_BUDGET = 50,

    /* Communication error with ASIC, after force on command */
    POE_PD_PSTAT_ON_ERR_ASIC = 51
} poe_pd_port_status_t;

STATIC char*
poe_pd_decode_port_status(poe_pd_port_status_t status)
{
    switch (status) {
    case POE_PD_PSTAT_ON_C:	
         return "On (valid capacitor detected)";
    case POE_PD_PSTAT_ON_R:   
         return "On (valid resistor detected)";
    case POE_PD_PSTAT_OFF_HI:  
         return "Off (main supply voltage higher than permitted)";
    case POE_PD_PSTAT_OFF_LO: 
         return "Off (main supply voltage lower than permitted)";
    case POE_PD_PSTAT_OFF_HW: 
         return "Off ('disable all ports' pin is active)";
    case POE_PD_PSTAT_OFF_NULL: 
         return "Off (non-existing port number)";
    case POE_PD_PSTAT_OFF_POWER_UP: 
         return "Off (power-up is in process)";
    case POE_PD_PSTAT_OFF_HW_INT: 
         return "Off (internal hardware fault)";
    case POE_PD_PSTAT_OFF_USER_CMD: 
         return "Off (user setting)";
    case POE_PD_PSTAT_OFF_DET: 
         return "Off (detection is in process)";
    case POE_PD_PSTAT_OFF_NON_PD: 
         return "Off (non-802.3af powered device)";
    case POE_PD_PSTAT_OFF_UNDER_OVER: 
         return "Off (overload & underload states)";
    case POE_PD_PSTAT_OFF_UNDER_CAP: 
         return "Off (underload state or capacitor value is too samll)";
    case POE_PD_PSTAT_OFF_OVER_CAP: 
         return "Off (overload state or capacitor value is too large)";
    case POE_PD_PSTAT_OFF_LACK: 
         return "Off (power budget exceeded)";
    case POE_PD_PSTAT_OFF_INT_HW: 
         return "Off (internal hardware fault)";
    case POE_PD_PSTAT_OFF_VOL_INJ: 
         return "Off (voltage injection into the port)";
    case POE_PD_PSTAT_OFF_CAP_OUT: 
         return "Off (improper capacitor detection result)";
    case POE_PD_PSTAT_OFF_CAP_DET: 
         return "Off (discharged load)";
    case POE_PD_PSTAT_ON_FORCE: 
         return "On (force on, unless systems error occurs)";
    case POE_PD_PSTAT_ON_UNDEF: 
         return "Undefined error during force on";
    case POE_PD_PSTAT_ON_VOL_HI: 
         return "Supply voltage higher than settings";
    case POE_PD_PSTAT_ON_VOL_LO : 
         return "Supply voltage lower than settings";
    case POE_PD_PSTAT_ON_PDU_FLAG: 
         return "Disable_PDU flag was raised during force on";
    case POE_PD_PSTAT_ON_OFF_CMD: 
         return "Port is forced on then disabled or disabled then forced on";
    case POE_PD_PSTAT_OFF_OVERLOAD: 
         return "Off (overload condition accouding to 802.3af)";
    case POE_PD_PSTAT_OFF_BUDGET: 
         return "Off (out of power budget while in forced power state)";
    case POE_PD_PSTAT_ON_ERR_ASIC: 
         return "Communication error with ASIC after force on command";
    default:     
         return "Unknown status ?";
    } 
}

STATIC void
poe_pd_pkt_dump(uint8 *src, int len, char *msg)
{
    int i;

    if (msg || !len)
        printk ("%s (%d):\n", msg, len );
    for (i = 0; i < len; i++) {
         printk ("0x%02x ", src[i]);
         if (!((i + 1) % 16))
             printk ("\n");
    }
    printk("\n");
}

STATIC void 
poe_pd_pkt_cksum(unsigned char *data)
{
    int i;
    uint16 sum=0;

    for (i = 0; i < 13; i++) {
         sum += data[i]; 
    }
    data[13] = (sum & 0xff00) >> 8; 
    data[14] =  sum & 0xff; 
}

STATIC int
poe_pd_pkt_txrx(int unit, int fd, unsigned char *data, 
                int seq, int debug)
{
    int rv;
    uint32 len;
    soc_timeout_t to;

    if ((rv = bcm_i2c_write(unit, fd, 0, data, 15)) < 0) {
        printk("ERROR: write byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (debug) {
        poe_pd_pkt_dump(data, 15, "Tx to PD63000");
    }

    memset(data, 0x0, 15);
    soc_timeout_init(&to, 100000, 0);
    for (;;) {
         if (soc_timeout_check(&to)) {
             break;
         }
    }

    if((rv = bcm_i2c_read(unit, fd, 0, data, &len)) < 0){
        printk("ERROR: read byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (debug) {
        poe_pd_pkt_dump(data, 15, "Rx from PD63000");
    }

    if (data[1] != seq) {
        seq++;
        printk("ERROR: read from PD630000 out of sync.\n");
        printk("       use 'xpoe read' to flush data.\n");
        poe_pd_pkt_dump(data, 15, "Rx from PD63000");
        return CMD_FAIL;
    }

    return CMD_OK;
}  

#define  PD63000_KEY_COMMAND 	 0x00
#define  PD63000_KEY_PROGRAM 	 0x01
#define  PD63000_KEY_REQUEST 	 0x02
#define  PD63000_KEY_TELEMETRY 	 0x03
#define  PD63000_KEY_REPORT 	 0x52
#define  PD63000_NULL_DATA       0x4e
#define  PD63000_SUB_GLOBAL      0x07
#define  PD63000_SUB_SW          0x21
#define  PD63000_SUB_VERZ        0x1e
#define  PD63000_SUB_STATUS      0x3d
#define  PD63000_SUB_SUPPLY      0x0b
#define  PD63000_SUB_SUPPLY1     0x15
#define  PD63000_SUB_MAIN        0x17
#define  PD63000_SUB_CHANNEL     0x05
#define  PD63000_SUB_PARAMZ      0x25
#define  PD63000_SUB_PORTSTATUS  0x0e

/*
 * Function: cmd_xpoe
 * Purpose: Communication with PowerDsine PD63000 
 *          8-bit PoE Microcontroller Unit
 */
char cmd_xpoe_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "       xpoe ver | status | power | port | measure | raw | read | debug\n"
#else
    "       xpoe ver\n\t"
    "           - Get PD63000 software version.\n\t"
    "       xpoe status\n\t"
    "           - Get System Status.\n\t"
    "       xpoe power <value>\n\t"
    "           - Get/Set power supply parameters.\n\t"
    "       xpoe power status\n\t"
    "           - Get main power source supply parameters.\n\t"
    "       xpoe port <port_num>\n\t"
    "           - Get port status (use * for all ports).\n\t"
    "       xpoe measure <port_num>\n\t"
    "           - Get port measurements (use * for all ports).\n\t"
    "       xpoe raw <byte0> ... <byte14>  \n\t"
    "           - Send raw data to PD63000.\n\t"
    "       xpoe read\n\t"
    "           - Read one packet from PD63000.\n\t"
    "       xpoe debug on/off\n\t"
    "           - Turn packet dump to/from PD63000 on/off.\n\t"
    "       xpoe verbose on/off\n\t"
    "           - Turn on/off verbose output (default is on).\n"
#endif
    ;

cmd_result_t
cmd_xpoe(int unit, args_t *a)
{
    char *subcmd, *s;
    int  port, i, fd, rv, no_cksum=0;
    uint8 data[15], echo;
    uint32 len;
    uint16 val;
    static uint8 seq=0;
    static int debug=0;
    static int verbose=1;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if ( (fd = bcm_i2c_open(unit, I2C_POE_0, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_POE_0, bcm_errmsg(fd));
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "verbose") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        if (sal_strcasecmp(subcmd, "on") == 0) {
            verbose = 1;
        } else {
            verbose = 0;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "debug") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        if (sal_strcasecmp(subcmd, "on") == 0) {
            debug = 1;
        } else {
            debug = 0;
        }
        return CMD_OK;
    }

    memset(data, 0x0, 15);
    
    if (sal_strcasecmp(subcmd, "ver") == 0) {
        memset(data, PD63000_NULL_DATA, 15);
        data[0] = PD63000_KEY_REQUEST;
        data[1] = seq;
        data[2] = PD63000_SUB_GLOBAL;
        data[3] = PD63000_SUB_VERZ; 
        data[4] = PD63000_SUB_SW;
        poe_pd_pkt_cksum(data);

        if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
            return rv;
        }

        if (data[0] == PD63000_KEY_TELEMETRY) {
            if (verbose) {
                printk("H.W. Version %d (0x%x)\n", data[2], data[2]);
                printk("S.W. Version %d (0x%x)\n", ((data[5]<<8)|data[6]), 
                                                   ((data[5]<<8)|data[6]));
            }
        }
        seq++;
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "status") == 0) {
        memset(data, PD63000_NULL_DATA, 15);
        data[0] = PD63000_KEY_REQUEST;
        data[1] = seq;
        data[2] = PD63000_SUB_GLOBAL;
        data[3] = PD63000_SUB_STATUS;
        poe_pd_pkt_cksum(data); 
        
        if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
            return rv;
        }

        if (data[0] == PD63000_KEY_TELEMETRY) {
            printk("%s%s",(data[2] & 0x1) ? "CPU error; " : ""
                         ,(data[2] & 0x2) ? "Programming required; " : "");
            printk("%s%s",(data[3] & 0x1) ? "EEPROM error; " : ""
                         ,((data[3] & 0x2) && (data[8] != 0xfe)) ? "ASIC error; " : "");
            printk("%s",(data[4] & 0x1) ? "Factory default set; " : "");
            printk("%s",(data[5] != 0) ? "Internal error; " : "");
            if ((data[2]|data[3]|data[4]|data[5]) && (data[8] != 0xfe)) {
              printk("\n");
              printk("ASIC fail = 0x%x\n",data[8]);
            } else {
              if (verbose)
              printk("No system status error.\n");
            }
            if (verbose) {
                printk("General use = %d\n",data[6]);
                printk("User byte = 0x%x\n",data[7]);
            }
        }
        seq++;
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "power") == 0) {
        if ((s = ARG_GET(a)) == NULL) {
            memset(data, PD63000_NULL_DATA, 15);
            data[0] = PD63000_KEY_REQUEST;
            data[1] = seq;
            data[2] = PD63000_SUB_GLOBAL;
            data[3] = PD63000_SUB_SUPPLY;
            data[4] = PD63000_SUB_SUPPLY1;
            poe_pd_pkt_cksum(data);
            
            if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
                return rv;
            }

            if (data[0] == PD63000_KEY_TELEMETRY) {
                if (verbose) {
                    printk("Power permitted %d W\n", (data[2]<<8)|data[3]);
                    printk("Max Voltage %4d.%01d V\n", 
                           INT_FRAC_1PT((data[4]<<8)|data[5]));
                    printk("Min Voltage %4d.%01d V\n", 
                           INT_FRAC_1PT((data[6]<<8)|data[7]));
                }
            }
            seq++;
        } else {
           if (sal_strcasecmp(s, "status") == 0) {
               memset(data, PD63000_NULL_DATA, 15);
               data[0] = PD63000_KEY_REQUEST;
               data[1] = seq;
               data[2] = PD63000_SUB_GLOBAL;
               data[3] = PD63000_SUB_SUPPLY;
               data[4] = PD63000_SUB_MAIN;
               poe_pd_pkt_cksum(data);
            
               if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
                   return rv;
               }

               if (data[0] == PD63000_KEY_TELEMETRY) {
                   if (verbose) {
                       printk("Power Consumption %d W\n", (data[2]<<8)|data[3]);
                       printk("Max Shutdown Voltage %4d.%01d V\n", 
                              INT_FRAC_1PT((data[4]<<8)|data[5]));
                       printk("Min Shutdown Voltage %4d.%01d V\n", 
                              INT_FRAC_1PT((data[6]<<8)|data[7]));
                       printk("Internal Power Loss %d W\n", data[8]);
                       printk("Power Source %s\n",(data[9] ? "PS1" : "PS2"));
                   }
               }
               seq++;
           } else {
               int i2c_poe_power;

               i2c_poe_power = soc_property_get(unit, spn_I2C_POE_POWER, 0);
               /* set power source 1 */
               memset(data, PD63000_NULL_DATA, 15);
               data[0] = PD63000_KEY_COMMAND;
               data[1] = seq;
               data[2] = PD63000_SUB_GLOBAL;
               data[3] = PD63000_SUB_SUPPLY;
               data[4] = PD63000_SUB_SUPPLY1;
               if (i2c_poe_power) {
                   data[7] = 0x2;
                   data[8] = 0x39;
                   data[9] = 0x1;
                   data[10] = 0xb7;
                   data[11] = 0x13;
               } else {
                   data[7] = 0x2;
                   data[8] = 0x49;
                   data[9] = 0x1;
                   data[10] = 0xb8;
                   data[11] = 0x1;
               }
               val = parse_integer(s);
               data[5] = val>>8;
               data[6] = val&0xff;
               poe_pd_pkt_cksum(data);
            
               if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
                   return rv;
               }

               if (data[0] == PD63000_KEY_REPORT) {
                   val = (data[2] << 8) | data[3];
                   if (val) {
                       printk("Return error 0x%x\n", val);
                       if ((val > 0) && (val <= 0x7fff)) {
                           printk("\tFailed execution (conflict in subject bytes).\n");
                       }
                       if ((val > 0x8000) && (val <= 0x8fff)) {
                           printk("\tFailed execution (wrong data byte value).\n");
                       }
                       if (val == 0xffff) {
                           printk("\tFailed execution (undefined key value).\n");
                       }
                   } else {
                       if (verbose) 
                           printk("Command received and correctly executed.\n");
                   }
               }
               seq++;
           }
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "port") == 0) {
        if ((s = ARG_GET(a)) != NULL) {
            if (sal_strcasecmp(s, "*") == 0) {
                port = 24;
            } else {
                port = 1;
            }

            for (i = 0; i < port; i++) {
                 memset(data, PD63000_NULL_DATA, 15);
                 data[0] = PD63000_KEY_REQUEST;
                 data[1] = seq;
                 data[2] = PD63000_SUB_CHANNEL;
                 data[3] = PD63000_SUB_PORTSTATUS;
                 if (port == 1)
                     data[4] = parse_integer(s);
                 else
                    data[4] = i;
                 if (data[4] > 24) {
                     printk("Channel number:  0 ~ 23.\n");
                     return CMD_FAIL;
                 }
                 poe_pd_pkt_cksum(data);
            
                 if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
                     return rv;
                 }

                 if (data[0] == PD63000_KEY_TELEMETRY) {
                     if (port != 1 && verbose)
                         printk("Port %d:\n", i);
                     if (verbose)
                         printk("Port %s\n", (data[2] ? "enable" : "disable"));
                     s = poe_pd_decode_port_status(data[3]);
                     if (verbose) {
                         printk("Actual port status %s\n",s);
                         printk("Port in %s\n",(data[4] ? "test mode" : "auto mode"));
                         printk("Latch 0x%x\n",data[5]);
                         printk("Class %d\n",data[6]);
                     }
                 }
                 seq++;
               }
        } else {
            return CMD_USAGE;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "measure") == 0) {
        if ((s = ARG_GET(a)) != NULL) {
            if (sal_strcasecmp(s, "*") == 0) {
                port = 24;
            } else { 
                port = 1;
            }

            for (i = 0; i < port; i++) {
                memset(data, PD63000_NULL_DATA, 15);
                data[0] = PD63000_KEY_REQUEST;
                data[1] = seq;
                data[2] = PD63000_SUB_CHANNEL;
                data[3] = PD63000_SUB_PARAMZ;
                if (port == 1)
                    data[4] = parse_integer(s);
                else
                   data[4] = i;
                if (data[4] > 24) {
                    printk("Channel number:  0 ~ 23.\n");
                    return CMD_FAIL;
                }
                poe_pd_pkt_cksum(data); 
             
                if ((rv = poe_pd_pkt_txrx(unit, fd, data, seq, debug)) < 0) {
                     return rv;
                }

                if (data[0] == PD63000_KEY_TELEMETRY) {
                    if (port != 1 && verbose)
                        printk("Port %d:\n", i);
                    if (verbose) {
                        printk("Voltage = %4d.%01d V\t", 
                               INT_FRAC_1PT((data[2]<<8)|data[3]));
                        printk("Current = %d mA\t\t", (data[4]<<8)|data[5]);
                        printk("Power = %4d.%03d W\n",
                               INT_FRAC_3PT((data[6]<<8)|data[7]));
                    }
                }
                seq++;
            }
         } else {
            return CMD_USAGE;
         }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "raw") == 0) {
        for (i = 0; i < 15; i++) {
             if ((s = ARG_GET(a)) == NULL) {
                  if (i == 13)  {
                      no_cksum = 1;
                      break;
                  } else {
                      printk("Not enough data values (have %d, "
			     "need 15 or 13 (for auto check-sum))\n", i);
                      return CMD_FAIL;
                  }
             }
             data[i] = parse_integer(s);
        }     

        if (no_cksum) {
            poe_pd_pkt_cksum(data);
        }

        echo = data[1];
        if ((rv = poe_pd_pkt_txrx(unit, fd, data, echo, 1)) < 0) {
            return rv;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "read") == 0) {
	if ((rv = bcm_i2c_read(unit, fd, 0, data, &len)) < 0){
	    printk("ERROR: read byte failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
        poe_pd_pkt_dump(data, 15, "Rx from PD63000");
        return CMD_OK;
    }

    return CMD_USAGE;
} /* end cmd_xpoe() */

/*
 * Pretty-print a byte in binary format.
 */
STATIC char *
get_bits(uint8 byte)
{
    static char buf[12];
    int i;
    memset(buf,0x0,12);
    strcat(buf,"0b");
    for(i = 7; i >= 0;i--){
	if( byte & (1 << i)){
	    printk("bit %d set\n",i);
	    strcat(buf,"1");
	}
	else	
	    strcat(buf,"0");
    }
    return buf;
}
    
  
/*
 * Use lpt0 (PCF8574 Parallel port) to select a MUX line
 * for multi-clock and multi-dac boards. Some boards have
 * multiple devices at the same slave address, the lpt0 outputs
 * will function in this case as a chip-select.
 */
char cmd_muxsel_usage[] =
    "Usages:\n\t"
    "       muxsel [value] [mux_device]\n\t"
    "           - show muxsel, if value specified, set to that value.\n\t"
    "           - mux_device = lpt0, mux0, mux1\n\t"
    "\n";

cmd_result_t
cmd_muxsel(int unit, args_t *a)
{
    char *dataByte = ARG_GET(a);
    int fd, rv = SOC_E_NONE;    
    uint32 len;
    uint8 lptVal;
    char * dev_name = ARG_GET(a);
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (dev_name == NULL) {
        dev_name = _i2c_mux_dev_name_get(I2C_MUX_VOLTAGE_CONTROL);
    }
    if ( (fd = bcm_i2c_open(unit, dev_name, 0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), dev_name, bcm_errmsg(fd));
	return CMD_FAIL;
    }

    if (! dataByte ){
	/* Show LPT bits */
	if( (rv = bcm_i2c_read(unit, fd, 0, &lptVal, &len)) < 0){
	    printk("ERROR: read byte failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	printk("Read I2C MuxSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    } else {
	/* Write LPT bits */
	lptVal = (uint8)parse_integer(dataByte);
	if( (rv = bcm_i2c_write(unit, fd, 0, &lptVal, 1)) < 0){
	    printk("ERROR: write byte failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	
	printk("Write I2C MuxSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    }
    return CMD_OK ;
}

/*
 * Use lpt2 (PCF8574 Parallel port) to select a MUX line
 * for multi-clock.
 */
char cmd_hclksel_usage[] =
    "Usages:\n\t"
    "       hclksel [value]\n\t"
    "           - show hclksel, if value specified, set to that value.\n\t"
    "\n";

cmd_result_t
cmd_hclksel(int unit, args_t *a)
{
    char *dataByte = ARG_GET(a);
    int fd, rv = SOC_E_NONE;
    uint32 len;
    uint8 lptVal;

    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if ( (fd = bcm_i2c_open(unit, I2C_LPT_2, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_2, bcm_errmsg(fd));
        return CMD_FAIL;
    }

    if (! dataByte ){
        /* Show LPT bits */
        if( (rv = bcm_i2c_read(unit, fd, 0, &lptVal, &len)) < 0){
            printk("ERROR: read byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk("Read I2C HClkSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    } else {
        /* Write LPT bits */
        lptVal = (uint8)parse_integer(dataByte);
        if( (rv = bcm_i2c_write(unit, fd, 0, &lptVal, 1)) < 0){
            printk("ERROR: write byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }

        printk("Write I2C HClkSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    }
    return CMD_OK ;
}

/*
 * Use lpt3 (PCF8574 Parallel port) for PoE I2C expander.
 */
char cmd_poesel_usage[] =
    "Usages:\n\t"
    "       poesel enable\n\t"
    "            - Enable POE controller.\n\t"
    "       poesel disble\n\t"
    "            - Disable POE controller.\n\t"
    "       poesel reset\n\t"
    "            - Reset POE controller.\n\t"
    "       poesel [value]  \n\t"
    "            - show poesel, if value specified, set to that value.\n\t"
    "\n";

cmd_result_t
cmd_poesel(int unit, args_t *a)
{
    char *subcmd;
    int subcmd_s = 0;
    int fd, rv = SOC_E_NONE;
    uint32 len;
    uint8 lptVal;
#define I2C_POE_ENABLE    1
#define I2C_POE_DISABLE   2
#define I2C_POE_RESET     3

    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if ( (fd = bcm_i2c_open(unit, I2C_LPT_3, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_3, bcm_errmsg(fd));
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        if( (rv = bcm_i2c_read(unit, fd, 0, &lptVal, &len)) < 0){
            printk("ERROR: read byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk("Read I2C PoeSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
        return CMD_OK ;
    }

    if (sal_strcasecmp(subcmd, "enable") == 0) {
        subcmd_s = I2C_POE_ENABLE;
    } else if (sal_strcasecmp(subcmd, "disable") == 0) {
        subcmd_s = I2C_POE_DISABLE;
    } else if (sal_strcasecmp(subcmd, "reset") == 0) {
        subcmd_s = I2C_POE_RESET;
    } else {
        lptVal = (uint8)parse_integer(subcmd);
    }

    /*    coverity[new_values]    */
    if (subcmd_s > 0) {
        switch (subcmd_s) {
        case I2C_POE_ENABLE:
            lptVal = 0x8c;
            break;
        case I2C_POE_DISABLE:
            lptVal = 0x8e;
            break;
        case I2C_POE_RESET:
            lptVal = 0x8d;
            break;
        default:
            return CMD_USAGE;
        }
    }

    if ((rv = bcm_i2c_write(unit, fd, 0, &lptVal, 1)) < 0){
        printk("ERROR: write byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (subcmd_s == I2C_POE_RESET) {
        sal_sleep(5);
    }

    if (subcmd_s == 0) {
        printk("Write I2C PoeSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    }

    return CMD_OK ;
}


#ifdef BCM_ESW_SUPPORT
#define SYN_DELAY 200000
/*
 * Use lpt4 and lpt5 (PCF8574 Parallel port) to select a 
 * synthesizer frequency.
 */
char cmd_synth_usage[] =
    "\n\t"
    "synth [value]\n\t"
    "   - show synth freq, if value specified, set frequency.\n\t"
    "\n";

cmd_result_t
cmd_synth(int unit, args_t *a)
{
#ifdef COMPILER_HAS_DOUBLE
    char *value = ARG_GET(a);
    char *syndbg = ARG_GET(a);
    double cur_freq, freq, m_div, n_div;
    double n_div_map[8] = { 1, 1.5, 2, 3, 4, 6, 8, 12 };
    double freq_min[8] = { 250.0, 166.7, 125.0,  83.3,  62.5, 41.7, 31.3, 20.8 };
    double freq_max[8] = { 500.0, 333.3, 250.0, 166.7, 125.0, 83.3, 62.5, 41.7 };
    int adj[8] = { 9, 6, 3, 3, 2, 1, 1, 1 };
    int rv, sel1, sel2, m, n, cur_n, inc;
    uint8 lptVal;
    uint32 reg, len;
    char *str;

    if (!sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if ((sel1 = bcm_i2c_open(unit, I2C_LPT_4, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_4, bcm_errmsg(sel1));
        return CMD_FAIL;
    }

    if ((sel2 = bcm_i2c_open(unit, I2C_LPT_5, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_5, bcm_errmsg(sel2));
        return CMD_FAIL;
    }

    /* Read current frequency */
    if ((rv = bcm_i2c_read(unit, sel1, 0, &lptVal, &len)) < 0) {
        printk("ERROR: read byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }
    m = lptVal;
    if ((rv = bcm_i2c_read(unit, sel2, 0, &lptVal, &len)) < 0) {
        printk("ERROR: read byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }
    if (lptVal & 0x80) m |= 0x100;
    n = lptVal & 0x7;
    m_div = m;
    n_div = n_div_map[n];
    cur_freq = m_div / n_div;

    if (!value) {
        /* Show frequency */
        freq = cur_freq;
        str = "";
    } else {
        /* Set frequency */
        freq = atof(value);
        if (freq > freq_max[0] || freq < freq_min[7]) {
            printk("%s: valid frequency range: %.1f MHz to %.1f MHz\n",
                   ARG_CMD(a), freq_min[7], freq_max[0]);
            return CMD_FAIL;
        }
        if (syndbg) printk("Change from %.1f MHz to %.1f MHz\n", cur_freq, freq);

        /*
         * Since the I2C clock is derived from the core clock and the
         * I2C access only works within a certain frequency range, we
         * need to take precautions when changing the core clock in
         * order not to lock ourselves out from the I2C bus. This
         * means that for large changes in core clock frequency we
         * will have to change the core clock in smaller steps and
         * adjust the CMIC divider in between the steps.
         */
        for (cur_n = 0; cur_n < 8; cur_n++) {
            if (cur_freq >= freq_min[cur_n]) {
                break;
            }
        }
        for (n = 0; n < 8; n++) {
            if (freq >= freq_min[n]) {
                break;
            }
        }

        if (cur_n >= 8 || n >= 8) {
            /* should never get here */
            printk("ERROR: internal error\n");
            return CMD_FAIL;
        }

        inc = cur_n > n ? -1 : 1;
        do {
            if (n == cur_n) {
                cur_freq = freq;
                inc = 0;
            } else {
              if((cur_n+inc) < 0 || (cur_n+inc) > 7) {
                  printk("ERROR: Internal error\n");
                  return CMD_FAIL;
    /*    coverity[overrun-local : FALSE]    */
              } else {
    /* coverity[overrun-local] */
                  cur_freq = (inc < 0) ? freq_min[cur_n+inc] : freq_max[cur_n+inc];
              }
            }
            if (syndbg) printk("Adjust to %.1f/%d\n", cur_freq, adj[cur_n+inc]);
            n_div = n_div_map[cur_n+inc];
            m = cur_freq * n_div;
            lptVal = (uint8)(m & 0xff);
            if ((rv = bcm_i2c_write(unit, sel1, 0, &lptVal, 1)) < 0) {
                printk("ERROR: write byte failed: %s\n",
                       bcm_errmsg(rv));
                return CMD_FAIL;
            }
            sal_usleep(SYN_DELAY);
            lptVal = (uint8)cur_n;
            if (m & 0x100) lptVal |= 0x80;
            if ((rv = bcm_i2c_write(unit, sel2, 0, &lptVal, 1)) < 0) {
                printk("ERROR: write byte failed: %s\n",
                       bcm_errmsg(rv));
                return CMD_FAIL;
            }
            sal_usleep(SYN_DELAY);
            READ_CMIC_RATE_ADJUSTr(unit, &reg);
            soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &reg, 
                              DIVISORf, adj[cur_n]);
            WRITE_CMIC_RATE_ADJUSTr(unit, reg);
            sal_usleep(SYN_DELAY);
            if (n == cur_n) {
                break;
            }
            cur_n = cur_n + inc;
            if (cur_n < 0 || cur_n > 7) {
                printk("ERROR: Internal error\n");
                return CMD_FAIL;
            }
        } while (1);
        /* Dummy read */
        if ((rv = bcm_i2c_read(unit, sel1, 0, &lptVal, &len)) < 0) {
            printk("ERROR: dummy read byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        str = "Write I2C ";
    }
    printk("%sSynth Freq = %.1f MHz", str, freq);
    printk("  [ 0%c%c%c%c%c%c%c%c%c 0%c%c%c ]\n",
           m & 0x100 ? '1' : '0', 
           m & 0x080 ? '1' : '0', 
           m & 0x040 ? '1' : '0', 
           m & 0x020 ? '1' : '0', 
           m & 0x010 ? '1' : '0', 
           m & 0x008 ? '1' : '0', 
           m & 0x004 ? '1' : '0', 
           m & 0x002 ? '1' : '0', 
           m & 0x001 ? '1' : '0', 
           n & 0x4 ? '1' : '0', 
           n & 0x2 ? '1' : '0', 
           n & 0x1 ? '1' : '0');
#endif

    return CMD_OK ;
}

/*
 * Use lpt6 (PCF8574 Parallel port) to set PPD clock delay.
 */
char cmd_ppdclk_usage[] =
    "\n\t"
    "ppdclk\n\t"
    "   - show PPD clock delay and core clock divider\n\t"
    "ppdclk delay value\n\t"
    "   - set PPD clock delay in ps (0-1270)\n\t"
    "ppdclk div value\n\t"
    "   - set core clock divider enable (0 or 1)\n\t"
    "\n";

cmd_result_t
cmd_ppdclk(int unit, args_t *a)
{
    char *source = ARG_GET(a);
    char *value = ARG_GET(a);
    int fd, fd2, rv = SOC_E_NONE;
    uint32 len, ppd_delay;
    uint8 lptVal, lptVal2;

    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if ((fd = bcm_i2c_open(unit, I2C_LPT_6, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_6, bcm_errmsg(fd));
        return CMD_FAIL;
    }

    if( (rv = bcm_i2c_read(unit, fd, 0, &lptVal, &len)) < 0){
        printk("ERROR: read byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if ((fd2 = bcm_i2c_open(unit, I2C_LPT_7, 0,0)) < 0) {
        printk("%s: cannot open I2C device %s: %s\n",
               ARG_CMD(a), I2C_LPT_6, bcm_errmsg(fd));
        return CMD_FAIL;
    }

    if( (rv = bcm_i2c_read(unit, fd2, 0, &lptVal2, &len)) < 0){
        printk("ERROR: read byte failed: %s\n",
               bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (!source){
        /* Show PPD clock delay and core clock divider */
        ppd_delay = lptVal2 & 0x03;
        ppd_delay <<= 8;
        ppd_delay |= lptVal;
        printk("PPD Clock Delay    = %d0 ps\n", ppd_delay);
        printk("Core Clock Divider = %sabled\n", 
               lptVal2 & 0x80 ? "en" : "dis");
    } else if (!strcmp(source, "delay") && value) {
        /* Write PPD clock delay */
        ppd_delay = parse_integer(value) / 10;
        lptVal = (uint8)(ppd_delay & 0xff);
        lptVal2 &= ~0x03;
        lptVal2 |= (uint8)(ppd_delay >> 8);
        if( (rv = bcm_i2c_write(unit, fd, 0, &lptVal, 1)) < 0){
            printk("ERROR: write byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        if( (rv = bcm_i2c_write(unit, fd2, 0, &lptVal2, 1)) < 0){
            printk("ERROR: write byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk("Write I2C PPD Clock Delay = %d0 ps\n", ppd_delay);
    } else if (!strcmp(source, "div") && value) {
        /* Enable/disable core clock divider */
        lptVal &= ~0x80;
        if (parse_integer(value)) lptVal |= 0x80;
        if( (rv = bcm_i2c_write(unit, fd, 0, &lptVal, 1)) < 0){
            printk("ERROR: write byte failed: %s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk("Write I2C Core Clock Divider = %sabled\n", 
               lptVal & 0x80 ? "en" : "dis");
    } else {
        return CMD_USAGE ;
    }
    return CMD_OK ;
}
#endif /* BCM_ESW_SUPPORT */

/*
 * Configure a DAC chip.
 */
char cmd_dac_usage[] =
    "Usages:\n\t"
    "       dac [chipnum] [value]\n\t"
    "           - set DAC chip number to specified value.\n\t"
    "\n";

cmd_result_t
cmd_dac(int unit, args_t *a)
{


    int fd, num, rv = SOC_E_NONE;    
    uint16 dacVal;
    char dev[25];
    char *chip = ARG_GET(a);
    char *dataWord = ARG_GET(a);
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if ( (! dataWord) || (!chip) )
	return CMD_USAGE ;

    /* Select ADC chip number */
    num = parse_integer(chip);
    sprintf(dev,"%s%d","dac",num);

    dacVal = (uint16)parse_integer(dataWord);
    
    if ( (fd = bcm_i2c_open(unit, dev, 0, 0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), dev, bcm_errmsg(fd));
	return CMD_FAIL;
    }
    /* Write command and data bytes to DAC */
    if( (rv = bcm_i2c_write(unit, fd, 0, (uint8*)&dacVal, 2)) < 0){
	printk("ERROR:write of DAC word failed:%s\n",
	       bcm_errmsg(rv));
	return CMD_FAIL;
    }		
    
    return CMD_OK ;
}


/*
 * Use Matrix Orbital LCD to display messages.
 */
char cmd_lcdmsg_usage[] =
    "Usages:\n\t"
    "       lcdmsg [string]\n\t"
    "           - print string on lcd\n\t"
    "       lcdmsg -x [0-255]\n\t"
    "           - set contrast (0-255)\n\t"
    "       lcdmsg -c\n\t"
    "           - clear screen\n\t"
    "       lcdmsg -n\n\t"
    "           - scroll down one line\n\t"
"\n";

cmd_result_t
cmd_lcdmsg(int unit, args_t *a)
{
    char *msg = ARG_GET(a);
    char *dat = ARG_GET(a);
    uint8 db;
    int fd, rv = SOC_E_NONE;    

    if( ! msg)
	return CMD_USAGE;
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if ( (fd = bcm_i2c_open(unit, I2C_LCD_0, 0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), I2C_LCD_0, bcm_errmsg(fd));
	return CMD_FAIL;
    }    
    /* Echo a newline */
    if(!sal_strncasecmp(msg, "-n", 2)){
	if( (rv = bcm_i2c_write(unit, fd, 0, (uint8 *)"\n",1)) < 0){
	    printk("ERROR: I2C write failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
    }
    /* Set contrast level */
    else if(!sal_strncasecmp(msg,"-x",2)){
	if(!dat)
	    return CMD_USAGE;

	db = (uint8) parse_integer(dat);

	if( (rv = bcm_i2c_ioctl(unit, fd, I2C_LCD_CONTRAST,
				(void*)&db,1)) < 0){
	    printk("ERROR: I2C ioctl failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
    }
    /* Clear screen */
    else if(!sal_strncasecmp(msg, "-c",2)){
	if( (rv = bcm_i2c_ioctl(unit, fd, I2C_LCD_CLS, 0,0)) < 0){
	    printk("ERROR: I2C ioctl failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
    }
    else {
	/* Display text */
	if( (rv = bcm_i2c_write(unit, fd, 0, (uint8 *)msg, strlen(msg))) < 0){
	    printk("ERROR: I2C ioctl failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
    }
    return CMD_OK ;
}


/*
 * XGS/StrataSwitch board level support.
 * On Broadcom SDK Systems, the internal I2C controller is used to
 * determine board configuration and configurable options.
 *
 * Some boards have configurable options, other's don't and for
 * those which do, we provide additional support for the SDK
 * with the following tools:
 *
 * G1 - I2C NVRAM for SAL configuration, I2C Temperature monitoring,
 *      W229n clock speed configuration (Core).
 * G2 - All of G1, plus configurable Core Voltage, and PCI, SDRAM amd
 *      core clocks.
 * G3 - All of G1, pluse Turbo clock, Power, Temperature, and Thickness.
 *      In addition, PHY and 2.5V power supplies configurable.
 *
 * See also: "baseboard" or "bb" command.
 */
/*
 * NOTE: To add new boards, edit board_type definition above
 * and be certain that this is used for indexing to work
 * (these tables commented as "INDEXED_BY_BOARD_TYPE").
 */

/*
 * Board identifiers.
 */
typedef enum{
    G5,     /* BCM5670 */
    G6,     /* BCM5690 */
    G7,     /* BCM5690_P48_00 */
    G8,     /* BCM5673 SDK */
    G9,     /* BCM5645 PoE */
    G10,    /* BCM5673_P8 */
    G11,    /* BCM5665_P48, BCM5650_P24 */
    G12,    /* BCM5695P24_00 */
    G13,    /* BCM56504P24_00 */
    G14,    /* BCM56600P12_00 */
    G15,    /* BCM56580K16 */
    G16,    /* BCM56800K20C */
    G17,    /* BCM56600K20 */
    G18,    /* BCM56218k48 */
    G19,    /* BCM56224k24 */
    G20,    /* BCM56624K49S, BCM56680K24TS, BCM56628K8XG, BCM56624K49DV */
    G21,    /* BCM56820K24XG, BCM56820K24C, BCM56820K24DV */ 
    G22,    /* BCM56524/BCM56685 */
    G23,    /* BCM88230 */
    G24,    /* BCM56840 */
    G25,    /* BCM56740 */
    G26,    /* BCM56142 */
    ENG,    /* Special */
    UNK     /* Unknown */
} board_type_t;

/* Board information structure */

typedef struct
bb_type_s{
    int  type;
    int  bbcmd_flags; /* Supported baseboard commands */
    char *name_text;
} bb_type_t;

static char *BaseboardName = NULL;

/* Defines for bbcmd_flags field */
#define BB_NONE 0x00000000
#define BB_CLK  0x00000001
#define BB_VOLT 0x00000002
#define BB_PWR  0x00000004

/* Currently supported platforms 
 * INDEXED_BY_BOARD_TYPE */

bb_type_t
baseboards[] = {
    { G5,  BB_CLK | BB_VOLT | BB_PWR,
     "XGS Hercules BCM5670_P8_00 Platform" },
    { G6,  BB_CLK | BB_VOLT | BB_PWR,
     "XGS Draco BCM5690_P24_00 Platform" },            
    { G7,  BB_CLK | BB_VOLT         ,
     "XGS Draco BCM5690_P48_00 Platform" },            
    { G8,  BB_CLK | BB_VOLT | BB_PWR,
     "XGS Lynx BCM5673_P4_00 Platform" },            
    { G9,  BB_CLK                   ,
     "BCM5645 PoE Platform" },            
    { G10, BB_CLK                   ,
     "XGS Lynx BCM5673_P8_00 Platform" },            
    { G11, BB_CLK | BB_VOLT | BB_PWR,
     NULL }, /* Tucana, Magnum SDKs */            
    { G12, BB_CLK | BB_VOLT | BB_PWR,
     "XGS Draco 1.5 BCM5695P24_00 Platform" },            
    { G13, BB_CLK | BB_VOLT | BB_PWR,
     "XGS Firebolt BCM56504P24_00 Platform" },            
    { G14, BB_VOLT | BB_PWR,
     "XGS Easyrider BCM56600P12_00 Platform" },            
    { G15, BB_VOLT | BB_PWR,
     "XGS Goldwing BCM56580K16 Platform" },            
    { G16, BB_VOLT | BB_PWR,
     "XGS Bradley BCM56800K20C Platform" },            
    { G17, BB_VOLT | BB_PWR,
     "XGS Bradley BCM56800K20 Platform" },            
    { G18, BB_VOLT | BB_PWR,
     "XGS raptor BCM56218k48 Platform" },            
    { G19, BB_VOLT,
     "XGS Raven BCM56224k24 Platform" },  
    { G20, BB_CLK | BB_VOLT   ,
     "XGS Triumph Platform"},
    { G21, BB_CLK | BB_VOLT   ,
     "XGS Scorpion Platform"},
    { G22, BB_CLK | BB_VOLT   ,
     "XGS Apollo Platform"},
    { G23, BB_CLK | BB_VOLT   ,
     "SBX Sirius Platform"},
    { G24, BB_CLK | BB_VOLT   ,
     "XGS Trident Platform"},
    { G25, BB_CLK | BB_VOLT   ,
     "XGS Titan Platform"},
    { G26, BB_VOLT   ,
     "XGS Hurricane Platform"},
    { ENG, BB_NONE                  ,
     "ENG Version" },
    { UNK, BB_NONE                  ,
     "No Info or I2C unavailable on this platform" }
};

/* Number of possible board types */
#define NUM_BOARD_TYPES COUNTOF(baseboards)

/* Max number of clocks per board */
#define MAX_CLOCKS_PER_BOARD 10

/* Max number of voltages per board */
#define MAX_VOLTAGES_PER_BOARD 32

/* Set once at bootup */
static int soc_board_idx = -1;

/*
 * Get system baseboard info, either by reading it
 * from EEPROM or by probing the PCI and I2C buses.
 * This routine attempts to determine "board type",
 * which is used subsequently as an index by tables
 * commented with INDEXED_BY_BOARD_TYPE.
 */
bb_type_t*
soc_i2c_detect_board(void)
{
    int dev, bd_idx, id ;
    int mux_detected =0; 
    uint32 id_detected =0;
    uint8 board_id =0;
    int n_adc=0, n_dac=0, n_pll=0, n_temp=0, n_eeprom=0, n_poe=0;
    int found = 0, board_type = -1;
    i2c_device_t* i2c_dev = NULL;

    dev = 0;
    if (soc_ndev > 0) {
        if (!soc_i2c_is_attached(dev)) {
            if (soc_i2c_attach(dev, 0, 0) < 0) {
                soc_cm_print("unit %d soc_i2c_detect_board: "
                    "error attaching to I2C bus\n",
                    dev);
                return NULL;
            }
        }

        if (soc_i2c_probe(dev) < 0) {
            soc_cm_print("unit %d soc_i2c_detect_board: "
                "error probing the I2C bus\n",
                dev);
            return NULL;
        }

        for( id = 0; id < soc_i2c_device_count(dev); id++) {
            i2c_dev = soc_i2c_device(dev, id) ;
            if ( i2c_dev ) {

                /* Bus Multiplexor (MUX) */
                if ( ( i2c_dev->driver->id == PCF8574_DEVICE_TYPE ) &&
                    ( i2c_dev->saddr == I2C_LPT_SADDR0 ) ) {
                        mux_detected = 1;
                }
                /* Board ID register */
                if ( ( i2c_dev->driver->id == PCF8574_DEVICE_TYPE ) &&
                    ( i2c_dev->saddr == I2C_LPT_SADDR1 ) ) {
                        /* id_detected will be set to 1 by the following call (length) */
                        /* Read the board ID register */
                        i2c_dev->driver->read(dev, id, 0, &board_id, &id_detected);
                }
                /* ADC */
                if ( i2c_dev->driver->id == MAX127_DEVICE_TYPE){
                    n_adc++;
                }
                /* DAC */
                if ( i2c_dev->driver->id == LTC1427_DEVICE_TYPE){
                    n_dac++;
                }

                /* W229B PLL */
                if ( i2c_dev->driver->id == W229B_DEVICE_TYPE ) {
                    n_pll++;
                }

                /* EEPROM */
                if ( i2c_dev->driver->id == LC24C64_DEVICE_TYPE){
                    n_eeprom++;
                    
                }

                /* LM75 Temp */
                if ( i2c_dev->driver->id == LM75_DEVICE_TYPE ){
                    n_temp++;
                }		

                /* POE controller */
                if ( i2c_dev->driver->id == LTC4258_DEVICE_TYPE ){
                    n_poe++;
                }
            }
        }
    }

    

    /* Here if we did not find any board-specific info in the EEPROM.   */
    /* We know what the soc_ndev devices are, and what exists on the    */
    /* I2C bus. Use this information to identify the hardware platform. */

    /**********************************************************/
    /* Specific, uniquely populated, or valid board_id boards */
    /**********************************************************/

    if ( soc_ndev >= 5) {
        if (SOC_IS_HERCULES1(0) && SOC_IS_DRACO(1)) {
            /* There are several versions of the "Lancelot" board.
            * Use the I2C information to discern between the versions.
            */
            if (n_temp > 0)
                board_type = G7;
            else 
                board_type = ENG;
            goto found;
        }
        else if (SOC_IS_HERCULES1(0) && SOC_IS_LYNX(1)) {
            /* Herculynx board. */
            board_type = G10;
            goto found;
        }
    }

    else if (SOC_IS_LYNX(0)) {
        /* For now, any 5673 board is a G8 */
        board_type = G8;
        goto found;
    }

    else if (id_detected && (board_id == 0x15)) {
        /* Draco 1.5 ref design ("could" have regular Dracos)  */
        /* Draco 1.5 hooks, different power measurement params */
        board_type = G12;
    }

    else if (id_detected && (board_id == 0x11)) {
        /* Tucana    */
        BaseboardName = "Tucana BCM5665_P48 Platform";
        board_type = G11;
    }

    else if (id_detected && (board_id == 0x18)) {
        /* Magnum    */
        BaseboardName = "Magnum BCM5650_P24 Platform";
        board_type = G11;
    }

    else if (id_detected && (board_id == 0x26)) {
        /* Firebolt    */
        board_type = G13;
    }

    else if (SOC_IS_RAPTOR(0)) {
        /* Raptor */
        board_type = G18;
    }

    else if (SOC_IS_RAVEN(0)) {
        /* Raven */
        board_type = G19;
    }

    else if (SOC_IS_FB_FX_HX(0)) { /* Fallback */
        /* Firebolt/Helix/Felix */
        board_type = G13;
    }

    else if (id_detected && (board_id == 0x2d)) {
        /* Easyrider    */
        board_type = G14;
    }

    else if (id_detected && (board_id == 0x34)) {
        /* Goldwing     */
        board_type = G15;
    }

    else if (id_detected && (board_id == 0x39)) {
        /* Bradley 1G   */
        board_type = G16;
    }

    else if (id_detected && (board_id == 0x3a)) {
        /* Bradley      */
        board_type = G17;
    }

    else if (id_detected && (board_id == 0x53)){
        /* Triumph     */
        board_type = G20;
    }

    else if (id_detected && (board_id == 0x52)){
        /* Scorpion    */
        board_type = G21;
    }

    else if (id_detected && (board_id == 0x55 || board_id == 0x49)){
        /* Apollo   */
        board_type = G22;
    }

    else if (id_detected && (board_id == 0x59)){
        /* Sirius */
        board_type = G23;
    }

    else if (id_detected && (board_id == 0x5D)){
        /* Trident */
        board_type = G24;
        /* make sure all I2C devices behind the I2C mux device are initialized */
        if (dac_devs_init(0,G24,I2C_MUX_3) < 0) {
            soc_cm_debug(DK_ERR, "ERROR: dac_devs_init fail\n");
            return NULL; 
        }
    }

    else if (id_detected && (board_id == 0x60 || board_id == 0x61)){
        /* Titan */
        board_type = G25;
    }

    else if (id_detected && (board_id == 0x5E || board_id == 0x5F)){
        /* Hurricane */
        board_type = G26;
    }

    /**************************/
    /* Other multi-SOC boards */
    /**************************/

    else if (soc_ndev > 1) {
        if (SOC_IS_DRACO(0) || SOC_IS_DRACO(1)) {
            /* Draco - Rev 1 Board ID == any */
            board_type = G6;
            goto found;
        }

        /***************************************/
        /* Other unidentified multi-soc boards */
        /***************************************/
        else {
            board_type = ENG;
        }
    } /* soc_ndev > 1 */

    /*****************************/
    /* Other (single) SOC boards */
    /*****************************/

    else {
        /* Original StrataSwitch (G1) boards */
        if (n_poe >= 5)  {
            /* For now, use this criteria to ID the 5645 PoE board  */
            /* For this board, n_poe should equal 6. If n_poe == 5, */ 
            /* the LTC4258 addressing resistors need rework.        */
            if (n_poe == 5)
                printk("Warning: POE I2C controllers may be addressed incorrectly\n");
            board_type = G9;
        } else if (SOC_IS_XGS12_FABRIC(0)) {
            /* Hercules */
            board_type = G5;
        } else if (SOC_IS_DRACO(0)) {
            /* Draco    */
            board_type = G6;
        }
        /****************************************/
        /* Other unidentified single-soc boards */
        /****************************************/
        else {
            board_type = ENG;
        }
    } 



found:
    for( bd_idx = 0; bd_idx < NUM_BOARD_TYPES; bd_idx++ ) {
        if (baseboards[bd_idx].type == board_type){
            found=1;
            break;
        }
    }
    /* Probably know Board */
    if ( found ) {
        if (BaseboardName == NULL)
            BaseboardName = baseboards[bd_idx].name_text;
        /* soc_cm_print("Detected: %s\n", BaseboardName); */
        return &baseboards[bd_idx];
    }
    BaseboardName = NULL;
    return NULL;
} /* end soc_i2c_detect_board() */



/*
 * Use I2C to detect the correct type of board */
void
soc_i2c_board_probe(void)
{
    bb_type_t* board = soc_i2c_detect_board();
    if ( board ) {
	soc_board_idx = board->type;
    } else {
	soc_cm_print("NOTICE: board type unknown.\n");
	soc_board_idx = UNK;
    }
}

/*
 * Return number of possible boards.
 */
int
soc_num_boards(void)
{
    return NUM_BOARD_TYPES;
}

/*
 * Possibly probe, but always return correct
 * known index.
 */
int
soc_get_board_id(void)
{
    if (soc_board_idx < 0) {
      soc_i2c_board_probe();
    }
    return soc_board_idx;
}

/*
 * Return raw board info struct.
 */
bb_type_t*
soc_get_board(void)
{
    int inx;
    inx = soc_get_board_id();

    if ((inx < 0) || (inx >= NUM_BOARD_TYPES)) {
        return NULL; 
    }
    return &baseboards[inx];
}

STATIC char *
_i2c_mux_dev_name_get (int mux_type)
{
    char *dev_name = NULL;

    if (mux_type == I2C_MUX_VOLTAGE_CONTROL) {
        if (soc_get_board_id() == G26) {
            dev_name = I2C_MUX_0;
        } else if ((soc_get_board_id() == G25) || (soc_get_board_id() == G24)) {
            dev_name = I2C_MUX_3;
        } else {
            dev_name = I2C_LPT_0;
        }
    }
    return dev_name;
}

/*
 * Function: cmd_i2c
 * Purpose: I2C probe/attach/show, configuration commands.
 *          Also sets up board index and configures I2C drivers
 *          based on the inferred system board type.
 */
char cmd_i2c_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "i2c probe | log | backlog | clearlog | reset | speeds | show\n"
#else
    "i2c probe [pio|intr] [speed] [quiet]\n\t"
    "    - probe devices on I2C bus and build device tree.\n\t"
    "      If \"intr\" or \"pio\" is specified, change to that bus mode.\n\t"
    "      If a valid speed is specified, change the bus to that speed.\n\t"
    "      If \"quiet\" is specified, suppresses probe output.\n\t"
    "i2c scan [pio|intr] [saddr] [quiet]\n\t"
    "    - Scan devices on I2C bus and display the device list.\n\t"
    "i2c log \n\t"
    "    - show I2C bus transaction log.\n\t"
    "i2c backlog \n\t"
    "    - show I2C bus transaction log (in reverse order).\n\t"
    "i2c clearlog \n\t"
    "    - reset I2C bus transaction log.\n\t"
    "i2c reset\n\t"
    "    - reset I2C bus controller core.\n\t"
    "i2c speeds\n\t"
    "    - show supported I2C bus controller clock rates.\n\t"
    "i2c show\n\t"
    "    - show devices found and their attributes.\n"
#endif
    ;

cmd_result_t
cmd_i2c(int unit, args_t *a)
{
    char *function, *op;
    uint32 flags;
    int	rv, quiet, speed;
    
    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;
    
    function = ARG_GET(a);
    if (! function ) {
	return CMD_USAGE;
    } else if (! sal_strncasecmp(function, "show", strlen(function)) ){
	soc_i2c_show(unit);
    } else if ( ! sal_strncasecmp(function,"reset", strlen(function))) {
	soc_i2c_reset(unit);
    } else if ( !sal_strncasecmp(function, "speeds", strlen(function))) {
	soc_i2c_show_speeds(unit);
    } else if ( !sal_strncasecmp(function, "log", 3)) {
	soc_i2c_show_log(unit, FALSE);
    } else if ( !sal_strncasecmp(function, "clearlog", strlen(function))) {
	soc_i2c_clear_log(unit);
    } else if ( !sal_strncasecmp(function, "backlog", 7)) {
	soc_i2c_show_log(unit, TRUE);
    } else if ( ( !sal_strncasecmp(function,"probe", strlen(function))) ||
                ( !sal_strncasecmp(function,"scan", strlen(function)))) {
	quiet = 0;
	flags = 0;
	speed = -1; /* Use current speed, or default initially. */

	while ((op = ARG_GET(a)) != NULL) {
	    if (sal_strncasecmp(op, "quiet", strlen(op)) == 0) {
		quiet = 1;
		continue;
	    }
	    if (sal_strncasecmp(op, "pio", strlen(op)) == 0) {
		flags = SOC_I2C_MODE_PIO;
		continue;
	    }
	    if (sal_strncasecmp(op, "intr", strlen(op)) == 0) {
		flags = SOC_I2C_MODE_INTR;
		continue;
	    }
	    if (isdigit((unsigned)op[0])) {
		speed = parse_integer(op);
		continue;
	    }
	    soc_cm_debug(DK_ERR, "%s: ERROR: unknown probe argument: %s\n",
			 ARG_CMD(a), op);
	    return CMD_FAIL;
	}

        if ( !sal_strncasecmp(function,"probe", strlen(function))) {
            /*
             * (Re-)attach to the I2C bus if the INTR/PIO mode or
             * bus speed has been specified (presumably new setting).
             */
            if ( ((flags != 0) || (speed > 0)) &&
                 ((rv=soc_i2c_attach(unit, flags, speed)) < 0) ) {
                soc_cm_debug(DK_ERR, "%s: ERROR: attach failed: %s\n",
                             ARG_CMD(a), bcm_errmsg(rv));
            }

            rv = soc_i2c_probe(unit); /* Will attach if not yet attached */
            if (rv < 0) {
                soc_cm_debug(DK_ERR, "%s: ERROR: probe failed: %s\n",
                             ARG_CMD(a), bcm_errmsg(rv));
            } else if (!quiet) {
                printk("%s: detected %d device%s\n",
                       ARG_CMD(a), rv, rv==1 ? "" : "s");
            }
        } else {
            i2c_saddr_t saddr = 0x00;
            int saddr_start = 0x00;
            int saddr_end = 0x7F;

            /*
             * (Re-)attach to the I2C bus if the INTR/PIO mode or
             * bus speed has been specified (presumably new setting).
             */
            if ( (flags != 0) &&
                 ((rv=soc_i2c_attach(unit, flags, -1)) < 0) ) {
                soc_cm_debug(DK_ERR, "%s: ERROR: attach failed: %s\n",
                             ARG_CMD(a), bcm_errmsg(rv));
            }

            if  (speed > 0) {
                saddr_start = saddr_end = speed & 0x7F;
            }
            for(;saddr_start <= saddr_end; saddr_start++) {
                saddr = saddr_start & 0x7F;
                if (soc_i2c_device_present(unit, saddr) >= 0) {
                    printk("I2C device found at slave address 0X%02X (%s)\n",
                            saddr_start, soc_i2c_saddr_to_string(unit, saddr));
                }
            }
        }
            /* Try to Identify board */
    }
    else {
	return CMD_USAGE;
    }
    return CMD_OK;
}

/*
 * Function: cmd_clk
 * Purpose: Set W229b/W311 clock speed for core clocks
 */
char cmd_clk_usage[] =
    "Usages:\n\t"
    "       clocks w311mode \n\t"
    "           - enable Cypress W311 clock chip driver mode.\n\t"
    "       clocks [speed] \n\t"
    "           - set BCM56xx core clock speed.\n\t"
    "\n";

cmd_result_t
cmd_clk(int unit, args_t *a)
{
    char *speed = ARG_GET(a);
    uint32 clockval = 0;
#ifdef COMPILER_HAS_DOUBLE
    double clockFVal = 0;
#else
    int clockFVal = 0;
#endif
    static int isInW311Mode = 0;
    int fd;
    int	rv;

    if (! sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    /* Open PLL */
    if ((fd = bcm_i2c_open(unit, I2C_PLL_0, 0,0)) < 0) {
	printk("%s: cannot open I2C device %s: %s\n",
	       ARG_CMD(a), I2C_PLL_0, bcm_errmsg(fd));
	return CMD_FAIL;
    }

    /* When detecting P48, set clock mode automagically
     * to W311
     */
    printk("Clock configuration: %s mode\n",
	   isInW311Mode ? "W311" : "W229b");
    
    if (speed && !sal_strncasecmp(speed, "w311mode", strlen(speed))) {
	isInW311Mode = 1;
	printk("Set W311 mode enable\n");
	return CMD_OK;
    }
    if (isInW311Mode) {
	if(!speed){
	    printk("ERROR: no speed specified!\n");
	    return CMD_FAIL;
	}
#ifdef COMPILER_HAS_DOUBLE
	clockFVal = atof( speed ) ;
#else
        clockFVal = _shr_atof_exp10(speed, 6);
#endif
	
	/* Enable W311 mode */
	rv = bcm_i2c_ioctl(unit, fd, I2C_PLL_SETW311,
			   &isInW311Mode, sizeof(isInW311Mode));
	if (rv < 0) {
	    printk("ERROR: clock mode configuration failed: %s\n",
		   bcm_errmsg(rv));
	}
	
	/* Set speed */
	rv = bcm_i2c_ioctl(unit, fd, 0, &clockFVal, 0);
	if (rv < 0) {
	    printk("ERROR: clock speed configuration failed: %s\n",
		   bcm_errmsg(rv));
	    return CMD_FAIL;
	}
#ifdef COMPILER_HAS_DOUBLE
	printk("Set %s clock to %2.2fMhz\n",
	       I2C_PLL_0, clockFVal);
#else
        printk("Set %s clock to %d.%02dMhz\n",
               I2C_PLL_0,
               INT_FRAC_2PT_4(clockFVal));
#endif
    } else {
	/* Use W229b access mode */
	/* ioctl: will print out all values on bad speed value */
	if (! speed ){
	    clockval = -1;
	}  else{
	    clockval = parse_integer(speed) ;
	}

	rv = bcm_i2c_ioctl(unit, fd, I2C_PLL_SETW311,
			   &isInW311Mode, sizeof(isInW311Mode));
	if (rv < 0) {
	    printk("ERROR: clock mode configuration failed: %s\n",
		   bcm_errmsg(rv));
	}
	rv = bcm_i2c_ioctl(unit, fd, clockval, NULL, 0);
	if (rv < 0) {
	    printk("ERROR: clock speed configuration failed: %s\n",
		   bcm_errmsg(rv));
	}
    }
    return CMD_OK ;
}



/*
 * I2C specific commands for Baseboards which allow for control of
 * clocks, voltages and provide other measurements like power,
 * temperature, thickness, etc.
 */

/*
 * Tunable voltage parameters for P48 and Gen2 baseboards
 * This table encodes DAC/ADC VCC chip control and comm data.
 */
typedef 
struct bb_vconfig_params{
    int cal_idx;       /* DAC calibration table index: See ltc1427.c */
    char *function; /* Voltage source to configure */
    uint8 mux;         /* MUXSEL (lpt0) value */
    char *adc;       /* ADC device name */
    int  chan;         /* A/D channel #   */
    char *dac;       /* DAC Device name */
    int flags;    /* DAC calibrated, power computation IDs */
} bb_vparams_t;

/* Defines for the bb_vparams_t flags field */
#define DAC_UNCALIBRATED 0x00000000
#define DAC_CALIBRATED   0x00000001
#define PWR_CORE_A_SRC   0x00000002
#define PWR_CORE_A_DROP  0x00000004
#define PWR_CORE_B_SRC   0x00000008
#define PWR_CORE_B_DROP  0x00000010
#define PWR_PHY_SRC      0x00000020
#define PWR_PHY_DROP     0x00000040
#define PWR_CORECAL_SRC  0x00000080
#define PWR_CORECAL_DROP 0x00000100
#define PWR_PHYCAL_SRC   0x00000200
#define PWR_PHYCAL_DROP  0x00000400
#define PWR_IOCAL_SRC    0x00000800
#define PWR_IOCAL_DROP   0x00001000
#define PWR_IO_SRC       0x00002000
#define PWR_IO_DROP      0x00004000
#define PWR_CORE_A_LOAD  0x00008000
#define PWR_CORE_B_LOAD  0x00010000
#define PWR_PHY_LOAD     0x00020000

/*
 * Clock chip configuration table for all clock sources.
 * This table encodes the necessary data to communicate with every w311
 */
typedef struct bb_clock_config_s{
    char *name;   /* Clock output name */
    char *alias;  /* Clock output name (alias) */
    char *device; /* I2C device name   */
    int unit;     /* PCI Device */
    uint8 mux;    /* BBMUX (lpt0) value; 0xFF means no MUX control needed */
    int xpll;     /* For CY22393 (& CY22150), the PLL (or clock) to modify */
} bb_clock_config_t;



/* For P48 (G3) and BaseBoard/P48 command, we have a separate
 * Clock chip table since we are using W311's in a Mux'd fashion
 * which is specific to just this board. Other boards use the new
 * Cypress CY22393 clock chip and should use the "xclocks" command.
 * INDEXED_BY_BOARD_TYPE
 */
bb_clock_config_t
bb_clocks[NUM_BOARD_TYPES][MAX_CLOCKS_PER_BOARD] = {
    /* G5, Single (non-MUXed) CY22150, 2 clocks */
    {{"Core",   NULL, "clk0", 0, 0xFF,I2C_XPLL_CORE},
     {"PCI",    NULL, "clk0", 0, 0xFF,I2C_XPLL_PCI},
    },
    /* G6, Single (non-MUXed) CY22393, 3 clocks */
    {{"Core",   NULL,         "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Ref125", "Draco_Core", "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",    NULL,         "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G7, Single (non-MUXed) CY22393, 3 clocks */
    {{"Herc_Core",  NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Draco_Core", "Ref125", "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",        NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G8, Single (non-MUXed) CY22393, 3 clocks */
    {{"Lynx_Core", NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Test",      NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",       NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G9, Single (non-MUXed) CY22393, 3 clocks */
    {{"Core",      NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"SDRAM",     NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",       NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G10, Single (non-MUXed) CY22393, 3 clocks */
    {{"Lynx_Core", NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Herc_Core", NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",       NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G11, Single (non-MUXed) CY22393, 3 clocks */
    {{"Test1",       NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Core",        NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"Test2",       NULL, "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G12, Single (non-MUXed) CY22393, 3 clocks */
    {{"Test",        NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Draco_Core",  "Ref125", "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",         NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    /* G13, Single (non-MUXed) CY22393, 3 clocks */
    {{"Test",        NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL3},
     {"Core",        "Ref133", "clk0", 0, 0xFF,I2C_XPLL_PLL2},
     {"PCI",         NULL,     "clk0", 0, 0xFF,I2C_XPLL_PLL1},
    },
    {{0}}, /* G14, uses hclksel command for clock control */
    {{0}}, /* G15, uses synth command for clock control */
    {{0}}, /* G16, uses synth command for clock control */
    {{0}}, /* G17, uses synth command for clock control */
    {{0}}, /* G18, uses synth command for clock control */
    {{0}}, /* G19, uses synth command for clock control */
    {{0}}, /* G20, uses synth command for clock control */
    {{0}}, /* G21, uses synth command for clock control */
    {{0}}, /* G22, uses synth command for clock control */
    {{0}}, /* G23, uses synth command for clock control */
    {{0}}, /* G24, uses synth command for clock control */
    {{0}}, /* G25, uses synth command for clock control */
    {{0}}, /* G26, uses synth command for clock control */
    {{0}}, /* ENG */
    {{0}}  /* UNK */
};

/* 
 * bb_clock_len[] table contains the number of configurable
 * clocks available for each board type.
 * Used by cmd_bb() for "Baseboard" and "P48" commands.
 * A zero disables the clock function for Baseboard/P48.
 * INDEXED_BY_BOARD_TYPE 
 */
int bb_clock_len[] = {
    2, /* G5 */
    3, /* G6 */
    3, /* G7 */
    3, /* G8 */
    3, /* G9 */
    3, /* G10 */
    3, /* G11 */
    3, /* G12 */
    3, /* G13 */
    0, /* G14 */ /* Uses hclksel command */
    0, /* G15 */ /* Uses synth command */
    0, /* G16 */ /* Uses synth command */
    0, /* G17 */ /* Uses synth command */
    0, /* G18 */ /* Uses synth command */
    0, /* G19 */ /* Uses synth command */
    0, /* G20 */ /* Uses synth command */
    0, /* G21 */ /* Uses synth command */
    0, /* G22 */ /* Uses synth command */
    0, /* G23 */ /* Uses synth command */
    0, /* G24 */ /* Uses synth command */
    0, /* G25 */ /* Uses synth command */
    0, /* G26 */ /* Uses synth command */
    0, /* ENG */    
    0  /* UNK */    
};


/* Calibration table for DAC/ADC Voltage parameters.
 * Used to calibrate the DAC->PowerSupply->ADC loop.
 * NOTE: Turbo is special (MAX dacval = min voltage)
 * INDEXED_BY_BOARD_TYPE 
 * NOTE: For each board type, the order of each DAC entry
 *       must match that of their associated entries in 
 *       the bb_vs_config table.
 */
/* On HUMV/Bradley/GW slow part, max and min should be set to  960 and
 * 64 to prevent the system crashes during voltage calibration.
 */
#define HVSPO 64  /* HUMV Slow Part Offset */
/* idx, name, max, min, step, dac_last, dac_min, dac_max, dac_use_max */
static dac_calibrate_t
dac_params[NUM_BOARD_TYPES][4] = {
    /* Hercules: aka G5 */
    {{0,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0}},
    /* Draco: aka G6 */
    {{0,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"CoreA",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"CoreB",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Lancelot: aka G7 */
    {{0,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0}},
    /* Lynx SDK baseboard: aka G8 */
    {{0,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0}},
    {{0},{0},{0},{0}}, /* G9  */
    {{0},{0},{0},{0}}, /* G10 */
    /* Tucana/Magnum SDK baseboards: aka G11 */
    {{0,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"3.3",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0}},
    /* Draco 1.5 ref design: aka G12 */
    {{0,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"CoreA",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"CoreB",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Firebolt SDK design: aka G13 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"3.3",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Easyrider SDK design: aka G14 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"1.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"1.8",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Goldwing SDK design: aka G15 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {1,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {0},{0}},
    /* Bradley 1G SDK design: aka G16 */
    {{0,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {2,"Core",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {3,"1.2",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1}},
    /* Bradley SDK design: aka G17 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {1,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {0},{0}},
    /* Raptor SDK design: aka G18 */
    {{0,"1.2",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"PHY",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Raven SDK design: aka G19 */ 
    {{0,"1.2",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"PHY",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Triumph SDK design: aka G20 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"1.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Scorpion SDK design : aka G21 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0},{0}},
    /* Apollo SDK design: aka G22 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {0}},
    /* Sirius SDK design: aka G23 - TODO: Make sure this is correct */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"1.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"2.5",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1}},
    /* Trident SDK design: aka G24 */
    {{0,"Core",-1,1,0,0,ADP4000_MIN_DACVAL,ADP4000_MAX_DACVAL,1},
     {1,"PHY",-1,1,0,0,ADP4000_MIN_DACVAL,ADP4000_MAX_DACVAL,1},
     {2,"Analog",-1,1,0,0,ADP4000_MIN_DACVAL,ADP4000_MAX_DACVAL,1},
     {0}},
    /* Titan SDK design: aka G25 */
    {{0,"Core",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {1,"3.3",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {2,"5.0",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     {3,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL,LTC1427_MAX_DACVAL,1},
     },
	/* Hurricane  SDK design: aka G26 */
    {{0,"Core",  -1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {1,"PHY",   -1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {2,"Analog",-1,1,0,0,LTC1427_MIN_DACVAL+HVSPO,LTC1427_MAX_DACVAL-HVSPO,1},
     {0}},
    {{0},{0},{0},{0}}, /* ENG */
    {{0},{0},{0},{0}}  /* UNK */            
};

/* Each DAC parameter table, has a number of entries.
 * This value must be set for each board entry for the above table
 * to work correctly.
 * INDEXED_BY_BOARD_TYPE 
 */
static int dac_param_len[] = {
		       3, /* G5 */
		       4, /* G6 */
		       3, /* G7 */
		       3, /* G8 */
		       0, /* G9 */
		       0, /* G10 */
		       3, /* G11 */
		       4, /* G12 */
		       4, /* G13 */
		       4, /* G14 */
		       2, /* G15 */
		       4, /* G16 */
		       2, /* G17 */
		       4, /* G18 */
		       4, /* G19 */
		       4, /* G20 */	
		       2, /* G21 */	
		       4, /* G22 */	
		       4, /* G23 */	
		       3, /* G24 */	
		       3, /* G25 */	
		       3, /* G26 */	
		       0, /* ENG */
		       0  /* UNK */
};

#define MUX_0 0
#define MUX_1 1
#define MUX_2 2
#define MUX_3 3
#define MUX_4 4
#define MUX_8 8
#define MUX_C 0x0C

/* Adjustable Voltage Sources parameters.                     */
/* Idx: Index of each table entry (per board)                 */
/* Muxsel: MUX control value needed to access the DAC         */
/*         associated with the desired voltage                */
/*         (0xFF = No MUX control needed)                     */
/* NOTE: For each board type, the order of each voltage entry */
/*       must match that of their associated entries in the   */
/*       dac_params table.                                    */
/* INDEXED_BY_BOARD_TYPE                                      */
bb_vparams_t
bb_vs_config[NUM_BOARD_TYPES][4] = {
    /* Idx, Name, Muxsel, ADC Device Name, ADC Channel, DAC Device, Calibrated */
    /* G5: Hercules */
    {{0, "Analog",MUX_0, "adc1", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "2.5",   MUX_1, "adc1", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "Core",  MUX_2, "adc1", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {0}},
    /* G6: Draco */
    {{0, "2.5",    MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "Analog", MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "CoreA",  MUX_3, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "CoreB",  MUX_2, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED}},
    /* G7: Lancelot */
    {{0, "Analog", MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "Core",   MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "2.5",    MUX_3, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {0}},
    /* G8: Lynx */
    {{0, "Analog", MUX_0, "adc1", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "2.5",    MUX_1, "adc1", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "Core",   MUX_2, "adc1", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {0}},
    {{0},{0},{0},{0}}, /* G9  */        
    {{0},{0},{0},{0}}, /* G10 */        
    /* G11: Tucana/Magnum */
    {{0, "Core",  MUX_0,  "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {1, "2.5",   MUX_1,  "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {2, "3.3",   MUX_2,  "adc0", I2C_ADC_CH5, "dac0", DAC_UNCALIBRATED},
     {0}},
    /* G12: Draco 1.5 */
    {{0, "2.5",    MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "Analog", MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "CoreA",  MUX_3, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "CoreB",  MUX_2, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED}},
    /* Firebolt SDK design: aka G13 */
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "2.5",   MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "3.3",   MUX_2, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "Analog",MUX_3, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED}},
    /* Easyrider SDK design: aka G14 */
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "1.5",   MUX_1, "adc0", I2C_ADC_CH4, "dac0", DAC_UNCALIBRATED},
     {2, "1.8",   MUX_2, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "Analog",MUX_3, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED}},
    /* Goldwing SDK design: aka G15 */
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED},
     {1, "Analog",MUX_2, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {0},{0}},
    /* Bradley 1G SDK design: aka G16 */
    {{0, "Analog",MUX_0, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED},
     {1, "2.5",   MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "1.2",   MUX_3, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "Core",  MUX_2, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED}},
    /* Bradley SDK design: aka G17 */
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "Analog",MUX_2, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED},
     {0},{0}}, /* Note: MUX_3 is used for clock control on this board */
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "VDD2.5",MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "PHY",   MUX_2, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "Analog",MUX_3, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED},
    }, /* Raptor SDK G18 */        
    {{0, "Core",  MUX_0, "adc0", I2C_ADC_CH0, "dac0", DAC_UNCALIBRATED},
     {1, "VDD2.5",MUX_1, "adc0", I2C_ADC_CH1, "dac0", DAC_UNCALIBRATED},
     {2, "PHY",   MUX_2, "adc0", I2C_ADC_CH2, "dac0", DAC_UNCALIBRATED},
     {3, "Analog",MUX_3, "adc0", I2C_ADC_CH3, "dac0", DAC_UNCALIBRATED},
    }, /* Raven SDK G19 */
    {{0, "Core",  MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "2.5" ,  MUX_2 , "adc0" , I2C_ADC_CH1 , "dac0" , DAC_UNCALIBRATED},
     {2, "1.5" ,  MUX_1 , "adc0" , I2C_ADC_CH4 , "dac0" , DAC_UNCALIBRATED},
     {3, "Analog",MUX_3 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED}
    }, /* Triumph  SDK G20 */
    {{0, "Core",  MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "Analog",MUX_2 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED},
     {0},{0}/* Note: MUX_3 is used for clock control on this board */ 	
    }, /* Scorpion SDK G21 */ 
    {{0, "Core",  MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "2.5" ,  MUX_1 , "adc0" , I2C_ADC_CH1 , "dac0" , DAC_UNCALIBRATED},
     {2, "Analog",MUX_3 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED},
     {0}
    }, /* Apollo SDK G22 */
    {{0, "Core",  MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "2.5" ,  MUX_2 , "adc0" , I2C_ADC_CH1 , "dac0" , DAC_UNCALIBRATED},
     {2, "1.5" ,  MUX_1 , "adc0" , I2C_ADC_CH4 , "dac0" , DAC_UNCALIBRATED},
     {3, "Analog",MUX_3 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED}
    }, /* Sirius  SDK G23 */
    {{0, "Core",  1<<MUX_0 , "adc0" , I2C_ADC_CH0 , "pwctrl0" , DAC_UNCALIBRATED},
     {1, "PHY" ,  1<<MUX_2 , "adc0" , I2C_ADC_CH2 , "pwctrl0" , DAC_UNCALIBRATED},
     {2, "Analog",1<<MUX_3 , "adc0" , I2C_ADC_CH3 , "pwctrl0" , DAC_UNCALIBRATED},
     {0}
    }, /* Trident SDK G24 */
    {{0, "Core",  1 << MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "3.3",   1 << MUX_1 , "adc0" , I2C_ADC_CH5 , "dac0" , DAC_UNCALIBRATED},
     {2, "5.0" ,  1 << MUX_2 , "adc0" , I2C_ADC_CH6 , "dac0" , DAC_UNCALIBRATED},
     {3, "Analog",1 << MUX_3 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED}
    }, /* Titan SDK G25 */
    {{0, "Core",  1<<MUX_0 , "adc0" , I2C_ADC_CH0 , "dac0" , DAC_UNCALIBRATED},
     {1, "PHY",   1<<MUX_2 , "adc0" , I2C_ADC_CH2 , "dac0" , DAC_UNCALIBRATED},
     {2, "Analog",MUX_3 , "adc0" , I2C_ADC_CH3 , "dac0" , DAC_UNCALIBRATED},
     {0}
    }, /* Hurricane SDK G26 */
    {{0},{0},{0},{0}}, /* ENG */        
    {{0},{0},{0},{0}}  /* UNK */        
};

/* Map of number of configurable voltages, one per board.
 * Each one of these is the number of voltage parameters
 * (from above) for each table entry for each board.
 * INDEXED_BY_BOARD_TYPE
 */
int bb_vs_config_len[] = {			
			  3, /* G5 */
			  4, /* G6 */
			  3, /* G7 */
			  3, /* G8 */
			  0, /* G9 */
			  0, /* G10 */
			  3, /* G11 */
			  4, /* G12 */
			  4, /* G13 */
			  4, /* G14 */
			  2, /* G15 */
			  4, /* G16 */
			  2, /* G17 */
			  4, /* G18 */
			  4, /* G19 */
			  4, /* G20 */
			  2, /* G21 */
			  4, /* G22 */
			  4, /* G23 */
			  3, /* G24 */
			  4, /* G25 */
			  3, /* G26 */
			  0, /* ENG */
			  0  /* UNK */
};

/* All (non-configurable and configurable) voltages, per board.
 * For display only (only name, adc device and channel used).
 * NOTE: The text names which include an asterisk denote a
 *       configurable voltage source. The name in parentheses
 *       should match the name field in the corresponding entry
 *       in the bb_vs_config[] table.
 * INDEXED_BY_BOARD_TYPE
 */
bb_vparams_t
bb_vs_all[NUM_BOARD_TYPES][MAX_VOLTAGES_PER_BOARD] = {
    /* Hercules: aka G5 */
    {{ 0,"VDD_5.0V          ", 0, "adc1", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc1", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_Core (Core) * ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_1.2 (Analog) *", 0, "adc1", I2C_ADC_CH0, NULL, PWR_PHY_SRC},
     { 0,"VDD_2.5 (2.5) *   ", 0, "adc1", I2C_ADC_CH1, NULL, 0},     
     { 0,"IVDD_Core         ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_DROP},
     { 0,"IVDD_1.2          ", 0, "adc0", I2C_ADC_CH2, NULL, PWR_PHY_DROP},
     { 0,"ICAL_Core         ", 0, "adc0", I2C_ADC_CH1, NULL, PWR_CORECAL_DROP},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0},{0},{0},{0}},
    /* Draco: aka G6 */
    {{ 0,"VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"CORE_A (CoreA)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"CORE_B (CoreB)*   ", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"TP4001            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"TP4000            ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_CoreA        ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_CORE_A_DROP},
     { 0,"IVDD_CoreB        ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_CORE_B_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH4, NULL, PWR_PHY_DROP},
     { 0,"CoreA_SRC         ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"CoreB_SRC         ", 0, "adc1", I2C_ADC_CH6, NULL, PWR_CORE_B_SRC},
     { 0,"Analog_SRC        ", 0, "adc1", I2C_ADC_CH7, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC}, /* 16 */
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},
    /* Lancelot: aka G7 */
    {{ 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"CORE Far End      ", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"Analog Far End    ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"Analog @ PS       ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0},{0},{0},{0}},
    /* Lynx: aka G8 */
    {{ 0,"VDD_5.0V          ", 0, "adc1", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc1", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_Core (Core) * ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_1.2 (Analog) *", 0, "adc1", I2C_ADC_CH0, NULL, PWR_PHY_SRC},
     { 0,"VDD_2.5 (2.5) *   ", 0, "adc1", I2C_ADC_CH1, NULL, 0},     
     { 0,"IVDD_Core         ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_DROP},
     { 0,"IVDD_1.2          ", 0, "adc0", I2C_ADC_CH2, NULL, PWR_PHY_DROP},
     { 0,"ICAL_Core         ", 0, "adc0", I2C_ADC_CH1, NULL, PWR_CORECAL_DROP},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0},{0},{0},{0}},
    /* BCM5645 PoE, aka G9 */
    {{0}},
    /* BCM5673_P8_00 HercuLynx, aka G10 */
    {{0}},
    /* Tucana/Magnum boards: aka G11 */
    {{ 0,"VDD_2.5V (2.5) *  ", 0, "adc0", I2C_ADC_CH0, NULL, 0},     
     { 0,"VDD_1.25V (Core)* ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.8V          ", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"VDD_5.0V          ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V (3.3) *  ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"ADC0.6            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"ADC0.7            ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     { 0,"ICAL_1.25         ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_1.25         ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_CORE_A_DROP},
     { 0,"VDD_1.25V_SRC     ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_1.8V_SRC      ", 0, "adc1", I2C_ADC_CH6, NULL, 0},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0}},
    /* Draco 1.5 ref board, aka G12 */
    {{ 0,"VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"CORE_A (CoreA)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"CORE_B (CoreB)*   ", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"TP4001            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"TP4000            ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_CoreA        ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_CORE_A_DROP},
     { 0,"IVDD_CoreB        ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_CORE_B_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH4, NULL, PWR_PHY_DROP},
     { 0,"CoreA_SRC         ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC},
     { 0,"CoreB_SRC         ", 0, "adc1", I2C_ADC_CH6, NULL, PWR_CORE_B_SRC},
     { 0,"Analog_SRC        ", 0, "adc1", I2C_ADC_CH7, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC}, 
    },
    /* Firebolt SDK design: aka G13 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_LOAD},
     { 0,"VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_3.3V (3.3)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, PWR_PHY_LOAD | PWR_PHY_SRC | PWR_PHYCAL_SRC},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"TP4003            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"TP4002            ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_CORE_A_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
     { 0,"Core_SRC          ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_2.5           ", 0, "adc1", I2C_ADC_CH6, NULL, 0},
    },
    /* Easyrider SDK design: aka G14 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_LOAD},
     { 0,"VDD_2.5V          ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_1.8V (1.8)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, PWR_PHY_LOAD | PWR_PHY_SRC | PWR_PHYCAL_SRC},
     { 0,"VDD_1.5V (1.5)*   ", 0, "adc0", I2C_ADC_CH4, NULL, PWR_IO_SRC | PWR_IOCAL_SRC},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"TP4502            ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
     { 0,"ICal_1.5          ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_IOCAL_DROP},
     { 0,"IVDD_1.5          ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_IO_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH4, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_DROP},
     { 0,"VDD_2.5           ", 0, "adc1", I2C_ADC_CH6, NULL, 0},
    },
    /* Goldwing SDK design: aka G15 */
    {{ 0,"VDD_1.0V (Analog)*", 0, "adc0", I2C_ADC_CH0, NULL, PWR_PHY_LOAD},
     { 0,"VDD_2.5V          ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH3, NULL, PWR_CORE_A_LOAD},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V_B        ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_CORE_A_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
     { 0,"VDD_1.0V_PS       ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC},
     { 0,"CORE_PS           ", 0, "adc1", I2C_ADC_CH6, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
    },
    /* Bradley 1G SDK design: aka G16 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_LOAD},
     { 0,"VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_1.2V (1.2)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.0V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, PWR_PHY_LOAD},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_CORE_A_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
     { 0,"VDD_1.0V_PS       ", 0, "adc1", I2C_ADC_CH4, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC},
     { 0,"CORE_PS           ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_2.5V_PS       ", 0, "adc1", I2C_ADC_CH6, NULL, 0},
    },
    /* Bradley SDK design: aka G17 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_LOAD},
     { 0,"VDD_2.5V          ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_3.3V          ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.0V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, PWR_PHY_LOAD},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0,"VDD_3.3V_B        ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_CORE_A_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
     { 0,"CORE_PS           ", 0, "adc1", I2C_ADC_CH5, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_1.0V_PS       ", 0, "adc1", I2C_ADC_CH6, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC},
    },
    /* Raptor SDK design: aka G18 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, PWR_CORE_A_SRC | PWR_CORECAL_SRC},
     { 0,"VDD_2.5V (VDD2.5)*", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_1.2V/1.8V (Phy)*", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, PWR_PHY_SRC | PWR_PHYCAL_SRC},
     { 0,"VDD_3.3V_B        ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0,"ICal_Core         ", 0, "adc1", I2C_ADC_CH0, NULL, PWR_CORECAL_DROP},
     { 0,"IVDD_Core         ", 0, "adc1", I2C_ADC_CH1, NULL, PWR_CORE_A_DROP},
     { 0,"ICal_Analog       ", 0, "adc1", I2C_ADC_CH2, NULL, PWR_PHYCAL_DROP},
     { 0,"IVDD_Analog       ", 0, "adc1", I2C_ADC_CH3, NULL, PWR_PHY_DROP},
    },
    /* Raven SDK design: aka G19 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0,"VDD_2.5V (VDD2.5)*", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0,"VDD_1.0V/1.8V(Phy)*", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"VDD_3.3V_B        ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
    },
    /* Triumph SDK design : aka G20 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "VDD_1.8V          ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0, "VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "VDD_1.5V (1.5)*   ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0, "VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0, "VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
    }, 
    /* Scorpion SDK design : aka G21 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V	     ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "VDD_3.3V	     ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0, "VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "VDD_5V	     ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0, "VDD_3.3V	     ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
    },    
    /* Apollo SDK design : aka G22 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0, "VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
    }, 
    /* Sirius SDK design : aka G23 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V (2.5)*   ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "VDD_1.8V          ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0, "VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "VDD_1.5V (1.5)*   ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0, "VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0, "VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
    }, 
    /* Trident SDK design : aka G24 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V (2.5)    ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "PHY*              ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0, "VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "VDD_1.5V (1.5)    ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0, "VDD_3.3V          ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0, "VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0, "VDD_1.8V          ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
    },
    /* Titan SDK design : aka G25 */
    {{ 0, "CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0, "VDD_2.5V (2.5)    ", 0, "adc0", I2C_ADC_CH1, NULL, 0},
     { 0, "NC                ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0, "VDD_1.0V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0, "NC                ", 0, "adc0", I2C_ADC_CH4, NULL, 0},
     { 0, "VDD_3.3V (3.3)*   ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0, "VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
     { 0, "NC                ", 0, "adc0", I2C_ADC_CH7, NULL, 0},
    },
    /* Hurricane SDK design: aka G26 */
    {{ 0,"CORE (Core)*      ", 0, "adc0", I2C_ADC_CH0, NULL, 0},
     { 0,"VDD_1.2V (Phy)*   ", 0, "adc0", I2C_ADC_CH2, NULL, 0},
     { 0,"VDD_1.2V (Analog)*", 0, "adc0", I2C_ADC_CH3, NULL, 0},
     { 0,"VDD_3.3V_B        ", 0, "adc0", I2C_ADC_CH5, NULL, 0},
     { 0,"VDD_5V            ", 0, "adc0", I2C_ADC_CH6, NULL, 0},
    },
    /* ENG */
    {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},
    /* UNK */
    {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
     {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}
};

/* MAP of voltage display table lengths.
 * One per board, each one is the number of table entries in the
 * table above.
 * INDEXED_BY_BOARD_TYPE
 */
int bb_vs_all_len[] = {
    8,  /* G5 */
    16, /* G6 */
    8,  /* G7 */
    8,  /* G8 */
    0,  /* G9 */
    0,  /* G10 */
    11, /* G11 */
    15, /* G12 */
    14, /* G13 */
    15, /* G14 */
    12, /* G15 */
    13, /* G16 */
    12, /* G17 */
    10, /* G18 */
    6,  /* G19 */
    7,  /* G20 */	
    6,  /* G21 */	
    5,  /* G22 */	
    7,  /* G23 */	
    8,  /* G24 */	
    8,  /* G25 */	
    5,  /* G26 */
    0,  /* ENG */
    0   /* UNK */
};

/*
 *	Tc=0.000000
 *
 *	Tp=0.000000
 *
 *	Rp = (10.000000)        - Resistor for calibration current measurement
 *                                XXX measures voltage drop across
 *                                this resistor
 *
 *	TCoeff=0.003900         - Temperature Coefficient
 *
 *	Rcu=0.0000006787        - Resistivity
 *
 *	Lcal=1.000000           - Length of calibration strip
 *
 *	Lpwr=1.000000           - Length of Power Strip
 *
 *	Wcal=0.300000           - Width of calibration strip
 *
 *	Wpwr=0.700000           - Width of Power Strip
 *
 *	Vdrop_cal_core          - Voltage Drop across calibration strip
 *
 *	Vdrop_cal_phy           - Voltage Drop across calibration strip
 *
 *	Vdrop_cal_io            - Voltage Drop across calibration strip
 *
 *	Vdrop_coreA/Vdrop_coreB - Voltage Drop at the core calibration strip.
 *
 *	Vcal_core               - Core Voltage measured at the source.
 *
 *	Vcal_phy                - Phy voltage measured at the source
 *
 *	Vcal_io                 - IO voltage measured at the source.
 *
 *	Ical_core               - Calculated using Vcal_core and Resistance
 *                                value of Rp
 *
 *	Ical_phy                - Calculated using Vcal_core and Resistance
 *                                value of Rp
 *
 *	Ical_io                 - Calculated using Vcal_core and Resistance
 *                                value of Rp
 *
 *	Rseg_coreA/Rseg_coreB   - Resistance value of calibration strip is
 *      Rseg_phy                  estimated using the geometry of the
 *                                calibration strip and the resistance
 *                                computed for the Core calibration strip.
 *
 *	IcoreA                  - Calculated using Vdrop_coreA and Resistance
 *                                value of the calibration strip.
 *                                Resistance value of calibration strip is
 *                                estimated using the geometry of the
 *                                calibration strip and the resistance
 *                                computed for the Core calibration strip.
 *
 *	Rcal_core               - Resistance of calibration strip calculated
 *                                using the drop measured across the
 *                                calibration strip and the current flowing
 *                                through it.
 *                                Can also be extimated using the Resistivity
 *                                of copper, temprature and geometry of the
 *                                calibration strip.
 *
 *	Rcal_phy                - Resistance of calibration strip calculated
 *                                using the drop measured across the
 *                                calibration strip and the current flowing
 *                                through it.
 *                                Can also be extimated using the Resistivity
 *                                of copper, temprature and geometry of the
 *                                calibration strip.
 *
 */

#ifdef COMPILER_HAS_DOUBLE

/* Power computation definitions */
#define STD_RP          10.00         /* 10 Ohms, defines cal currents */
#define STD_LENGTH      2.00          /* Inches, length of CAL/PWR strips */
#define STD_WC          0.100         /* Inches, width of CAL strip */
#define STD_WP          0.700         /* Inches, width of PWR strip */
#define STD_RGAIN_PWR   130.0         /* RGp, sets gain of LT1167A amp */
#define STD_RGAIN_CAL   49.90	      /* RGc, sets gain of LT1167A amp */

/* The LT1167s are configured to output 100s of milliamps.  */
/* Lower than this (or negative) indicates a probable empty */
/* socket.                                                  */
#define MIN_AMPLIFIED_VOLTAGE_DROP 0.050

/* Computes actual voltage drop from (amplified) value at LT1167A's output */
#define GAIN(v,r)   ( (v) / (1 + (49900.0/(r)))  )

#define VNULL       -9.99        /* Uninitialized voltage value */
#define RNULL       999.999     /* Uninitialized resistance value */

/* Thickness constants, based on PCB manufacturing parameters */
#define RCu         6.787E-07   /* Resistivity of Copper (Ohms * Inches) */
#define TCoeff      0.0039      /* Thermal Coefficient (1 / C) */   

#else /* !COMPILER_HAS_DOUBLE */

/* Power computation definitions */
#define STD_RP          10000         /* mOhms, defines cal currents */
#define STD_LENGTH      2000          /* 1/1000 Inches, length of CAL/PWR strips */
#define STD_WC          100           /* 1/1000 Inches, width of CAL strip */
#define STD_WP          700           /* 1/1000 Inches, width of PWR strip */
#define STD_RGAIN_PWR   130000        /* RGp, sets gain of LT1167A amp */
#define STD_RGAIN_CAL   49900	      /* RGc, sets gain of LT1167A amp */

/* The LT1167s are configured to output 100s of milliamps.  */
/* Lower than this (or negative) indicates a probable empty */
/* socket.                                                  */
#define MIN_AMPLIFIED_VOLTAGE_DROP 50000

/* Computes actual voltage drop from (amplified) value at LT1167A's output */
#define GAIN(v,r)   ( (v) / (1 + (49900000/(r)))  )

#define VNULL       -9990000          /* Uninitialized voltage value */
#define RNULL       999999            /* Uninitialized resistance value */

/* Thickness constants, based on PCB manufacturing parameters */
#define RCu         6787              /* Resistivity of Copper (Ohms * Inches) * 1e-10 */
#define TCoeff      39                /* Thermal Coefficient (1 / C) * 1e-4 */   

#define RCAL_MUL    100               /* Rcal mulyiplier for better precision */

#endif /* COMPILER_HAS_DOUBLE */

/*
 * Control Voltage, and SDRAM/Core Clocks on P48 integrated board.
 */
char cmd_bb_usage[] =
    "Baseboard commands (bb) for I2C satellite devices\n"
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "       bb board | clock | spread | voltage | power | temperature\n"
#else
    "       bb board\n\t"
    "         - show board type detected\n\t"
    "       bb clock [Core_A SDRAM_A Core_B SDRAM_B Turbo] value\n\t"
    "         - set specified clock speed.\n\t"
    "       bb spread [Core_A SDRAM_A Core_B SDRAM_B Turbo] value\n\t"
    "         - value: 0=normal,1=-0.5%,2=+/-0.5%,3=+/-0.25%,4=+/-0.38%\n\t"
    "       bb voltage [A B TRef PHY] [<volts>|calibrate|nominal]\n\t"
    "         - set voltage output, if not specified, value is displayed.\n\t"
    "         - if \"calibrate\" is specified, calibrate specified DAC.\n\t"
    "         - if \"nominal\" is specified, DAC set to mid-range.\n\t"
    "       bb power\n\t"
    " 	      - computer power utilization.\n\t"
    "       bb temperature Tp Tc\n\t"
    " 	      - trace temperature computation.\n\t"	 
    " 	        - Tp :PHY trace thickness.\n\t"	 
    " 	        - Tc :Core trace thickness.\n\t"	 
    "       bb thickness  Tp Tc\n\t"
    " 	      - trace thickness computation.\n\t"
    " 	        - Tp :PHY trace temperature.\n\t"	 
    " 	        - Tc :Core trace temperature.\n\t"	 	 
    "       bb pot number value\n\t"
    "         - set potentiometer.\n\t"
    "           - number: potentiometer number 0..7\n\t"
    "           - value : value to set 0..255\n\t"
    "NOTE: calibration of DAC is performed (automatically) only once.\n"
#endif
    ;


cmd_result_t
cmd_bb(int unit, args_t *a)
{
    int i, bx, clk_unit, pll, adc, dac = -1, mux, rv = SOC_E_NONE;
    char *function = ARG_GET(a);
    char *source = ARG_GET(a);
    char *value = ARG_GET(a);
    uint32 flags = 0;
#ifdef COMPILER_HAS_DOUBLE
    double volts, clockval;
#else
    int volts, clockval;
#endif
    int show_cal = 0;
    max127_t  max127;
    bb_type_t* baseboard = NULL;


    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if ( (! function) )
        return CMD_USAGE ;

    /* Get board type */
    baseboard = soc_get_board();
    if(!baseboard || (baseboard->bbcmd_flags == BB_NONE)){
        soc_cm_print("Error: baseboard command unavailable on %s\n",
                BaseboardName);
        return CMD_FAIL;
    }

    if (!sal_strcasecmp(function, "board")) {
        soc_cm_print("Baseboard: %s (type %d)\n",
                BaseboardName, baseboard->type);
        return CMD_OK;
    }

    /* Get board index */
    bx = baseboard->type;

    /* Configure or display clock frequency or spread-spectrum */
    if(!sal_strcasecmp(function,"clock")  ||
            !sal_strcasecmp(function,"spread") ){
        /* Check for supported clock chips (W311, CY22393, CY22150) */
        if ( !(baseboard->bbcmd_flags & BB_CLK) ) {
            printk("Baseboard clock configuration function not supported.\n");
            printk("use clocks or xclocks command instead.\n");
            return CMD_FAIL;
        }
        /* Set clock speed or spread spectrum */
        if( source && value ) { 

            /* First, find clock table entry ... */
            for( i = 0; i < bb_clock_len[bx]; i++) {
                if ( !sal_strcasecmp(bb_clocks[bx][i].name, source))
                    break;
                if ( (bb_clocks[bx][i].alias != NULL) &&
                        !sal_strcasecmp(bb_clocks[bx][i].alias, source))
                    break;
            }
            if (i == bb_clock_len[bx]) {	/* unknown board clock */
                printk("ERROR: unknown clock source: %s\n", source);
                printk("ERROR: valid clocks are");
                for(i = 0; i < bb_clock_len[bx]; i++) {
                    printk(" %s", bb_clocks[bx][i].name);
                }
                printk("\n");
                return CMD_FAIL;
            }

            /* If indicated, set the MUX appropriately */
            if (bb_clocks[bx][i].mux != 0xFF) {
                /* Open MUX (lpt0) device */
                if ( (mux = bcm_i2c_open(unit, I2C_LPT_0, flags, 0)) < 0) {
                    printk("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), I2C_LPT_0, bcm_errmsg(mux));
                    return CMD_FAIL;
                }

                /* Set mux value so we can see the device when we try to open it*/
                if( (rv = bcm_i2c_write(unit, mux, 0,
                                &bb_clocks[bx][i].mux, 1)) < 0){
                    printk("ERROR: I2C write failed: %s\n",
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            } /* Need to set MUX for the clock chip */

            /* Open clock chip, this could be on the second bus,
             * so we need to check for that here ..
             */
            if ( bb_clocks[bx][i].unit != unit){
                clk_unit = bb_clocks[bx][i].unit;
                printk("NOTICE: clock chip on i2c%d\n", clk_unit);
            }
            else
                clk_unit = unit;

            /* Open clock chip, this will cause a probe to happen
             * if not already done.
             */
            if ( (pll = bcm_i2c_open(clk_unit,
                            bb_clocks[bx][i].device, flags, 0)) < 0) {
                printk("%s: cannot open I2C device %s: %s\n",
                        ARG_CMD(a), bb_clocks[bx][i].device, bcm_errmsg(pll));
                return CMD_FAIL;
            }

            /*
            * Use Cypress CY22393x (or CY22150)
            * Both devices are named "clk0"
            */
#ifdef COMPILER_HAS_DOUBLE
            clockval = atof( value ) ;
#else
            /* Parse input in MHz and convert to Hz */
            clockval = _shr_atof_exp10(value, 6);
#endif

            /* Set speed */
            if(!sal_strncasecmp(function,"clock",strlen(function))){

                if (!sal_strcasecmp(bb_clocks[bx][i].name, "CPU")) {
                    printk("NOTICE: changing CPU clock requires removal"
                        " of jumper JP302.\n");
                }
                /* Set speed */
                if ( (bcm_i2c_ioctl(clk_unit, pll,
                    bb_clocks[bx][i].xpll,
                    &clockval, 0) < 0)) {
#ifdef COMPILER_HAS_DOUBLE
                        printk("ERROR: failed to set %s clock to %2.2fMHz\n",
                            bb_clocks[bx][i].name, clockval);
#else
                        printk("ERROR: failed to set %s clock to %d.%02dMHz\n",
                            bb_clocks[bx][i].name, INT_FRAC_2PT_4(clockval));
#endif
                        return CMD_FAIL;
                }	    
            }  
            else{
                /* Spread spectrum mode */
                printk("ERROR: CY22xxx does not have spread spectrum.\n");
                return CMD_FAIL;
            }

            return CMD_OK;

        } /* (source && value), set a clock */ 

        else {
            /* Display single clock or all clocks */

            i = 0; /* Top of table unless a specific clock is requested */

            if (source) {
                /* Request to show a single clock. Find it first... */
                for(  ; i < bb_clock_len[bx]; i++) {
                    if ( !sal_strcasecmp(bb_clocks[bx][i].name, source))
                        break;
                    if ( (bb_clocks[bx][i].alias != NULL) &&
                            !sal_strcasecmp(bb_clocks[bx][i].alias, source))
                        break;
                }
                if (i == bb_clock_len[bx]) {	/* unknown board clock */
                    printk("ERROR: unknown clock source: %s\n", source);
                    printk("ERROR: valid clocks are");
                    for(i = 0; i < bb_clock_len[bx]; i++) {
                        printk(" %s", bb_clocks[bx][i].name);
                    }
                    printk("\n");
                    return CMD_FAIL;
                }
            } /* Requested a single clock display */

            /* Already have entry number (single clock), or we'll go */
            /* through the whole clock table entry (all clocks)...   */
            for(  ; i < bb_clock_len[bx]; i++) {

                /* If indicated, set the MUX appropriately */
                if (bb_clocks[bx][i].mux != 0xFF) {
                    /* Open MUX (lpt0) device */
                    if ( (mux = bcm_i2c_open(unit, I2C_LPT_0, flags, 0)) < 0) {
                        printk("Could not open %s for mux selection:%s\n",
                                I2C_LPT_0, bcm_errmsg(mux));
                        return CMD_FAIL;
                    }

                    /* Set mux value so we can see the device ...*/
                    if( (rv = bcm_i2c_write(unit,
                                    mux, 0,
                                    &bb_clocks[bx][i].mux, 1)) < 0){
                        printk("Error: write of lpt byte failed:%s\n",
                                bcm_errmsg(rv));
                        return CMD_FAIL;
                    }
                } /* Need to set MUX for the clock chip */

                if ( bb_clocks[bx][i].unit != unit){
                    clk_unit = bb_clocks[bx][i].unit;
                    printk("NOTICE: clock chip on i2c%d\n", clk_unit);
                }
                else
                    clk_unit = unit;

                /* Open clock chip */
                if ( (pll = bcm_i2c_open(clk_unit,
                                bb_clocks[bx][i].device,
                                flags, 0)) < 0) {
                    printk("Could not open %s for PLL speed selection:%s\n",
                            I2C_PLL_0, bcm_errmsg(pll));
                    return CMD_FAIL;
                }

                /* CY2239x or CY22150 clock chip */

                /* Get clock speed */
                clockval = 0;
                if ( (bcm_i2c_ioctl(clk_unit, pll,
                    I2C_XPLL_GET_OP(bb_clocks[bx][i].xpll),
                    &clockval, 0) < 0)){
                        printk("Error: failed to retrieve clock speed.\n");
                }
#ifdef COMPILER_HAS_DOUBLE
                printk("\t\"%s\" clock running at %2.2fMhz\n",
                    bb_clocks[bx][i].name, clockval);
#else
                printk("\t\"%s\" clock running at %d.%02dMhz\n",
                    bb_clocks[bx][i].name, INT_FRAC_2PT_4(clockval));
#endif
                if (source)
                    break; /* ... if showing single clock only */
            } /* For each entry in the bb_clocks[] table */
            return CMD_OK;
        } /* display clock(s) */
    } /* function is "clock" or "spread" */

    /* Power, Thickness, and Temperature computations */
    if(!sal_strncasecmp(function,"power",strlen(function)) ||
            (!sal_strncasecmp(function,"thickness",strlen(function))) ||
            (!sal_strncasecmp(function,"temperature",strlen(function)))){	

        /* Check for support of these functions... */
        if ((baseboard->bbcmd_flags & BB_PWR)) {
            int op = 0;
#define BB_OP_POWER    1
#define BB_OP_TEMP     2
#define BB_OP_THICK    3	

#ifdef COMPILER_HAS_DOUBLE
            double Rp, l_cal, l_pwr, w_cal, w_pwr, RGp, RGc;
            double VcoreA= VNULL, Vdrop_coreA= VNULL;
            double VcoreB= VNULL, Vdrop_coreB= VNULL;
            double Vphy= VNULL, Vdrop_phy= VNULL;
            double Vio= VNULL, Vdrop_io= VNULL;
            double Vcal_core= VNULL, Vdrop_cal_core= VNULL;
            double Vcal_phy= VNULL, Vdrop_cal_phy= VNULL;	    
            double Vcal_io= VNULL, Vdrop_cal_io= VNULL;	    
            double Ical_core = 0, Rcal_core = 0;
            double Ical_phy = 0, Rcal_phy = 0;
            double Ical_io = 0, Rcal_io = 0;
            double Rseg_coreA, IcoreA, PcoreA;
            double Rseg_coreB, IcoreB, PcoreB;
            double Rseg_phy, Iphy, Pphy;
            double Rseg_io, Iio, Pio;
            double tCore, tPhy, Tp, Tc;
#else
            int Rp, l_cal, l_pwr, w_cal, w_pwr, RGp, RGc;
            int VcoreA= VNULL, Vdrop_coreA= VNULL;
            int VcoreB= VNULL, Vdrop_coreB= VNULL;
            int Vphy= VNULL, Vdrop_phy= VNULL;
            int Vio= VNULL, Vdrop_io= VNULL;
            int Vcal_core= VNULL, Vdrop_cal_core= VNULL;
            int Vcal_phy= VNULL, Vdrop_cal_phy= VNULL;	    
            int Vcal_io= VNULL, Vdrop_cal_io= VNULL;	    
            int Ical_core = 0, Rcal_core = 0;
            int Ical_phy = 0, Rcal_phy = 0;
            int Ical_io = 0, Rcal_io = 0;
            int Rseg_coreA, IcoreA, PcoreA;
            int Rseg_coreB, IcoreB, PcoreB;
            int Rseg_phy, Iphy, Pphy;
            int Rseg_io, Iio, Pio;
            int tCore, tPhy, Tp, Tc;
            int t1;
            char *s1;

            if (SOC_IS_XGS(unit)) {
                s1 = "Analog";
            } else {
                s1 = "Phy";
            }
#endif

            /* Define the geometries and gains of the    */
            /* calibration and power measurement strips. */

            Rp = STD_RP;         /* Calibration current resistor */
            l_cal = STD_LENGTH;  /* Length of calibration strip */
            l_pwr = STD_LENGTH;  /* Length of power strip */
            w_cal = STD_WC;      /* Width of calibration strip */
            w_pwr = STD_WP;      /* Width of power strip */
            RGc = STD_RGAIN_CAL; /* Gain of calibration strip voltage amp */
            RGp = STD_RGAIN_PWR; /* Gain of power strip voltage amp */

            /* Special case boards with different dimensions/parameters */

#ifdef COMPILER_HAS_DOUBLE
            if ( bx == G6) {
                /* Draco baseboard */
                l_cal = 1.0;
                l_pwr = 1.0;
            }
            else if (bx == G11) {
                /* Tucana and Magnum baseboards */
                w_cal = 0.3;
            }
            else if (bx == G12) {
                /* Draco 1.5 ref design */
                Rp = 5.0;        
                l_cal = 1.0;
                l_pwr = 1.0;
                w_cal = 0.3;
                RGc = 24.9;
            }
            else if ((bx == G13) || (bx == G14)) {
                /* Firebolt, Easyrider SDKs */
                l_cal = 1.0;
                l_pwr = 1.0;
                w_cal = 0.3;
            }
            else if ((bx == G15) || (bx == G16) || (bx == G17)) {
                /* Goldwing, Bradley SDKs */
                l_cal = 1.0;
                l_pwr = 1.0;
                w_cal = 0.3;
            } else if ((bx == G18) || (bx == G19)) {
                /* Raptor  and Raven */
                Rp = 5.0;        
                l_cal = 1.0;
                l_pwr = 1.0;
                w_cal = 0.3;
            }
#else
            if ( bx == G6) {
                /* Draco baseboard */
                l_cal = 1000;
                l_pwr = 1000;
            }
            else if (bx == G11) {
                /* Tucana and Magnum baseboards */
                w_cal = 300;
            } else if (bx == G12) {
                /* Draco 1.5 ref design */
                Rp = 5000;        
                l_cal = 1000;
                l_pwr = 1000;
                w_cal = 300;
                RGc = 24900;
            } else if ((bx == G13) || (bx == G14)) {
                /* Firebolt, Easyrider SDKs */
                l_cal = 1000;
                l_pwr = 1000;
                w_cal = 300;
            } else if ((bx == G15) || (bx == G16) || (bx == G17)) {
                /* Goldwing, Bradley SDKs */
                l_cal = 1000;
                l_pwr = 1000;
                w_cal = 300;
            } else if ((bx == G18) || (bx == G19)) {
                /* Raptor  and Raven */
                Rp = 5.0;        
                l_cal = 1.0;
                l_pwr = 1.0;
                w_cal = 0.3;
            }
#endif

            /* Select computation */
            if(!sal_strncasecmp(function,"thickness",strlen(function))){
                op = BB_OP_THICK;
                if( !source || !value)
                    return CMD_USAGE;
#ifdef COMPILER_HAS_DOUBLE
                Tp = atof(source);
                Tc = atof(value);
#else
                /* Temperature is 1/10 C */
                Tp = _shr_atof_exp10(source, 1);
                Tc = _shr_atof_exp10(value, 1);
#endif
            } else if(!sal_strncasecmp(function,"temperature",strlen(function))){
                op = BB_OP_TEMP;
                if( !source || !value)
                    return CMD_USAGE;
#ifdef COMPILER_HAS_DOUBLE
                Tp = atof(source);
                Tc = atof(value);
#else
                Tp = _shr_atof_exp10(source, 6);
                Tc = _shr_atof_exp10(value, 6);
#endif
            } else {
                op = BB_OP_POWER;
                Tp = Tc = 0;
            }

            /* Collect all voltages */	
            for(i = 0; i < bb_vs_all_len[bx]; i++){
                if ( (adc = bcm_i2c_open(unit,
                                bb_vs_all[bx][i].adc,
                                flags, 0)) < 0) {
                    printk("Could not open %s:%s\n",
                            bb_vs_all[bx][i].adc,
                            bcm_errmsg(adc));
                    return CMD_FAIL;
                }
                /* Display voltage */
                if ( (bcm_i2c_ioctl(unit, adc,
                                bb_vs_all[bx][i].chan,
                                &max127, 0) < 0)) {
                    printk("Error: failed to perform A/D conversions.\n");
                }

                /* Assign each voltage to the proper Vxxx variable
                 * based upon bb_vs_all[bx][i].flags.
                 * VcoreA          PWR_CORE_A_SRC
                 * Vdrop_coreA     PWR_CORE_A_DROP
                 * VcoreB          PWR_CORE_B_SRC
                 * Vdrop_coreB     PWR_CORE_B_DROP
                 * Vphy            PWR_PHY_SRC
                 * Vdrop_phy       PWR_PHY_DROP
                 * Vcal_core       PWR_CORECAL_SRC
                 * Vdrop_cal_core  PWR_CORECAL_DROP
                 * Vcal_phy        PWR_PHYCAL_SRC
                 * Vdrop_cal_phy   PWR_PHYCAL_DROP
                 */ 

                /*
                 * PWR_CORE_A_SRC is the voltage at the power supply, and
                 * PWR_CORE_A_LOAD is the voltage at the chip, however,
                 * PWR_CORE_A_LOAD is not available in all designs.
                 *
                 * For the most accurate measurement, PWR_CORE_A_LOAD
                 * should be used if it is available.
                 */
                if (bb_vs_all[bx][i].flags & PWR_CORE_A_LOAD) {
                    VcoreA = max127.val;
                } else if ((bb_vs_all[bx][i].flags & PWR_CORE_A_SRC) &&
                           VcoreA == VNULL) {
                    VcoreA = max127.val;
                }
                if (bb_vs_all[bx][i].flags & PWR_CORE_A_DROP) { 
                    if (max127.val < 0) max127.val *= -1;
                    if (max127.val > MIN_AMPLIFIED_VOLTAGE_DROP)
                        Vdrop_coreA = GAIN(max127.val,RGp);
                    else
                        Vdrop_coreA = 0;
                }
                if (bb_vs_all[bx][i].flags & PWR_CORE_B_SRC)
                    VcoreB = max127.val;
                if (bb_vs_all[bx][i].flags & PWR_CORE_B_DROP) {
                    if (max127.val < 0) max127.val *= -1;
                    if (max127.val > MIN_AMPLIFIED_VOLTAGE_DROP)
                        Vdrop_coreB = GAIN(max127.val,RGp);
                    else
                        Vdrop_coreB = 0;
                }
                /*
                 * PWR_PHY_SRC is the voltage at the power supply, and
                 * PWR_PHY_LOAD is the voltage at the chip, however,
                 * PWR_PHY_LOAD is not available in all designs.
                 *
                 * For the most accurate measurement, PWR_PHY_LOAD
                 * should be used if it is available.
                 */
                if (bb_vs_all[bx][i].flags & PWR_PHY_LOAD) {
                    Vphy = max127.val;
                } else if ((bb_vs_all[bx][i].flags & PWR_PHY_SRC) &&
                           Vphy == VNULL) {
                    Vphy = max127.val;
                }
                if (bb_vs_all[bx][i].flags & PWR_PHY_DROP) {
                    if (max127.val < 0) max127.val *= -1;
                    if (max127.val > MIN_AMPLIFIED_VOLTAGE_DROP)
                        Vdrop_phy = GAIN(max127.val,RGp);
                    else
                        Vdrop_phy = 0;
                }
                if (bb_vs_all[bx][i].flags & PWR_IO_SRC)
                    Vio = max127.val;
                if (bb_vs_all[bx][i].flags & PWR_IO_DROP) {
                    if (max127.val < 0) max127.val *= -1;
                    if (max127.val > MIN_AMPLIFIED_VOLTAGE_DROP)
                        Vdrop_io = GAIN(max127.val,RGp);
                    else
                        Vdrop_io = 0;
                }
                if (bb_vs_all[bx][i].flags & PWR_CORECAL_SRC)
                    Vcal_core = max127.val;
                if (bb_vs_all[bx][i].flags & PWR_CORECAL_DROP) {
                    if (max127.val < 0) max127.val *= -1;
                    Vdrop_cal_core = GAIN(max127.val,RGc);
		}

		if (bb_vs_all[bx][i].flags & PWR_PHYCAL_SRC)
		    Vcal_phy = max127.val;
		if (bb_vs_all[bx][i].flags & PWR_PHYCAL_DROP) {
		    if (max127.val < 0) max127.val *= -1;
		    Vdrop_cal_phy = GAIN(max127.val,RGc);
		}

		if (bb_vs_all[bx][i].flags & PWR_IOCAL_SRC)
		    Vcal_io = max127.val;
		if (bb_vs_all[bx][i].flags & PWR_IOCAL_DROP) {
		    if (max127.val < 0) max127.val *= -1;
		    Vdrop_cal_io = GAIN(max127.val,RGc);
		}

	    }
	    
	    /* calculations */

	    /* There is at least one calibration strip. If there are multiple */
	    /* strips, it is assumed that they have the same geometry.        */

	    if (Vdrop_cal_core != VNULL) { 
	        /* Current flowing through Rp and core cal strip to ground */
	        Ical_core = Vcal_core/Rp;
	        /* Resistance of core calibration strip */
#ifdef COMPILER_HAS_DOUBLE
	        Rcal_core = Vdrop_cal_core/Ical_core;
#else
	        Rcal_core = (RCAL_MUL * Vdrop_cal_core) / Ical_core;
#endif
	    }

	    if (Vdrop_cal_phy != VNULL) {
		/* Current flowing through Rp and phy cal strip to ground */
		Ical_phy = Vcal_phy/Rp;
		/* Resistance of phy calibration strip */
#ifdef COMPILER_HAS_DOUBLE
		Rcal_phy = Vdrop_cal_phy/Ical_phy;
#else
		Rcal_phy = (RCAL_MUL * Vdrop_cal_phy) / Ical_phy;
#endif
	    }

	    if (Vdrop_cal_io != VNULL) {
		/* Current flowing through Rp and IO cal strip to ground */
		Ical_io = Vcal_io/Rp;
		/* Resistance of IO calibration strip */
#ifdef COMPILER_HAS_DOUBLE
		Rcal_io = Vdrop_cal_io/Ical_io;
#else
		Rcal_io = (RCAL_MUL * Vdrop_cal_io) / Ical_io;
#endif
	    }

	    if (Vdrop_cal_core == VNULL) {
	      /* No separate core calibration strip. */
	      /* Use PHY calibration strip.          */
		Ical_core = Ical_phy;
		Rcal_core = Rcal_phy;
	    }
	    
	    if (Vdrop_cal_phy == VNULL) {
	      /* No separate PHY calibration strip. */
	      /* Use core calibration strip.        */
		Ical_phy = Ical_core;
		Rcal_phy = Rcal_core;
	    }

	    if (Vdrop_cal_io == VNULL) {
	      /* No separate IO calibration strip. */
	      /* Use phy calibration strip.        */
		Ical_io = Ical_phy;
		Rcal_io = Rcal_phy;
	    }


	    if (Vdrop_coreA != VNULL) {
	        /* Power : Core A */
#ifdef COMPILER_HAS_DOUBLE
	        Rseg_coreA = Rcal_core * (w_cal/w_pwr) * (l_pwr/l_cal);
		IcoreA = Vdrop_coreA/Rseg_coreA;
		PcoreA = IcoreA * VcoreA;
#else
	        Rseg_coreA = (((Rcal_core * w_cal) / w_pwr) * l_pwr) / l_cal;
		IcoreA = ((RCAL_MUL * Vdrop_coreA * w_pwr) / l_pwr) * l_cal;
                IcoreA /= (Rcal_core * w_cal);
		PcoreA = (IcoreA * (VcoreA / 1000)) / 1000; /* mW */
#endif
	    }
	    else {
		IcoreA = PcoreA = 0;
		Rseg_coreA = RNULL;
	    }

	    if (Vdrop_coreB != VNULL) {
		/* Power: Core B */
#ifdef COMPILER_HAS_DOUBLE
		Rseg_coreB = Rcal_core * (w_cal/w_pwr) * (l_pwr/l_cal);
		IcoreB = Vdrop_coreB/Rseg_coreB;
		PcoreB = IcoreB * VcoreB;
#else
	        Rseg_coreB = (((Rcal_core * w_cal) / w_pwr) * l_pwr) / l_cal;
		IcoreB = ((RCAL_MUL * Vdrop_coreB * w_pwr) / l_pwr) * l_cal;
                IcoreB /= (Rcal_core * w_cal);
		PcoreB = (IcoreB * (VcoreB / 1000)) / 1000; /* mW */
#endif
	    }
	    else {
		IcoreB = PcoreB = 0;
		Rseg_coreB = RNULL;
	    }

	    if (Vdrop_phy != VNULL) {
		/* Power: Phy */
#ifdef COMPILER_HAS_DOUBLE
		Rseg_phy = Rcal_phy * (w_cal / w_pwr) * (l_pwr / l_cal);
		Iphy = Vdrop_phy / Rseg_phy;
		Pphy = Iphy * Vphy;
#else
	        Rseg_phy = (((Rcal_phy * w_cal) / w_pwr) * l_pwr) / l_cal;
                Iphy = ((RCAL_MUL * Vdrop_phy * w_pwr) / l_pwr) * l_cal;
                Iphy /= (Rcal_phy * w_cal);
                Pphy = (Iphy * (Vphy / 1000)) / 1000; /* mW */
#endif
	    }
	    else {
		Iphy = Pphy = 0;
		Rseg_phy = RNULL;
	    }

	    if (Vdrop_io != VNULL) {
		/* Power: IO */
#ifdef COMPILER_HAS_DOUBLE
		Rseg_io = Rcal_io * (w_cal / w_pwr) * (l_pwr / l_cal);
		Iio = Vdrop_io / Rseg_io;
		Pio = Iio * Vio;
#else
	        Rseg_io = (((Rcal_io * w_cal) / w_pwr) * l_pwr) / l_cal;
                Iio = ((RCAL_MUL * Vdrop_io * w_pwr) / l_pwr) * l_cal;
                Iio /= (Rcal_io * w_cal);
                Pio = (Iphy * (Vphy / 1000)) / 1000; /* mW */
#endif
	    }
	    else {
		Iio = Pio = 0;
		Rseg_io = RNULL;
	    }

	    if (bx == G5) {
		/* For this board, the calibration strip was placed on
		 * a different PCB plane than the measurement strips.
		 * Adjust current and power values to compensate for
		 * the difference in plane thicknesses. 
		 */
		PcoreA = (PcoreA * 540) / 1000; /* PcoreA * 0.54 */
		IcoreA = (IcoreA * 540) / 1000; /* IcoreA * 0.54 */
		Pphy = (Pphy * 540) / 1000; /* Pphy * 0.54 */
		Iphy = (Iphy * 540) / 1000; /* Iphy * 0.54 */
	    }

#define BB_DEBUG
#ifdef BB_DEBUG  
	    /* Debugging */
#ifdef COMPILER_HAS_DOUBLE
#define PFMT "%f"
#else
#define PFMT "%d"
#endif
            /* Constants */
	    soc_cm_debug(DK_VERBOSE, "\nConstants\n");
#ifdef COMPILER_HAS_DOUBLE
	    soc_cm_debug(DK_VERBOSE, "Rcu=%1.10f\n", RCu);
#else
	    soc_cm_debug(DK_VERBOSE, "Rcu=%d\n", RCu);
#endif
	    soc_cm_debug(DK_VERBOSE, "Lcal=" PFMT "\n", l_cal);
	    soc_cm_debug(DK_VERBOSE, "Lpwr=" PFMT "\n", l_pwr);
	    soc_cm_debug(DK_VERBOSE, "Wcal=" PFMT "\n", w_cal);
	    soc_cm_debug(DK_VERBOSE, "Wpwr=" PFMT "\n", w_pwr);	    
	    soc_cm_debug(DK_VERBOSE, "Rp=" PFMT "\n", Rp);

	    soc_cm_debug(DK_VERBOSE, "\nMeasured Voltage Values\n");
	    soc_cm_debug(DK_VERBOSE, "Vdrop_cal_core=" PFMT "\n", Vdrop_cal_core);
	    soc_cm_debug(DK_VERBOSE, "Vdrop_cal_phy=" PFMT "\n", Vdrop_cal_phy);
	    soc_cm_debug(DK_VERBOSE, "Vdrop_cal_io=" PFMT "\n", Vdrop_cal_io);
	    soc_cm_debug(DK_VERBOSE, "Vcal_core=" PFMT "\n", Vcal_core);
	    soc_cm_debug(DK_VERBOSE, "Vcal_phy=" PFMT "\n", Vcal_phy);
	    soc_cm_debug(DK_VERBOSE, "Vcal_io=" PFMT "\n", Vcal_io);
	    soc_cm_debug(DK_VERBOSE, "Vdrop_coreA=" PFMT "\n", Vdrop_coreA);

	    soc_cm_debug(DK_VERBOSE, "\nCalculated Current Values\n");
	    soc_cm_debug(DK_VERBOSE, "Ical_core=" PFMT "\n", Ical_core);
	    soc_cm_debug(DK_VERBOSE, "Ical_phy=" PFMT "\n", Ical_phy);	
	    soc_cm_debug(DK_VERBOSE, "Ical_io=" PFMT "\n", Ical_io);	

	    soc_cm_debug(DK_VERBOSE, "\nCalculated Resistance Values\n");
	    soc_cm_debug(DK_VERBOSE, "Rcal_core=" PFMT "\n", Rcal_core);
	    soc_cm_debug(DK_VERBOSE, "Rcal_phy=" PFMT "\n", Rcal_phy);	    
	    soc_cm_debug(DK_VERBOSE, "Rcal_io=" PFMT "\n", Rcal_io);	    
	    soc_cm_debug(DK_VERBOSE, "Rseg_coreA=" PFMT "\n", Rseg_coreA);
	    soc_cm_debug(DK_VERBOSE, "Rseg_phy=" PFMT "\n", Rseg_phy);	    
	    soc_cm_debug(DK_VERBOSE, "Rseg_io=" PFMT "\n", Rseg_io);	    

	    soc_cm_debug(DK_VERBOSE, "Tc=" PFMT "\n", Tc);
	    soc_cm_debug(DK_VERBOSE, "Tp=" PFMT "\n", Tp);
#endif
#undef BB_DEBUG
	    /* Show power */
	    if( op == BB_OP_POWER) {

	        printk("Power Measurement\n");

		if (Vdrop_coreA == VNULL) {
		    /* 
		     * Only 1 core power measurement (Core B) 
		     */
#ifdef COMPILER_HAS_DOUBLE
		    printk("PCore  = %2.3fW  ICore = %2.3fA  VCore = %2.3fV\n",
			   PcoreB, IcoreB, VcoreB);
#else
		    printk("PCore  = %d.%03dW  ICore = %d.%03dA  VCore = %d.%03dV\n",
			   INT_FRAC_3PT(PcoreB), INT_FRAC_3PT(IcoreB),
                           INT_FRAC_3PT(VcoreB));
#endif
		}
		else if (Vdrop_coreB == VNULL) {
		    /* 
		     * Only 1 core power measurement (Core A)
		     */
#ifdef COMPILER_HAS_DOUBLE
		    printk("PCore  = %2.3fW  ICore = %2.3fA  VCore = %2.3fV\n",
			   PcoreA, IcoreA, VcoreA);
#else
		    printk("PCore  = %d.%03dW  ICore = %d.%03dA  VCore = %d.%03dV\n",
			   INT_FRAC_3PT(PcoreA), INT_FRAC_3PT(IcoreA),
                           INT_FRAC_3PT(VcoreA));
#endif
		}
		else {
		    /* 
		     * Both core power measurements 
		     */
#ifdef COMPILER_HAS_DOUBLE
		    printk("PCoreA = %2.3fW ICoreA = %2.3fA  VCoreA = %2.3fV\n",
			   PcoreA, IcoreA, VcoreA);
		    printk("PCoreB = %2.3fW ICoreB = %2.3fA  VCoreB = %2.3fV\n",
			   PcoreB, IcoreB, VcoreB);
#else
		    printk("PCoreA = %d.%03dW  ICoreA = %d.%03dA  VCoreA = %d.%03dV\n",
			   INT_FRAC_3PT(PcoreA), INT_FRAC_3PT(IcoreA),
                           INT_FRAC_3PT(VcoreA));
		    printk("PCoreB = %d.%03dW  ICoreB = %d.%03dA  VCoreB = %d.%03dV\n",
			   INT_FRAC_3PT(PcoreB), INT_FRAC_3PT(IcoreB),
                           INT_FRAC_3PT(VcoreB));
#endif
		}

		if (Vdrop_phy != VNULL) { 
		    /* 
		     * PHY (analog) power measurement 
		     */
		    if (bx == G8) {
		        /* The BCM5673_P4_00 platform has incorrect    */
		        /* plane geometry to be accurate. Ignore Pphy. */ 
			Pphy = 0;
		    } else {
#ifdef COMPILER_HAS_DOUBLE
			printk("PPhy   = %2.3fW   IPhy = %2.3fA   VPhy = %2.3fV\n",
			       Pphy, Iphy, Vphy);
#else
			printk("PPhy   = %d.%03dW   IPhy = %d.%03dA   VPhy = %d.%03dV\n",
			       INT_FRAC_3PT(Pphy), INT_FRAC_3PT(Iphy), 
                               INT_FRAC_3PT(Vphy));
#endif
		    }
		}

		if (Vdrop_io != VNULL) { 
		    /* 
		     * IO power measurement 
		     */
#ifdef COMPILER_HAS_DOUBLE
		    printk("P_IO   = %2.3fW   I_IO = %2.3fA   V_IO = %2.3fV\n",
			   Pio, Iio, Vio);
#else
		    printk("P_IO   = %d.%03dW   I_IO = %d.%03dA   V_IO = %d.%03dV\n",
			   INT_FRAC_3PT(Pio), INT_FRAC_3PT(Iio), INT_FRAC_3PT(Vio));
#endif
		}

#ifdef COMPILER_HAS_DOUBLE
		printk("%2.3f Watts Total\n",
		       (PcoreA + PcoreB + Pphy + Pio));
#else
		printk("%d.%03d Watts Total\n",
		       INT_FRAC_3PT(PcoreA + PcoreB + Pphy + Pio));
#endif


	    } else if (op == BB_OP_THICK) {
		/* Thickness : Tp/Tc are temperatures */
#ifdef COMPILER_HAS_DOUBLE
		tCore = tPhy = (l_cal * RCu);
		tCore /= (Rcal_core * w_cal)/ (1.0 + (Tc - 20.0) * TCoeff);
		tPhy /=  (Rcal_phy * w_cal)/ (1.0 + (Tp - 20.0) * TCoeff);

		if (SOC_IS_XGS(unit)){
		    printk("Thickness Core (mil) = %2.3f\n",1000*tCore);
		    printk("Thickness Analog (mil) = %2.3f\n",1000*tPhy);
		} else {
		    printk("Thickness Core (mil) = %2.3f\n",1000*tCore);
		    printk("Thickness Phy (mil) = %2.3f\n",1000*tPhy);
		}
#else
		t1 = l_cal * RCu * RCAL_MUL;
		tCore = t1 / (Rcal_core * w_cal);
                tCore *= 10000 + (((Tc - 200) * TCoeff) / 10);
                tCore /= 100000;
		tPhy =  t1 / (Rcal_phy * w_cal);
                tPhy *= 10000 + (((Tp - 200) * TCoeff) / 10);
                tPhy /= 100000;

                printk("Thickness Core (mil) = %d.%03d\n", INT_FRAC_3PT(tCore));
                printk("Thickness %s (mil) = %d.%03d\n", s1, INT_FRAC_3PT(tPhy));
#endif

	    } else {
		/* Temperature */
#ifdef COMPILER_HAS_DOUBLE
		tCore = (((Rcal_core * w_cal * Tc) /
			  (l_cal * RCu) - 1.0) / TCoeff) + 20.0;
		tPhy  = (((Rcal_phy *  w_cal * Tp) /
			  (l_cal * RCu) - 1.0) / TCoeff) + 20.0;

		if (SOC_IS_HERCULES1(unit) || SOC_IS_DRACO(unit)){
		    printk("Temperature Core = %2.3fC/%2.3fF\n",tCore,
			   (9.0/5.0) * tCore + 32);
		    printk("Temperature Analog  = %2.3fC/%2.3fF\n",tPhy,
			   (9.0/5.0) * tPhy + 32);
		} else {
		    printk("Temperature Core = %2.3fC/%2.3fF\n",tCore,
			   (9.0/5.0) * tCore + 32);
		    printk("Temperature Phy  = %2.3fC/%2.3fF\n",tPhy,
			   (9.0/5.0) * tPhy + 32);
		}
#else
		t1 = (l_cal * RCu) / (100000 / RCAL_MUL);
		tCore = (Rcal_core * w_cal * Tc) / t1;
                tCore = ((10 * (tCore - 10000)) / TCoeff) + 200;
		tPhy = (Rcal_phy * w_cal * Tc) / t1;
                tPhy = ((10 * (tPhy - 10000)) / TCoeff) + 200;

                t1 = (tCore * 9) / 5 + 32;
                printk("Temperature Core = %d.%01dC/%d.%01dF\n", 
                       INT_FRAC_1PT(tCore), INT_FRAC_1PT(t1));
                t1 = (tPhy * 9) / 5 + 32;
                printk("Temperature %s = %d.%01dC/%d.%01dF\n", s1,
                       INT_FRAC_1PT(tPhy), INT_FRAC_1PT(t1));
#endif
	    }
	} /* Power calculation functions supported */ 
	else {
	    printk("Platform[%s] does not support the power features.\n",
		   BaseboardName);
	    return CMD_FAIL;
	}
	
	return CMD_OK;
    } /* function is "power", "thickness" or "temperature" */

    if(!sal_strncasecmp(function,"pot",strlen(function))){
        int pot_num, val, pot;
        uint8  p8, v8;
        max5478_t cmd;

        if (!source || !value) {
            return CMD_USAGE;
        }
        pot_num = sal_ctoi(source, 0);
        if ((pot_num < 0) || (pot_num > 7)) {
            return CMD_USAGE;
        }
        val = sal_ctoi(value, 0);
        if ((val < 0) || (val > 255)) {
            return CMD_USAGE;
        }

        /* Select device in mux */
        if ( (mux = bcm_i2c_open(unit, I2C_LPT_0, flags, 0)) < 0) {
            printk("Could not open %s for mux selection:%s\n",
                   I2C_LPT_0, bcm_errmsg(mux));
            return CMD_FAIL;
        }

        /* 
         * Set mux value so we can see the device when we try to open it
         * Device is specified pot_num / 2 
         */
        p8 = pot_num / 2;
        if( (rv = bcm_i2c_write(unit, mux, 0, &p8, 1)) < 0){
            printk("Error: write of lpt byte failed:%s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if ( (pot = bcm_i2c_open(unit, I2C_POT_0, flags, 0)) < 0) {
            printk("Could not open %s :%s\n",
                   I2C_POT_0, bcm_errmsg(pot));
            return CMD_FAIL;
        }

        v8 = (uint8) val;
        /* Now set potentiometer */
        cmd.wiper = pot_num % 2;
        cmd.value = v8;
        rv = bcm_i2c_ioctl(unit, pot, I2C_POT_IOC_SET, (void *)&cmd, 0);
        if (rv < 0) {
            printk("Error: failed setting pot(%d, %d): %s\n",
                   pot_num, val, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    } /* pot */
        
    /* Configure or display voltage for any or all BB voltage
     * sources, abbreviate to "v"
     */
    if(!sal_strncasecmp(function,"voltage",strlen(function))){

	/* Check Board Index, which should have voltage config
	 * parameters.
	 */
	if( !(baseboard->bbcmd_flags & BB_VOLT) || bb_vs_config_len[bx] == 0) {
	    printk("Error: %s does not support voltage control subsystem.\n",
		   BaseboardName);
	    return CMD_FAIL;
	}

	
	/* Show all voltages, note that ADC's are always visible,
	 * hence we do not need to set the BBMUX value
	 */
	if( ! source){
	    for(i = 0; i < bb_vs_all_len[bx]; i++){
		if ( (adc = bcm_i2c_open(unit,
					 bb_vs_all[bx][i].adc,
					 flags, 0)) < 0) {
		    printk("Could not open %s:%s\n",
			   bb_vs_all[bx][i].adc,
			   bcm_errmsg(adc));
		    return CMD_FAIL;
		}
		/* Display voltage */
		if ( (bcm_i2c_ioctl(unit, adc,
				    bb_vs_all[bx][i].chan,
				    &max127, 0) < 0)) {
		    printk("Error: failed to perform A/D conversions.\n");
		}

		
#ifdef COMPILER_HAS_DOUBLE
		printk("%s%s%2.3f Volts "
		       "(min=%2.3f max=%2.3f delta=%2.3f samples=%d)\n",
		       bb_vs_all[bx][i].function,
		       max127.val >= 0 ? "  " : " ",
		       max127.val,
		       max127.min,max127.max, max127.delta,
		       soc_i2c_max127_iterations);
#else
		printk("%s%s%d.%03d Volts "
		       "(min=%d.%03d max=%d.%03d delta=%d.%03d samples=%d)\n",
		       bb_vs_all[bx][i].function,
		       max127.val >= 0 ? "  " : " ",
		       INT_FRAC_3PT_3(max127.val),
		       INT_FRAC_3PT_3(max127.min),
                       INT_FRAC_3PT_3(max127.max),
                       INT_FRAC_3PT_3(max127.delta),
		       soc_i2c_max127_iterations);
#endif
		       
	    }
	    return CMD_OK;
	} /* Show all voltages */

	/* Here if a Voltage Source was specified. 
	 * Show or configure just that one voltage.
	 * This currently works only for configurable voltages.
	 */

	/* First, find voltage table entry ... */
	for( i = 0; i < bb_vs_config_len[bx]; i++) {
	    if ( !sal_strcasecmp(bb_vs_config[bx][i].function, source))
		break;
	}
	/* Not found - ERROR! */
	if(i == bb_vs_config_len[bx]){
	    printk("Unknown Voltage source: %s\n", source);
	    return CMD_USAGE;
	}

	if (value && (bb_vs_config[bx][i].mux != 0xFF)) {
            char *dev_name;
	    /* Going to set a voltage (need DAC), and the DAC is MUXed */
	    /* Open MUX (lpt0) device */
            dev_name = _i2c_mux_dev_name_get(I2C_MUX_VOLTAGE_CONTROL);
	    if ( (mux = bcm_i2c_open(unit, dev_name, flags, 0)) < 0) {
		printk("Could not open %s for mux selection:%s\n",
		       dev_name?dev_name:"MUX_DEV", bcm_errmsg(mux));
		return CMD_FAIL;
	    }
	  
	    /* Set mux value so we can see the device when we try to open it*/
	    if( (rv = bcm_i2c_write(unit, mux, 0, &bb_vs_config[bx][i].mux, 1)) < 0){
		printk("Error: write of lpt byte failed:%s\n",
		       bcm_errmsg(rv));
		return CMD_FAIL;
	    }
	} /* Setting a voltage, needed to manipulate MUX for the DACs */

	if (value) {
	    /* Going to calibrate and/or set a voltage; open the correct DAC */
	    if ( (dac = bcm_i2c_open(unit, bb_vs_config[bx][i].dac,
				     flags, 0)) < 0) {
	        
		printk("Could not open %s:%s\n",
		       bb_vs_config[bx][i].dac,
		       bcm_errmsg(dac));
		return CMD_FAIL;
	    }
	}

	/* Open the ADC Device (either adc0 or adc1) */
	if ( (adc = bcm_i2c_open(unit, bb_vs_config[bx][i].adc,
				 flags, 0)) < 0) {
	    printk("Could not open %s:%s\n",
		   bb_vs_config[bx][i].adc,
		   bcm_errmsg(adc));
	    return CMD_FAIL;
	}

	/* From this point on, we have an ADC (and possibly its 
	 * associated DAC) open and ready for commands.
	 */

	/* Set and/or possibly calibrate voltage, we need to do this
	 * at least once (the first time), and possibly later if the
	 * user requests a calibration of a source ...
	 */
	if( value ) {
	    /* printk("calibration: board %d: subsys%d\n",bx, i); */

	    /* Always designate the DAC calibration table. This info
	     * is needed for any voltage setting operations. If
	     * a calibration is performed, the table is updated.  
	     */
	    if ( (bcm_i2c_ioctl(unit, dac,
				I2C_DAC_IOC_SET_CALTAB,
				(void*)dac_params[bx],
				dac_param_len[bx]) < 0)) {
		printk("Error: failed to set DAC calibration table.\n");
	    }
	    
	    /* Calibrate the DAC->ADC loop 
	     * (if not yet done, or explicitly requested). */
	    if ( (show_cal = !sal_strcasecmp(value,"calibrate")) ||
		 (show_cal = !sal_strcasecmp(value,"cal")) ||
		 !(show_cal = bb_vs_config[bx][i].flags & DAC_CALIBRATED) ) {

	        if (show_cal)
		    printk("calibration: board %d: subsys%d\n",bx, i);

		/* We need to calibrate the DAC to compute the min,
		 * max, and the Volts/step values that result at the
		 * associated ADC. 
		 */

		/* Set the DAC to the (digital) value which         */
		/* generates the minimum analog voltage at the ADC. */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_SETDAC_MIN,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to set min DAC Vout.\n");
		}

		/* Read the resultant (minimum) voltage at the ADC. */
		if ( (bcm_i2c_ioctl(unit, adc,
				    bb_vs_config[bx][i].chan,
				    &max127, 0) < 0)) {
		    printk("Error: failed to read A/D min voltage:VDD_%s.\n",
			   bb_vs_config[bx][i].function);
		}
		volts = max127.val;

	        if (show_cal)
#ifdef COMPILER_HAS_DOUBLE
		    printk("Min=%2.3fV, ", volts);
#else
		    printk("Min=%d.%03dV, ", INT_FRAC_3PT_3(volts));
#endif

		/* Update DAC calibration table with this minimum voltage. */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_CALIBRATE_MIN,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to update min DAC Vout.\n");
		}

		/* Set the DAC to the (digital) value which         */
		/* generates the maximum analog voltage at the ADC. */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_SETDAC_MAX,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to set max DAC Vout.\n");
		}

		/* Read the resultant (maximum) voltage at the ADC. */
		if ( (bcm_i2c_ioctl(unit, adc,
				    bb_vs_config[bx][i].chan, &max127, 0) < 0)) {
		    printk("Error: failed to read A/D max voltage:VDD_%s.\n",
			   bb_vs_config[bx][i].function);
		}
		volts = max127.val;
		
	        if (show_cal)
#ifdef COMPILER_HAS_DOUBLE
		    printk("Max=%2.3fV\n", volts);
#else
		    printk("Max=%d.%03dV\n", INT_FRAC_3PT_3(volts));
#endif

		/* Update DAC calibration table with this maximum voltage. */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_CALIBRATE_MAX,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to update max DAC Vout.\n");
		}

		/* Very important, set the DAC back to nominal 
		 * (bootup midscale).
		 */
		if( (rv = bcm_i2c_ioctl(unit, dac, 
					I2C_DAC_IOC_SETDAC_MID,
					&volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: resetting of DAC to nominal value failed:%s\n",
			   bcm_errmsg(rv));
		    return CMD_FAIL;
		}		

				
		/* Now, update the calibration table with the
		 * analog voltage step per DAC digital step.
		 * (also displays all of the calibration table info)
		 */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_CALIBRATE_STEP,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to calibrate DAC VDD_%s.\n",
			   bb_vs_config[bx][i].function);
		}
		/* Indicate that this DAC-ADC pair is calibrated */
		bb_vs_config[bx][i].flags = DAC_CALIBRATED;
	    } /* DAC->ADC not yet calibrated or calibration was requested */

	    /* Get the user's desired analog voltage value to set. */

	    if (!sal_strncasecmp(value,"nominal",strlen(value))) {
		printk("Resetting %s voltage to nominal\n", source);
	        if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_SETDAC_MID,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
		    printk("Error: failed to reset VDD_%s DAC to mid-range value.\n",
			   bb_vs_config[bx][i].function);
		    
		    return CMD_FAIL;
		}

		/* Read the ADC to get the actual resultant voltage value. */
		if ( (bcm_i2c_ioctl(unit, adc,
				    bb_vs_config[bx][i].chan, &max127, 0) < 0)) {
		    printk("Error: failed to perform A/D conversions.\n");
		}

		/* Display the recently configured voltage */
#ifdef COMPILER_HAS_DOUBLE
		printk("VDD_%s = %2.3f Volts "
		       "(min=%2.3f max=%2.3f delta=%2.3f samples=%d)\n",
		       bb_vs_config[bx][i].function, max127.val,
		       max127.min,max127.max, max127.delta,
		       soc_i2c_max127_iterations);
#else
		printk("VDD_%s = %d.%03d Volts "
		       "(min=%d.%03d max=%d.%03d delta=%d.%03d samples=%d)\n",
		       bb_vs_config[bx][i].function,
                       INT_FRAC_3PT_3(max127.val),
		       INT_FRAC_3PT_3(max127.min),
                       INT_FRAC_3PT_3(max127.max),
                       INT_FRAC_3PT_3(max127.delta),
		       soc_i2c_max127_iterations);
#endif
	        return CMD_OK;
	    }

	    /* Here if a voltage level was specified */
	    
#ifdef COMPILER_HAS_DOUBLE
	    volts = atof(value); 
#else
            /* Parse input in V and convert to uV */
            volts = _shr_atof_exp10(value, 6);
#endif
	    if(volts > 0){

		printk("Configuring voltage on [%s]\n", source);
	    
		/* Finally, set the actual voltage ... */
		if ( (bcm_i2c_ioctl(unit, dac,
				    I2C_DAC_IOC_SET_VOUT,
				    &volts, bb_vs_config[bx][i].cal_idx) < 0)) {
#ifdef COMPILER_HAS_DOUBLE
		    printk("Error: failed to set VDD_%s to %2.3fV.\n",
			   bb_vs_config[bx][i].function, volts);
#else
		    printk("Error: failed to set VDD_%s to %d.%03dV.\n",
			   bb_vs_config[bx][i].function, INT_FRAC_3PT_3(volts));
#endif
		    printk("Run \"bb voltage %s calibrate\" to view allowable range.\n",
			   bb_vs_config[bx][i].function);

		    return CMD_FAIL;
		}

		

		/* Read the ADC to get the actual resultant voltage value. */
		if ( (bcm_i2c_ioctl(unit, adc,
				    bb_vs_config[bx][i].chan, &max127, 0) < 0)) {
		    printk("Error: failed to perform A/D conversions.\n");
		}

		/* Display the recently configured voltage */
#ifdef COMPILER_HAS_DOUBLE
		printk("VDD_%s = %2.3f Volts "
		       "(min=%2.3f max=%2.3f delta=%2.3f samples=%d)\n",
		       bb_vs_config[bx][i].function, max127.val,
		       max127.min,max127.max, max127.delta,
		       soc_i2c_max127_iterations);
#else
		printk("VDD_%s = %d.%03d Volts "
		       "(min=%d.%03d max=%d.%03d delta=%d.%03d samples=%d)\n",
		       bb_vs_config[bx][i].function,
                       INT_FRAC_3PT_3(max127.val),
		       INT_FRAC_3PT_3(max127.min),
                       INT_FRAC_3PT_3(max127.max),
                       INT_FRAC_3PT_3(max127.delta),
		       soc_i2c_max127_iterations);
#endif
	    }
	} /* Set a single voltage */
	else {
	    /* Display a single voltage */
	    if ( (bcm_i2c_ioctl(unit, adc,
				bb_vs_config[bx][i].chan, &max127, 0) < 0)) {
		printk("Error: failed to perform A/D conversions.\n");
	    }

#ifdef COMPILER_HAS_DOUBLE
	    printk("VDD_%s = %2.3f Volts "
		   "(min=%2.3f max=%2.3f delta=%2.3f samples=%d)\n",
		   bb_vs_config[bx][i].function, max127.val,
		   max127.min,max127.max, max127.delta,
		   soc_i2c_max127_iterations);
#else
	    printk("VDD_%s = %d.%03d Volts "
		   "(min=%d.%03d max=%d.%03d delta=%d.%03d samples=%d)\n",
		   bb_vs_config[bx][i].function, 
                   INT_FRAC_3PT_3(max127.val),
		   INT_FRAC_3PT_3(max127.min),
                   INT_FRAC_3PT_3(max127.max),
                   INT_FRAC_3PT_3(max127.delta),
		   soc_i2c_max127_iterations);
#endif

	} /* Display a single voltage */
    } /* function is "voltage" */

    else {
	return CMD_USAGE;
    } /* Unknown function */ 

    return CMD_OK ;
} /* end cmd_bb() */

STATIC int dac_devs_init (int unit,int bx,char *dev_name) 
{
    int mux;
    int i;
    int dac;
    int rv = 0;
    int flags = 0;
    
    /* open mux device used for all channel's DAC devices
     * All DAC devices behind the mux device must be the same device and have
     * same I2C address
     */
    if ( (mux = bcm_i2c_open(unit, dev_name, flags, 0)) < 0) {
        printk("Could not open %s for mux selection:%s\n",
               dev_name?dev_name:"MUX_DEV", bcm_errmsg(mux));
        return CMD_FAIL;
    }

    /* get dac driver's device id */
    if ((dac = bcm_i2c_open(unit, bb_vs_config[bx][0].dac, flags, 0)) < 0) {
        printk("Could not open %s:%s\n", bb_vs_config[bx][0].dac, bcm_errmsg(dac));
        return CMD_FAIL;
    }

   /* initialize each dac device behind the I2C mux device */
    for (i = 0; i < bb_vs_config_len[bx]; i++) {
        if (bb_vs_config[bx][i].dac == NULL) {
            break;
        }
        if( (rv = bcm_i2c_write(unit, mux, 0, &bb_vs_config[bx][i].mux, 1)) < 0){
            printk("Error: write of mux device byte failed:%s\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
        soc_i2c_device(unit, dac)->driver->load(unit,dac,NULL,0);
    }

    return SOC_E_NONE;
}


#endif /* INCLUDE_I2C */


#endif /* NO_SAL_APPL */
