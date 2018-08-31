
#include "bgp4util.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"


/*memory support API S*/
#if 0
/*memory object linked to free memory list,it is not
used when memory is in used*/
struct bgp_mobject{
   struct bgp_mobject *p_next;
};

/*memory block,include all memory objects with same length*/
struct bgp_mblock{
   /*memory object length*/ 
   u_int len;

   /*memory object count in this size*/    
   u_int count;

   /*current free memory count*/
   u_int free_count;
   
   /*starting buffer pointer*/
   void *p_buf;

   /*first free object*/
   struct bgp_mobject *p_free;
};
/*memory stat counter*/
typedef struct {
    u_int add;
    
    u_int fail;
    
    u_int del;
}tBGP4_MEMSTAT;

#define BGP_MAX_MBLOCK 8
static struct bgp_mblock bgp_mblocks[BGP_MAX_MBLOCK];
static tBGP4_MEMSTAT bgp_mstat[MEM_BGP_MAX];
    
static void
bgp4_mem_increase(
     u_int size,
     u_int count)
{
    u_int i;

    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
        if (bgp_mblocks[i].len >= size)
        {
            bgp_mblocks[i].count += count;
            break;
        }
    }
    return;
}

/*prepare to init memory.called when bgp is started*/
void 
bgp4_mem_init(
       u_int max_route,
       u_int max_path,
       u_int max_peer,
       u_int max_vpn_instance)
{
    u_int i = 0; 
    u_int count = 0;
    struct bgp_mblock *p_block;
    struct bgp_mobject *p_obj;

    /*clear memory statistic*/
    memset(bgp_mstat, 0, sizeof(bgp_mstat));
    
    /*clear memory block*/
    memset(bgp_mblocks, 0, sizeof(bgp_mblocks));

    /*init fixed length of all mblocks*/
    bgp_mblocks[0].len = 24;
    bgp_mblocks[1].len = 64;
    bgp_mblocks[2].len = ((sizeof(tBGP4_ROUTE)+3)/4)*4;/*route table*/
    bgp_mblocks[3].len = 160;
    bgp_mblocks[4].len = 320;
    bgp_mblocks[5].len = 1500;
    bgp_mblocks[6].len = 5000;
    bgp_mblocks[7].len = BGP4_MAX_VRF_ID*12;

    /*calculate memory object count for each mblock size*/
    /*route link node*/
    bgp4_mem_increase(sizeof(tBGP4_LINK), max_route/20);

    /*network/range...*/
    bgp4_mem_increase(sizeof(tBGP4_NETWORK), max_path + max_path/10);

    /*route*/
    bgp4_mem_increase(sizeof(tBGP4_ROUTE), max_route + max_route/2 + max_route/20);

    /*path*/
    bgp4_mem_increase(sizeof(tBGP4_PATH), max_path + max_path/10);

    /*peer*/
    bgp4_mem_increase(sizeof(tBGP4_PEER), max_peer);

    /*instance*/
    bgp4_mem_increase(sizeof(tBGP4_VPN_INSTANCE), max_vpn_instance);

    /*rib*/
    bgp4_mem_increase(sizeof(tBGP4_RIB), max_vpn_instance + max_vpn_instance/2);

    /*route flooding*/
    bgp4_mem_increase(sizeof(tBGP4_FLOOD_FLAG), max_route);

    bgp4_mem_increase(sizeof(tBGP4_EXT_FLOOD_FLAG), max_route/2);

    /*route msg*/
    bgp4_mem_increase(1200, max_vpn_instance*2);

    /*rxd buffer*/
    bgp4_mem_increase(BGP4_MAX_MSG_LEN, 20);

    /*vpn lookup msg*/
    bgp4_mem_increase(BGP4_MAX_VRF_ID*12, 4);

    /*policy*/
    bgp4_mem_increase(sizeof(tBGP4_REDISTRIBUTE_CONFIG), max_vpn_instance + max_vpn_instance/2);    
    
    /*allocate buffer for each blocks*/
    for (i = 0 ; i < BGP_MAX_MBLOCK; i++)
    {
        p_block = &bgp_mblocks[i];
        p_block->p_buf = gbgp4.malloc(MTYPE_BGP, p_block->count * p_block->len);
        if (p_block->p_buf == NULL)
        {
            continue;
        }
        memset(p_block->p_buf, 0, p_block->len*p_block->count);
        p_block->p_free = NULL;
        p_block->free_count = p_block->count;
        for (count = 0 ; count < p_block->count ; count++)
        {
            p_obj = (struct bgp_mobject *)((u_long)p_block->p_buf + count*p_block->len);
            p_obj->p_next = p_block->p_free;
            p_block->p_free = p_obj;
        }
    }
    return;
}

/*free all memory blocks,called when bgp is stopped*/
void 
bgp4_mem_deinit(void)
{
    u_int i = 0;
    struct bgp_mblock *p_block;

    /*create memory*/
    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
        p_block = &bgp_mblocks[i];
        if (p_block->p_buf)
        {
            gbgp4.mfree(p_block->p_buf);
        }
        p_block->p_buf = NULL;
        p_block->p_free = NULL;
        p_block->count = 0;
        p_block->free_count = 0;
    }  
    return;
}

/*allocate memory with special length and type*/
void *
bgp4_malloc(
     u_int len, 
     u_int type)
{
    struct bgp_mobject *p_obj = NULL;
    u_int i;
    
    bgp_mstat[type].add++;

    /*get first matched block*/
    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
        if ((bgp_mblocks[i].len >= len) && (bgp_mblocks[i].count > 0))
        {
            break;
        }
    }
    /*check if some block matched*/
    if (i >= BGP_MAX_MBLOCK)
    {
        bgp_mstat[type].fail++;
        return NULL;
    }

    /*if no free memory exist,stop*/
    p_obj = bgp_mblocks[i].p_free;
    if (p_obj == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT, "memory allocate failed %d.%d.%d", type, i, len);
        bgp_mstat[type].fail++;
        return NULL;
    }

    bgp_mblocks[i].free_count--;

    /*strip this object from free object table*/
    bgp_mblocks[i].p_free = p_obj->p_next;

    /*init memory*/
    memset(p_obj, 0, len);
    return p_obj;
}

/*release memory object*/
void 
bgp4_free(
    void *p_buf, 
    u_int type) 
{
    struct bgp_mobject *p_obj = NULL;
    u_int i = 0;
    u_long start = 0;
    u_long end = 0;
    
    bgp_mstat[type].del++; 

    /*try to found expected block*/
    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
        start = (u_long)bgp_mblocks[i].p_buf;
        end = start + bgp_mblocks[i].len*bgp_mblocks[i].count;
        if (((u_long)p_buf >= start) && ((u_long)p_buf < end))
        {
            /*got expected block,add object to free table*/
            p_obj = (struct bgp_mobject *)p_buf;
            
            /*reset buffer*/
            memset(p_obj, 0, bgp_mblocks[i].len);
            p_obj->p_next = bgp_mblocks[i].p_free;
            bgp_mblocks[i].p_free = p_obj;
            bgp_mblocks[i].free_count++;
            break;
        }
    }
    return;
}

void
bgp4_display_memory(void *vty)
{
    u_int i = 0;
    u_int total = 0;
    
    /*display for all mblocks*/
    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
       total += bgp_mblocks[i].len * bgp_mblocks[i].count;
    }

    vty_out(vty, "\n\rResource Configuration\n\r");

    vty_out(vty, "   Max Route:%d\n\r", gbgp4.max_route);
    vty_out(vty, "   Max Path:%d\n\r", gbgp4.max_path);
    vty_out(vty, "   Max Peer:%d\n\r", gbgp4.max_peer);
    vty_out(vty, "   Max Instance:%d\n\r", gbgp4.max_vpn_instance);

    vty_out(vty, "\n\rMemory Block Table:total %dbytes\n\r", total);

    /*len count free percent*/
    vty_out(vty, " %-6s%-10s%-10s%s\n\r","len","count","free","percent");
    for (i = 0 ; i < BGP_MAX_MBLOCK ; i++)
    {
        vty_out(vty, " %-6d%-10d%-10d%d\n\r",
            bgp_mblocks[i].len,
            bgp_mblocks[i].count,
            bgp_mblocks[i].free_count,
            bgp_mblocks[i].count ? \
            ((bgp_mblocks[i].free_count*100)/bgp_mblocks[i].count) : 0);
    }
    
    /*each type of memory*/
    vty_out(vty, "\n\rMemory Type Usage\n\r");

    /*create fail delete current len name*/
    vty_out(vty, "  %-10s%-8s%-10s%-10s%-5s%s\n\r", 
          "create", "fail", "delete", "current", "len","name");
    
    #define BGP4_MSHOW(type, len, name) \
    do{\
        vty_out(vty, "  %-10d%-8d%-10d%-10d%-5d%s\n\r",\
          bgp_mstat[type].add,\
          bgp_mstat[type].fail,\
          bgp_mstat[type].del,\
          bgp_mstat[type].add - bgp_mstat[type].fail - bgp_mstat[type].del,\
          len, name);}while(0)

    BGP4_MSHOW(MEM_BGP_VPN_INSTANCE, sizeof(tBGP4_VPN_INSTANCE), "VpnInstance");
    BGP4_MSHOW(MEM_BGP_PEER, sizeof(tBGP4_PEER), "Peer");
    BGP4_MSHOW(MEM_BGP_INFO, sizeof(tBGP4_PATH), "Path");
    BGP4_MSHOW(MEM_BGP_ASPATH, sizeof(tBGP4_ASPATH), "ASPath");
    BGP4_MSHOW(MEM_BGP_ROUTE, sizeof(tBGP4_ROUTE), "Route");
    BGP4_MSHOW(MEM_BGP_LINK, sizeof(tBGP4_LINK), "RouteLink");
    BGP4_MSHOW(MEM_BGP_NETWORK, sizeof(tBGP4_NETWORK), "Network");
    BGP4_MSHOW(MEM_BGP_RANGE, sizeof(tBGP4_RANGE), "Range");    
    BGP4_MSHOW(MEM_BGP_FLOOD, sizeof(tBGP4_FLOOD_FLAG), "FloodFlag");    
    BGP4_MSHOW(MEM_BGP_EXTFLOOD, sizeof(tBGP4_EXT_FLOOD_FLAG), "ExtFlood");
    BGP4_MSHOW(MEM_BGP_RXMSG, 0, "Rxbuf");    
    BGP4_MSHOW(MEM_BGP_BUF, 0, "Buffer");
    BGP4_MSHOW(MEM_BGP_NEXTHOP, sizeof(tBGP4_NEXTHOP_BLOCK), "Nexthop");
    BGP4_MSHOW(MEM_BGP_DAMP, sizeof(tBGP4_DAMP_ROUTE), "DampRoute");
    BGP4_MSHOW(MEM_BGP_POLICY_CONFIG, sizeof(tBGP4_POLICY_CONFIG), "Policy");    
    BGP4_MSHOW(MEM_BGP_RIB, sizeof(tBGP4_RIB), "RIB");
    BGP4_MSHOW(MEM_BGP_ORF, sizeof(tBGP4_ORF_ENTRY), "ORF");
    return;
}
#else
/*limit max and min memory zone size*/

/*min zone size is 4M bytes*/
#define BGP4_MIN_MZONE_SIZE (4*1024*1024)

/*max zone size is 64M bytes*/
#define BGP4_MAX_MZONE_SIZE (64*1024*1024)

/*64K bytes == 1 page*/
#define BGP4_PAGE_SIZE 65536

/*memory stat counter*/
typedef struct {
    u_int add;
    
    u_int fail;
    
    u_int del;
}tBGP4_MEMSTAT;

/*memory object linked to free memory list,it is not
used when memory is in used*/
struct bgp4_mobject{
   struct bgp4_mobject *p_next;
};

/*memory table*/
struct bgp4_mtable{
    /*max size of object in this table*/
    u_int size;
   
    /*allcoate page statistic count*/
    u_int page_alloc;
   
    /*free page statistic count*/
    u_int page_free;
    
    /* pages have free buffer*/
    avl_tree_t page_list;
   
    /*pages are full*/
    avl_tree_t full_page_list;
};

/*page struct*/
struct bgp4_mpage{
    /*node linked to memory table*/
    avl_node_t node;

    struct bgp4_mtable *p_mtable;

    /*total count of memory object*/
    u_short total;

    /*total count of memory object not used*/
    u_short free;
    
    /*first free memory object*/
    struct bgp4_mobject *p_free;
};  

/*memory zone struct,contain a linear buffer for page*/
struct bgp4_mzone{
    avl_node_t node;
   
    /*buffer start*/
    u_char *buf;
   
    /*sizeof zone in bytes*/
    u_int size : 31;

    u_int expand : 1;
};

/*max count of memory zone*/
#define BGP4_MAX_MZONE 64 

/*global struct for memory process*/
#define BGP4_MAX_MEMPOOL 8
struct bgp4_memory{
    tBGP4_MEMSTAT stat[MEM_BGP_MAX];

    tBGP4_MEMSTAT histroy_stat[MEM_BGP_MAX];

    struct bgp4_mtable table[BGP4_MAX_MEMPOOL];

    avl_tree_t free_page_list;

    /*allocated zone list*/
    avl_tree_t zone_list;

    /*timer to free not used mzone */
    tTimerNode mzone_timer;
    
    /*total length of buffer*/
    u_int total_len;

    /*total allocated length of buffer,there may be error*/
    u_int allocate_len;

    /*zone buffer allocate failed count*/
    u_int zone_allocate_failed;

    /*total expand len*/
    u_int expand_len;
    
    /*zone resource*/
    struct bgp4_mzone zone[BGP4_MAX_MZONE];
};
static struct bgp4_memory *p_memory = NULL;

/*for a special length of memory ,increase most matched unit count of memory*/
#define bgp4_increase_memory(s, c) do{p_memory->total_len += (s)*(c);}while(0)
#define BGP4_EXPAND_MBLOCK_SIZE 4*1024*1024
#define BGP4_MEMORY_TIMER_INTERVAL 3600
#define BGP4_MEMORY_INIT_SPLIT_SIZE 32*1024*1024

/*memory zone managment*/
/*compare of zone,use the laster buffer pointer as key*/
static int
bgp4_mzone_cmp(
    struct bgp4_mzone *p1,
    struct bgp4_mzone *p2)
{
    u_long end1 = (u_long)p1->buf + p1->size;
    u_long end2 = (u_long)p2->buf + p2->size;
    if (end1 != end2)
    {
        return (end1 > end2) ? 1 : -1;
    }
    return 0;
}

/*create a zone*/
static struct bgp4_mzone *
bgp4_mzone_create(
     u_char *buf,
     u_int size)
{
    u_int i;
    if ((buf == NULL) || (size == 0))
    {
        return NULL;
    }
    
    for (i = 0 ; i < BGP4_MAX_MZONE; i++)
    {
        if (p_memory->zone[i].buf == NULL)
        {
            p_memory->zone[i].buf = buf;
            p_memory->zone[i].size = size;
            bgp4_avl_add(&p_memory->zone_list, &p_memory->zone[i]);
            return &p_memory->zone[i];
        }
    }
    return NULL;
}

/*release a zone*/
static void
bgp4_mzone_free(struct bgp4_mzone *p)
{
    if (p->buf == NULL)
    {
        return;
    }
    bgp4_avl_delete(&p_memory->zone_list, p);
    p->buf = NULL;
    p->size = 0;
    return;
}

/*decide matched zone for special pointer,use when release memory*/
static struct bgp4_mzone *
bgp4_mzone_match(void *p)
{
    struct bgp4_mzone search;
    /*get zone which pointer is larger then input pointer*/
    search.buf = p;
    search.size = 0;
    return bgp4_avl_greator(&p_memory->zone_list, &search);
}

/*construct zone list for special memory size,we may allocate multiple zone for a large memory request*/
static void
bgp4_mzone_list_create(u_int total)
{
    struct bgp4_mzone *p_zone = NULL;
    u_char *buf = NULL;
    u_int zone_size = 0;
    u_int allocate = 0;
    u_int rest = 0;
    
    if (total == 0)
    {
        return;
    }
    
    /*decide closet zone size,from the min size to max size*/
    for (zone_size = BGP4_MIN_MZONE_SIZE;
           zone_size <= BGP4_MAX_MZONE_SIZE; 
           zone_size = (zone_size * 2))
    {
        /*if zone size is larger then request one,stop*/
        if ((zone_size * 2) > total)
        {
            break;
        }
    }

    if (total < BGP4_MIN_MZONE_SIZE)
    {
        zone_size = total;
    }
    /*allocate buffer using zone size,if full length allocated,or memory is not enough,stopped*/
    while (allocate < total)
    {
        /*allocate system buffer according to expected zone length*/
        buf = (void *)(gbgp4.malloc)(MTYPE_BGP, zone_size);
        if (buf == NULL)
        {
            p_memory->zone_allocate_failed++;
            /*no buffer for this size,we may decrease zone size,but can not lower than min size*/
            if (zone_size == BGP4_MIN_MZONE_SIZE)
            {
                break;
            }
            /*select a lower size*/
            zone_size = zone_size/2;
            continue;
        }
        memset(buf, 0, zone_size);
        /*allocate zone for this memory*/
        p_zone = bgp4_mzone_create(buf, zone_size);
        if (p_zone == NULL)
        {
            p_memory->zone_allocate_failed++;
            gbgp4.mfree(buf);
            buf = NULL;
            break;
        }
   
        /*zone allocate sucessfully,increase allocate length*/
        allocate += zone_size;
   
        /*get rest length to be allocated*/
        rest = total - allocate;
        /*if zone size larger than rest length,use a shorter zone*/
        while ((zone_size > rest) && (zone_size > BGP4_MIN_MZONE_SIZE))
        {
            zone_size = zone_size/2;
        }
        if (rest < BGP4_MIN_MZONE_SIZE)
        {
            zone_size = rest;
        }
    }
    return;
}

/*called when ospf shutdown,and free all memory*/
static void
bgp4_mzone_list_clear(void)
{
    struct bgp4_mzone *p_zone = NULL;
    struct bgp4_mzone *p_next = NULL;
    bgp4_avl_for_each_safe(&p_memory->zone_list, p_zone, p_next)
    {
        gbgp4.mfree(p_zone->buf);
        bgp4_mzone_free(p_zone);
    }
    return;
}

void
bgp4_memory_check_timeout(void)
{
    struct bgp4_mzone *p_zone = NULL;
    struct bgp4_mzone *p_next_zone = NULL;
    struct bgp4_mpage *page = NULL;
    u_int used = FALSE;
    u_int page_count = 0;
    u_int i = 0;
    
    bgp4_avl_for_each_safe(&p_memory->zone_list, p_zone, p_next_zone)
    {
        used = FALSE;
        if (p_zone->expand == TRUE)
        {
            page = (struct bgp4_mpage *)p_zone->buf;
            page_count = p_zone->size/BGP4_PAGE_SIZE;
            
            for (i = 0 ; i < page_count ; i++)
            {    
                /*this mzone have page used ,can't free*/
                if (page->free != page->total)
                {
                     used = TRUE;
                     break;
                }
                page = (struct bgp4_mpage *)((u_char *)page + BGP4_PAGE_SIZE);
            }
            /*page all free ,can free this mzone*/
            if (used == FALSE)
            {
                page = (struct bgp4_mpage *)p_zone->buf;
                for (i = 0 ; i < page_count ; i++)
                {    
                    if (page->p_mtable)
                    {
                        bgp4_avl_delete(&page->p_mtable->page_list, page);
                        page->p_mtable->page_free++; 
                        page->p_mtable = NULL;
                    }
                    else
                    {
                        bgp4_avl_delete(&p_memory->free_page_list, page);  
                    }
                    page->total = 0;
                    page->free = 0;
                    page->p_free = NULL;  
                    page = (struct bgp4_mpage *)((u_char *)page + BGP4_PAGE_SIZE);
                }
                gbgp4.mfree(p_zone->buf);
                bgp4_mzone_free(p_zone);
            }
        }
    }
    bgp4_timer_start(&p_memory->mzone_timer, BGP4_MEMORY_TIMER_INTERVAL);
    return ;
}

static void
bgp4_memory_expand(void)
{
    struct bgp4_mzone *p_zone = NULL;
    struct bgp4_mpage *page = NULL;
    u_int page_count = 0;    
    u_int i = 0;
    u_char *buf = NULL;
    u_int expand_size = BGP4_EXPAND_MBLOCK_SIZE;

    buf = (u_char *)(gbgp4.malloc)(MTYPE_BGP, expand_size);
    if (NULL == buf)
    {
        return;
    }
    p_zone = bgp4_mzone_create(buf, expand_size);
    if (NULL == p_zone)
    {
        p_memory->zone_allocate_failed++;
        gbgp4.mfree(buf);
        buf = NULL;
        return;
    }
    p_zone->expand = TRUE;
    memset(buf, 0, expand_size);
    
    /*construct free page list*/
    page = (struct bgp4_mpage *)buf;
    page_count = expand_size/BGP4_PAGE_SIZE;
    
    for (i = 0 ; i < page_count ; i++)
    {
        bgp4_avl_add(&p_memory->free_page_list, page);
        page = (struct bgp4_mpage *)((u_char *)page + BGP4_PAGE_SIZE);
    }
    p_memory->expand_len += expand_size;
    bgp4_timer_start(&p_memory->mzone_timer, BGP4_MEMORY_TIMER_INTERVAL);
    return;
}

/*prepare to init memory.called when bgp is started*/
void 
bgp4_mem_init(
       u_int max_route,
       u_int max_path,
       u_int max_peer,
       u_int max_vpn_instance)
{
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mzone *p_zone = NULL;
    struct bgp4_mzone *p_next = NULL;
    u_int page_count = 0;
    u_int i;

    /*create memory information*/
    if (NULL == p_memory)
    {
        p_memory = (struct bgp4_memory *)malloc(sizeof(struct bgp4_memory));
        if (NULL == p_memory)
        {
            bgp4_log(BGP_DEBUG_ERROR,"memory init failed");
            return;
        }
        memset(p_memory, 0, sizeof(struct bgp4_memory));
        for (i = 0 ; i < BGP4_MAX_MEMPOOL; i++)
        {
            bgp4_unsort_avl_init(&p_memory->table[i].page_list);
            bgp4_unsort_avl_init(&p_memory->table[i].full_page_list);
        }
        bgp4_unsort_avl_init(&p_memory->free_page_list);
        bgp4_avl_init(&p_memory->zone_list, bgp4_mzone_cmp);
        bgp4_timer_init(&p_memory->mzone_timer, bgp4_memory_check_timeout, NULL);
    }

    /*reset statistics*/
    memset(p_memory->stat, 0, sizeof(p_memory->stat));

    p_memory->zone_allocate_failed = 0;
    
    /*init fixed length of all mblocks*/
    p_memory->table[0].size = 24;
    p_memory->table[1].size = 64;
    p_memory->table[2].size = ((sizeof(tBGP4_ROUTE)+3)/4)*4;/*route table*/
    p_memory->table[3].size = 160;
    p_memory->table[4].size = 320;
    p_memory->table[5].size = 1500;
    p_memory->table[6].size = 5000;
    p_memory->table[7].size = BGP4_MAX_VRF_ID*12 + 4;
    
    p_memory->total_len = 0;
    
    /*calculate memory object count for each mblock size*/
    /*route link node*/
    bgp4_increase_memory(sizeof(tBGP4_LINK), max_route/20);

    /*network/range...*/
    bgp4_increase_memory(sizeof(tBGP4_NETWORK), max_path + max_path/10);

    /*route*/
    bgp4_increase_memory(sizeof(tBGP4_ROUTE), max_route + max_route/2 + max_route/20);

    /*path*/
    bgp4_increase_memory(sizeof(tBGP4_PATH), max_path + max_path/10);

    /*peer*/
    bgp4_increase_memory(sizeof(tBGP4_PEER), max_peer);

    /*instance*/
    bgp4_increase_memory(sizeof(tBGP4_VPN_INSTANCE), max_vpn_instance);

    /*rib*/
    bgp4_increase_memory(sizeof(tBGP4_RIB), max_vpn_instance + max_vpn_instance/2);

    /*route flooding*/
    bgp4_increase_memory(sizeof(tBGP4_FLOOD_FLAG), max_route);

    bgp4_increase_memory(sizeof(tBGP4_EXT_FLOOD_FLAG), max_route/2);

    /*route msg*/
    bgp4_increase_memory(1200, max_vpn_instance*2);

    /*rxd buffer*/
    bgp4_increase_memory(BGP4_MAX_MSG_LEN, 20);

    /*vpn lookup msg*/
    bgp4_increase_memory(BGP4_MAX_VRF_ID*12, 4);

    /*policy*/
    bgp4_increase_memory(sizeof(tBGP4_REDISTRIBUTE_CONFIG), max_vpn_instance + max_vpn_instance/2);    

    /*aligment according to page*/
    p_memory->total_len = ((p_memory->total_len / BGP4_PAGE_SIZE) + 1) * BGP4_PAGE_SIZE;

    /*construct memory zone list*/
    if (p_memory->total_len > BGP4_MEMORY_INIT_SPLIT_SIZE)        
    {
        bgp4_mzone_list_create(p_memory->total_len/4);
    }
    else
    {
        bgp4_mzone_list_create(p_memory->total_len);
    }

    /*split all memory into pages*/
    bgp4_avl_for_each_safe(&p_memory->zone_list, p_zone, p_next)
    {
        /*construct free page list*/
        p_page = (struct bgp4_mpage *)p_zone->buf;
        page_count = p_zone->size/BGP4_PAGE_SIZE;
        
        for (i = 0 ; i < page_count ; i++)
        {
            bgp4_avl_add(&p_memory->free_page_list, p_page);
            p_page = (struct bgp4_mpage *)((u_char *)p_page + BGP4_PAGE_SIZE);
        }
    }
    return;
}

/*delete all memory table*/
void 
bgp4_mem_deinit(void)
{
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mpage *p_next = NULL;
    u_int i = 0;

    /*clear all page list*/
    for (i = 0 ; i < BGP4_MAX_MEMPOOL; i++)
    {
        bgp4_avl_for_each_safe(&p_memory->table[i].page_list, p_page, p_next)
        {
            bgp4_avl_delete(&p_memory->table[i].page_list, p_page);
            p_memory->table[i].page_free++;
        }
        bgp4_avl_for_each_safe(&p_memory->table[i].full_page_list, p_page, p_next)
        {
            bgp4_avl_delete(&p_memory->table[i].full_page_list, p_page);
            p_memory->table[i].page_free++;
        }
    }
 
    /*clear free page list*/
    bgp4_avl_for_each_safe(&p_memory->free_page_list, p_page, p_next)
    {
        bgp4_avl_delete(&p_memory->free_page_list, p_page);
    }
    /*free global buffer*/
    bgp4_mzone_list_clear();
    return;
}

static struct bgp4_mpage *
bgp4_page_alloc(struct bgp4_mtable *p_table)
{
    struct bgp4_mobject *p_hdr = NULL;
    struct bgp4_mobject *p_next = NULL;
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mpage *p_nextpage = NULL;
    u_int total = 0;
    u_int i;

    if (!p_table->size)
    {
        return NULL;
    }
    total = (uint16_t)((BGP4_PAGE_SIZE - sizeof(struct bgp4_mpage)) / p_table->size);
    if (total < 1)
    {
        return NULL;
    }

    /*no free object,try to get pages from free list*/
    p_page = bgp4_avl_first(&p_memory->free_page_list);
    if (!p_page)
    {
        /*try to get free page from other table*/
        for (i = 0 ; i < BGP4_MAX_MEMPOOL; i++)
        {
            bgp4_avl_for_each_safe(&p_memory->table[i].page_list, p_page, p_nextpage)
            {
                if (p_page->total == p_page->free)
                {
                    bgp4_avl_delete(&p_memory->table[i].page_list, p_page);
                    bgp4_avl_add(&p_memory->free_page_list, p_page);
                    p_page->p_mtable->page_free++;
                    p_page->p_mtable = NULL;
                }
            }
        }      
    }
    p_page = bgp4_avl_first(&p_memory->free_page_list);
    if (!p_page)
    {
        /*allocate burst buffer */ 
        bgp4_memory_expand();
        p_page = bgp4_avl_first(&p_memory->free_page_list);
        if (NULL == p_page)
        {
            return NULL;
        }       
    }
    bgp4_avl_delete(&p_memory->free_page_list, p_page);
    bgp4_avl_add(&p_table->page_list, p_page);
    p_page->total = total;
    
    p_hdr = (struct bgp4_mobject *)(p_page + 1);
    p_page->p_free = p_hdr;
    p_hdr->p_next = NULL;
    p_page->free = p_page->total;    
    
    p_page->p_mtable = p_table;

    for (i = 0 ; i < (p_page->total - 1); i++)
    {
        p_next = (void*)(((u_long)p_hdr) + p_table->size);
        p_hdr->p_next = p_next;
        p_next->p_next = NULL;
        p_hdr = p_next;
    }
    p_table->page_alloc++;
    return p_page;
}

/*ospf_malloc:allocate one buffer unit from buffer table*/ 
void *
bgp4_malloc(
         u_int len, 
         u_int type)
{
    struct bgp4_mtable *p_table = NULL;
    struct bgp4_mpage *p_page = NULL;
    u_int i;
    u_char *p_buf = NULL;

    p_memory->stat[type].add++;
    p_memory->histroy_stat[type].add++;

    /*get matched memory table*/
    for (i = 0 ; i < BGP4_MAX_MEMPOOL; i++)
    {
        if (p_memory->table[i].size >= len)
        {
            p_table = &p_memory->table[i];
            break;
        }
    }
    
    /*no matched pool, do nothing*/
    if (!p_table)
    {
        goto FAIL;
    }

    /*get first page,if not exist,try to allocate a page from free list*/
    p_page = bgp4_avl_first(&p_table->page_list);
    if (!p_page)
    {
        p_page = bgp4_page_alloc(p_table);        
        if (!p_page)
        {
            goto FAIL;
        }
    }

    p_buf = (void *)p_page->p_free;
    p_page->p_free = p_page->p_free->p_next;
    p_page->free--;
    memset(p_buf, 0, len);

    /*if no free memory,add to full list*/
    if (!p_page->p_free)
    {
        bgp4_avl_delete(&p_table->page_list, p_page);
        bgp4_avl_add(&p_table->full_page_list, p_page);
    }
    return p_buf;        
FAIL:
    p_memory->stat[type].fail++;
    p_memory->histroy_stat[type].fail++;
    return NULL;
}


/*release one buffer unit,insert it back into free list*/
void 
bgp4_free(
    void *ptr, 
    u_int type)
{   
    struct bgp4_mobject *p_hdr = (struct bgp4_mobject *)ptr;
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mzone *p_zone = NULL;
    u_long offset = 0;
    
    p_memory->stat[type].del++;
    p_memory->histroy_stat[type].del++;

    /*decide zone it belong to*/
    p_zone = bgp4_mzone_match(ptr);
    if (p_zone == NULL)
    {
       bgp4_log(BGP_DEBUG_ERROR,"no memory zone matched to free buffer %x",(int)ptr);
       return;
    }
    
    /*decide page*/
    offset = ((u_long)ptr-(u_long)p_zone->buf)/BGP4_PAGE_SIZE;
    p_page = (struct bgp4_mpage *)(p_zone->buf + offset*BGP4_PAGE_SIZE);

    /*add to free list*/
    p_hdr->p_next = p_page->p_free;
    p_page->p_free = p_hdr;
    p_page->free++;

    if (p_page->p_mtable)
    {
         /*if free count is 1,add to page list*/
         if (p_page->free == 1)
         {
             bgp4_avl_delete(&p_page->p_mtable->full_page_list, p_page);
             bgp4_avl_add(&p_page->p_mtable->page_list, p_page);
         }
    }
    return;
}
/*calculate total used memory*/
static u_int
bgp4_memory_used(void)
{
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mpage *p_next = NULL;
    struct bgp4_mtable *p_table;
    u_int used = 0;
    u_int i;

    for (i = 0; i < BGP4_MAX_MEMPOOL; i++)        
    {
        p_table = &p_memory->table[i];

        bgp4_avl_for_each_safe(&p_table->page_list, p_page, p_next)
        {
            used += (p_page->total - p_page->free) * p_table->size;
        }
        used += bgp4_avl_count(&p_table->full_page_list) * BGP4_PAGE_SIZE;
    }
    return used;
}

void
bgp4_display_memory(void *vty)
{
    struct bgp4_mtable *p_table;
    struct bgp4_mpage *p_page = NULL;
    struct bgp4_mpage *p_next = NULL;
    struct bgp4_mzone *p_zone = NULL;
    u_int i = 0;
    u_int total = 0;
    u_int free_count = 0;
    u_int emptypage = 0;
    u_int allocate = 0;
    u_int used = 0;
    if (p_memory == NULL)
    {
        return;
    }
    total = p_memory->total_len;
    vty_out(vty, "\n\rResource Configuration\n\r");

    vty_out(vty, "   Max Route:%d\n\r", gbgp4.max_route);
    vty_out(vty, "   Max Path:%d\n\r", gbgp4.max_path);
    vty_out(vty, "   Max Peer:%d\n\r", gbgp4.max_peer);
    vty_out(vty, "   Max Instance:%d\n\r", gbgp4.max_vpn_instance);

    vty_out(vty, "\n\r Memory Block Table:expected %dbytes\n\r", total);

    vty_out(vty, "\n\r Memory Table Size %d,Zone Count %d\n\r", 
        sizeof(struct bgp4_memory), (int)bgp4_avl_count(&p_memory->zone_list));    

    vty_out(vty, " Page size %d,Empty page count %d\n\r", BGP4_PAGE_SIZE, (int)bgp4_avl_count(&p_memory->free_page_list));  

    vty_out(vty, "\n\r Memory Zone Table\n\r");
    vty_out(vty, " %-5s%-16s%-s\n\r","id","size","buffer");
    for (i = 0 ; i < BGP4_MAX_MZONE ; i++)
    {
        p_zone = &p_memory->zone[i];
        if (p_zone->buf)
        {
            vty_out(vty, " %-5d%-16d%x\n\r",(int)i,(int)p_zone->size,(int)p_zone->buf);
            allocate += p_zone->size;
        }
    }
    used = bgp4_memory_used();
    vty_out(vty, "\n\r Expected/Alloc/Used:%d/%d/%d bytes\n\r", 
        (int)p_memory->total_len,
        (int)allocate,
        (int)used);

    /*len count free percent*/
    vty_out(vty, "\n\r Memory Table\n\r");    
    vty_out(vty, " %-5s%-6s%-6s%-10s%-10s%-10s%-10s%-10s%-10s\n\r",
          "id","size","page","fullpage","emptypage","allocpage","freepage","total","avaliable");

    for (i = 0 ; i < BGP4_MAX_MEMPOOL; i++)
    {
        p_table = &p_memory->table[i];

        free_count = 0;
        total = 0;
        bgp4_avl_for_each_safe(&p_table->page_list, p_page, p_next)
        {
            free_count += p_page->free;
            total += p_page->total;
        }
        bgp4_avl_for_each_safe(&p_table->full_page_list, p_page, p_next)
        {
            total += p_page->total;
        }
        emptypage = 0;
        bgp4_avl_for_each_safe(&p_table->page_list, p_page, p_next)
        {
            if (p_page->free == p_page->total)
            {
                emptypage++;
            }
        }
        
        vty_out(vty, " %-5d%-6d%-6d%-10d%-10d%-10d%-10d%-10d%-10d\n\r",
        (int)(i+1),
        (int)p_table->size,
        (int)bgp4_avl_count(&p_table->page_list),
        (int)bgp4_avl_count(&p_table->full_page_list),
        (int)emptypage,
        (int)p_table->page_alloc,(int)p_table->page_free, (int)total, (int)free_count); 
    }
    
    /*each type of memory*/
    vty_out(vty, "\n\rMemory Type Usage\n\r");

    /*create fail delete current len name*/
    vty_out(vty, "  %-10s%-8s%-10s%-10s%-5s%s\n\r", 
          "create", "fail", "delete", "current", "len","name");
    
    #define BGP4_MSHOW(type, len, name) \
    do{\
        vty_out(vty, "  %-10d%-8d%-10d%-10d%-5d%s\n\r",\
          p_memory->stat[type].add,\
          p_memory->stat[type].fail,\
          p_memory->stat[type].del,\
          p_memory->stat[type].add - p_memory->stat[type].fail - p_memory->stat[type].del,\
          len, name);}while(0)

    BGP4_MSHOW(MEM_BGP_VPN_INSTANCE, sizeof(tBGP4_VPN_INSTANCE), "VpnInstance");
    BGP4_MSHOW(MEM_BGP_PEER, sizeof(tBGP4_PEER), "Peer");
    BGP4_MSHOW(MEM_BGP_INFO, sizeof(tBGP4_PATH), "Path");
    BGP4_MSHOW(MEM_BGP_ASPATH, sizeof(tBGP4_ASPATH), "ASPath");
    BGP4_MSHOW(MEM_BGP_ROUTE, sizeof(tBGP4_ROUTE), "Route");
    BGP4_MSHOW(MEM_BGP_LINK, sizeof(tBGP4_LINK), "RouteLink");
    BGP4_MSHOW(MEM_BGP_NETWORK, sizeof(tBGP4_NETWORK), "Network");
    BGP4_MSHOW(MEM_BGP_RANGE, sizeof(tBGP4_RANGE), "Range");    
    BGP4_MSHOW(MEM_BGP_FLOOD, sizeof(tBGP4_FLOOD_FLAG), "FloodFlag");    
    BGP4_MSHOW(MEM_BGP_EXTFLOOD, sizeof(tBGP4_EXT_FLOOD_FLAG), "ExtFlood");
    BGP4_MSHOW(MEM_BGP_RXMSG, 0, "Rxbuf");    
    BGP4_MSHOW(MEM_BGP_BUF, 0, "Buffer");
    BGP4_MSHOW(MEM_BGP_NEXTHOP, sizeof(tBGP4_NEXTHOP_BLOCK), "Nexthop");
    BGP4_MSHOW(MEM_BGP_DAMP, sizeof(tBGP4_DAMP_ROUTE), "DampRoute");
    BGP4_MSHOW(MEM_BGP_POLICY_CONFIG, sizeof(tBGP4_POLICY_CONFIG), "Policy");    
    BGP4_MSHOW(MEM_BGP_RIB, sizeof(tBGP4_RIB), "RIB");
    BGP4_MSHOW(MEM_BGP_ORF, sizeof(tBGP4_ORF_ENTRY), "ORF");
    return;
}

#endif
/*memory support API E*/

u_int 
bgp4_mask2len(u_int mask)
{
    UINT prefixlen = 1;

    if (mask == 0)
    {
            return 0;
    }
    while ((mask << 1) & 0x80000000)
    { 
            mask <<= 1;
            prefixlen++;
    }
    return prefixlen;  
}

u_int 
bgp4_len2mask(u_char masklen)
{  
    if ((masklen <= 0) ||(masklen > 32))
    {
        return 0;
    }
    return(0xFFFFFFFF << (32-masklen));  
}

void 
bgp4_ip6_prefix_make(
        u_char *addr, 
        u_int len)
{
    u_int bytes = 0;
    u_int bits = 0;
    u_int i;
    u_char mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};

    if (len >= 128)
    {
        return;
    }

    bytes = len / 8;
    bits = len % 8;

    /*clear fields after bytes*/
    i = bytes ;
    if (bits != 0)
    {
        /*clear invalid bits*/
        addr[bytes] &= mask[bits];
        i++;
    }
    for (; i < 16 ; i++)
    {
        addr[i] = 0;
    }
    return;
}

/*calculate mask length for special mask.... */
u_int 
bgp4_ip6_mask2len(struct in6_addr *mask)
{ 
    u_char *p_buf = (u_char *)mask;
    u_char flag[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    u_int masklen = 0 ;
    u_int i = 0 ;
    u_int bit = 0 ;

    for (i = 0 ; i < 16; i++)
    {
        if (p_buf[i] == 0xff)
        {
            masklen += 8;
        }
        else 
        {
            for (bit = 1 ; bit < 9; bit ++)
            {
                if (flag[bit] == p_buf[i])
                {
                    return masklen+bit;
                }
            }
        }
    }    
    return masklen;
}

int 
bgp4_subnet_match( 
    tBGP4_ADDR *ip1, 
    tBGP4_ADDR *ip2,
    u_int if_unit)
{  
    u_int prefix_len = 0;
    
    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        return TRUE;
    }
    
    if(ip1 == ip2 || ip1 == NULL || ip2 == NULL)
    {
        return TRUE;
    }        
    prefix_len = bgp4_sys_ifunit_to_prefixlen(if_unit, ip2->afi);
    return (bgp4_prefixmatch(ip1->ip, ip2->ip, prefix_len));
}

int 
bgp4_prefixmatch(
       u_char *p1, 
       u_char *p2, 
       u_int len)
{
    u_int byte = len/8;
    u_int bit = len%8;
    u_char mask[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};

    if (byte && memcmp(p1, p2, byte))
    {
        return FALSE;
    }
    return ((p1[byte] & mask[bit]) == (p2[byte] & mask[bit])) ? TRUE : FALSE;
}

int 
bgp4_prefixcmp(
      tBGP4_ADDR *a1, 
      tBGP4_ADDR *a2)
{
    u_int byte = 0;
    u_int bit = 0;
    u_char mask[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    u_char a1_bit = 0;
    u_char a2_bit = 0;
    int   rc = 0;

    if (a1->afi != a2->afi)
    {
        return (a1->afi > a2->afi) ? 1 : -1;
    }

    /*VPLS:compare full length*/
    if (a1->afi == BGP4_PF_L2VPLS)
    {
        rc = memcmp(a1->ip, a2->ip, BGP4_VPLS_NLRI_LEN);
        if (rc != 0)
        {
            return rc > 0 ? 1 : -1; 
        }
        return 0;
    }
    
    if (a1->prefixlen != a2->prefixlen)
    {
        return (a1->prefixlen > a2->prefixlen) ? 1 : -1;
    }
    
    byte = a1->prefixlen/8;
    bit = a1->prefixlen%8;

    if (byte)
    {
        rc = memcmp(a1->ip, a2->ip, byte);
        if (rc != 0)
        {
            return rc > 0 ? 1 : -1; 
        }
    } 
    
    if (bit == 0)
    {
        return 0;
    }
    
    a1_bit = a1->ip[byte] & mask[bit];
    a2_bit = a2->ip[byte] & mask[bit];

    if (a1_bit == a2_bit)
    {
        return 0;
    }
    return (a1_bit > a2_bit ? 1 : -1);
}

/*translate af index into standard afi/safi*/
u_short 
bgp4_index_to_afi(u_int flag)
{
    u_short afi = 0 ;

    if ((flag == BGP4_PF_IPUCAST) || (flag == BGP4_PF_IPMCAST)
            || (flag == BGP4_PF_IPLABEL) || (flag == BGP4_PF_IPVPN))
    {
        afi = BGP4_AF_IP;
    }
    else if ((flag == BGP4_PF_IP6UCAST) || (flag == BGP4_PF_IP6MCAST)
            || (flag == BGP4_PF_IP6LABEL) || (flag == BGP4_PF_IP6VPN))
    {
        afi = BGP4_AF_IP6;
    }
    else if (flag == BGP4_PF_L2VPLS)
    {
        afi = BGP4_AF_L2VPN;
    }
    return afi;
}

/*translate af flag into afi/safi*/
u_short 
bgp4_index_to_safi(u_int flag)
{
    u_int safi = 0 ;

    if ((flag == BGP4_PF_IPUCAST) || (flag == BGP4_PF_IP6UCAST))
    {
        safi = BGP4_SAF_UCAST;
    }
    else if ((flag == BGP4_PF_IPMCAST) || (flag == BGP4_PF_IP6MCAST))
    {
        safi = BGP4_SAF_MCAST;
    }
    else if ((flag == BGP4_PF_IPLABEL) || (flag == BGP4_PF_IP6LABEL))
    {
        safi = BGP4_SAF_LBL;
    }
    else if ((flag == BGP4_PF_IPVPN) || (flag == BGP4_PF_IP6VPN))
    {
        safi = BGP4_SAF_VLBL;
    }
    else if (flag == BGP4_PF_L2VPLS)
    {
        safi = BGP4_SAF_VPLS;
    }
    return safi;
}

u_int 
bgp4_afi_to_index(
     u_short afi, 
     u_short safi)
{
    u_int i = 0;

    for (i = 0 ; i < BGP4_PF_MAX; i++)
    {
        if ((bgp4_index_to_afi(i) == afi) && (bgp4_index_to_safi(i) == safi))
        {
            return i;
        }
    }   
    return BGP4_PF_MAX;
}

/*shell show func*/
void bgp_show_vrf_routes(u_int vrf_id)
{
    u_char dstr[64],mstr[64],hstr[64];
    tBGP4_ROUTE *p_rt;
    int first =  1;
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf_id);
    u_int af;
    if (p_instance == NULL)
        return;

    bgp_sem_take();

    for (af = 0; af < BGP4_PF_MAX ; af++)
    {
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_rt)
        {
            if (first )
            {
                if (p_rt->dest.afi == BGP4_PF_IPMCAST)
                    printf("\n\r  Address Family:IPv4 Multicast\n\r");
                else
                    printf("\n\r");
                
                printf("  %-16s%-16s%-16s%-7s%-9s%s\n\r","Destination","NetMask","NextHop","MED","LocPref","Proto");
                first = 0;
            }
    
            if(p_rt->dest.afi ==BGP4_PF_IPUCAST)
            {
                        inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip));
                        inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen));
            }
            else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
            {
                    inet_ntop(AF_INET6,p_rt->dest.ip,dstr,64);
            }
            else if(p_rt->dest.afi ==BGP4_PF_IPVPN)
            {
                        inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip + BGP4_VPN_RD_LEN));
                       inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen - 8*BGP4_VPN_RD_LEN));
            }
    
            if (p_rt->p_path)
            {
                if (p_rt->dest.afi == BGP4_PF_IPUCAST || p_rt->dest.afi == BGP4_PF_IPVPN)
                {
                    inet_ntoa_1(hstr,bgp_ip4(p_rt->p_path->nexthop.ip));
                    printf("  %-16s%-16s%-16s%-7d%-9d%s\n\r",
                                    dstr,mstr,hstr,
                                    p_rt->p_path->med,p_rt->p_path->localpref,
                                    bgp4_get_route_desc(p_rt->proto));
                }
                else if (p_rt->dest.afi == BGP4_PF_IP6UCAST)
                {
                    inet_ntop(AF_INET6,p_rt->p_path->nexthop.ip,hstr,64);
                    printf("  %-16s%-16d%-16s%-7d%-9d%s\n\r",
                                    dstr,p_rt->dest.prefixlen,hstr,
                                    p_rt->p_path->med,p_rt->p_path->localpref,
                                    bgp4_get_route_desc(p_rt->proto));
                }
            }
            else
            {
                if (p_rt->dest.afi ==BGP4_PF_IPUCAST || p_rt->dest.afi ==BGP4_PF_IPVPN)
                {
                    printf("  %-16s%-16s%-16s%-7s%-9s%s\n\r",
                                        dstr,mstr,"0.0.0.0",
                     "N/A","N/A",
                                        bgp4_get_route_desc(p_rt->proto));
                }
                else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
                {
                    printf("  %-16s%-16d%-16s%-7s%-9s%s\n\r",
                                        dstr,p_rt->p_path->nexthop.prefixlen,"0.0.0.0",
                      "N/A","N/A",
                                        bgp4_get_route_desc(p_rt->proto));
                }
            }
        }
    }
    bgp_sem_give();
}

void bgp4_show_path()
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_next = NULL ;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_int af = 0;
    u_int path_cnt = 0;
    u_char peer_str[64] = {0};

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        bgp4_avl_for_each_safe(&p_instance->rib[af]->path_table, p_path, p_next)
        {
            path_cnt++;
            if (p_path->p_peer)
            {
                printf("\r\n path %d,af %d,peer %s,route count %d",
                                    path_cnt,
                                    af,
                                    bgp4_printf_peer(p_path->p_peer,peer_str),
                                    (int)bgp4_avl_count(&p_path->route_list));
            }
            else
            {
                printf("\r\n path %d,af %d,no peerroute count %d",
                                    path_cnt,
                                    af,
                                    (int)bgp4_avl_count(&p_path->route_list));
            }

        }
    }
    printf("\r\n TOTAL COUNT %d",path_cnt);
    bgp_sem_give();
    return;
}

void bgp4_set_as_substitution(u_int vrf_id,u_char set_flag)
{
    tBGP4_PEER* p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf_id);

    if (p_instance == NULL)
    {
        return;
    }

    bgp_sem_take();
    bgp4_peer_for_each(p_instance, p_peer)
    {
        p_peer->as_substitute_enable = set_flag;
    }

    bgp_sem_give();
    return;
}

void bgp4_clear_statistics()
{
    /*remain count of memory*/
    
    memset(&gbgp4.stat.sock, 0, sizeof(gbgp4.stat.sock));
    memset(&gbgp4.stat.sys_route, 0, sizeof(gbgp4.stat.sys_route));
    memset(&gbgp4.stat.sys_route6, 0, sizeof(gbgp4.stat.sys_route6));
    memset(&gbgp4.stat.mpls_route, 0, sizeof(gbgp4.stat.mpls_route));
    memset(&gbgp4.stat.sync, 0, sizeof(gbgp4.stat.sync));    
    memset(&gbgp4.stat.msg_tx, 0, sizeof(gbgp4.stat.msg_tx)); 
    memset(&gbgp4.stat.msg_rx, 0, sizeof(gbgp4.stat.msg_rx)); 
    gbgp4.stat.vrf_not_exist = 0;
    return;
}

/*1/4 rand offset*/
#define BGP_RAND_RATE 4

u_int
bgp4_rand(u_int base)
{
    u_int a = rand();
    
    if (base < BGP_RAND_RATE)
    {
       return base;
    }
    if (a%2)
    {
       return base + a%(base/BGP_RAND_RATE);
    }
    return base - a%(base/BGP_RAND_RATE);
}
void
bgp4_show_detail_route(u_int dest)    
{
    tBGP4_VPN_INSTANCE *p_instance;
    tBGP4_RIB *p_rib = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int af = 0;
    u_char str[64];
    
    bgp_sem_take();
    
    bgp4_avl_for_each(&gbgp4.instance_table, p_instance)
    {
        for (af = 0; af < BGP4_PF_MAX ; af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            p_rib = p_instance->rib[af];
            bgp4_avl_for_each(&p_rib->rib_table, p_route)
            {
                if (bgp_ip4(p_route->dest.ip) == dest)
                {
                    printf("\n\r Route:%s\n\r", 
                        bgp4_printf_route(p_route, str));
                    printf("   Path:%x\n\r", (unsigned int)p_route->p_path);
                    printf("   Table:%x\n\r", (unsigned int)p_route->p_table);
                    printf("   Proto:%d\n\r", p_route->proto);
                    printf("   Inlabel:%d\n\r", p_route->in_label);
                    printf("   Outlabel:%d\n\r", p_route->out_label);
                    printf("   IsSummary:%d\n\r", p_route->summary_route);
                    printf("   ActiveSummary:%d\n\r", p_route->active);
                    printf("   FilterbySymmary:%d\n\r", p_route->summary_filtered);
                    printf("   Stale:%d\n\r", p_route->stale);
                    printf("   TobeDelete:%d\n\r", p_route->is_deleted);
                    printf("   SystemSelect:%d\n\r", p_route->system_selected);
                    printf("   SystemFiltered:%d\n\r", p_route->system_filtered);
                    printf("   NoNexthop:%d\n\r", p_route->system_no_nexthop);
                    printf("   InKernal:%x\n\r", p_route->in_kernalx);
                    printf("   InHardware:%x\n\r", p_route->in_hardwarex);
                    printf("   SyncWait:%d\n\r", p_route->igp_sync_wait);
                    printf("   Withdraw:%x\n\r", (unsigned int)p_route->p_withdraw);
                    printf("   Feasible:%x\n\r", (unsigned int)p_route->p_feasible);

                    if (p_route->p_path)
                    {
                        p_path = p_route->p_path;
                        printf("   Path AF:%d\n\r", p_path->af);
                        printf("   Path Instance:%d\n\r", p_path->p_instance->vrf);
                        printf("   Origin VRF:%d\n\r", p_path->origin_vrf);
                        if (p_path->p_peer)
                        {
                            printf("   Peer:%s\n\r", bgp4_printf_peer(p_path->p_peer, str));
                        }
                        printf("   Path Route Count:%d\n\r", (int)bgp4_avl_count(&p_path->route_list));
                        printf("   Nexthop:%s\n\r", bgp4_printf_addr(&p_path->nexthop, str));
                        if (p_path->p_direct_nexthop)
                        {
                            printf("   Direct Nexthop:%s\n\r",
                                bgp4_printf_addr(&p_path->p_direct_nexthop->ip[0], str));
                        }
                    }
                }
            }
        }
    }
    bgp_sem_give();
    return;    
}

#else

#include "bgp4com.h"


/*init close timerlist*/
int bgp4_shutdown_timer()
{
    timerListDelete(gBgp4.p_timerlist);
    gBgp4.p_timerlist = NULL ;
    return TRUE;
}

/*init timer list*/
int 
bgp4_init_timer_list(void)
{
    gBgp4.p_timerlist  = timerListCreate();

    return gBgp4.p_timerlist ? TRUE : FALSE ;
}

/*close timerlist*/
void
bgp4_shutdown_timer_list(void)
{
    timerListDelete(gBgp4.p_timerlist);
    gBgp4.p_timerlist = NULL ;
    return;
}

/*check if any timer expired*/
void 
bgp4_check_timer_list(void)
{
    timerListCheck(gBgp4.p_timerlist,50);
    return ;
}

/*peer timer expired callback function,call peer fsm according to special event*/
void 
bgp4_peer_timer_expired(
                   void* arg1,/*event*/
                   void* arg2,/*peer pointer*/
                   void* arg3/*timer pointer*/)
{
    bgp4_fsm((tBGP4_PEER *)arg2, (u_int)arg1);
    return ;
}

/*start peer timer*/
void 
bgp4_start_peer_timer(
                  tTimerNode *p_timer, 
                  tBGP4_PEER* p_peer, 
                  u_int duration,
                  u_int event)
{
    timerStop(gBgp4.p_timerlist, p_timer);

    timerStart(gBgp4.p_timerlist, p_timer, 
        duration*gBgp4.timerate,
        bgp4_peer_timer_expired, (void *)event,  p_peer, p_timer);

    return;
}

void
bgp4_stop_timer(tTimerNode *p_timer)
{               
    timerStop(gBgp4.p_timerlist, p_timer);
    return ;
}




void bgp4_deferral_timer_expire(void* arg1,void* arg2,void* arg3)
{
    bgp4_gr_deferral_seletion_timeout();
    return;
}



/*deferral_timer,global*/
u_int bgp4_deferral_timer_start(tTimerNode *p_timer,u_int duration)
{
    int rc = VOS_OK;

        /*To avoid starting a timer which has not expired,stop it first*/   
    timerStop(gBgp4.p_timerlist, p_timer);

    rc = timerStart(gBgp4.p_timerlist, p_timer, 
            duration*gBgp4.timerate,
            bgp4_deferral_timer_expire,(void*)NULL, (void*)NULL, (void*)p_timer);

    if(rc == VOS_ERR)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 deferral timer start failed!");  
    }

    bgp4_log(BGP_DEBUG_EVT,1,"bgp4 deferral timer start!expire time is %d",duration);
   
    return rc;
}


u_char bgp4_mask2len(u_int mask)
{
    UINT prefixlen = 1;
    
    if (mask == 0)
    {
        return 0;
    }
    while ((mask << 1) & 0x80000000)
    { 
        mask <<= 1;
        prefixlen++;
    }
    return prefixlen;  
}

u_int bgp4_len2mask(u_char masklen)
{  
    if ((masklen <= 0) ||(masklen > 32))
    {
        return 0;
    }
    
    return(0xFFFFFFFF << (32-masklen));  
}

#ifdef BGP_IPV6_WANTED
void bgp4_ip6_prefix_make(u_char *addr, u_int len)
{
    u_int bytes = 0 ;
    u_int bits = 0 ;
    u_int i ;
    u_char mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
    
    if (len >= 128)
    {
        return ;
    }
   
    bytes = len / 8;
    bits = len % 8;

    /*clear fields after bytes*/
    i = bytes ;
    if (bits != 0)
    {
        /*clear invalid bits*/
        addr[bytes] &= mask[bits];
        i++;
    }
    for (; i < 16 ; i++)
    {
        addr[i] = 0 ;
    }
    return ;
}

/*calculate mask length for special mask.... */
u_int bgp4_ip6_mask2len(struct in6_addr *mask)
{ 
    u_char *p_buf = (u_char *)mask;
    u_char flag[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    u_int masklen = 0 ;
    u_int i = 0 ;
    u_int bit = 0 ;

    for (i = 0 ; i < 16; i++)
    {
        if (p_buf[i] == 0xff)
        {
            masklen += 8;
        }
        else 
        {
            for (bit = 1 ; bit < 9; bit ++)
            {
                if (flag[bit] == p_buf[i])
                {
                    return masklen+bit;
                }
            }
        }
    }    
    return masklen ;
}
#endif
int bgp4_subnet_match( tBGP4_ADDR* ip1, tBGP4_ADDR*  ip2,u_int if_unit)
{  
    u_int prefix_len = 0;
    
    if(gBgp4.work_mode == BGP4_MODE_SLAVE)
    {
        return TRUE;
    }
    
    if(ip1 == ip2 || ip1 == NULL || ip2 == NULL)
    {
        return TRUE;
    }   

    
    prefix_len = bgp4_get_ifmask(if_unit,ip2->afi);
    
    
    return (bgp4_prefixmatch(ip1->ip,ip2->ip,prefix_len));
}

int bgp4_prefixmatch(u_char *p1, u_char *p2, u_int len)
{
    u_int byte = len/8;
    u_int bit = len%8;
    u_char mask[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
   
    if (byte && memcmp(p1, p2, byte))
    {
        return FALSE;
    }
    return ((p1[byte] & mask[bit]) == (p2[byte] & mask[bit])) ? TRUE : FALSE;
}

int bgp4_prefixcmp(tBGP4_ADDR *a1, tBGP4_ADDR *a2)
{
    u_int byte = 0;
    u_int bit = 0;
    u_char mask[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    u_char a1_bit = 0;
    u_char a2_bit = 0;
    int   rc = 0;

    if (a1->afi != a2->afi)
    {
        return (a1->afi > a2->afi) ? 1 : -1;
    }
    
    if (a1->prefixlen != a2->prefixlen)
    {
        return (a1->prefixlen > a2->prefixlen) ? 1 : -1;
    }
    
    byte = a1->prefixlen/8;
    bit = a1->prefixlen%8;

    if(byte)
    {
        rc = memcmp(a1->ip, a2->ip, byte);
        if (rc != 0)
        {
            return rc > 0 ? 1 : -1; 
        }
    } 
    
    if(bit == 0)
    {
        return 0;
    }
    
    a1_bit = a1->ip[byte] & mask[bit];
    a2_bit = a2->ip[byte] & mask[bit];
   
    if(a1_bit == a2_bit)
    {
        return 0;
    }
    
    return (a1_bit > a2_bit ? 1:-1);
}

/*translate af index into standard afi/safi*/
u_short bgp4_index_to_afi(u_int flag)
{
    u_short afi = 0 ;
   
    if ((flag == BGP4_PF_IPUCAST) || (flag == BGP4_PF_IPMCAST)
        || (flag == BGP4_PF_IPLABEL) || (flag == BGP4_PF_IPVPN))
    {
        afi = BGP4_AF_IP ;
    }
    else if ((flag == BGP4_PF_IP6UCAST) || (flag == BGP4_PF_IP6MCAST)
        || (flag == BGP4_PF_IP6LABEL) || (flag == BGP4_PF_IP6VPN))
    {
        afi = BGP4_AF_IP6 ;
    }
    else if (flag == BGP4_PF_L2VPLS)
    {
        afi = BGP4_AF_L2VPN ;
    }
    return afi;
}

/*translate af flag into afi/safi*/
u_short bgp4_index_to_safi(u_int flag)
{
    u_int safi = 0 ;

    if ((flag == BGP4_PF_IPUCAST) || (flag == BGP4_PF_IP6UCAST))
    {
        safi = BGP4_SAF_UCAST;
    }
    else if ((flag == BGP4_PF_IPMCAST) || (flag == BGP4_PF_IP6MCAST))
    {
        safi = BGP4_SAF_MCAST;
    }
    else if ((flag == BGP4_PF_IPLABEL) || (flag == BGP4_PF_IP6LABEL))
    {
        safi = BGP4_SAF_LBL;
    }
    else if ((flag == BGP4_PF_IPVPN) || (flag == BGP4_PF_IP6VPN))
    {
        safi = BGP4_SAF_VLBL;
    }
    else if (flag == BGP4_PF_L2VPLS)
    {
        safi = BGP4_SAF_VPLS;
    }
    return safi;
}

u_int bgp4_afi_to_index(u_short afi, u_short safi)
{
    u_int i = 0 ;

    for (i = 0 ; i < BGP4_PF_MAX; i++)
    {
        if ((bgp4_index_to_afi(i) == afi) && (bgp4_index_to_safi(i) == safi))
        {
            return i;
        }
    }   
    return BGP4_PF_MAX ;
}
#if 0
tBGP4_LISTNODE* bgp4_lstfirst(tBGP4_TREE* p_list)
{
    tBGP4_LISTNODE* p_first=NULL;
    
    if(p_list==NULL)
    {
        return NULL;
    }

    if(list_empty( p_list))
    {
        return NULL;
    }
    
    p_first=(tBGP4_LISTNODE*)p_list->next;

    return p_first;
}

tBGP4_LISTNODE* bgp4_lstnext(tBGP4_TREE* p_list, tBGP4_LISTNODE* p_node)
{   
    if(p_list==NULL||p_node==NULL||p_node->next==NULL)
    {
        return NULL;
    }

    if(list_is_last(p_node,p_list))
    {
        return NULL;
    }
    else
    {
        return p_node->next;
    }
}

u_int bgp4_lstcnt(tBGP4_TREE* p_list)
{
    u_int count=0;
    tBGP4_LISTNODE* p_node=NULL;

    if(p_list==NULL)
    {
        return 0;
    }
    
    LST_EACH(p_list, p_node)
    {
        count++;
    }

    return count;
}
#endif
/*memory*/
#define BGP_MAX_BUF_TYPE 6
struct bgp_mem_object{
   struct bgp_mem_object *p_next;
};

struct bgp_mem_list{
   u_int len;
   u_int count;
   void *p_buf;
   struct bgp_mem_object *p_free;
};

struct bgp_mem_list bgp_mlist[BGP_MAX_BUF_TYPE];

int bgp4_mem_init(
                           u_int max_route,
                           u_int max_path,
                           u_int max_peer,
                           u_int max_vpn_instance
                             )
{
    u_int len = 0 ;
    u_int i = 0 ;
    u_int count = 0 ;    
    struct bgp_mem_list *p_list;
    struct bgp_mem_object *p_obj;
     
    /*Index 0*/
    len = sizeof(tBGP4_ASPATH);
    if (len < sizeof(tBGP4_LINK))    
    {
        len = sizeof(tBGP4_LINK);
    }
    bgp_mlist[0].len = len;
    bgp_mlist[0].count = max_route*4+1000;


    /*index 1*/     
    len = sizeof(tBGP4_AGGR);
    if (len < sizeof(tBGP4_EXT_COMM))   
    {
        len = sizeof(tBGP4_EXT_COMM);
    }
    if (len < sizeof(tBGP4_NETWORK))
    {
        len = sizeof(tBGP4_NETWORK);
    }
    if(len < sizeof(tBGP4_POLICY_CONFIG))
    {
        len = sizeof(tBGP4_POLICY_CONFIG);
    }
    
    bgp_mlist[1].len = len;
    bgp_mlist[1].count = max_path + max_path/10;
    
    /*index2*/
    bgp_mlist[2].len = sizeof(tBGP4_ROUTE);
    bgp_mlist[2].count = max_route + max_route/10;

    /*index 3*/
    bgp_mlist[3].len = sizeof(tBGP4_PATH);
    bgp_mlist[3].count = max_path + max_path/10;

    bgp_mlist[4].len = sizeof(tBGP4_PEER);
    bgp_mlist[4].count = max_peer;

    bgp_mlist[5].len = sizeof(tBGP4_VPN_INSTANCE);
    bgp_mlist[5].count = max_vpn_instance;

    /*create memory*/
    for (i = 0 ; i < BGP_MAX_BUF_TYPE ; i++)
    {
        p_list  = &bgp_mlist[i];
        p_list->p_buf = gBgp4.memAllocFunc(p_list->count,p_list->len);
        if (p_list->p_buf)
        {
            memset(p_list->p_buf, 0, p_list->len*p_list->count);
        }
        else
        {   
            bgp4_mem_deinit();
            return VOS_ERR;
        }
        p_list->p_free = NULL ;
        for (count = 0 ; count < p_list->count ; count++)
        {
            p_obj = (struct bgp_mem_object *)((u_long)p_list->p_buf + count*p_list->len);
            p_obj->p_next = p_list->p_free ;
            p_list->p_free = p_obj;
        }
    }
    return VOS_OK;
}

void bgp4_mem_deinit()
{
    u_int i = 0 ;
    struct bgp_mem_list *p_list;

    /*create memory*/
    for (i = 0 ; i < BGP_MAX_BUF_TYPE ; i++)
    {
        p_list  = &bgp_mlist[i];
        if (p_list->p_buf)
        {
            gBgp4.memFreeFunc(p_list->p_buf);
        }
        p_list->p_buf = NULL;
        p_list->p_free = NULL ;
    }    
}

void *bgp4_malloc(u_int len, u_int type)
{
    struct bgp_mem_object *p_obj = NULL;
        
    u_int id = BGP_MAX_BUF_TYPE ;
    gBgp4.stat.mem[type].add++;
    
    switch (type)
    {
        case MEM_BGP_PEER :
            id = 4;
            break;
        case MEM_BGP_ROUTE :
            id = 2;
            break;
        case MEM_BGP_LINK :
        case MEM_BGP_ASPATH :   
            id = 0;
            break;
        case MEM_BGP_INFO :
            id = 3;
            break;
        case MEM_BGP_VPN_INSTANCE:
            id = 5;
            break;
        case MEM_BGP_BUF :
        case MEM_BGP_COMMUNITY :
        case MEM_BGP_NETWORK :
        case MEM_BGP_POLICY_CONFIG:  
            id = 1;
            break;
        default :
            break;
    }
    if (id >= BGP_MAX_BUF_TYPE || (bgp_mlist[id].len < len))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"Memory allocate failed for length");
        gBgp4.stat.mem[type].fail++;
        return NULL;
    }
    p_obj = bgp_mlist[id].p_free;
    if (p_obj == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"Memory allocate failed for empty,type:%d,id:%d,length:%d",type,id,len);
        gBgp4.stat.mem[type].fail++;
        return NULL;
    }       
    bgp_mlist[id].p_free = p_obj->p_next;
    
    memset(p_obj, 0, len);
    return p_obj;
}

u_int bgp4_mem_init_maximum(u_int type)
{
    u_int id = BGP_MAX_BUF_TYPE ;

    switch (type)
    {
        case MEM_BGP_PEER :
            id = 4;
            break;
        case MEM_BGP_ROUTE :
            id = 2;
            break;
        case MEM_BGP_LINK :
        case MEM_BGP_ASPATH :   
            id = 0;
            break;
        case MEM_BGP_INFO :
            id = 3;
            break;
        case MEM_BGP_VPN_INSTANCE:
            id = 5;
            break;
        case MEM_BGP_BUF :
        case MEM_BGP_COMMUNITY :
        case MEM_BGP_NETWORK :
        case MEM_BGP_POLICY_CONFIG:  
            id = 1;
            break;
        default :
            break;
    }
    
    if (id >= BGP_MAX_BUF_TYPE)
    {
        return 0;
    }

    return bgp_mlist[id].count;
}

void bgp4_free(void *p_buf, u_int type) 
{
    struct bgp_mem_object *p_obj = NULL;
    u_int id = BGP_MAX_BUF_TYPE ;

    gBgp4.stat.mem[type].del++; 
    
    switch (type){
        case MEM_BGP_PEER :
            id = 4;
            break;
        case MEM_BGP_ROUTE :
            id = 2;
            break;
        case MEM_BGP_LINK :
        case MEM_BGP_ASPATH :   
            id = 0;
            break;
        case MEM_BGP_INFO :
            id = 3;
            break;
        case MEM_BGP_VPN_INSTANCE:
            id = 5;
            break;
        case MEM_BGP_BUF :
        case MEM_BGP_COMMUNITY :
        case MEM_BGP_NETWORK :
        case MEM_BGP_POLICY_CONFIG:
            id = 1;
            break;
        default :
            break;
    }
    if (id >= BGP_MAX_BUF_TYPE)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"Memory Free failed for length");
        return;
    }
    p_obj = p_buf;
    p_obj->p_next = bgp_mlist[id].p_free;
    bgp_mlist[id].p_free = p_obj;
    return;
}
#if 0
void bgp_sem_take_timeout()
{
    vos_pthread_timedlock(&gbgp4.sem, 5*vos_get_clock_rate());
    return ();
}
#endif
void bgp4_test_originate_route(u_int dest, u_int len, u_int count, u_int add)
{
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(0);
    tBGP4_PATH path; 
    tBGP4_ROUTE rt;
    u_int i;
    u_int mask = bgp4_len2mask(len);
    u_int step = ~mask;
    step++;

    if(p_instance == NULL)
    {
        return;
    }
   
    memset(&rt, 0,sizeof(rt));
    memset(&path, 0, sizeof(path));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);

    rt.dest.afi = BGP4_PF_IPUCAST;
    rt.proto = 2/*M2_ipRouteProto_local*/;
    rt.dest.prefixlen = len;
    rt.p_path = &path;

    path.afi = BGP4_AF_IP;
    path.safi = BGP4_SAF_UCAST;
   
    bgp_sem_take();

    for (i = 0 ; i < count ; i++)
    {
        *(u_int*)rt.dest.ip = (dest+ step*i);
        bgp4_import_route(p_instance,&rt, add);
    }
   
    bgp_sem_give();

    return;
}

void bgp4_test_originate_route6(u_char *network, u_int len, u_int count, u_int add)
{
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(0);
    u_char dest[16];
    tBGP4_PATH path; 
    tBGP4_ROUTE rt;
    u_int i;

    if(p_instance == NULL)
    {
        return;
    }
   
    memcpy(dest, network, 16);
    memset(&rt, 0,sizeof(rt));
    memset(&path, 0, sizeof(path));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);

    rt.dest.afi = BGP4_PF_IP6UCAST;
    rt.dest.prefixlen=len;
    rt.proto = 2/*M2_ipRouteProto_local*/;
    rt.p_path = &path;

    path.afi = BGP4_AF_IP6;
    path.safi = BGP4_SAF_UCAST;
    
    bgp_sem_take();

    for (i = 0 ; i < count ; i++)
    {
        (*(u_int *)(dest+4))++;
        memcpy(rt.dest.ip, dest, 16);
        bgp4_import_route(p_instance,&rt, add);
    }  
    bgp_sem_give();

    return;
}

/*display special ipv4 route*/
void
bgp4_show_detail_route(u_int dest)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    tBGP4_PATH *p_path = NULL;
    tBGP4_ROUTE_VECTOR vector;
    u_int i = 0;
    u_char str[64];
    
    bgp_sem_take();
    LST_LOOP(&gBgp4.vpn_instance_list, p_instance, node, tBGP4_VPN_INSTANCE)
    {
        memset(&vector, 0, sizeof(vector));
        RIB_LOOP(&p_instance->rib, p_route, p_next)
        {
            if (bgp_ip4(p_route->dest.ip) == dest)
            {
                vector.p_route[vector.count] = p_route;
                vector.count++;               
            }
        }
         for (i = 0; i < vector.count ; i++)
         {
                 p_route = vector.p_route[i];
                 bgp4_printf_route(p_route, str);
                 printf("\n\r #%d route:%s\n\r", i, str);
                 printf("  label:%d\n\r", p_route->vpn_label);
                 printf("  proto:%d\n\r", p_route->proto);
                 printf("  reference:%d\n\r", p_route->refer);
                 printf("  preference:%d\n\r", p_route->preference);
                 printf("  summary:%d\n\r", p_route->is_summary);
                 printf("  summary active:%d\n\r", p_route->summary_active);
                 printf("  range filtered:%d\n\r", p_route->filter_by_range);
                 printf("  policy filtered:%d\n\r", p_route->is_filtered);
                 printf("  stale:%d\n\r", p_route->stale);
                 printf("  delay process:%d\n\r", p_route->deferral);
                 printf("  in rib:%d\n\r", p_route->rib_add);
                 printf("  to be deleted:%d\n\r", p_route->is_deleted);
                 printf("  ip action:%d\n\r", p_route->ip_action);
                 printf("  ip update finish:%d\n\r", p_route->ip_finish);
                 printf("  ip msg finish:%d\n\r", p_route->rtmsg_finish);
                 printf("  mpls update finish:%d\n\r", p_route->mpls_notify_finish);
                 printf("  direction:%d\n\r", p_route->route_direction);
                 printf("  igp sync wait:%d\n\r", p_route->igp_sync_wait);
                 printf("  in ip table:%d\n\r", p_route->ip_table);
                 printf("  withdraw:%02x.%02x\n\r", p_route->withdraw_bits[0], p_route->withdraw_bits[1]);
                 printf("  feasible:%02x.%02x\n\r", p_route->update_bits[0], p_route->update_bits[1]);
                 printf("  path:%x\n\r", p_route->p_path);
                 if (p_route->p_path)
                     {
                         p_path = p_route->p_path;
                         if (p_path->p_peer)
                             printf("    peer %s\n\r", bgp4_printf_peer(p_path->p_peer, str));
                         
                         printf("    aspath len %d\n\r", bgp4_aspath_len(p_path, BGP4_IBGP));
                         printf("    origin %d\n\r", p_path->origin);
                         printf("    med %d\n\r", p_path->rcvd_med);
                         printf("    local pref %d\n\r", p_path->rcvd_localpref);
                         printf("    nexthop %s\n\r", bgp4_printf_addr(&p_path->nexthop, str));
                     }
                 if (p_route == bgp4_best_route(&p_instance->rib,&p_route->dest))
                 {
                     printf("    this is best route\n\r");
                 }
                 else
                 {
                     printf("    this is not best route\n\r");
                 }
     
                 if (p_route == bgp4_best_route_vector(&vector))
                 {
                     printf("    this is best route vector\n\r");
                 }
                 else
                 {
                     printf("    this is not best route vector\n\r");
                 }
         }
         
         if (vector.count > 1)
         {
            printf("    ECMP_CMP(1, 2) %d\n\r", 
             bgp4_route_ECMP_priority_cmp(vector.p_route[0], vector.p_route[1]));
         }
         if (vector.count > 2)
         {
            printf("    ECMP_CMP(2, 3) %d\n\r", 
             bgp4_route_ECMP_priority_cmp(vector.p_route[1], vector.p_route[2]));
         }
         if (vector.count > 3)
         {
            printf("    ECMP_CMP(3, 4) %d\n\r", 
             bgp4_route_ECMP_priority_cmp(vector.p_route[2], vector.p_route[3]));
         }
    }
    bgp_sem_give();
    return;
}

#endif
