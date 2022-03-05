/*
 * $Id: bcm-diag-full.c,v 1.30 Broadcom SDK $
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

#include <gmodule.h> /* Must be included first */

#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <sal/core/dpc.h>
#include <sal/appl/sal.h>
#include <sal/appl/pci.h>

#include <linux-bde.h>

#include <appl/diag/sysconf.h>
#include <appl/diag/system.h>

#include <bcmx/cosq.h>
#include <bcmx/switch.h>

/* All shell io is done using a user/kernel proxy service. */
#include <linux-uk-proxy.h>


MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("BCM Diag Shell Full");
MODULE_LICENSE("Proprietary");

/* Debug output */
static int debug;
LKM_MOD_PARAM(debug, "i", int, 0);
MODULE_PARM_DESC(debug,
"Set debug level (default 0)");

/* Create proxy service */
static int proxy = 1;
LKM_MOD_PARAM(proxy, "i", int, 0);
MODULE_PARM_DESC(proxy,
"Create proxy service (default 1)");

static int boot_flags = 0;
LKM_MOD_PARAM(boot_flags, "i", int, 0);
MODULE_PARM_DESC(boot_flags, "boot flags");

/*
 * Linux Module/Library References
 *
 * Not all of the BCM/BCMX functions are referenced by the driver
 * itself. Because of this, some of the orphan functions may not
 * get linked into the module. Since we want these functions to
 * be available to client modules which might be inserted later,
 * we reference them here to make sure they get linked in.
 *
 */
typedef void (*orphan)(void);

static orphan orphans[] = {
#if defined(INCLUDE_BCMX)
    (orphan)bcmx_cosq_config_set,
    (orphan)bcmx_cosq_mapping_set,
    (orphan)bcmx_switch_control_set
#endif /* INCLUDE_BCMX */
};

/* Module Information */
#define MODULE_MAJOR 124
#define MODULE_MINOR 0
#define MODULE_NAME      "linux-bcm-diag-full"
#define PROXY_SERVICE	 "BCM DIAG SHELL"

/* Maximum string we can handle in printf */
#define PROXY_STRING_MAX (LUK_MAX_DATA_SIZE * 6)

/* Message buffer */
#define DBUF_DATA_SIZE 128
typedef struct _dbuf_t {
    char data[DBUF_DATA_SIZE];
} dbuf_t;
#define DBUF_DATA(dbuf) dbuf->data

/* Buffer console messages from interrupt context */
#define DBUF_CNT_MAX 32
static dbuf_t dbuf[DBUF_CNT_MAX];
static volatile int dbuf_cnt;

/* Thread control flags */
static volatile int _bcm_shell_running = 0;
static sal_thread_t _bcm_shell_thread = 0;

/*
 * The system BDE. 
 * This BDE can be used by all client modules. 
 */
ibde_t* bde;

/* Linux kernel threads must check signals explicitely. */
void
check_exit_signals(void)
{
    if(signal_pending(current)) {
        sal_dpc_term();
        sal_thread_exit(0);
    }
}

/* Proc filesystem information */
static int
_pprint(void)
{	
    pprintf("Broadcom Linux BCM Full Diagnostic Shell\n"); 
    pprintf("\tProxy Service: '%s'\n", PROXY_SERVICE); 
    return 0;
}

static void
_tty_flush(void *p1, void *p2, void *p3, void *p4, void *p5)
{
    int cnt;
    int spl;
    dbuf_t *d;
    char *p;

    spl = sal_splhi();
    if (dbuf_cnt) {
        for (cnt = 0; cnt < dbuf_cnt; cnt++) {
            d = &dbuf[cnt];
            p = DBUF_DATA(d);
            sal_spl(spl);
            linux_uk_proxy_send(PROXY_SERVICE, p, strlen(p));
            spl = sal_splhi();
        }
        dbuf_cnt = 0;
    }
    sal_spl(spl);
}

int 
tty_vprintf(const char* fmt, va_list args)
{
    int cnt, tmp_cnt, offset=0; 
    static char s[PROXY_STRING_MAX]; 

    if (in_interrupt()) {
        /* Schedule flush function */
        if (dbuf_cnt == 0) {
            sal_dpc(_tty_flush, 0, 0, 0, 0, 0);
        }
        /* Buffer message */
        if (dbuf_cnt < DBUF_CNT_MAX) {
            dbuf_t *d = &dbuf[dbuf_cnt++];
            cnt = vsnprintf(DBUF_DATA(d), DBUF_DATA_SIZE-1, fmt, args);
        }
        return 0;
    }

    if (dbuf_cnt) {
        /* Flush buffered messages */
        _tty_flush(0, 0, 0, 0, 0);
    }

    tmp_cnt = cnt = vsnprintf(s, PROXY_STRING_MAX - 1, fmt, args);
    if (tmp_cnt >= PROXY_STRING_MAX) {
        tmp_cnt = PROXY_STRING_MAX;
    }
    while (tmp_cnt > 0) {
        linux_uk_proxy_send(PROXY_SERVICE, &s[offset], 
                   (tmp_cnt < LUK_MAX_DATA_SIZE) ? tmp_cnt : LUK_MAX_DATA_SIZE); 
        tmp_cnt -= LUK_MAX_DATA_SIZE;
        offset += LUK_MAX_DATA_SIZE;
    }
    check_exit_signals();
    return cnt; 
}

int
tty_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return tty_vprintf(fmt, args);
}

char* 
tty_gets(char* dst, int size)
{
    while (linux_uk_proxy_recv(PROXY_SERVICE, dst, (unsigned int *)&size) < 0) {
        sal_sleep(1);
    }
    check_exit_signals();
    return dst; 
}

char* 
tty_fgets(char* dst, int size, int fh)
{
    while (linux_uk_proxy_recv(PROXY_SERVICE, dst, (unsigned int *)&size) < 0) {
        sal_sleep(1);
    }
    check_exit_signals();
    return dst; 
}

/*
 * Function: _bcm_shell
 *
 * Purpose:
 *    Thread entry used to run an instance of the BCM diag shell
 * Parameters:
 *    None
 * Returns:
 *    Nothing
 */
static void
_bcm_shell(void* p)
{
    if (sal_core_init() < 0 || sal_appl_init() < 0) {
	gprintk("SAL Initialization failed\n");
	sal_thread_exit(0); 
    }
    _bcm_shell_thread = sal_thread_self();

    if (boot_flags) {
        sal_boot_flags_set(boot_flags);
    }

    if (debug >= 1) gprintk("BCM Diag Module Initialized. Starting proxy...\n");

    /* A small delay here prevents the telnet proxy 
     * from choking on the first command.
     */
    sal_usleep(100*1000);
    
    _bcm_shell_running = 1; 
    if (debug >= 1) gprintk("Starting Diag Shell...\n"); 
    diag_shell();
    if (debug >= 1) gprintk("Diag Shell is done.\n"); 
    sal_dpc_term();
    linux_bde_destroy(bde);
    _bcm_shell_running = 0; 
}


/*
 * Function: bde_create
 *
 * Purpose:
 *    Create BDE for hardware platform.
 * Parameters:
 *    None
 * Returns:
 *    0 if no errors, otherwise -1.
 */
int
bde_create(void)
{	
    linux_bde_bus_t bus;
    bus.be_pio = SYS_BE_PIO;
    bus.be_packet = SYS_BE_PACKET;
    bus.be_other = SYS_BE_OTHER;
    if (debug >= 2) gprintk("BDE create\n"); 
    return linux_bde_create(&bus, &bde);
}

/*
 * Function: sal_dma_alloc
 *
 * Purpose:
 *    SAL DMA memory allocation function
 * Parameters:
 *    size - number of bytes to allocate
 *    s - string associated with allocation
 * Returns:
 *    Pointer to allocated memory or NULL.
 */
void*
sal_dma_alloc(unsigned int size, char * name)
{
    return bde->salloc(0, size, name); 
}

/*
 * Function: sal_dma_free
 *
 * Purpose:
 *    SAL DMA memory free function
 * Parameters:
 *    ptr - pointer to memory allocated memory with sal_dma_alloc
 * Returns:
 *    Nothing
 */
void	
sal_dma_free(void *ptr)
{
    bde->sfree(0, ptr); 
}

/* 
 * When compiled in debug mode (-DBROADCOM_DEBUG) the sal_alloc_purge function
 * will cleanup all memory allocated by sal_alloc. The optional callback
 * function will allow the caller to print information about memory
 * freed by the sal_alloc_purge function.
 */
extern void
sal_alloc_purge(void (*print_func)(void *ptr, unsigned int size, char *s));

/*
 * Function: _purge_print_func
 *
 * Purpose:
 *    Print information about unfreed memory when module is unloaded.
 * Parameters:
 *    ptr - pointer to memory allocated memory with sal_alloc
 *    size - size of allocated memory
 *    s - user description of memory block
 * Returns:
 *    Nothing
 * Notes:
 *    Since this function may print a lot of information (which will
 *    take a while on a 9600 bps serial link), it will only be active
 *    if the module is loaded with the debug flag set to a non-zero
 *    value.
 */
static void
_purge_print_func(void *ptr, unsigned int size, char *s)
{
    if (debug >= 1) {
        gprintk("Freeing %d bytes @ %08lx (%s)\n", size, (unsigned long)ptr, s);
    }
}

/*
 * Generic module functions
 */

/*
 * Function: _init
 *
 * Purpose:
 *    Module initialization.
 *    Starts the BCM diag thread. 
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_init(void)
{
    if (proxy) {
        linux_uk_proxy_service_create(PROXY_SERVICE, 1, 0); 
    } else {
        gprintk("Proxy service disabled\n");
    }
    sal_thread_create("bcm-shell", 0, 0, _bcm_shell, 0);
    return 0;
}

/*
 * Function: _cleanup
 *
 * Purpose:
 *    Module cleanup function
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 * Notes:
 *    The BCM diag thread will be destroyed to avoid page faults.
 */
static int
_cleanup(void)
{
    /* 
     * Try and close the shell
     */

    if (_bcm_shell_thread) {
        sal_thread_destroy(_bcm_shell_thread);
	sal_usleep(200000);
    }

    if (_bcm_shell_running) {
	sal_usleep(200000);
    }

    linux_uk_proxy_service_destroy(PROXY_SERVICE);

    /* Clean up all unfreed memory (debug mode only) */
    sal_alloc_purge(_purge_print_func);

    return 0;
}	

/* Our module vectors */
static gmodule_t _gmodule = {
    name: MODULE_NAME, 
    major: MODULE_MAJOR, 
    minor: MODULE_MINOR, 
    init: _init,
    cleanup: _cleanup, 
    pprint: _pprint, 
    ioctl: NULL,
    open: NULL, 
    close: NULL, 
}; 

gmodule_t*
gmodule_get(void)
{
    COMPILER_REFERENCE(orphans);
    return &_gmodule;
}

/*
 * These stubs are here for legacy compatability reasons. 
 * They are used only by the diag/test code, not the driver, 
 * so they are really not that important. 
 */

int pci_dma_putw(pci_dev_t *dev, sal_paddr_t addr, unsigned int data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned int *)vaddr) = data;
  return 0;
}

int pci_dma_puth(pci_dev_t *dev, sal_paddr_t addr, unsigned short data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned short *)vaddr) = data;
  return 0;
}

int pci_dma_putb(pci_dev_t *dev, sal_paddr_t addr, unsigned char data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned char *)vaddr) = data;
  return 0;
}

unsigned int pci_dma_getw(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned int data;

  data = *((unsigned int *)vaddr);
  return data;
}

unsigned short pci_dma_geth(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned short data;

  data = *((unsigned short *)vaddr);
  return data;
}

unsigned char pci_dma_getb(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned char data;

  data = *((unsigned char *)vaddr);
  return data;
}

void pci_print_all(void)
{
}

LKM_EXPORT_SYM(check_exit_signals);
LKM_EXPORT_SYM(tty_vprintf);
LKM_EXPORT_SYM(tty_printf);
LKM_EXPORT_SYM(tty_gets);
LKM_EXPORT_SYM(bde_create);
LKM_EXPORT_SYM(sal_dma_alloc);
LKM_EXPORT_SYM(sal_dma_free);
LKM_EXPORT_SYM(sal_alloc_purge);
LKM_EXPORT_SYM(pci_dma_putw);
LKM_EXPORT_SYM(pci_dma_puth);
LKM_EXPORT_SYM(pci_dma_putb);
LKM_EXPORT_SYM(pci_dma_getw);
LKM_EXPORT_SYM(pci_dma_geth);
LKM_EXPORT_SYM(pci_dma_getb);
LKM_EXPORT_SYM(pci_print_all);
LKM_EXPORT_SYM(bde);
