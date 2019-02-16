
   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       doCPLC.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_DOLPC_H
   #define __iLBC_DOLPC_H

   void compCorr(
       float *cc,      /* (o) cross correlation coefficient */
       float *gc,      /* (o) gain */
       float *pm,
       float *buffer,  /* (i) signal buffer */
       int lag,    /* (i) pitch lag */
       int bLen,       /* (i) length of buffer */
       int sRange      /* (i) correlation search length */
   );


   void doThePLC(
       float *PLCresidual, /* (o) concealed residual */
       float *PLClpc,      /* (o) concealed LP parameters */
       int PLI,        /* (i) packet loss indicator
                                  0 - no PL, 1 = PL */
       float *decresidual, /* (i) decoded residual */
       float *lpc,         /* (i) decoded LPC (only used for no PL) */
       int inlag,          /* (i) pitch lag */
       iLBC_Dec_Inst_t *iLBCdec_inst
                           /* (i/o) decoder instance */
   );

   #endif

