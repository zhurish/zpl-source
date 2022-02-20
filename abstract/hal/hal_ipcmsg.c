#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"


int hal_ipcmsg_msg_init(struct hal_ipcmsg *ipcmsg, char *buf, int len)
{
    ipcmsg->setp = ipcmsg->getp = 0;
    ipcmsg->length_max = len;
    ipcmsg->buf = buf;
    memset(buf, 0, len);
    return OK;
}

int hal_ipcmsg_putnull(struct hal_ipcmsg *ipcmsg, zpl_uint32 len)
{
    if((ipcmsg->setp + len) > ipcmsg->length_max)
        return ERROR;
    return (ipcmsg->setp += len);
}
int hal_ipcmsg_put(struct hal_ipcmsg *ipcmsg, void *buf, int len)
{
    if((ipcmsg->setp + len) > ipcmsg->length_max)
        return ERROR;
    memcpy(ipcmsg->buf + ipcmsg->setp, buf, len);
    return (ipcmsg->setp += len);
}

int hal_ipcmsg_putc(struct hal_ipcmsg *ipcmsg, zpl_uint8 c)
{
    zpl_uint8 *val = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 1) > ipcmsg->length_max)
        return ERROR;
    *val = c;
    return (ipcmsg->setp += 1);
}
int hal_ipcmsg_putw(struct hal_ipcmsg *ipcmsg, zpl_uint16 s)
{
    zpl_uint16 *val = (zpl_uint16 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 2) > ipcmsg->length_max)
        return ERROR;
    *val = htons(s);
    return (ipcmsg->setp += 2);
}
int hal_ipcmsg_putl(struct hal_ipcmsg *ipcmsg, zpl_uint32 s)
{
    zpl_uint32 *val = (zpl_uint32 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 4) > ipcmsg->length_max)
        return ERROR;
    *val = htonl(s);
    return (ipcmsg->setp += 4);
}
int hal_ipcmsg_put_reset(struct hal_ipcmsg *ipcmsg)
{
    ipcmsg->setp = 0;
    return OK;
}

int hal_ipcmsg_getnull(struct hal_ipcmsg *ipcmsg, zpl_uint32 len)
{
    zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + len) > ipcmsg->length_max || (ipcmsg->getp + len) > ipcmsg->setp)
        return ERROR;
    ipcmsg->getp += len;
    return 1;
}
int hal_ipcmsg_get(struct hal_ipcmsg *ipcmsg, void *buf, int len)
{
    zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + len) > ipcmsg->length_max || (ipcmsg->getp + len) > ipcmsg->setp)
        return ERROR;
    memcpy(buf, pval, len);
    ipcmsg->getp += len;
    return len;
}
int hal_ipcmsg_getc(struct hal_ipcmsg *ipcmsg, zpl_uint8* val)
{
    zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 1) > ipcmsg->length_max || (ipcmsg->getp + 1) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = (*pval);
    ipcmsg->getp += 1;
    return 1;
}
int hal_ipcmsg_getw(struct hal_ipcmsg *ipcmsg,  zpl_uint16 *val)
{
    zpl_uint16 *pval = (zpl_uint16 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 2) > ipcmsg->length_max || (ipcmsg->getp + 2) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = ntohs(*pval);
    ipcmsg->getp += 2;
    return 2;
}
int hal_ipcmsg_getl(struct hal_ipcmsg *ipcmsg,  zpl_uint32 *val)
{
    zpl_uint32 *pval = (zpl_uint32 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 4) > ipcmsg->length_max || (ipcmsg->getp + 4) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = ntohl(*pval);
    ipcmsg->getp += 4;
    return 4;
}
int hal_ipcmsg_get_reset(struct hal_ipcmsg *ipcmsg)
{
    ipcmsg->getp = 0;
    return OK;
}

int hal_ipcmsg_create(struct hal_ipcmsg *ipcmsg)
{
    if (ipcmsg->buf == NULL)
    {
        ipcmsg->buf = XMALLOC(MTYPE_HALIPCMSG, ipcmsg->length_max);
    }
    return OK;
}

int hal_ipcmsg_destroy(struct hal_ipcmsg *ipcmsg)
{
    if (ipcmsg->buf)
        XFREE(MTYPE_HALIPCMSG, ipcmsg->buf);
    return OK;
}

int hal_ipcmsg_reset(struct hal_ipcmsg *ipcmsg)
{
    ipcmsg->getp = 0;
    ipcmsg->setp = 0;
    return OK;
}

int hal_ipcmsg_create_header(struct hal_ipcmsg *ipcmsg, zpl_uint32 command)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(HAL_IPCMSG_HEADER_SIZE);
    hdr->marker = HAL_IPCMSG_HEADER_MARKER;
    hdr->version = HAL_IPCMSG_VERSION;
    hdr->command = htonl(command);
    ipcmsg->setp = sizeof(struct hal_ipcmsg_header);
    return ipcmsg->setp;
}

int hal_ipcmsg_hdr_unit_set(struct hal_ipcmsg *ipcmsg, zpl_uint32 unit)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->command = htonl(unit);
    return OK;
}

int hal_ipcmsg_hdr_unit_get(struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    return ntohl(hdr->unit);
}

int hal_ipcmsg_msglen_set(struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(ipcmsg->setp);
    return OK;
}
int hal_ipcmsg_msglen_get(struct hal_ipcmsg *ipcmsg)
{
    return ipcmsg->setp;
}

int hal_ipcmsg_msg_add(struct hal_ipcmsg *ipcmsg, void *msg, int len)
{
    memcpy(ipcmsg->buf + ipcmsg->setp, msg, len);
    return (ipcmsg->setp + len);
}

int hal_ipcmsg_send_message(int unit, zpl_uint32 command, void *ipcmsg, int len)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_send_message(unit, command, ipcmsg, len, 30);
}

int hal_ipcmsg_port_set(struct hal_ipcmsg *ipcmsg, ifindex_t ifindex)
{
    hal_ipcmsg_putl(ipcmsg, ifindex);
    hal_ipcmsg_putl(ipcmsg, IF_IFINDEX_VRFID_GET(ifindex));
    hal_ipcmsg_putw(ipcmsg, IF_IFINDEX_VLAN_GET(ifindex));
    hal_ipcmsg_putl(ipcmsg, IF_IFINDEX_PHYID_GET(ifindex));
    return OK;
}

int hal_ipcmsg_global_set(ifindex_t ifindex, hal_global_header_t *glo)
{
    //glo->type = IF_IFINDEX_TYPE_GET(ifindex);
    glo->vrfid = htonl(IF_IFINDEX_VRFID_GET(ifindex));
    //port->ifindex = htonl(ifindex);
    //glo->vlanid = htons(IF_IFINDEX_VLAN_GET(ifindex));
    //glo->value = htonl(IF_IFINDEX_PHYID_GET(ifindex));
    return OK;
}

int hal_ipcmsg_table_set(ifindex_t ifindex, hal_table_header_t *table)
{
    //glo->type = IF_IFINDEX_TYPE_GET(ifindex);
    table->vrfid = htonl(IF_IFINDEX_VRFID_GET(ifindex));
    //port->ifindex = htonl(ifindex);
    //port->vlanid = htons(IF_IFINDEX_VLAN_GET(ifindex));
    //port->phyport = htonl(IF_IFINDEX_PHYID_GET(ifindex));
    return OK;
}


int hal_ipcmsg_global_get(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo)
{
    hal_ipcmsg_getl(ipcmsg, &glo->vrfid);
    hal_ipcmsg_getw(ipcmsg, &glo->vlanid);
    hal_ipcmsg_getl(ipcmsg, &glo->value);
    return OK;
}

int hal_ipcmsg_port_get(struct hal_ipcmsg *ipcmsg, hal_port_header_t *bspport)
{
    hal_ipcmsg_getl(ipcmsg, &bspport->ifindex);
    hal_ipcmsg_getl(ipcmsg, &bspport->vrfid);
    hal_ipcmsg_getw(ipcmsg, &bspport->vlanid);
    hal_ipcmsg_getl(ipcmsg, &bspport->phyport);
    return OK;
}

int hal_ipcmsg_table_get(struct hal_ipcmsg *ipcmsg, hal_table_header_t *table)
{
    hal_ipcmsg_getl(ipcmsg, &table->vrfid);
    hal_ipcmsg_getw(ipcmsg, &table->vlanid);
    hal_ipcmsg_getl(ipcmsg, &table->table);
    return OK;
}



int hal_ipcmsg_hexmsg(struct hal_ipcmsg *ipcmsg, zpl_uint32 len, char *hdr)
{
    int ret = 0;
    zpl_char format[4096];
    memset(format, 0, sizeof(format));
    ret = os_loghex(format, sizeof(format), (const zpl_uchar *)ipcmsg->buf,  len);
    zlog_debug(MODULE_HAL, "%s %d [%d] bytes:\r\n  hex:%s", hdr, len, ret, format);
    return OK;
}

















































































