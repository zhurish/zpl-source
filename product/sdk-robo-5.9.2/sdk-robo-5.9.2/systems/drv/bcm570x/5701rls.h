/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2000 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* History:                                                                   */
/******************************************************************************/

typedef unsigned long U32;
int t3FwReleaseMajor = 0x0;
int t3FwReleaseMinor = 0x0;
int t3FwReleaseFix = 0x0;
U32 t3FwStartAddr = 0x08000000;
U32 t3FwTextAddr = 0x08000000;
int t3FwTextLen = 0x9c0;
U32 t3FwRodataAddr = 0x080009c0;
int t3FwRodataLen = 0x60;
U32 t3FwDataAddr = 0x08000a40;
int t3FwDataLen = 0x20;
U32 t3FwSbssAddr = 0x08000a60;
int t3FwSbssLen = 0xc;
U32 t3FwBssAddr = 0x08000a70;
int t3FwBssLen = 0x10;
U32 t3FwText[(0x9c0/4) + 1] = {
0x0, 
0x10000003, 0x0, 0xd, 0xd, 
0x3c1d0800, 0x37bd3ffc, 0x3a0f021, 0x3c100800, 
0x26100000, 0xe000018, 0x0, 0xd, 
0x3c1d0800, 0x37bd3ffc, 0x3a0f021, 0x3c100800, 
0x26100034, 0xe00021c, 0x0, 0xd, 
0x0, 0x0, 0x0, 0x27bdffe0, 
0x3c1cc000, 0xafbf0018, 0xaf80680c, 0xe00004c, 
0x241b2105, 0x97850000, 0x97870002, 0x9782002c, 
0x9783002e, 0x3c040800, 0x248409c0, 0xafa00014, 
0x21400, 0x621825, 0x52c00, 0xafa30010, 
0x8f860010, 0xe52825, 0xe000060, 0x24070102, 
0x3c02ac00, 0x34420100, 0x3c03ac01, 0x34630100, 
0xaf820490, 0x3c02ffff, 0xaf820494, 0xaf830498, 
0xaf82049c, 0x24020001, 0xaf825ce0, 0xe00003f, 
0xaf825d00, 0xe000140, 0x0, 0x8fbf0018, 
0x3e00008, 0x27bd0020, 0x2402ffff, 0xaf825404, 
0x8f835400, 0x34630400, 0xaf835400, 0xaf825404, 
0x3c020800, 0x24420034, 0xaf82541c, 0x3e00008, 
0xaf805400, 0x0, 0x0, 0x3c020800, 
0x34423000, 0x3c030800, 0x34633000, 0x3c040800, 
0x348437ff, 0x3c010800, 0xac220a64, 0x24020040, 
0x3c010800, 0xac220a68, 0x3c010800, 0xac200a60, 
0xac600000, 0x24630004, 0x83102b, 0x5040fffd, 
0xac600000, 0x3e00008, 0x0, 0x804821, 
0x8faa0010, 0x3c020800, 0x8c420a60, 0x3c040800, 
0x8c840a68, 0x8fab0014, 0x24430001, 0x44102b, 
0x3c010800, 0xac230a60, 0x14400003, 0x4021, 
0x3c010800, 0xac200a60, 0x3c020800, 0x8c420a60, 
0x3c030800, 0x8c630a64, 0x91240000, 0x21140, 
0x431021, 0x481021, 0x25080001, 0xa0440000, 
0x29020008, 0x1440fff4, 0x25290001, 0x3c020800, 
0x8c420a60, 0x3c030800, 0x8c630a64, 0x8f84680c, 
0x21140, 0x431021, 0xac440008, 0xac45000c, 
0xac460010, 0xac470014, 0xac4a0018, 0x3e00008, 
0xac4b001c, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x2000008, 
0x0, 0xa0001e3, 0x3c0a0001, 0xa0001e3, 
0x3c0a0002, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x3c0a0007, 0xa0001e3, 0x3c0a0008, 0xa0001e3, 
0x3c0a0009, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x3c0a000b, 0xa0001e3, 
0x3c0a000c, 0xa0001e3, 0x3c0a000d, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x3c0a000e, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x0, 0xa0001e3, 
0x0, 0xa0001e3, 0x3c0a0013, 0xa0001e3, 
0x3c0a0014, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x27bdffe0, 
0x1821, 0x1021, 0xafbf0018, 0xafb10014, 
0xafb00010, 0x3c010800, 0x220821, 0xac200a70, 
0x3c010800, 0x220821, 0xac200a74, 0x3c010800, 
0x220821, 0xac200a78, 0x24630001, 0x1860fff5, 
0x2442000c, 0x24110001, 0x8f906810, 0x32020004, 
0x14400005, 0x24040001, 0x3c020800, 0x8c420a78, 
0x18400003, 0x2021, 0xe000182, 0x0, 
0x32020001, 0x10400003, 0x0, 0xe000169, 
0x0, 0xa000153, 0xaf915028, 0x8fbf0018, 
0x8fb10014, 0x8fb00010, 0x3e00008, 0x27bd0020, 
0x3c050800, 0x8ca50a70, 0x3c060800, 0x8cc60a80, 
0x3c070800, 0x8ce70a78, 0x27bdffe0, 0x3c040800, 
0x248409d0, 0xafbf0018, 0xafa00010, 0xe000060, 
0xafa00014, 0xe00017b, 0x2021, 0x8fbf0018, 
0x3e00008, 0x27bd0020, 0x24020001, 0x8f836810, 
0x821004, 0x21027, 0x621824, 0x3e00008, 
0xaf836810, 0x27bdffd8, 0xafbf0024, 0x1080002e, 
0xafb00020, 0x8f825cec, 0xafa20018, 0x8f825cec, 
0x3c100800, 0x26100a78, 0xafa2001c, 0x34028000, 
0xaf825cec, 0x8e020000, 0x18400016, 0x0, 
0x3c020800, 0x94420a74, 0x8fa3001c, 0x221c0, 
0xac830004, 0x8fa2001c, 0x3c010800, 0xe000201, 
0xac220a74, 0x10400005, 0x0, 0x8e020000, 
0x24420001, 0xa0001df, 0xae020000, 0x3c020800, 
0x8c420a70, 0x21c02, 0x321c0, 0xa0001c5, 
0xafa2001c, 0xe000201, 0x0, 0x1040001f, 
0x0, 0x8e020000, 0x8fa3001c, 0x24420001, 
0x3c010800, 0xac230a70, 0x3c010800, 0xac230a74, 
0xa0001df, 0xae020000, 0x3c100800, 0x26100a78, 
0x8e020000, 0x18400028, 0x0, 0xe000201, 
0x0, 0x14400024, 0x0, 0x8e020000, 
0x3c030800, 0x8c630a70, 0x2442ffff, 0xafa3001c, 
0x18400006, 0xae020000, 0x31402, 0x221c0, 
0x8c820004, 0x3c010800, 0xac220a70, 0x97a2001e, 
0x2442ff00, 0x2c420300, 0x1440000b, 0x24024000, 
0x3c040800, 0x248409dc, 0xafa00010, 0xafa00014, 
0x8fa6001c, 0x24050008, 0xe000060, 0x3821, 
0xa0001df, 0x0, 0xaf825cf8, 0x3c020800, 
0x8c420a40, 0x8fa3001c, 0x24420001, 0xaf835cf8, 
0x3c010800, 0xac220a40, 0x8fbf0024, 0x8fb00020, 
0x3e00008, 0x27bd0028, 0x27bdffe0, 0x3c040800, 
0x248409e8, 0x2821, 0x3021, 0x3821, 
0xafbf0018, 0xafa00010, 0xe000060, 0xafa00014, 
0x8fbf0018, 0x3e00008, 0x27bd0020, 0x8f82680c, 
0x8f85680c, 0x21827, 0x3182b, 0x31823, 
0x431024, 0x441021, 0xa2282b, 0x10a00006, 
0x0, 0x401821, 0x8f82680c, 0x43102b, 
0x1440fffd, 0x0, 0x3e00008, 0x0, 
0x3c040800, 0x8c840000, 0x3c030800, 0x8c630a40, 
0x64102b, 0x54400002, 0x831023, 0x641023, 
0x2c420008, 0x3e00008, 0x38420001, 0x27bdffe0, 
0x802821, 0x3c040800, 0x24840a00, 0x3021, 
0x3821, 0xafbf0018, 0xafa00010, 0xe000060, 
0xafa00014, 0xa000216, 0x0, 0x8fbf0018, 
0x3e00008, 0x27bd0020, 0x0, 0x27bdffe0, 
0x3c1cc000, 0xafbf0018, 0xe00004c, 0xaf80680c, 
0x3c040800, 0x24840a10, 0x3802821, 0x3021, 
0x3821, 0xafa00010, 0xe000060, 0xafa00014, 
0x2402ffff, 0xaf825404, 0x3c0200aa, 0xe000234, 
0xaf825434, 0x8fbf0018, 0x3e00008, 0x27bd0020, 
0x0, 0x0, 0x0, 0x27bdffe8, 
0xafb00010, 0x24100001, 0xafbf0014, 0x3c01c003, 
0xac200000, 0x8f826810, 0x30422000, 0x10400003, 
0x0, 0xe000246, 0x0, 0xa00023a, 
0xaf905428, 0x8fbf0014, 0x8fb00010, 0x3e00008, 
0x27bd0018, 0x27bdfff8, 0x8f845d0c, 0x3c0200ff, 
0x3c030800, 0x8c630a50, 0x3442fff8, 0x821024, 
0x1043001e, 0x3c0500ff, 0x34a5fff8, 0x3c06c003, 
0x3c074000, 0x851824, 0x8c620010, 0x3c010800, 
0xac230a50, 0x30420008, 0x10400005, 0x871025, 
0x8cc20000, 0x24420001, 0xacc20000, 0x871025, 
0xaf825d0c, 0x8fa20000, 0x24420001, 0xafa20000, 
0x8fa20000, 0x8fa20000, 0x24420001, 0xafa20000, 
0x8fa20000, 0x8f845d0c, 0x3c030800, 0x8c630a50, 
0x851024, 0x1443ffe8, 0x851824, 0x27bd0008, 
0x3e00008, 0x0, 0x0, 0x0 };
U32 t3FwRodata[(0x60/4) + 1] = {
0x35373031, 0x726c7341, 0x0, 
0x0, 0x53774576, 0x656e7430, 0x0, 
0x726c7045, 0x76656e74, 0x31000000, 0x556e6b6e, 
0x45766e74, 0x0, 0x0, 0x0, 
0x0, 0x66617461, 0x6c457272, 0x0, 
0x0, 0x4d61696e, 0x43707542, 0x0, 
0x0, 0x0 };
U32 t3FwData[(0x20/4) + 1] = {
0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 
0x0, 0x0 };
