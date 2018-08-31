#include "ospf.h"
#include "ospf_policy.h"
#ifdef OSPF_POLICY
//#include "pcl_nm.h"
#endif
#include "ospf_table.h"
#include "ospf_nm.h"


u_int ospf_str_to_filter_policy(char *pcString)
{
    if (pcString == NULL)
    {
        return 0;
    }
    return (!strcmp(pcString,"ip-prefix"))?OSPF_TYPE_PLIST:
        (!strcmp(pcString,"route-policy"))?OSPF_TYPE_RMAP:0;
}


char * ospf_filter_policy_to_str(u_int uiType, char *pcString)
{
    if (pcString == NULL)
    {
        return NULL;
    }
    switch(uiType)
    {
        case OSPF_TYPE_PLIST:
        {
            sprintf(pcString,"%s","ip-prefix");
            break;
        }
        case OSPF_TYPE_RMAP:
        {
            sprintf(pcString,"%s","route-policy");
            break;
        }
        default:
        {
            break;
        }
    }
    return pcString;
}

#ifndef HAVE_ROUTEPOLICY
int ospf_policy_func(char *pcPolicyName, u_long ulCmd, tOSPF_POLICY_ENTRY *pstPolicy)
{
    int iRet = OSPF_POLICY_PERMIT;
#ifdef OSPF_POLICY
    struct prefix stPrefix = {0};
    pcl_cfg_info_t stRoutePcl = {0} ;
    u_char cszNexthop[24] = {0};
    u_char cszIpAddre[1024] = {0};
    char destStr[64]={0},nxthopStr[16]={0};

    if ((pcPolicyName == NULL)
        || (pstPolicy == NULL))
    {
        return ERR;
    }

    ospf_logx(ospf_debug_policy, "pcPolicyName:%s", pcPolicyName);

    stRoutePcl.nexthop.address = cszNexthop;
    stRoutePcl.prefix = cszIpAddre;

    stPrefix.prefixlen = pstPolicy->uiDMask;
    stPrefix.family = AF_INET;
    memcpy(&stPrefix.u.prefix, &pstPolicy->uiDest, 4);

    #if 1 /*TODO:���´��봦��������*/
    ospf_inet_ntoa(destStr, ntohl(pstPolicy->uiDest));
    ospf_inet_ntoa(nxthopStr,stRoutePcl.nexthop.address);
    #endif
    switch (ulCmd)
    {
        case OSPF_TYPE_RMAP:
        {
            memcpy(stRoutePcl.prefix, &stPrefix, sizeof(struct prefix));
            stRoutePcl.uiCost = pstPolicy->uiCost;
            memcpy(stRoutePcl.nexthop.address, &pstPolicy->uiNextHop, 4);
            
            ospf_logx(ospf_debug_policy, "OSPF_TYPE_RMAP:");
            ospf_logx(ospf_debug_policy, "stPrefix dest %s, mask 0x%x, next %s", 
                destStr, stPrefix.prefixlen, nxthopStr);
            ospf_logx(ospf_debug_policy, "uiCost 0x%x", pstPolicy->uiCost);

            iRet = pcl_apply_set_api(pcPolicyName, PCL_TYPE_RMAP, (void *)&stRoutePcl);

            pstPolicy->ucPref = stRoutePcl.ucPref;
            ospf_logx(ospf_debug_policy, "ucPref 0x%x", pstPolicy->ucPref);
            switch (iRet)
            {
                case OSPF_RMAP_DENYMATCH:
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return RMAP_DENYMATCH set OSPF_POLICY_DENY");
                    iRet = OSPF_POLICY_DENY;
                    break;
                }
                case OSPF_RMAP_NOMATCH:
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return RMAP_NOMATCH set OSPF_POLICY_NOMATCH");
                    iRet = OSPF_POLICY_NOMATCH;
                    break;
                }
                case OSPF_RMAP_NO_ENTRY:
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return RMAP_NO_ENTRY set OSPF_POLICY_NOENTRY");
                    iRet = OSPF_POLICY_NOENTRY;
                    break;
                }
                default :
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return default set OSPF_POLICY_PERMIT");
                    iRet = OSPF_POLICY_PERMIT;
                    break;
                }
            }
            break;
        }
        case OSPF_TYPE_PLIST:
        {
            ospf_logx(ospf_debug_policy, "OSPF_TYPE_PLIST:");
            ospf_logx(ospf_debug_policy, "stPrefix dest %s, mask 0x%x", destStr, stPrefix.prefixlen);
            iRet = pcl_apply_set_api(pcPolicyName, PCL_TYPE_PLIST, (void *)&stPrefix);
            switch (iRet)
            {
                case OSPF_PREFIX_DENY:
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return PREFIX_DENY set OSPF_POLICY_DENY");
                    iRet = OSPF_POLICY_DENY;
                    break;
                }
                case OSPF_PREFIX_NO_ENTRY:
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return PREFIX_NO_ENTRY set OSPF_POLICY_NOENTRY");
                    iRet = OSPF_POLICY_NOENTRY;
                    break;
                }
                default :
                {
                    ospf_logx(ospf_debug_policy, "pcl_apply_set_api return PREFIX_PERMIT set OSPF_POLICY_PERMIT");
                    iRet = OSPF_POLICY_PERMIT;
                    break;
                }
            }
            break;
        }
        default :
        {
            return ERR;
        }
    }
#endif
    return iRet;
}


void ospf_policy_fill(struct ospf_iproute *pstRoute, tOSPF_POLICY_ENTRY *pstPolicy)
{
    struct in_addr netmask;

    if ((pstRoute == NULL)
        || (pstPolicy == NULL))
    {
        return;
    }

    netmask.s_addr = (in_addr_t)ntohl(pstRoute->mask);

    pstPolicy->uiDest = ntohl(pstRoute->dest);
    pstPolicy->uiDMask = ip_masklen(netmask);
    pstPolicy->uiNextHop = ntohl(pstRoute->fwdaddr);
    pstPolicy->uiCost = (pstRoute->metric & (~OSPF_ASE_EBIT));
}


int ospf_filter_policy_im_func(void *pRoute)
{
    struct ospf_iproute *pstRoute = (struct ospf_iproute *)pRoute;
    struct ospf_process *pstProcess = pstRoute->p_process;
    struct ospf_policy *pstPolicy = NULL;
    tOSPF_POLICY_ENTRY stPoliclyInfo = {0};
    char *pcPolicyName = NULL;
    u_long ulPlicyType = 0;
    int iRet = OSPF_POLICY_PERMIT;

    ospf_logx(ospf_debug_policy, "----start ");

    pstPolicy = ospf_filter_policy_lookup(pstProcess, OSPF_IM_POLICY);
    if ((pstPolicy != NULL) && (pstPolicy->active == TRUE))
    {
        ospf_policy_fill(pstRoute, &stPoliclyInfo);
        pcPolicyName = pstPolicy->policy_name;
        ulPlicyType = pstPolicy->policy_type;
        iRet = ospf_policy_func(pcPolicyName, ulPlicyType, &stPoliclyInfo);
        if (iRet != OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_policy_func ret is not OSPF_POLICY_DENY set OSPF_POLICY_PERMIT\n");
            iRet = OSPF_POLICY_PERMIT;
        }
    }
    if (pstPolicy != NULL)
    {
        if (iRet == OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_im_func DENY\n");
        }
        else
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_im_func PERMIT\n");
        }
    }
    ospf_logx(ospf_debug_policy, "----end \n");

    return iRet;
}


int ospf_filter_policy_ex_func(void *pRoute)
{
    struct ospf_iproute *pstRoute = (struct ospf_iproute *)pRoute;
    struct ospf_process *pstProcess = pstRoute->p_process;
    struct ospf_policy *pstPolicy = NULL;
    tOSPF_POLICY_ENTRY stPoliclyInfo = {0};
    char *pcPolicyName = NULL;
    u_long ulPlicyType = 0;
    int iRet = OSPF_POLICY_PERMIT;

    ospf_logx(ospf_debug_policy, "----start ");

    pstPolicy = ospf_filter_policy_lookup(pstProcess, OSPF_EX_POLICY);
    if ((pstPolicy != NULL) && (pstPolicy->active == TRUE))
    {
        ospf_policy_fill(pstRoute, &stPoliclyInfo);
        pcPolicyName = pstPolicy->policy_name;
        ulPlicyType = pstPolicy->policy_type;
        iRet = ospf_policy_func(pcPolicyName, ulPlicyType, &stPoliclyInfo);
        if (iRet != OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_policy_func ret is not OSPF_POLICY_DENY set OSPF_POLICY_PERMIT");
            iRet = OSPF_POLICY_PERMIT;
        }
    }
    if (pstPolicy != NULL)
    {
        if (iRet == OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_ex_func DENY\n");
        }
        else
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_ex_func PERMIT\n");
        }
    }
    ospf_logx(ospf_debug_policy, "----end \n");

    return iRet;
}


int ospf_filter_policy_pre_func(void *pRoute)
{
    struct ospf_iproute *pstRoute = (struct ospf_iproute *)pRoute;
    struct ospf_process *pstProcess = pstRoute->p_process;
    struct ospf_policy *pstPolicy = NULL;
    tOSPF_POLICY_ENTRY stPoliclyInfo = {0};
    char *pcPolicyName = NULL;
    u_long ulPlicyType = 0;
    int iRet = OSPF_POLICY_DENY;

    ospf_logx(ospf_debug_policy, "----start ");

    pstPolicy = ospf_filter_policy_lookup(pstProcess, OSPF_PRE_POLICY);
    if ((pstPolicy != NULL) && (pstPolicy->active == TRUE))
    {
        ospf_policy_fill(pstRoute, &stPoliclyInfo);
        pcPolicyName = pstPolicy->policy_name;
        ulPlicyType = pstPolicy->policy_type;
        iRet = ospf_policy_func(pcPolicyName, ulPlicyType, &stPoliclyInfo);
        /*������ֵ��ΪOSPF_POLICY_PERMITʱ���ͻ��޸����ȼ�*/
        if (iRet != OSPF_POLICY_PERMIT)
        {
            ospf_logx(ospf_debug_policy, "ospf_policy_func ret is not OSPF_POLICY_PERMIT set OSPF_POLICY_DENY");
            iRet = OSPF_POLICY_DENY;
        }
        pstProcess->preference_policy = stPoliclyInfo.ucPref;
    }
    if (pstPolicy != NULL)
    {
        if (iRet == OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_pre_func DENY\n");
        }
        else
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_pre_func PERMIT\n");
        }
    }
    ospf_logx(ospf_debug_policy, "----end\n");

    return iRet;
}


int ospf_filter_policy_ase_pre_func(void *pRoute)
{
    struct ospf_iproute *pstRoute = (struct ospf_iproute *)pRoute;
    struct ospf_process *pstProcess = pstRoute->p_process;
    struct ospf_policy *pstPolicy = NULL;
    tOSPF_POLICY_ENTRY stPoliclyInfo = {0};
    char *pcPolicyName = NULL;
    u_long ulPlicyType = 0;
    int iRet = OSPF_POLICY_DENY;

    ospf_logx(ospf_debug_policy, "----start ");

    pstPolicy = ospf_filter_policy_lookup(pstProcess, OSPF_ASE_PRE_POLICY);
    if ((pstPolicy != NULL) && (pstPolicy->active == TRUE))
    {
        ospf_policy_fill(pstRoute, &stPoliclyInfo);
        pcPolicyName = pstPolicy->policy_name;
        ulPlicyType = pstPolicy->policy_type;
        iRet = ospf_policy_func(pcPolicyName, ulPlicyType, &stPoliclyInfo);
        /*������ֵ��ΪOSPF_POLICY_PERMITʱ���ͻ��޸����ȼ�*/
        if (iRet != OSPF_POLICY_PERMIT)
        {
            ospf_logx(ospf_debug_policy, "ospf_policy_func ret is not OSPF_POLICY_PERMIT set OSPF_POLICY_DENY");
            iRet = OSPF_POLICY_DENY;
        }
        pstProcess->preference_ase_policy = stPoliclyInfo.ucPref;
    }
    if (pstPolicy != NULL)
    {
        if (iRet == OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_ase_pre_func DENY\n");
        }
        else
        {
            ospf_logx(ospf_debug_policy, "ospf_filter_policy_ase_pre_func PERMIT\n");
        }
    }
    ospf_logx(ospf_debug_policy, "----end \n");

    return iRet;
}


int ospf_import_policy_func(void *pRoute)
{
    struct ospf_iproute *pstRoute = (struct ospf_iproute *)pRoute;
    struct ospf_process *pstProcess = pstRoute->p_process;
    struct ospf_policy *pstPolicy = NULL;
    tOSPF_POLICY_ENTRY stPoliclyInfo = {0};
    char *pcPolicyName = NULL;
    u_long ulPlicyType = 0;
    int iRet = OSPF_POLICY_PERMIT;

    ospf_logx(ospf_debug_policy, "----start ");

    pstPolicy = ospf_redis_policy_lookup(pstProcess, pstRoute->proto, pstRoute->proto, pstRoute->process_id);
    if ((pstPolicy != NULL) && (pstPolicy->active == TRUE))
    {
        ospf_policy_fill(pstRoute, &stPoliclyInfo);
        pcPolicyName = pstPolicy->policy_name;
        ulPlicyType = pstPolicy->policy_type;
        iRet = ospf_policy_func(pcPolicyName, ulPlicyType, &stPoliclyInfo);
        if (iRet != OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_policy_func ret is not OSPF_POLICY_DENY set OSPF_POLICY_PERMIT");
            iRet = OSPF_POLICY_PERMIT;
        }
    }
    if (pstPolicy != NULL)
    {
        if (iRet == OSPF_POLICY_DENY)
        {
            ospf_logx(ospf_debug_policy, "ospf_import_policy_func DENY\n");
        }
        else
        {
            ospf_logx(ospf_debug_policy, "ospf_import_policy_func PERMIT\n");
        }
    }
    ospf_logx(ospf_debug_policy, "----end \n");

    return iRet;
}
#endif

void  ospf_policy_update_func(char *PolicyName, u_long ulPolicy)
{
    struct ospf_process *pstProFrist = NULL; 
    struct ospf_process *pstProNext = NULL; 
    struct ospf_process stSearch = {0};
    struct ospf_policy *pstPolicy = NULL;
    struct ospf_policy *pstPolicyNext = NULL; 
    u_char ucExFlg = 0;
    u_long ulAf = 0, ulProto= 0;

    ospf_semtake_try();

    pstProFrist = ospf_lstfirst(&ospf.process_table);
    while(pstProFrist != NULL)
    {
        stSearch.process_id = pstProFrist->process_id;
        pstProNext = ospf_lstgreater(&ospf.process_table, &stSearch);
        /*filter-policy import ǰ׺��·�ɲ��Ծ�����Ӧ*/
        pstPolicy = ospf_filter_policy_lookup(pstProFrist, OSPF_IM_POLICY);
        if ((pstPolicy != NULL) && (pstPolicy->policy_type == ulPolicy)
            && (memcmp(pstPolicy->policy_name, PolicyName, strlen(PolicyName)) == 0))
        {
            ospf_logx(ospf_debug_policy, "filter-policy import matching");
            ospf_timer_start(&pstPolicy->update_timer, 5);
        }
        /*filter-policy export ƥ��ǰ׺���ԡ�·�ɲ���*/
        pstPolicy = ospf_filter_policy_lookup(pstProFrist, OSPF_EX_POLICY);
        if ((pstPolicy != NULL) && (pstPolicy->policy_type == ulPolicy)
            && (memcmp(pstPolicy->policy_name, PolicyName, strlen(PolicyName)) == 0))
        {
            ospf_logx(ospf_debug_policy, "filter-policy export matching");
            ucExFlg = 1;
        }

        if (ulPolicy == OSPF_TYPE_RMAP)
        {
            /*preference ƥ��·�ɲ���*/
            pstPolicy = ospf_filter_policy_lookup(pstProFrist, OSPF_PRE_POLICY);
            if ((pstPolicy != NULL) && (pstPolicy->policy_type == ulPolicy)
                && (memcmp(pstPolicy->policy_name, PolicyName, strlen(PolicyName)) == 0))
            {
                ospf_logx(ospf_debug_policy, "preference matching");
                //zhurish ospf_update_route_for_pre_chg(pstProFrist->process_id, OSPF_GLB_PREFERENCE);
            }
            /*preference ase ƥ��·�ɲ���*/
            pstPolicy = ospf_filter_policy_lookup(pstProFrist, OSPF_ASE_PRE_POLICY);
            if ((pstPolicy != NULL) && (pstPolicy->policy_type == ulPolicy)
                && (memcmp(pstPolicy->policy_name, PolicyName, strlen(PolicyName)) == 0))
            {
                ospf_logx(ospf_debug_policy, "preference ase matching");
                //zhurish ospf_update_route_for_pre_chg(pstProFrist->process_id, OSPF_GLB_ASE_PREFERENCE);
            }
            /*import ƥ��·�ɲ���*/
            for_each_node(&ospf.nm.redistribute_policy_table, pstPolicy, pstPolicyNext)
            {
                if((pstPolicy->policy_type == ulPolicy)
                    && (memcmp(pstPolicy->policy_name, PolicyName, strlen(PolicyName)) == 0))
                {
                    ospf_logx(ospf_debug_policy, "import matching");
                    ucExFlg = 1;
                    break;
                }
            }
        }

        if (ucExFlg == 1)
        {
#ifdef OSPF_REDISTRIBUTE
            ospf_stimer_start(&pstProFrist->import_timer, OSPF_IMPORT_INTERVAL);
            pstProFrist->import_update = TRUE;
#endif
        }

        pstProFrist = pstProNext;
    }

    ospf_semgive();

    return;
}


void ospf_route_map_update(char *PolicyName)
{
    ospf_logx(ospf_debug_policy, "OSPF_TYPE_RMAP %s have change", PolicyName);
    ospf_policy_update_func(PolicyName, OSPF_TYPE_RMAP);
    return;
}


void ospf_route_prefix_update(char *PolicyName)
{
    ospf_logx(ospf_debug_policy, "OSPF_TYPE_PLIST %s have change", PolicyName);
    ospf_policy_update_func(PolicyName, OSPF_TYPE_PLIST);
    return;
}





