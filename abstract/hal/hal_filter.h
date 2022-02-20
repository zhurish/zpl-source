/*
 * hal_filter.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_FILTER_H__
#define __HAL_FILTER_H__
#ifdef __cplusplus
extern "C" {
#endif

/* Initialize filter software subsystem. */
extern int hal_filter_init(void);

/* Create a blank filter template. */
extern int hal_filter_create(
    zpl_uint32 *f_return);

/* Create a blank filter template with requested filter ID. */
extern int hal_filter_create_id(
    zpl_uint32 f);

/* Destroy a filter template. */
extern int hal_filter_destroy(
    zpl_uint32 f);

/* 
 * Set priority of this filter relative to other filters that match
 * simultaneously.
 */
extern int hal_filter_qualify_priority(
    zpl_uint32 f, 
    int prio);

/* Set ingress port(s) that the packet must match to trigger filter. */
extern int hal_filter_qualify_ingress(
    zpl_uint32 f, 
    ifindex_t ifindex);

/* Add egress port(s) that the packet must match to trigger filter. */
extern int hal_filter_qualify_egress(
    zpl_uint32 f, 
    ifindex_t ifindex);


/* Set a filter to match only unknown unicast packets. */
extern int hal_filter_qualify_unknown_ucast(
    zpl_uint32 f);

/* Set a filter to match only unknown multicast packets. */
extern int hal_filter_qualify_unknown_mcast(
    zpl_uint32 f);

/* Set a filter to match only known unicast packets. */
extern int hal_filter_qualify_known_ucast(
    zpl_uint32 f);

/* Set a filter to match only known multicast packets. */
extern int hal_filter_qualify_known_mcast(
    zpl_uint32 f);

/* Set a filter to match only broadcast packets. */
extern int hal_filter_qualify_broadcast(
    zpl_uint32 f);

/* Set a filter to match only a particular packet format. */
extern int hal_filter_qualify_format(
    zpl_uint32 f, 
    zpl_uint32 format);

/* Install a filter into the hardware tables. */
extern int hal_filter_install(
    zpl_uint32 f);

/* Re-Install a filter into the hardware tables. */
extern int hal_filter_reinstall(
    zpl_uint32 f);

/* Remove a filter from the hardware tables. */
extern int hal_filter_remove(
    zpl_uint32 f);

/* Quickly remove all filters from the hardware tables. */
extern int hal_filter_remove_all(void);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_FILTER_H__ */
