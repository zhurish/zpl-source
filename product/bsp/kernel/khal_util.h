#ifndef __KHAL_IPC_UTIL_H__
#define __KHAL_IPC_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif


#define ETH_ALEN	6
#define VLAN_TABLE_MAX	4096
#define PHY_PORT_MAX	8
#define ETH_MAC_CACHE_MAX	4096


typedef struct
{
  zpl_uchar bitmap[VLAN_TABLE_MAX/8+1];
}zpl_vlan_bitmap_t;


typedef enum if_mode_s
{
   IF_MODE_NONE,
   IF_MODE_ACCESS_L2,
   IF_MODE_TRUNK_L2,
   IF_MODE_L3,
   IF_MODE_DOT1Q_TUNNEL, //QINQ
   IF_MODE_BRIGDE,
} if_mode_t;

struct sdk_driver_port 
{
    zpl_uint32    mode;
    vlan_t      pvid;
	zpl_phyport_t		phyport;
	zpl_vlan_bitmap_t	vlanbitmap;
};

#pragma pack(1)
typedef struct khal_mac_cache_s
{
	zpl_uint8 port;
	zpl_uint8 mac[ETH_ALEN];
	vlan_t vid;
	zpl_uint8 use:1;
	zpl_uint8 is_valid:1;
	zpl_uint8 is_age:1;
	zpl_uint8 is_static:1;
	zpl_uint8 res:4;
}khal_mac_cache_t;
#pragma pack(0)



void zpl_vlan_bitmap_init(zpl_vlan_bitmap_t bitmap);
void zpl_vlan_bitmap_set(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
void zpl_vlan_bitmap_clr(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
int zpl_vlan_bitmap_tst(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
void zpl_vlan_bitmap_or(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);
void zpl_vlan_bitmap_xor(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);			
void zpl_vlan_bitmap_and(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);

int zpl_vlan_bitmap_cmp(const zpl_vlan_bitmap_t src1, const zpl_vlan_bitmap_t src2);
void zpl_vlan_bitmap_copy(const zpl_vlan_bitmap_t src, zpl_vlan_bitmap_t dst);

typedef struct
{
  zpl_uchar bitmap[16];   //128 bit
}zpl_bitmap_t;

void os_bitmap_init(zpl_bitmap_t bitmap);
void os_bitmap_set(zpl_bitmap_t bitmap, zpl_uint32  bit);
void os_bitmap_clr(zpl_bitmap_t bitmap, zpl_uint32  bit);
int os_bitmap_tst(zpl_bitmap_t bitmap, zpl_uint32  bit);

void os_bitmap_and(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2);

int os_bitmap_cmp(const zpl_bitmap_t src1, const zpl_bitmap_t src2);
void os_bitmap_copy(const zpl_bitmap_t src, zpl_bitmap_t dst);



#ifdef __cplusplus
}
#endif

#endif /* __KHAL_IPC_UTIL_H__ */