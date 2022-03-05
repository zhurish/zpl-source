
#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <dosFsLib.h>

#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/vxworks/hal.h>
#include "mbz.h"
#include "flashDrvLib.h"
#include "flashFsLib.h"
#include "srecLoad.h"
#include "config.h"

static int robo_4704_upgrade_image(char *fname, unsigned int flags, 
                 int (*f_loader)(char *fname, char *fbuf, 
                                 int bufSize, int *entry_pt))
{
    char		*buf = 0;
    int			rv = -1;
    int			i = 0;
    int			entry;
#if !defined(NSSIOLE)
    char		tmpc;
#endif /* !NSSIOLE || BCM_ICS */

    if (flashDrvLibInit() == ERROR) {
        printf("flashBoot: Boot flash not found (jumpered right?)\n");
        goto done;
    }

    if ((buf = malloc(FLASH_BOOT_SIZE)) == 0) {
	printf("flashBoot: out of memory\n");
	goto done;
    }

    if (f_loader(fname, buf, FLASH_BOOT_SIZE, &entry) < 0) {
	printf("flashBoot: Failed to read image.\n");
	goto done;
    }

#if !defined(NSSIOLE)
    for(i = 0; i < (512 * 1024); i += 4) {
        tmpc = buf[i];
        buf[i] = buf[i + 3];
        buf[i + 3] = tmpc;

        tmpc = buf[i + 1];
        buf[i + 1] = buf[i + 2];
        buf[i + 2] = tmpc;
    }
#endif

    printf("%d\nErasing boot area ...", i);

    for (i = FLASH_BOOT_START_SECTOR;
         i < FLASH_BOOT_START_SECTOR + FLASH_BOOT_SIZE_SECTORS; i++) {
        printf("FLASH_BOOT_START = 0x%08x\n", FLASH_BOOT_START);
        printf("FLASH_SECTOR_SIZE = %d\n", FLASH_SECTOR_SIZE);
        printf("FLASH_BOOT_START_SECTOR = %d\n", FLASH_BOOT_START_SECTOR);
        printf("FLASH_BOOT_SIZE_SECTORS = %d\n", FLASH_BOOT_SIZE_SECTORS);
    

        if (flashEraseBank(i, 1) != OK) {
	    printf("\nflashBoot: failed erasing -- PROM DESTROYED\n");
            goto done;
        }

        printf(".");
    }

    printf("done\nWriting boot data ...");

    if (flashBlkWrite(FLASH_BOOT_START_SECTOR, buf,
		      0, FLASH_BOOT_SIZE) != OK) {
        printf("\nflashBoot: failed writing -- PROM DESTROYED\n");
        goto done;
    }

    printf("\nDone\n");

    rv = 0;

 done:

    if (buf) {
	free(buf);
    }

    return rv;
}

static int robo_4704_print_info(void)
{
    int		core, sb,pci;

    sys47xxClocks(&core, &sb, &pci);

    if (core) {
        printf("CPU: %d MHz, MEM: 0x%x bytes, ", core/1000000,
            (UINT)(sysMemTop () - (char *)LOCAL_MEM_LOCAL_ADRS));
    } else {
        printf("CPU: Unknown Mhz, MEM: Unknown Mhz, ");
    }

    if (sb) {
        printf("SB: %d MHz, ", sb);
    } else {
        printf("SB: Unknown MHz, ");
    }

    if (pci) {
        printf("PCI: %d MHz\n", pci);
    } else {
        printf("PCI: Unknown MHz\n");
    }
    return 0;
}


static char * robo_4704_flash_dev_name(void)
{
    return FLASH_FS_NAME;
}

static int robo_4704_format_flash(int format, unsigned int flags)
{
    int	rv = 0;

    if (format) {
# ifdef DOS_OPT_DEFAULT
	if (dosFsVolFormat(FLASH_FS_NAME, DOS_OPT_DEFAULT, 0)) {
	    rv |= -1;
	}
# else
	if (diskInit(FLASH_FS_NAME)) {
	    rv |= -1;
	}
# endif
        flashFsSync();
    }
    return(rv);
}

static platform_hal_t robo_4704_hal_info = {
    "robo_4704",                                   /* name */
    0,                                             /* flags */
    (PLATFORM_CAP_DMA_MEM_UNCACHABLE |
     PLATFORM_CAP_PCI | PLATFORM_CAP_FLASH_FS),    /* caps */
    0,                                             /* bus_caps */
    robo_4704_print_info,                          /* f_dump_info */
    robo_4704_upgrade_image,                       /* f_upgrade_image */
    robo_4704_flash_dev_name,                      /* f_flash_device_name */
    robo_4704_format_flash,                        /* f_format_fs */
    flashFsSync,                                   /* f_fs_sync */
    NULL,                                          /* f_fs_check */
    sysTodSet,                                     /* f_tod_set */
    sysTodGet,                                     /* f_tod_get */
    NULL,                                          /* f_start_wdog */
    NULL,                                          /* f_reboot */
    NULL                                           /* f_led_write_string */
};

int platform_attach(platform_hal_t **platform_info)
{
    if (!platform_info) {
        return -1;
    }

    *platform_info = &robo_4704_hal_info;
    return 0;
}
