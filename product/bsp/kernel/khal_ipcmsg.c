
#include "kbsp_types.h"
#include "khal_ipcmsg.h"
#include "khal_ipccmd.h"



int khal_ipcmsg_msg_init(struct khal_ipcmsg *ipcmsg, char *buf, int len)
{
    ipcmsg->setp = ipcmsg->getp = 0;
    ipcmsg->length_max = len;
    ipcmsg->buf = buf;
    memset(buf, 0, len);
    return OK;
}

int khal_ipcmsg_putnull(struct khal_ipcmsg *ipcmsg, zpl_uint32 len)
{
    if((ipcmsg->setp + len) > ipcmsg->length_max)
        return ERROR;
    return (ipcmsg->setp += len);
}
int khal_ipcmsg_put(struct khal_ipcmsg *ipcmsg, void *buf, int len)
{
    if((ipcmsg->setp + len) > ipcmsg->length_max)
        return ERROR;
    memcpy(ipcmsg->buf + ipcmsg->setp, buf, len);
    return (ipcmsg->setp += len);
}

int khal_ipcmsg_putc(struct khal_ipcmsg *ipcmsg, zpl_uint8 c)
{
    zpl_uint8 *val = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 1) > ipcmsg->length_max)
        return ERROR;
    *val = c;
    return (ipcmsg->setp += 1);
}
int khal_ipcmsg_putw(struct khal_ipcmsg *ipcmsg, zpl_uint16 s)
{
    zpl_uint16 *val = (zpl_uint16 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 2) > ipcmsg->length_max)
        return ERROR;
    *val = htons(s);
    return (ipcmsg->setp += 2);
}
int khal_ipcmsg_putl(struct khal_ipcmsg *ipcmsg, zpl_uint32 s)
{
    zpl_uint32 *val = (zpl_uint32 *)(ipcmsg->buf + ipcmsg->setp);
    if((ipcmsg->setp + 4) > ipcmsg->length_max)
        return ERROR;
    *val = htonl(s);
    return (ipcmsg->setp += 4);
}

int khal_ipcmsg_putc_at(struct khal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint8 c)
{
    zpl_uint8 *val = (zpl_uint8 *)(ipcmsg->buf + s);
    if((ipcmsg->setp + 1) > ipcmsg->length_max)
        return ERROR;
    *val = c;
    return (ipcmsg->setp);
}

int khal_ipcmsg_putw_at(struct khal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint16 w)
{
    zpl_uint16 *val = (zpl_uint16 *)(ipcmsg->buf + s);
    if((ipcmsg->setp + 2) > ipcmsg->length_max)
        return ERROR;
    *val = htons(w);
    return (ipcmsg->setp);
}

int khal_ipcmsg_put_reset(struct khal_ipcmsg *ipcmsg)
{
    ipcmsg->setp = 0;
    return OK;
}

int khal_ipcmsg_getnull(struct khal_ipcmsg *ipcmsg, zpl_uint32 len)
{
    //zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + len) > ipcmsg->length_max || (ipcmsg->getp + len) > ipcmsg->setp)
        return ERROR;
    ipcmsg->getp += len;
    return 1;
}
int khal_ipcmsg_get(struct khal_ipcmsg *ipcmsg, void *buf, int len)
{
    zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + len) > ipcmsg->length_max || (ipcmsg->getp + len) > ipcmsg->setp)
        return ERROR;
    memcpy(buf, pval, len);
    ipcmsg->getp += len;
    return len;
}
int khal_ipcmsg_getc(struct khal_ipcmsg *ipcmsg, zpl_uint8* val)
{
    zpl_uint8 *pval = (zpl_uint8 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 1) > ipcmsg->length_max || (ipcmsg->getp + 1) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = (*pval);
    ipcmsg->getp += 1;
    return 1;
}
int khal_ipcmsg_getw(struct khal_ipcmsg *ipcmsg,  zpl_uint16 *val)
{
    zpl_uint16 *pval = (zpl_uint16 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 2) > ipcmsg->length_max || (ipcmsg->getp + 2) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = ntohs(*pval);
    ipcmsg->getp += 2;
    return 2;
}
int khal_ipcmsg_getl(struct khal_ipcmsg *ipcmsg,  zpl_uint32 *val)
{
    zpl_uint32 *pval = (zpl_uint32 *)(ipcmsg->buf + ipcmsg->getp);
    if((ipcmsg->getp + 4) > ipcmsg->length_max || (ipcmsg->getp + 4) > ipcmsg->setp)
        return ERROR;
    if(val)    
        *val = ntohl(*pval);
    ipcmsg->getp += 4;
    return 4;
}
int khal_ipcmsg_get_reset(struct khal_ipcmsg *ipcmsg)
{
    ipcmsg->getp = 0;
    return OK;
}

int khal_ipcmsg_create(struct khal_ipcmsg *ipcmsg)
{
    if (ipcmsg->buf == NULL)
    {
        ipcmsg->buf =  XMALLOC(MTYPE_HALIPCMSG, ipcmsg->length_max);
        ipcmsg->getp = 0;
        ipcmsg->setp = 0;
    }
    if(ipcmsg->buf)
        return OK;
    return ERROR;    
}

int khal_ipcmsg_destroy(struct khal_ipcmsg *ipcmsg)
{
    if (ipcmsg->buf)
        XFREE(MTYPE_HALIPCMSG, ipcmsg->buf);
    return OK;
}

int khal_ipcmsg_reset(struct khal_ipcmsg *ipcmsg)
{
    ipcmsg->getp = 0;
    ipcmsg->setp = 0;
    memset(ipcmsg->buf, 0, ipcmsg->length_max);
    return OK;
}

int khal_ipcmsg_create_header(struct khal_ipcmsg *ipcmsg, zpl_uint32 command)
{
    struct khal_ipcmsg_header *hdr = (struct khal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(HAL_IPCMSG_HEADER_SIZE);
    hdr->marker = HAL_IPCMSG_HEADER_MARKER;
    hdr->version = HAL_IPCMSG_VERSION;
    hdr->command = htonl(command);
    ipcmsg->setp = sizeof(struct khal_ipcmsg_header);
    return ipcmsg->setp;
}

int khal_ipcmsg_get_header(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_header *header)
{
    struct khal_ipcmsg_header *hdr = (struct khal_ipcmsg_header *)ipcmsg->buf;
    header->length = ntohs(hdr->length);
    header->marker = hdr->marker ;
    header->version = hdr->version;
    header->command = ntohl(hdr->command);
    ipcmsg->getp = sizeof(struct khal_ipcmsg_header);
    return ipcmsg->getp;
}

int khal_ipcmsg_hdr_unit_set(struct khal_ipcmsg *ipcmsg, zpl_uint32 unit)
{
    struct khal_ipcmsg_header *hdr = (struct khal_ipcmsg_header *)ipcmsg->buf;
    hdr->unit = htonl(unit);
    return OK;
}

int khal_ipcmsg_hdr_unit_get(struct khal_ipcmsg *ipcmsg)
{
    struct khal_ipcmsg_header *hdr = (struct khal_ipcmsg_header *)ipcmsg->buf;
    return ntohl(hdr->unit);
}

int khal_ipcmsg_hdrlen_set(struct khal_ipcmsg *ipcmsg)
{
    struct khal_ipcmsg_header *hdr = (struct khal_ipcmsg_header *)ipcmsg->buf;
    hdr->length = htons(ipcmsg->setp);
    return OK;
}

int khal_ipcmsg_msglen_get(struct khal_ipcmsg *ipcmsg)
{
    return khal_ipcmsg_get_setp(ipcmsg);
}
int khal_ipcmsg_get_setp(struct khal_ipcmsg *ipcmsg)
{
    return ipcmsg->setp;
}
int khal_ipcmsg_get_getp(struct khal_ipcmsg *ipcmsg)
{
    return ipcmsg->getp;
}

int khal_ipcmsg_msg_copy(struct khal_ipcmsg *dst_ipcmsg, struct khal_ipcmsg *src_ipcmsg)
{
    memcpy(dst_ipcmsg->buf + dst_ipcmsg->setp, src_ipcmsg->buf, src_ipcmsg->setp);
    dst_ipcmsg->setp += src_ipcmsg->setp;
    return OK;
}

int khal_ipcmsg_msg_clone(struct khal_ipcmsg *dst_ipcmsg, struct khal_ipcmsg *src_ipcmsg)
{
    memcpy(dst_ipcmsg->buf, src_ipcmsg->buf, src_ipcmsg->setp);
    dst_ipcmsg->setp = src_ipcmsg->setp;
    return OK;
}

int khal_ipcmsg_send_message(int unit, zpl_uint32 command, void *ipcmsg, int len)
{
    HAL_ENTER_FUNC();
    return 0;//hal_ipcsrv_send_message(unit, command, ipcmsg, len);
}

int khal_ipcmsg_send_andget_message(int unit, zpl_uint32 command, void *ipcmsg, int len,  struct khal_ipcmsg_result *getvalue)
{
    HAL_ENTER_FUNC();
    return 0;//hal_ipcsrv_send_and_get_message(unit, command, ipcmsg, len, getvalue);
}


int khal_ipcmsg_send_cmd(int unit, zpl_uint32 command, struct khal_ipcmsg *src_ipcmsg)
{
    HAL_ENTER_FUNC();
    return 0;//hal_ipcsrv_copy_send_ipcmsg(unit, command, src_ipcmsg);
}

int khal_ipcmsg_send(int unit, struct khal_ipcmsg *src_ipcmsg)
{
    HAL_ENTER_FUNC();
    return 0;//hal_ipcsrv_send_ipcmsg(unit, src_ipcmsg);
}

int khal_ipcmsg_getmsg_callback(int unit, zpl_uint32 command, void *ipcmsg, int len, 
    struct khal_ipcmsg_result *getvalue, struct khal_ipcmsg_callback *callback)
{
    HAL_ENTER_FUNC();
    return 0;//hal_ipcsrv_getmsg_callback(unit, command, ipcmsg, len, getvalue, callback);
}

int khal_ipcmsg_result_set(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_result *val)
{
    khal_ipcmsg_putl(ipcmsg, val->result);
    khal_ipcmsg_putl(ipcmsg, val->vrfid);
    khal_ipcmsg_putl(ipcmsg, val->l3ifid);
    khal_ipcmsg_putl(ipcmsg, val->nhid);
    khal_ipcmsg_putl(ipcmsg, val->routeid);
    khal_ipcmsg_putl(ipcmsg, val->resource);
    khal_ipcmsg_putl(ipcmsg, val->state);
    khal_ipcmsg_putl(ipcmsg, val->value);
    return OK;  
}

int khal_ipcmsg_result_get(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_result *val)
{
    khal_ipcmsg_getl(ipcmsg, &val->result);
    khal_ipcmsg_getl(ipcmsg, &val->vrfid);
    khal_ipcmsg_getl(ipcmsg, &val->l3ifid);
    khal_ipcmsg_getl(ipcmsg, &val->nhid);
    khal_ipcmsg_getl(ipcmsg, &val->routeid);
    khal_ipcmsg_getl(ipcmsg, &val->resource);
    khal_ipcmsg_getl(ipcmsg, &val->state);
    khal_ipcmsg_getl(ipcmsg, &val->value);
    return OK;  
}


int khal_ipcmsg_port_set(struct khal_ipcmsg *ipcmsg, ifindex_t ifindex)
{ 
    /*
    khal_ipcmsg_putc(ipcmsg, IF_IFINDEX_UNIT_GET(ifindex)); 
    khal_ipcmsg_putc(ipcmsg, IF_IFINDEX_SLOT_GET(ifindex));
    khal_ipcmsg_putc(ipcmsg, IF_IFINDEX_TYPE_GET(ifindex));
    khal_ipcmsg_putl(ipcmsg, IF_IFINDEX_ID_GET(ifindex));
    khal_ipcmsg_putl(ipcmsg, IF_IFINDEX_PHYID_GET(ifindex));
    //khal_ipcmsg_putl(ipcmsg, (ifindex));
    khal_ipcmsg_putl(ipcmsg, if_ifindex2l3intfid(ifindex));
    khal_ipcmsg_putl(ipcmsg, IF_IFINDEX_VRFID_GET(ifindex));
    */
    return OK;
}

int khal_ipcmsg_port_get(struct khal_ipcmsg *ipcmsg, khal_port_header_t *bspport)
{
    khal_ipcmsg_getc(ipcmsg, &bspport->unit);
    khal_ipcmsg_getc(ipcmsg, &bspport->slot);
    khal_ipcmsg_getc(ipcmsg, &bspport->type);
    khal_ipcmsg_getl(ipcmsg, &bspport->lgport);
    khal_ipcmsg_getl(ipcmsg, &bspport->phyport);
    khal_ipcmsg_getl(ipcmsg, &bspport->l3ifindex);
    khal_ipcmsg_getl(ipcmsg, &bspport->vrfid);
    return OK;
}


int khal_ipcmsg_global_set(struct khal_ipcmsg *ipcmsg, khal_global_header_t *glo)
{
    khal_ipcmsg_putl(ipcmsg, glo->vrfid);
    khal_ipcmsg_putl(ipcmsg, glo->value);
    khal_ipcmsg_putl(ipcmsg, glo->value1);
    khal_ipcmsg_putl(ipcmsg, glo->value2);
    return OK;
}

int khal_ipcmsg_global_get(struct khal_ipcmsg *ipcmsg, khal_global_header_t *glo)
{
    khal_ipcmsg_getl(ipcmsg, &glo->vrfid);
    khal_ipcmsg_getl(ipcmsg, &glo->value);
    khal_ipcmsg_getl(ipcmsg, &glo->value1);
    khal_ipcmsg_getl(ipcmsg, &glo->value2);
    return OK;
}




int khal_ipcmsg_hexmsg(struct khal_ipcmsg *ipcmsg, zpl_uint32 len, char *hdr)
{
    return OK;
}

















































































