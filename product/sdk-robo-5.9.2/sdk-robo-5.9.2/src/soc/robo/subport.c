/*
 * $Id: subport.c,v 1.2 2009/04/25 06:39:14 ako Exp $
 *
 * $Copyright: 
 */
#include <soc/error.h>
#include <soc/types.h>
#include <soc/mcm/robo/memregs.h>
#include <assert.h>
#include <soc/register.h>
#include <soc/drv.h>
#include <soc/debug.h>


/*
 *  Function : drv_mcrep_vpgrp_vport_config_set
 *  Purpose :
 *      Set the multicast replication vport membership in a given group
 *  Parameters :
 *      unit        :   unit id
 *      mc_group    :   vPort group ID 
 *      port        :   port ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPGRP_OP_VPORT_MEMBER
 *              - DRV_MCREP_VPGRP_OP_VPGRP_RESET
 *      param       :   (In/Out)parameter for OP.
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_mcrep_vpgrp_vport_config_set(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_mcrep_vpgrp_vport_config_get
 *  Purpose :
 *      Get the multicast replication vport membership in a given group
 *  Parameters :
 *      unit        :   unit id
 *      mc_group    :   vPort group ID 
 *      port        :   Operation ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPGRP_OP_VPORT_MEMBER
 *              - DRV_MCREP_VPGRP_OP_ENTRY_ID
 *      param       :   (In/Out)parameter for OP.
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_mcrep_vpgrp_vport_config_get(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_mcrep_vport_config_set
 *  Purpose :
 *      Set the vport related configuration.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   Operation ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPORT_OP_VID
 *              - DRV_MCREP_VPORT_OP_VID_RESET
 *              - DRV_MCREP_VPORT_OP_UNTAG_VP
 *              - DRV_MCREP_VPORT_OP_UNTAG_RESET
 *      vport       :   vport_id
 *      vid         :   VID
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_mcrep_vport_config_set(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 vport, uint32 vid)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_mcrep_vport_config_get
 *  Purpose :
 *      Get the vport related configuration.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   Operation ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPORT_OP_VID
 *              - DRV_MCREP_VPORT_OP_UNTAG_VP
 *      vport       :   (IN/OUT)vport_id
 *      vid         :   (IN/OUT)VID
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_mcrep_vport_config_get(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 *vport, uint32 *vid)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_mcrep_vport_vid_search
 *  Purpose :
 *      Search the existed vport through a known VID.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   Operation ID
 *      vport       :   (IN/OUT)vport_id
 *      param       :   (In/Out)... TBD....
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_mcrep_vport_vid_search(int unit, uint32 port, 
        uint32 *vport, int *param)
{
    return SOC_E_UNAVAIL;
}

