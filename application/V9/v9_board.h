#ifndef __V9_BOARD_H__
#define __V9_BOARD_H__

//#define V9_CONFIG_TEST
#define V9_BOARD_TEST


#pragma pack(1)


typedef struct v9_address_s
{
	u_int8		id;				//ID编号
//	u_int8		addr;			//总线地址

	u_int32		ip;				//IP地址

	u_int8		temp;	//温度
	u_int8		vch;			//处理视频路数

	u_int8		synctime;		//时间是否已经同步
	u_int16		cpuload;		//CPU负载（百分比）
	u_int32		memtotal;		//内存(M)
	u_int8		memload;		//内存占用（百分比）
	u_int32		disktatol1;		//硬盘(M)
	u_int8		diskload1;		//硬盘占用（百分比）
	u_int32		disktatol2;		//硬盘(M)
	u_int8		diskload2;		//硬盘占用（百分比）

	//计算板状态，是否连接SDK相关操作依赖于这些状态，这些状态有D单片机通过串口传输过来
	u_int8		online:1;		//在线
	u_int8		power:1;		//上电
	u_int8		active:1;		//激活
	u_int8		autoip:1;		//
	u_int8		startup:1;		//
	u_int8		change:1;		//
	u_int8		use:1;			//使用标志

	u_int8		cnt;			//计数
}v9_address_t;


#pragma pack()

extern int v9_board_init(u_int8 id, v9_address_t *board);
extern v9_address_t * v9_board_lookup(u_int8 id);
extern int v9_board_update_board(u_int8 id, v9_address_t *);
extern BOOL v9_board_ready(u_int8 id);
extern int v9_board_set_ready(u_int8 id);
extern int v9_board_show(struct vty * vty, int id, int debug);

#endif /* __V9_BOARD_H__ */
