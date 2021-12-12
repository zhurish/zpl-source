#ifndef __V9_BOARD_H__
#define __V9_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define V9_CONFIG_TEST
#define V9_BOARD_TEST


#pragma pack(1)


typedef struct v9_board_s
{
	zpl_uint8		id;				//ID编号
//	zpl_uint8		addr;			//总线地址

	zpl_uint32		ip;				//IP地址

	zpl_uint8		temp;	//温度
	zpl_uint8		vch;			//处理视频路数

	zpl_uint8		synctime;		//时间是否已经同步
	zpl_uint16		cpuload;		//CPU负载（百分比）
	zpl_uint32		memtotal;		//内存(M)
	zpl_uint8		memload;		//内存占用（百分比）
	zpl_uint32		disktatol1;		//硬盘(M)
	zpl_uint8		diskload1;		//硬盘占用（百分比）
	zpl_uint32		disktatol2;		//硬盘(M)
	zpl_uint8		diskload2;		//硬盘占用（百分比）

	//计算板状态，是否连接SDK相关操作依赖于这些状态，这些状态有D单片机通过串口传输过来
	zpl_uint8		online:1;		//在线
	zpl_uint8		power:1;		//上电
	zpl_uint8		active:1;		//激活
	zpl_uint8		autoip:1;		//
	zpl_uint8		startup:1;		//
	zpl_uint8		change:1;		//
	zpl_uint8		use:1;			//使用标志

	zpl_uint8		cnt;			//计数
}v9_board_t;


#pragma pack()

extern int v9_board_init(zpl_uint8 id, v9_board_t *board);
extern v9_board_t * v9_board_lookup(zpl_uint8 id);
extern int v9_board_update_board(zpl_uint8 id, v9_board_t *);

extern int v9_board_show(struct vty * vty, int id, int debug);

#ifdef __cplusplus
}
#endif

#endif /* __V9_BOARD_H__ */
