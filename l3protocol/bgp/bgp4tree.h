#include "bgp4_api.h"
#include "bgp4main.h"
#include "bgp4peer.h"
#include "bgp4path.h"

#ifdef NEW_BGP_WANTED

#ifndef BGP4_TREE_H
#define BGP4_TREE_H
#ifdef __cplusplus
  extern "C" {
#endif
#include "avl.h"

/*wrapped avl API*/
/*avl tree init for node without offset*/
#define bgp4_avl_init(t, f) avl_create((t), (int(*)(const void *, const void *))(f), 128, 0);

/*avl tree init for node with offset*/
#define bgp4_avl_init2(t, f, o) avl_create((t), (int(*)(const void *, const void *))(f), 128+(o), (o));

/*first*/
#define bgp4_avl_first(t) avl_first(t)

/*get last node*/
#define bgp4_avl_last(t) avl_last(t)

/*get next node,input node is user node and must be valid node in table, we will translate it*/
#define bgp4_avl_next(t, info) ((info) ? AVL_NEXT(t, info) : NULL)

static INLINE void *bgp4_avl_greator(avl_tree_t *t, void *info)
{ 
    avl_index_t where; 
    void *p_node = NULL; 
    p_node = avl_find(t, info, &where); 
    if (p_node != NULL)
        p_node = AVL_NEXT(t, p_node); 
    else 
        p_node = avl_nearest(t, where, AVL_AFTER); 
    return p_node;
}

static INLINE void *bgp4_avl_lookup(avl_tree_t *t, void *info)
{ 
    avl_index_t where; 
    return avl_find(t, info, &where); 
}

#define bgp4_avl_count(t) avl_numnodes(t)

#define bgp4_avl_add(t, info) avl_add(t, info)

#define bgp4_avl_delete(t, info) avl_remove(t, info)

#define bgp4_avl_for_each(t, x) for ((x) = bgp4_avl_first(t); (x); (x) = bgp4_avl_next(t, (x)))

#define bgp4_avl_for_each_safe(t, x, n) for ((x) = bgp4_avl_first(t),\
    (n) = bgp4_avl_next(t, (x)); (x); (x) = (n), (n) = bgp4_avl_next(t, (x)))

#define bgp4_avl_for_each_greater(t, x, minnode) for (\
              (x) = bgp4_avl_greator(t, minnode) ; \
              (x) ;\
              (x) = bgp4_avl_next(t, (x)))

/*check all avl node and do special process*/
#define bgp4_avl_walkup(p_table, oper) do\
   {\
    void *xxx = NULL;\
    void *yyy = NULL;\
    bgp4_avl_for_each_safe(p_table, xxx, yyy)\
    {\
         oper(xxx);\
    }}while(0)

/*init rib tree*/
#define bgp4_route_table_init(t) bgp4_avl_init(t, bgp4_route_lookup_cmp)

#define bgp4_unsort_avl_init(t) bgp4_avl_init(t, bgp4_avl_unsort_cmp)

tBGP4_ROUTE *bgp4_rib_vector_lookup(tBGP4_ROUTE_VECTOR *p_vector, tBGP4_ROUTE *p_route);
tBGP4_ROUTE *bgp4_rib_vector_best(tBGP4_ROUTE_VECTOR *p_vector);
tBGP4_ROUTE *bgp4_best_route(avl_tree_t *p_root, tBGP4_ADDR *p_dest);
u_int bgp4_rib_vector_best_index(tBGP4_ROUTE_VECTOR *p_vector);
int bgp4_route_ECMP_priority_cmp(tBGP4_ROUTE *p_route1, tBGP4_ROUTE *p_route2);
int bgp4_avl_unsort_cmp(void *p1, void *p2);
int bgp4_route_lookup_cmp(tBGP4_ROUTE *p1, tBGP4_ROUTE *p2);
void bgp4_route_table_flush(avl_tree_t * p_root);
void bgp4_route_table_delete(tBGP4_ROUTE *) ;
void bgp4_route_table_add (avl_tree_t *p_root, tBGP4_ROUTE  *p_route); 
void bgp4_aggregated_route_get(tBGP4_VPN_INSTANCE *p_instance, tBGP4_ADDR *p_dest, avl_tree_t *p_list);
void bgp4_rib_lookup_dest(avl_tree_t *p_root, tBGP4_ADDR *p_dest, tBGP4_ROUTE_VECTOR *p_vector);
void bgp4_rib_vector_ecmp_best(tBGP4_ROUTE_VECTOR *p_vector_in,tBGP4_ROUTE_VECTOR *p_vector_out);

#ifdef __cplusplus
}
#endif 
#endif /* BGP4_TREE_H */

#else

#ifndef BGP4_TREE_H
#define BGP4_TREE_H
#ifdef __cplusplus
      extern "C" {
     #endif
#include "avl.h"

#define RIB_LOOP(root, rt, nxt) for (rt = bgp4_rib_first(root), \
    nxt = bgp4_rib_next(root, rt);\
    rt; rt = nxt, nxt = bgp4_rib_next(root, rt))

int bgp4_init_rib(tBGP4_RIB *p_root);
int bgp4_clear_rib(tBGP4_RIB * p_root);
tBGP4_ROUTE *bgp4_rib_lookup (tBGP4_RIB * p_root, tBGP4_ROUTE *);
void bgp4_rib_delete(tBGP4_RIB *p_root, tBGP4_ROUTE *) ;
tBGP4_ROUTE *bgp4_best_route(tBGP4_RIB *p_root, tBGP4_ADDR *p_dest);
tBGP4_ROUTE *bgp4_rib_first(tBGP4_RIB *p_root);
tBGP4_ROUTE *bgp4_rib_next(tBGP4_RIB *p_root, tBGP4_ROUTE *p_route);
STATUS bgp4_rib_add (tBGP4_RIB *p_root, tBGP4_ROUTE  *p_route); 
void bgp4_rib_get_by_range(tBGP4_RIB *p_root, tBGP4_ADDR *p_dest, tBGP4_LIST *p_list);
u_int bgp4_rib_count(tBGP4_RIB *p_root);
void bgp4_rib_lookup_vector(tBGP4_RIB *p_root, tBGP4_ADDR *p_dest, tBGP4_ROUTE_VECTOR *p_vector);
tBGP4_ROUTE *bgp4_best_route_vector(tBGP4_ROUTE_VECTOR *p_vector);
tBGP4_ROUTE *bgp4_special_route_vector(tBGP4_ROUTE_VECTOR *p_vector, tBGP4_ROUTE *p_expect);

#ifdef __cplusplus
     }
     #endif 
#endif /* BGP4_TREE_H */



#endif

