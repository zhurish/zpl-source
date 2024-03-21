#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "log.h"
#include "hal_ipcsrv.h"
#include "hal_ipcmsg.h"
#include "hal_ipccmd.h"



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

int hal_ipcmsg_putc_at(struct hal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint8 c)
{
    zpl_uint8 *val = (zpl_uint8 *)(ipcmsg->buf + s);
    if((ipcmsg->setp + 1) > ipcmsg->length_max)
        return ERROR;
    *val = c;
    return (ipcmsg->setp);
}

int hal_ipcmsg_putw_at(struct hal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint16 w)
{
    zpl_uint16 *val = (zpl_uint16 *)(ipcmsg->buf + s);
    if((ipcmsg->setp + 2) > ipcmsg->length_max)
        return ERROR;
    *val = htons(w);
    return (ipcmsg->setp);
}

int hal_ipcmsg_put_reset(struct hal_ipcmsg *ipcmsg)
{
    ipcmsg->setp = 0;
    return OK;
}

int hal_ipcmsg_getnull(struct hal_ipcmsg *ipcmsg, zpl_uint32 len)
{
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
        ipcmsg->getp = 0;
        ipcmsg->setp = 0;
    }
    if(ipcmsg->buf)
        return OK;
    return ERROR;    
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
    memset(ipcmsg->buf, 0, ipcmsg->length_max);
    return OK;
}

int hal_ipcmsg_create_header(struct hal_ipcmsg *ipcmsg, zpl_uint32 command)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(HAL_IPCMSG_HEADER_SIZE);
    hdr->marker = HAL_IPCMSG_HEADER_MARKER;
    hdr->version = HAL_IPCMSG_VERSION;
    hdr->command = htonl(command);
    hdr->from_unit = 0; 
    ipcmsg->setp = sizeof(struct hal_ipcmsg_header);
    return ipcmsg->setp;
}
int hal_ipcmsg_get_header(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_header *header)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    header->length = ntohs(hdr->length);
    header->marker = hdr->marker ;
    header->version = hdr->version;
    header->command = ntohl(hdr->command);
    header->to_unit = hdr->to_unit ;
    header->from_unit = hdr->from_unit;    
    ipcmsg->getp = sizeof(struct hal_ipcmsg_header);
    return ipcmsg->getp;
}
int hal_ipcmsg_hdr_unit_set(struct hal_ipcmsg *ipcmsg, zpl_uint8 unit)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->to_unit = (unit);
    return OK;
}

int hal_ipcmsg_hdr_unit_get(struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    return (hdr->to_unit);
}

int hal_ipcmsg_hdrlen_set(struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(ipcmsg->setp);
    return OK;
}

int hal_ipcmsg_msglen_get(struct hal_ipcmsg *ipcmsg)
{
    return hal_ipcmsg_get_setp(ipcmsg);
}
int hal_ipcmsg_get_setp(struct hal_ipcmsg *ipcmsg)
{
    return ipcmsg->setp;
}
int hal_ipcmsg_get_getp(struct hal_ipcmsg *ipcmsg)
{
    return ipcmsg->getp;
}

int hal_ipcmsg_msg_copy(struct hal_ipcmsg *dst_ipcmsg, struct hal_ipcmsg *src_ipcmsg)
{
    memcpy(dst_ipcmsg->buf + dst_ipcmsg->setp, src_ipcmsg->buf, src_ipcmsg->setp);
    dst_ipcmsg->setp += src_ipcmsg->setp;
    return OK;
}

int hal_ipcmsg_msg_clone(struct hal_ipcmsg *dst_ipcmsg, struct hal_ipcmsg *src_ipcmsg)
{
    memcpy(dst_ipcmsg->buf, src_ipcmsg->buf, src_ipcmsg->setp);
    dst_ipcmsg->setp = src_ipcmsg->setp;
    dst_ipcmsg->getp = src_ipcmsg->getp;
    return OK;
}

int hal_ipcmsg_send_message(int unit, zpl_uint32 command, void *ipcmsg, int len)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_send_message(unit, command, ipcmsg, len);
}

int hal_ipcmsg_send_andget_message(int unit, zpl_uint32 command, void *ipcmsg, int len,  struct hal_ipcmsg_result *getvalue)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_send_and_get_message(unit, command, ipcmsg, len, getvalue);
}


int hal_ipcmsg_send_cmd(int unit, zpl_uint32 command, struct hal_ipcmsg *src_ipcmsg)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_copy_send_ipcmsg(unit, command, src_ipcmsg);
}

int hal_ipcmsg_send(int unit, struct hal_ipcmsg *src_ipcmsg)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_send_ipcmsg(unit, src_ipcmsg);
}

int hal_ipcmsg_getmsg_callback(int unit, zpl_uint32 command, void *ipcmsg, int len, 
    struct hal_ipcmsg_result *getvalue, struct hal_ipcmsg_callback *callback)
{
    HAL_ENTER_FUNC();
    return hal_ipcsrv_getmsg_callback(unit, command, ipcmsg, len, getvalue, callback);
}

int hal_ipcmsg_result_set(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *val)
{
    hal_ipcmsg_putl(ipcmsg, val->result);
    hal_ipcmsg_putl(ipcmsg, val->vrfid);
    hal_ipcmsg_putl(ipcmsg, val->l3ifid);
    hal_ipcmsg_putl(ipcmsg, val->nhid);
    hal_ipcmsg_putl(ipcmsg, val->routeid);
    hal_ipcmsg_putl(ipcmsg, val->resource);
    hal_ipcmsg_putl(ipcmsg, val->state);
    hal_ipcmsg_putl(ipcmsg, val->value);
    return OK;  
}

int hal_ipcmsg_result_get(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *val)
{
    hal_ipcmsg_getl(ipcmsg, &val->result);
    hal_ipcmsg_getl(ipcmsg, &val->vrfid);
    hal_ipcmsg_getl(ipcmsg, &val->l3ifid);
    hal_ipcmsg_getl(ipcmsg, &val->nhid);
    hal_ipcmsg_getl(ipcmsg, &val->routeid);
    hal_ipcmsg_getl(ipcmsg, &val->resource);
    hal_ipcmsg_getl(ipcmsg, &val->state);
    hal_ipcmsg_getl(ipcmsg, &val->value);
    return OK;  
}


int hal_ipcmsg_port_set(struct hal_ipcmsg *ipcmsg, ifindex_t ifindex)
{ 
    hal_ipcmsg_putc(ipcmsg, IF_IFINDEX_UNIT_GET(ifindex)); 
    hal_ipcmsg_putc(ipcmsg, IF_IFINDEX_SLOT_GET(ifindex));
    hal_ipcmsg_putc(ipcmsg, IF_IFINDEX_TYPE_GET(ifindex));
    hal_ipcmsg_putl(ipcmsg, IF_IFINDEX_ID_GET(ifindex));
    hal_ipcmsg_putl(ipcmsg, IF_IFINDEX_PHYID_GET(ifindex));
    //hal_ipcmsg_putl(ipcmsg, (ifindex));
    hal_ipcmsg_putl(ipcmsg, if_ifindex2l3intfid(ifindex));
    hal_ipcmsg_putl(ipcmsg, IF_IFINDEX_VRFID_GET(ifindex));
    return OK;
}

int hal_ipcmsg_port_get(struct hal_ipcmsg *ipcmsg, hal_port_header_t *bspport)
{
    hal_ipcmsg_getc(ipcmsg, &bspport->unit);
    hal_ipcmsg_getc(ipcmsg, &bspport->slot);
    hal_ipcmsg_getc(ipcmsg, &bspport->type);
    hal_ipcmsg_getl(ipcmsg, &bspport->lport);
    hal_ipcmsg_getl(ipcmsg, &bspport->phyport);
    hal_ipcmsg_getl(ipcmsg, &bspport->l3ifindex);
    hal_ipcmsg_getl(ipcmsg, &bspport->vrfid);
    return OK;
}


int hal_ipcmsg_global_set(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo)
{
    hal_ipcmsg_putl(ipcmsg, glo->vrfid);
    hal_ipcmsg_putl(ipcmsg, glo->value);
    hal_ipcmsg_putl(ipcmsg, glo->value1);
    hal_ipcmsg_putl(ipcmsg, glo->value2);
    return OK;
}

int hal_ipcmsg_global_get(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo)
{
    hal_ipcmsg_getl(ipcmsg, &glo->vrfid);
    hal_ipcmsg_getl(ipcmsg, &glo->value);
    hal_ipcmsg_getl(ipcmsg, &glo->value1);
    hal_ipcmsg_getl(ipcmsg, &glo->value2);
    return OK;
}




int hal_ipcmsg_hexmsg(struct hal_ipcmsg *ipcmsg, zpl_uint32 len, char *hdr)
{
    int ret = 0;
    zpl_char format[4096];
    memset(format, 0, sizeof(format));
    ret = os_loghex(format, sizeof(format), (const zpl_uchar *)ipcmsg->buf,  len);
    zlog_debug(MODULE_HAL, "%s %d bytes:\r\n  hex:%s", hdr, len, format);
    return OK;
}

















































































