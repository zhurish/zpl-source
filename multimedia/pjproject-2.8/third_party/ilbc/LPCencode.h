
   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       LPCencode.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_LPCENCOD_H
   #define __iLBC_LPCENCOD_H

   void LPCencode(
       float *syntdenum,   /* (i/o) synthesis filter coefficients
                                  before/after encoding */
       float *weightdenum, /* (i/o) weighting denumerator coefficients
                                  before/after encoding */
       int *lsf_index,     /* (o) lsf quantization index */
       float *data,    /* (i) lsf coefficients to quantize */
       iLBC_Enc_Inst_t *iLBCenc_inst
                           /* (i/o) the encoder state structure */
   );

   void SimpleAnalysis(
       float *lsf,         /* (o) lsf coefficients */
       float *data,    /* (i) new data vector */
       iLBC_Enc_Inst_t *iLBCenc_inst
                           /* (i/o) the encoder state structure */
   );

   void LSFinterpolate2a_enc(
       float *a,       /* (o) lpc coefficients */
       float *lsf1,/* (i) first set of lsf coefficients */
       float *lsf2,/* (i) second set of lsf coefficients */
       float coef,     /* (i) weighting coefficient to use between
                              lsf1 and lsf2 */
       long length      /* (i) length of coefficient vectors */
   );

   void SimpleInterpolateLSF(
       float *syntdenum,   /* (o) the synthesis filter denominator
                                  resulting from the quantized
                                  interpolated lsf */
       float *weightdenum, /* (o) the weighting filter denominator
                                  resulting from the unquantized
                                  interpolated lsf */
       float *lsf,         /* (i) the unquantized lsf coefficients */
       float *lsfdeq,      /* (i) the dequantized lsf coefficients */
       float *lsfold,      /* (i) the unquantized lsf coefficients of
                                  the previous signal frame */
       float *lsfdeqold, /* (i) the dequantized lsf coefficients of
                                  the previous signal frame */
       int length,         /* (i) should equate LPC_FILTERORDER */
       iLBC_Enc_Inst_t *iLBCenc_inst
                           /* (i/o) the encoder state structure */
   );

   void SimplelsfQ(
       float *lsfdeq,    /* (o) dequantized lsf coefficients
                              (dimension FILTERORDER) */
       int *index,     /* (o) quantization index */
       float *lsf,      /* (i) the lsf coefficient vector to be
                              quantized (dimension FILTERORDER ) */
       int lpc_n     /* (i) number of lsf sets to quantize */
   );
   #endif

