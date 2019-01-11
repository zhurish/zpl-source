/*
 * voip_statistics.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __VOIP_STATISTICS_H__
#define __VOIP_STATISTICS_H__



extern int voip_rtp_stats_display(struct vty *vty);
extern int voip_bandwidth_display(struct vty *vty);

extern int voip_quality_display(struct vty *vty);

#endif /* __VOIP_STATISTICS_H__ */
