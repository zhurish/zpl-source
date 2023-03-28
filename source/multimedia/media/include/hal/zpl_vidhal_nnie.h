#ifndef __ZPL_VIDHAL_NNIE_H__
#define __ZPL_VIDHAL_NNIE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#include "zpl_vidhal_ive.h"
#include <sys/time.h>

/*16Byte align*/
#define ZPL_VIDHAL_SVP_NNIE_ALIGN_16 16
#define ZPL_VIDHAL_SVP_NNIE_ALIGN16(u32Num) ((u32Num + ZPL_VIDHAL_SVP_NNIE_ALIGN_16-1) / ZPL_VIDHAL_SVP_NNIE_ALIGN_16*ZPL_VIDHAL_SVP_NNIE_ALIGN_16)
/*32Byte align*/
#define ZPL_VIDHAL_SVP_NNIE_ALIGN_32 32
#define ZPL_VIDHAL_SVP_NNIE_ALIGN32(u32Num) ((u32Num + ZPL_VIDHAL_SVP_NNIE_ALIGN_32-1) / ZPL_VIDHAL_SVP_NNIE_ALIGN_32*ZPL_VIDHAL_SVP_NNIE_ALIGN_32)

#define ZPL_VIDHAL_SVP_NNIE_CONVERT_64BIT_ADDR(Type,Addr) (Type*)(HI_UL)(Addr)
#define ZPL_VIDHAL_SVP_COORDI_NUM                     4        /*num of coordinates*/
#define ZPL_VIDHAL_SVP_PROPOSAL_WIDTH                 6        /*the width of each proposal array*/
#define ZPL_VIDHAL_SVP_QUANT_BASE                     4096     /*the basic quantity*/
#define ZPL_VIDHAL_SVP_NNIE_MAX_SOFTWARE_MEM_NUM      4
#define ZPL_VIDHAL_SVP_NNIE_SSD_REPORT_NODE_NUM       12
#define ZPL_VIDHAL_SVP_NNIE_SSD_PRIORBOX_NUM          6
#define ZPL_VIDHAL_SVP_NNIE_SSD_SOFTMAX_NUM           6
#define ZPL_VIDHAL_SVP_NNIE_SSD_ASPECT_RATIO_NUM      6
#define ZPL_VIDHAL_SVP_NNIE_YOLOV1_WIDTH_GRID_NUM     7
#define ZPL_VIDHAL_SVP_NNIE_YOLOV1_HEIGHT_GRID_NUM    7
#define ZPL_VIDHAL_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM    2
#define ZPL_VIDHAL_SVP_NNIE_MAX_CLASS_NUM             30
#define ZPL_VIDHAL_SVP_NNIE_MAX_ROI_NUM_OF_CLASS      50
#define ZPL_VIDHAL_SVP_NNIE_REPORT_NAME_LENGTH        64

typedef struct hiZPL_VIDHAL_SVP_NNIE_MODEL_S
{
    SVP_NNIE_MODEL_S    stModel;
    SVP_MEM_INFO_S      stModelBuf;//store Model file
}ZPL_VIDHAL_SVP_NNIE_MODEL_S;


/*each seg input and output memory*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_SEG_DATA_S
{
	SVP_SRC_BLOB_S astSrc[SVP_NNIE_MAX_INPUT_NUM];
	SVP_DST_BLOB_S astDst[SVP_NNIE_MAX_OUTPUT_NUM];
}ZPL_VIDHAL_SVP_NNIE_SEG_DATA_S;

/*each seg input and output data memory size*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_BLOB_SIZE_S
{
	zpl_uint32 au32SrcSize[SVP_NNIE_MAX_INPUT_NUM];
	zpl_uint32 au32DstSize[SVP_NNIE_MAX_OUTPUT_NUM];
}ZPL_VIDHAL_SVP_NNIE_BLOB_SIZE_S;

/*NNIE Execution parameters */
typedef struct hiZPL_VIDHAL_SVP_NNIE_PARAM_S
{
    SVP_NNIE_MODEL_S*    pstModel;
    zpl_uint32 u32TmpBufSize;
    zpl_uint32 au32TaskBufSize[SVP_NNIE_MAX_NET_SEG_NUM];
    SVP_MEM_INFO_S      stTaskBuf;
	SVP_MEM_INFO_S      stTmpBuf;
    SVP_MEM_INFO_S      stStepBuf;//store Lstm step info
    ZPL_VIDHAL_SVP_NNIE_SEG_DATA_S astSegData[SVP_NNIE_MAX_NET_SEG_NUM];//each seg's input and output blob
    SVP_NNIE_FORWARD_CTRL_S astForwardCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
	SVP_NNIE_FORWARD_WITHBBOX_CTRL_S astForwardWithBboxCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
}ZPL_VIDHAL_SVP_NNIE_PARAM_S;

/*NNIE input or output data index*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_DATA_INDEX_S
{
	zpl_uint32 u32SegIdx;
	zpl_uint32 u32NodeIdx;
}ZPL_VIDHAL_SVP_NNIE_DATA_INDEX_S;

/*this struct is used to indicate the input data from which seg's input or report node*/
typedef ZPL_VIDHAL_SVP_NNIE_DATA_INDEX_S  ZPL_VIDHAL_SVP_NNIE_INPUT_DATA_INDEX_S;
/*this struct is used to indicate which seg will be executed*/
typedef ZPL_VIDHAL_SVP_NNIE_DATA_INDEX_S  ZPL_VIDHAL_SVP_NNIE_PROCESS_SEG_INDEX_S;

typedef enum hiZPL_VIDHAL_SVP_NNIE_NET_TYPE_E
{
	ZPL_VIDHAL_SVP_NNIE_ALEXNET_FASTER_RCNN       =  0x0,  /*FasterRcnn Alexnet*/
	ZPL_VIDHAL_SVP_NNIE_VGG16_FASTER_RCNN         =  0x1,  /*FasterRcnn Vgg16*/
	ZPL_VIDHAL_SVP_NNIE_PVANET_FASTER_RCNN        =  0x2, /*pavenet fasterRcnn*/

	ZPL_VIDHAL_SVP_NNIE_NET_TYPE_BUTT
}ZPL_VIDHAL_SVP_NNIE_NET_TYPE_E;


/*NNIE configuration parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_CFG_S
{
    zpl_char *pszPic;
    zpl_uint32 u32MaxInputNum;
    zpl_uint32 u32MaxRoiNum;
    zpl_uint64 au64StepVirAddr[ZPL_VIDHAL_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM*SVP_NNIE_MAX_NET_SEG_NUM];//virtual addr of LSTM's or RNN's step buffer
	SVP_NNIE_ID_E	aenNnieCoreId[SVP_NNIE_MAX_NET_SEG_NUM];
}ZPL_VIDHAL_SVP_NNIE_CFG_S;


/*CNN GetTopN parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_CNN_SOFTWARE_PARAM_S
{
    zpl_uint32 u32TopN;
	SVP_DST_BLOB_S stGetTopN;
	SVP_MEM_INFO_S stAssistBuf;
}ZPL_VIDHAL_SVP_NNIE_CNN_SOFTWARE_PARAM_S;

/*FasterRcnn software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S
{
	zpl_uint32 au32Scales[9];
	zpl_uint32 au32Ratios[9];
	zpl_uint32 au32ConvHeight[2];
	zpl_uint32 au32ConvWidth[2];
	zpl_uint32 au32ConvChannel[2];
	zpl_uint32 u32ConvStride;
	zpl_uint32 u32NumRatioAnchors;
	zpl_uint32 u32NumScaleAnchors;
	zpl_uint32 u32OriImHeight;
	zpl_uint32 u32OriImWidth;
	zpl_uint32 u32MinSize;
	zpl_uint32 u32SpatialScale;
	zpl_uint32 u32NmsThresh;
    zpl_uint32 u32FilterThresh;
    zpl_uint32 u32NumBeforeNms;
	zpl_uint32 u32MaxRoiNum;
	zpl_uint32 u32ClassNum;
	zpl_uint32 au32ConfThresh[21];
	zpl_uint32 u32ValidNmsThresh;
	zpl_int32* aps32Conv[2];
	SVP_MEM_INFO_S stRpnTmpBuf;
	SVP_DST_BLOB_S stRpnBbox;
	SVP_DST_BLOB_S stClassRoiNum;
	SVP_DST_BLOB_S stDstRoi;
	SVP_DST_BLOB_S stDstScore;
	SVP_MEM_INFO_S stGetResultTmpBuf;
	zpl_char* apcRpnDataLayerName[2];
}ZPL_VIDHAL_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S;
/*Array rect info*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_RECT_ARRAY_S
 {
     zpl_uint32 u32ClsNum;
     zpl_uint32 u32TotalNum;
     zpl_uint32 au32RoiNum[ZPL_VIDHAL_SVP_NNIE_MAX_CLASS_NUM];
     ZPL_VIDHAL_IVE_RECT_S astRect[ZPL_VIDHAL_SVP_NNIE_MAX_CLASS_NUM][ZPL_VIDHAL_SVP_NNIE_MAX_ROI_NUM_OF_CLASS];
 } ZPL_VIDHAL_SVP_NNIE_RECT_ARRAY_S;

/*RFCN software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_RFCN_SOFTWARE_PARAM_S
{
	zpl_uint32 au32Scales[9];
	zpl_uint32 au32Ratios[9];
	zpl_uint32 au32ConvHeight[2];
	zpl_uint32 au32ConvWidth[2];
	zpl_uint32 au32ConvChannel[2];
	zpl_uint32 u32ConvStride;
	zpl_uint32 u32NumRatioAnchors;
	zpl_uint32 u32NumScaleAnchors;
	zpl_uint32 u32OriImHeight;
	zpl_uint32 u32OriImWidth;
	zpl_uint32 u32MinSize;
	zpl_uint32 u32SpatialScale;
	zpl_uint32 u32NmsThresh;
    zpl_uint32 u32FilterThresh;
    zpl_uint32 u32NumBeforeNms;
	zpl_uint32 u32MaxRoiNum;
	zpl_uint32 u32ClassNum;
	zpl_uint32 au32ConfThresh[21];
	zpl_uint32 u32ValidNmsThresh;
	zpl_int32* aps32Conv[2];
    HI_FLOAT af32ScoreThr[ZPL_VIDHAL_SVP_NNIE_MAX_CLASS_NUM];
	SVP_MEM_INFO_S stRpnTmpBuf;
	SVP_DST_BLOB_S stRpnBbox;
	SVP_DST_BLOB_S stClassRoiNum;
	SVP_DST_BLOB_S stDstRoi;
	SVP_DST_BLOB_S stDstScore;
	SVP_MEM_INFO_S stGetResultTmpBuf;
    ZPL_VIDHAL_SVP_NNIE_RECT_ARRAY_S stRect;
	zpl_char* apcRpnDataLayerName[2];
}ZPL_VIDHAL_SVP_NNIE_RFCN_SOFTWARE_PARAM_S;

/*SSD software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_SSD_SOFTWARE_PARAM_S
{
	/*----------------- Model Parameters ---------------*/
	zpl_uint32 au32ConvHeight[12];
	zpl_uint32 au32ConvWidth[12];
	zpl_uint32 au32ConvChannel[12];
	/*----------------- PriorBox Parameters ---------------*/
	zpl_uint32 au32PriorBoxWidth[6];
	zpl_uint32 au32PriorBoxHeight[6];
	HI_FLOAT af32PriorBoxMinSize[6][1];
	HI_FLOAT af32PriorBoxMaxSize[6][1];
	zpl_uint32 u32MinSizeNum;
	zpl_uint32 u32MaxSizeNum;
	zpl_uint32 u32OriImHeight;
	zpl_uint32 u32OriImWidth;
	zpl_uint32 au32InputAspectRatioNum[6];
	HI_FLOAT af32PriorBoxAspectRatio[6][2];
	HI_FLOAT af32PriorBoxStepWidth[6];
	HI_FLOAT af32PriorBoxStepHeight[6];
	HI_FLOAT f32Offset;
	zpl_bool bFlip;
	zpl_bool bClip;
	zpl_int32 as32PriorBoxVar[4];
	/*----------------- Softmax Parameters ---------------*/
	zpl_uint32 au32SoftMaxInChn[6];
	zpl_uint32 u32SoftMaxInHeight;
	zpl_uint32 u32ConcatNum;
	zpl_uint32 u32SoftMaxOutWidth;
	zpl_uint32 u32SoftMaxOutHeight;
	zpl_uint32 u32SoftMaxOutChn;
	/*----------------- DetectionOut Parameters ---------------*/
	zpl_uint32 u32ClassNum;
	zpl_uint32 u32TopK;
	zpl_uint32 u32KeepTopK;
	zpl_uint32 u32NmsThresh;
	zpl_uint32 u32ConfThresh;
	zpl_uint32 au32DetectInputChn[6];
	zpl_uint32 au32ConvStride[6];
	SVP_MEM_INFO_S stPriorBoxTmpBuf;
	SVP_MEM_INFO_S stSoftMaxTmpBuf;
	SVP_DST_BLOB_S stClassRoiNum;
	SVP_DST_BLOB_S stDstRoi;
	SVP_DST_BLOB_S stDstScore;
	SVP_MEM_INFO_S stGetResultTmpBuf;
}ZPL_VIDHAL_SVP_NNIE_SSD_SOFTWARE_PARAM_S;

/*Yolov1 software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S
{
	zpl_uint32 u32OriImHeight;
	zpl_uint32 u32OriImWidth;
	zpl_uint32 u32BboxNumEachGrid;
	zpl_uint32 u32ClassNum;
	zpl_uint32 u32GridNumHeight;
	zpl_uint32 u32GridNumWidth;
	zpl_uint32 u32NmsThresh;
	zpl_uint32 u32ConfThresh;
	SVP_MEM_INFO_S stGetResultTmpBuf;
	SVP_DST_BLOB_S stClassRoiNum;
	SVP_DST_BLOB_S stDstRoi;
	SVP_DST_BLOB_S stDstScore;
}ZPL_VIDHAL_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S;

/*Yolov2 software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S
{
	zpl_uint32 u32OriImHeight;
	zpl_uint32 u32OriImWidth;
	zpl_uint32 u32BboxNumEachGrid;
	zpl_uint32 u32ClassNum;
	zpl_uint32 u32GridNumHeight;
	zpl_uint32 u32GridNumWidth;
	zpl_uint32 u32NmsThresh;
    zpl_uint32 u32ConfThresh;
    zpl_uint32 u32MaxRoiNum;
    HI_FLOAT af32Bias[10];
	SVP_MEM_INFO_S stGetResultTmpBuf;
	SVP_DST_BLOB_S stClassRoiNum;
	SVP_DST_BLOB_S stDstRoi;
	SVP_DST_BLOB_S stDstScore;
}ZPL_VIDHAL_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S;

/*Yolov3 software parameter*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S
{
    zpl_uint32 u32OriImHeight;
    zpl_uint32 u32OriImWidth;
    zpl_uint32 u32BboxNumEachGrid;
    zpl_uint32 u32ClassNum;
    zpl_uint32 au32GridNumHeight[3];
    zpl_uint32 au32GridNumWidth[3];
    zpl_uint32 u32NmsThresh;
    zpl_uint32 u32ConfThresh;
    zpl_uint32 u32MaxRoiNum;
    HI_FLOAT af32Bias[3][6];
    SVP_MEM_INFO_S stGetResultTmpBuf;
    SVP_DST_BLOB_S stClassRoiNum;
    SVP_DST_BLOB_S stDstRoi;
    SVP_DST_BLOB_S stDstScore;
}ZPL_VIDHAL_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S;

/*stat performance*/
#ifdef ZPL_VIDHAL_SVP_NNIE_PERF_STAT
typedef struct hiZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S
{
    zpl_uint64 u64SrcFlushTime;
    zpl_uint64 u64PreDstFulshTime;
    zpl_uint64 u64AferDstFulshTime;
    zpl_uint64 u64OPTime;
}ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S;
/*Yolo*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_YOLO_PERF_STAT_S
{
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stForwardPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stGRPerf; /*GetResult performance*/
}ZPL_VIDHAL_SVP_NNIE_YOLO_PERF_STAT_S;
/*SSD*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_SSD_PERF_STAT_S
{
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stForwardPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stGRPerf; /*GetResult performance*/
}ZPL_VIDHAL_SVP_NNIE_SSD_PERF_STAT_S;

/*PVANET*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_PVANET_PERF_STAT_S
{
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stForwardPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stRpnPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stRoiPoolingPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stGRPerf; /*GetResult performance*/
}ZPL_VIDHAL_SVP_NNIE_PVANET_PERF_STAT_S;

/*RFCN*/
typedef struct hiZPL_VIDHAL_SVP_NNIE_RFCN_PERF_STAT_S
{
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stForwardPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stRpnPerf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stPsRoiPooling1Perf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stPsRoiPooling2Perf;
    ZPL_VIDHAL_SVP_NNIE_OP_PERF_STAT_S stGRPerf; /*GetResult performance*/
}ZPL_VIDHAL_SVP_NNIE_RFCN_PERF_STAT_S;

#define ZPL_VIDHAL_SVP_NIE_PERF_STAT_DEF_VAR() \
    struct timeval stStart;\
    struct timeval stEnd;
#define ZPL_VIDHAL_SVP_NIE_PERF_STAT_DEF_FRM_VAR() \
    zpl_uint32 u32Frm;\

#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_BEGIN_LOOP() for(u32Frm = 0; u32Frm < ZPL_VIDHAL_SVP_NNIE_PERF_STAT_LOOP_TIMES; u32Frm++){
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_END_LOOP()   }
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_LOOP_TIMES  50

#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_BEGIN() gettimeofday(&stStart, NULL);
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_END()   gettimeofday(&stEnd, NULL);
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_TIME_DIFF()  ((stEnd.tv_sec - stStart.tv_sec) * 1000000 + (zpl_int32)(stEnd.tv_usec - stStart.tv_usec))
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_GET_DIFF_TIME(time) (time) = ZPL_VIDHAL_SVP_NNIE_PERF_STAT_TIME_DIFF();
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_ADD_DIFF_TIME(time) (time) += ZPL_VIDHAL_SVP_NNIE_PERF_STAT_TIME_DIFF();

#else
#define ZPL_VIDHAL_SVP_NIE_PERF_STAT_DEF_VAR()
#define ZPL_VIDHAL_SVP_NIE_PERF_STAT_DEF_FRM_VAR()
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_BEGIN_LOOP()
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_END_LOOP()
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_BEGIN()
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_END()
#define ZPL_VIDHAL_SVP_NNIE_PERF_STAT_TIME_DIFF()


#endif
 /*****************************************************************************
 *   Prototype    : zpl_vidhal_svp_nnie_ParamDeinit
 *   Description  : Deinit NNIE parameters
 *   Input        : ZPL_VIDHAL_SVP_NNIE_PARAM_S        *pstNnieParam     NNIE Parameter
 *                  ZPL_VIDHAL_SVP_NNIE_SOFTWARE_MEM_S *pstSoftWareMem   software mem
 *
 *
 *
 *
 *   Output       :
 *   Return Value :  zpl_int32,HI_SUCCESS:Success,Other:failure
 *   Spec         :
 *   Calls        :
 *   Called By    :
 *   History:
 *
 *       1.  Date         : 2017-11-20
 *           Author       :
 *           Modification : Create
 *
 *****************************************************************************/
 int zpl_vidhal_svp_nnie_ParamDeinit(ZPL_VIDHAL_SVP_NNIE_PARAM_S *pstNnieParam);

 /*****************************************************************************
 *   Prototype    : zpl_vidhal_svp_nnie_ParamInit
 *   Description  : Init NNIE  parameters
 *   Input        : ZPL_VIDHAL_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
 *                  ZPL_VIDHAL_SVP_NNIE_PARAM_S *pstNnieParam    NNIE parameters
 *
 *
 *
 *   Output       :
 *   Return Value : zpl_int32,HI_SUCCESS:Success,Other:failure
 *   Spec         :
 *   Calls        :
 *   Called By    :
 *   History:
 *
 *       1.  Date         : 2017-11-20
 *           Author       :
 *           Modification : Create
 *
 *****************************************************************************/
 int zpl_vidhal_svp_nnie_ParamInit(ZPL_VIDHAL_SVP_NNIE_CFG_S *pstNnieCfg,
     ZPL_VIDHAL_SVP_NNIE_PARAM_S *pstNnieParam);

 /*****************************************************************************
 *   Prototype    : zpl_vidhal_svp_nnie_UnloadModel
 *   Description  : unload NNIE model
 *   Input        : ZPL_VIDHAL_SVP_NNIE_MODEL_S *pstNnieModel      NNIE Model
 *
 *
 *
 *   Output       :
 *   Return Value : zpl_int32,HI_SUCCESS:Success,Other:failure
 *   Spec         :
 *   Calls        :
 *   Called By    :
 *   History:
 *
 *       1.  Date         : 2017-11-20
 *           Author       :
 *           Modification : Create
 *
 *****************************************************************************/
 int zpl_vidhal_svp_nnie_UnloadModel(ZPL_VIDHAL_SVP_NNIE_MODEL_S *pstNnieModel);

 /*****************************************************************************
 *   Prototype    : zpl_vidhal_svp_nnie_LoadModel
 *   Description  : load NNIE model
 *   Input        : zpl_char                 * pszModelFile    Model file name
 *                  ZPL_VIDHAL_SVP_NNIE_MODEL_S *pstNnieModel     NNIE Model
 *
 *
 *
 *   Output       :
 *   Return Value : zpl_int32,HI_SUCCESS:Success,Other:failure
 *   Spec         :
 *   Calls        :
 *   Called By    :
 *   History:
 *
 *       1.  Date         : 2017-11-20
 *           Author       :
 *           Modification : Create
 *
 *****************************************************************************/
 int zpl_vidhal_svp_nnie_LoadModel(zpl_char * pszModelFile,
     ZPL_VIDHAL_SVP_NNIE_MODEL_S *pstNnieModel);

/*****************************************************************************
*   Prototype    : zpl_vidhal_svp_nnie_FillRect
*   Description  : Draw rect
*   Input        : VIDEO_FRAME_INFO_S             *pstFrmInfo   Frame info
* 		            ZPL_VIDHAL_SVP_NNIE_RECT_ARRAY_S  *pstRect       Rect
*                  zpl_uint32                         u32Color      Color
*
*
*   Output       :
*   Return Value : zpl_int32
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-03-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
int zpl_vidhal_svp_nnie_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, ZPL_VIDHAL_SVP_NNIE_RECT_ARRAY_S* pstRect, zpl_uint32 u32Color);

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ZPL_VIDHAL_NNIE_H__ */
