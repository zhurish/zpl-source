#ifndef __ZPL_VIDHAL_SVP_H__
#define __ZPL_VIDHAL_SVP_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#ifdef ZPL_HISIMPP_MODULE

typedef enum hiZPL_VIDHAL_SVP_ERR_LEVEL_E
{
	ZPL_VIDHAL_SVP_ERR_LEVEL_DEBUG	 = 0x0,    /* debug-level								   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_INFO	 = 0x1,    /* informational 							   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_NOTICE  = 0x2,    /* normal but significant condition			   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_WARNING = 0x3,    /* warning conditions						   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR	 = 0x4,    /* error conditions							   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_CRIT	 = 0x5,    /* critical conditions						   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_ALERT	 = 0x6,    /* action must be taken immediately			   */
	ZPL_VIDHAL_SVP_ERR_LEVEL_FATAL	= 0x7,	   /* just for compatibility with previous version */

	ZPL_VIDHAL_SVP_ERR_LEVEL_BUTT
}ZPL_VIDHAL_SVP_ERR_LEVEL_E;


#define ZPL_VIDHAL_SVP_PRINTF(LevelStr,Msg, ...) do { fprintf(stderr,"[Level]:%s,[Func]:%s [Line]:%d [Info]:"Msg,LevelStr, __FUNCTION__, __LINE__,## __VA_ARGS__); } while (0)
#define ZPL_VIDHAL_SVP_PRINTF_RED(LevelStr,Msg, ...) do { fprintf(stderr,"\033[0;31m [Level]:%s,[Func]:%s [Line]:%d [Info]:"Msg"\033[0;39m\n",LevelStr, __FUNCTION__, __LINE__,## __VA_ARGS__); } while (0)
/* system is unusable	*/
#define ZPL_VIDHAL_SVP_TRACE_FATAL(Msg,...)   ZPL_VIDHAL_SVP_PRINTF_RED("Fatal",Msg,##__VA_ARGS__)
/* action must be taken immediately */
#define ZPL_VIDHAL_SVP_TRACE_ALERT(Msg,...)   ZPL_VIDHAL_SVP_PRINTF_RED("Alert",Msg,##__VA_ARGS__)
/* critical conditions */
#define ZPL_VIDHAL_SVP_TRACE_CRIT(Msg,...)    ZPL_VIDHAL_SVP_PRINTF_RED("Critical",Msg,##__VA_ARGS__)
/* error conditions */
#define ZPL_VIDHAL_SVP_TRACE_ERR(Msg,...)     ZPL_VIDHAL_SVP_PRINTF_RED("Error",Msg,##__VA_ARGS__)
/* warning conditions */
#define ZPL_VIDHAL_SVP_TRACE_WARN(Msg,...)    ZPL_VIDHAL_SVP_PRINTF("Warning",Msg,##__VA_ARGS__)
/* normal but significant condition  */
#define ZPL_VIDHAL_SVP_TRACE_NOTICE(Msg,...)  ZPL_VIDHAL_SVP_PRINTF("Notice",Msg,##__VA_ARGS__)
/* informational */
#define ZPL_VIDHAL_SVP_TRACE_INFO(Msg,...)    ZPL_VIDHAL_SVP_PRINTF("Info",Msg,##__VA_ARGS__)
/* debug-level messages  */
#define ZPL_VIDHAL_SVP_TRACE_DEBUG(Msg, ...)  ZPL_VIDHAL_SVP_PRINTF("Debug",Msg,##__VA_ARGS__)

#define ZPL_VIDHAL_SVP_TRACE(Level,Msg, ...)\
do\
{\
	switch(Level){\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_DEBUG:\
		    ZPL_VIDHAL_SVP_TRACE_DEBUG(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_INFO:\
		    ZPL_VIDHAL_SVP_TRACE_INFO(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_NOTICE:\
		    ZPL_VIDHAL_SVP_TRACE_NOTICE(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_WARNING:\
		    ZPL_VIDHAL_SVP_TRACE_WARN(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR:\
			ZPL_VIDHAL_SVP_TRACE_ERR(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_CRIT:\
			ZPL_VIDHAL_SVP_TRACE_CRIT(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_ALERT:\
			ZPL_VIDHAL_SVP_TRACE_ALERT(Msg,##__VA_ARGS__);\
			break;\
		case ZPL_VIDHAL_SVP_ERR_LEVEL_FATAL:\
			ZPL_VIDHAL_SVP_TRACE_FATAL(Msg,##__VA_ARGS__);\
			break;\
		default:\
			break;\
		}\
}while(0)
/****
*Expr is true,goto
*/
#define ZPL_VIDHAL_SVP_CHECK_EXPR_GOTO(Expr, Label,Level,Msg, ...)                    \
do{																				  \
	if(Expr)                                                                      \
	{                                                                             \
		ZPL_VIDHAL_SVP_TRACE(Level,Msg,## __VA_ARGS__);                               \
		goto Label;                                                               \
	}                                                                             \
}while(0)
/****
*Expr is true,return void
*/
#define ZPL_VIDHAL_SVP_CHECK_EXPR_RET_VOID(Expr,Level,Msg, ...)					     \
do{                                                                              \
	if(Expr)                                                                     \
	{                                                                            \
		ZPL_VIDHAL_SVP_TRACE(Level,Msg, ##__VA_ARGS__);                              \
		return;                                                                  \
	}                                                                            \
}while(0)
/****
*Expr is true,return Ret
*/
#define ZPL_VIDHAL_SVP_CHECK_EXPR_RET(Expr,Ret,Level,Msg, ...)					     \
do{                                                                              \
	if(Expr)                                                                     \
	{                                                                            \
		ZPL_VIDHAL_SVP_TRACE(Level,Msg, ##__VA_ARGS__);                              \
		return Ret;                                                              \
	}                                                                            \
}while(0)
/****
*Expr is true,trace
*/
#define ZPL_VIDHAL_SVP_CHECK_EXPR_TRACE(Expr,Level,Msg, ...)						 \
do{                                                                              \
	if(Expr)                                                                     \
	{                                                                            \
	   ZPL_VIDHAL_SVP_TRACE(Level,Msg, ##__VA_ARGS__);                               \
	}                                                                            \
}while(0)

#define ZPL_VIDHAL_SVP_ALIGN_16		   16
#define ZPL_VIDHAL_SVP_ALIGN_32		   32
#define ZPL_VIDHAL_SVP_D1_PAL_HEIGHT   576
#define ZPL_VIDHAL_SVP_D1_PAL_WIDTH    704

//free mmz
#define ZPL_VIDHAL_SVP_MMZ_FREE(phy,vir)\
do{\
	if ((0 != (phy)) && (0 != (vir)))\
	{\
		HI_MPI_SYS_MmzFree((phy),(void*)(HI_UL)(vir));\
		(phy) = 0;\
		(vir) = 0;\
	}\
}while(0)


#define ZPL_VIDHAL_SVP_CLOSE_FILE(fp)\
do{\
	if (NULL != (fp))\
	{\
		fclose((fp));\
		(fp) = NULL;\
	}\
}while(0)


/*
*System init
*/
//void zpl_vidhal_svp_CheckSysInit(void);
/*
*System exit
*/
//void zpl_vidhal_svp_CheckSysExit(void);

/*
*Align
*/
zpl_uint32 zpl_vidhal_svp_align(zpl_uint32 u32Size, zpl_uint16 u16Align);

/*
*Create Image memory
*/
int zpl_vidhal_svp_CreateImage(SVP_IMAGE_S *pstImg,SVP_IMAGE_TYPE_E enType,zpl_uint32 u32Width,
                    zpl_uint32 u32Height,zpl_uint32 u32AddrOffset);
/*
*Destory image memory
*/
zpl_void zpl_vidhal_svp_DestroyImage(SVP_IMAGE_S *pstImg,zpl_uint32 u32AddrOffset);

/*
*Create mem info
*/
int zpl_vidhal_svp_CreateMemInfo(SVP_MEM_INFO_S *pstMemInfo,zpl_uint32 u32Size,zpl_uint32 u32AddrOffset);

/*
Destory mem info
*/
zpl_void zpl_vidhal_svp_DestroyMemInfo(SVP_MEM_INFO_S *pstMemInfo,zpl_uint32 u32AddrOffset);
/*
*Malloc memory
*/
int zpl_vidhal_svp_MallocMem(zpl_char *pszMmb, zpl_char *pszZone, zpl_uint64 *pu64PhyAddr, zpl_void **ppvVirAddr, zpl_uint32 u32Size);
/*
*Malloc memory with cached
*/
int zpl_vidhal_svp_MallocCached(zpl_char *pszMmb, zpl_char *pszZone,zpl_uint64 *pu64PhyAddr, zpl_void **ppvVirAddr, zpl_uint32 u32Size);
/*
*Fulsh cached
*/
int zpl_vidhal_svp_FlushCache(zpl_uint64 u64PhyAddr, zpl_void *pvVirAddr, zpl_uint32 u32Size);
/*
*Gen rand data
*/
int zpl_vidhal_svp_GenRandS32(zpl_int32 s32Max,zpl_int32 s32Min);
/*
*Gen image
*/
zpl_void zpl_vidhal_svp_GenImage(zpl_uint64 au64Buff[3],zpl_uint32 au32Stride[3],SVP_IMAGE_TYPE_E enType,zpl_uint32 u32Width,zpl_uint32 u32Height);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ZPL_VIDHAL_SVP_H__ */
