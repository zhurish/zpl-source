#include "ospf.h"
#include "ospf_api.h"
//#include "if_manage.h"

#if 0
#include "port_nm.h"
#include "lldp_nm.h"
#include "vlan_nm.h"
#include "qos_nm.h"
#endif
#ifdef OSPF_TE
#include "rsvp_nm.h"
#endif
#ifdef OSPF_LSRID
#include "rsvp_nm.h"
#include "mpls_nm.h"
extern int32_t rsvpGetApi(void * index,int32_t cmd,void * var);
#endif

#define AUTHENTICATION_KEY_LEN  16

int cipher_fun(int x, int y) //��Կ e �� t �Ļ����ж�
{
	int t;
	while (y)
	{
		t = x;
		x = y;
		y = t % y;
	}
	if (x == 1)
		return 0; //x �� y ����ʱ���� 0
	else
		return 1; //x �� y ������ʱ���� 1
}

void cipher_password(char *pcPassWord)            //���ܺ���
{
	int iLen = 0, iC = 0, iAuthKeyLen = AUTHENTICATION_KEY_LEN;
	int i = 0, j = 0;
	int iCom = 105;
	int iSekey = 5;
	char szcStr[100];
	int iKey = 0;
	int p = 0, q = 0, n = 0, t = 0;
	if (pcPassWord == NULL)
	{
		return;
	}
	iLen = strlen(pcPassWord);
	p = 3, q = 5;
	n = p * q;
	t = (p - 1) * (q - 1);
	iKey = random() % 8;
	while ((iKey <= 1) || (iKey > t) || cipher_fun(iKey, t) || (iKey == 3))
	{
		iKey = random() % 8;
	}
	for (i = 0; i < iLen; i++)
	{
		iC = 1;
		for (j = 0; j < iSekey; j++)
		{
			iC = (iC * (pcPassWord[i] - '!')) % iCom;
		}
		szcStr[i] = iC + (iKey + '!' % (i + 1) + '!');
	}
	if (iLen < iAuthKeyLen)
	{
		for (i = iLen; i < iAuthKeyLen; i++)
		{
			szcStr[i] = '&' + random() % 54;
		}
	}
	szcStr[iAuthKeyLen] = '!' + iKey * 10;
	szcStr[iAuthKeyLen + 1] = '!' + iKey + iLen;
	szcStr[iAuthKeyLen + 2] = '\0';
	memcpy(pcPassWord, szcStr, iAuthKeyLen + 3);
	//plain_password(szcStr);
}

void plain_password(char *pcPassWord)          //���ܺ���
{
	int iLen = 0, iC = 0, iAuthKeyLen = AUTHENTICATION_KEY_LEN;
	char szcGoal[100] =
	{ 0 };
	int i = 0, j = 0;
	int iCom = 105;
	int iPrkey = 5;
	int iKey = 0;
	if (pcPassWord == NULL)
	{
		return;
	}
	iKey = (pcPassWord[iAuthKeyLen] - '!') / 10;
	iLen = pcPassWord[iAuthKeyLen + 1] - '!' - iKey;
	if (iLen > OSPF_AUTH_KEY_MAX_LEN)
	{
		return;
	}
	for (i = 0; i < iLen; i++)
	{
		pcPassWord[i] = pcPassWord[i] - (iKey + '!' % (i + 1) + '!');
	}
	for (i = 0; i < iLen; i++)      //ʵ�ֽ���
	{
		iC = 1;
		for (j = 0; j < iPrkey; j++)
		{
			iC = (iC * (pcPassWord[i])) % iCom;
		}
		szcGoal[i] = iC + '!';
	}
	szcGoal[i] = '\0';
	memcpy(pcPassWord, szcGoal, iLen + 1);
}

/*****************************************************************************
 �� �� ��  : ospf_getFirstProcessMib
 ��������  :��ȡ��һ��ʵ��ID
 �������  :pulIndex - ����ָ��
 �������  : ��
 �� �� ֵ  : ERR - ʧ��
 OK - �ɹ�

 �޸���ʷ	   :
 *****************************************************************************/

int ospf_getFirstProcessMib(u_int **pulIndex)
{
	if (pulIndex == NULL)
	{
		return ERR;
	}

	if (OK == ospfInstanceGetFirst(*pulIndex))
	{
		ospf_logx(ospf_debug, "  pulIndex=%d\r\n", **pulIndex);
		return OK;
	}
	return ERR;
}

/*****************************************************************************
 �� �� ��  : ospf_getNextProcessMib
 ��������  :��ȡ��һ��ʵ��ID
 �������  :pulIndex - ����ָ��
 �������  : ��
 �� �� ֵ  : ERR - ʧ��
 OK - �ɹ�

 �޸���ʷ	   :
 *****************************************************************************/

int ospf_getNextProcessMib(u_int *pulIndx, u_int **pulNextIndex)
{
	if ((pulIndx == NULL) || (pulNextIndex == NULL))
	{
		return ERR;
	}

	if (OK == ospfInstanceGetNext(*pulIndx, *pulNextIndex))
	{
		ospf_logx(ospf_debug, "  pulNextIndex=%d\r\n", **pulNextIndex);
		return OK;
	}
	return ERR;
}

/*���ýӿڿ�������mib����*/
int ospf_SetCost(u_int ulIfunit, u_int costValue)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFMETRIC_INDEX stOspfIfIndex =
	{ 0 };

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.ifip = ulAddr;
	stOspfIfIndex.ifindex = ulIfunit;
	stOspfIfIndex.tos = 0;      //0-7

	if (ospfIfMetricSetApi(&stOspfIfIndex, OSPF_IFMETRIC_VALUE, &costValue)!=OK)
	{
		ospf_logx(ospf_debug, " Failed to set OSPF interface cost!");
		return ERR;
	}
	return OK;

}

/*��ȡ�ӿڿ�������mib����*/

int ospf_GetCost(u_int ulIfunit, u_int *costValue)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFMETRIC_INDEX stOspfIfIndex =
	{ 0 };
	if (costValue == NULL)
	{
		return ERR;
	}
	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.ifip = ulAddr;
	stOspfIfIndex.ifindex = ulIfunit;
	stOspfIfIndex.tos = 0;      //0-7

	if (ospfIfMetricGetApi(&stOspfIfIndex, OSPF_IFMETRIC_VALUE, costValue) != OK)
	{
		ospf_logx(ospf_debug, " %%Failed to get OSPF interface cost!\n");
		return ERR;
	}
	return OK;

}

/*��ȡ�ӿ��Ƿ��ospf����mib����*/
int ospf_getif(u_int ulIfunit)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface index failed!\n");
		return ERR;
	}
	return OK;
}

int ospf_getAuthKeyId(u_int ulIfunit, u_int *pAuthKeyid)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };

	if (pAuthKeyid == NULL)
	{
		return ERR;
	}
	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHKEYID, pAuthKeyid) != OK)
	{
		return ERR;
	}
	return OK;
}

int ospf_setAuthKeyId(u_int ulIfunit, u_int *pKeyid)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0, ulType = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHTYPE, &ulType) != OK)
	{
		return ERR;
	}
	if (OSPF_AUTH_MD5 != ulType)
	{
		ospf_logx(ospf_debug, "Please set authentication md5 first!\n");
		return ERR;
	}
	ret = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_AUTHKEYID, pKeyid);
	ospf_logx(ospf_debug,
			"ospf set Auther Key Id Interface unit = 0x%x ,ret = 0x%x, Key id = 0x%x\n",
			ulIfunit, ret, *pKeyid);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int ospf_getAuthKey(u_int ulIfunit, u_char *pKeyStr)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };
	u_int uTyp = 0;
	u_char ucbuf[32] =
	{ 0 };

	if (pKeyStr == NULL)
	{
		return ERR;
	}

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHTYPE, &uTyp) != OK)
	{
		return ERR;
	}

	if (OSPF_AUTH_NONE == uTyp)
	{
		ospf_logx(ospf_debug, "Please set  authentication type first!\n");
		pKeyStr = NULL;
		return ERR;
	}
	ret = ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHKEY, pKeyStr);
	ospf_logx(ospf_debug,
			"ospf get Auther Key Interface unit = 0x%x ,ret = 0x%x,Key = %s\n",
			ulIfunit, ret, pKeyStr);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int ospf_setAuthKey(u_int ulIfunit, u_char *pKeyStr, u_int uKeylen)
{
	int ret = OK;
	u_int uTyp = 0;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	u_char ucbuf[OSPF_MD5_KEY_LEN * 2] =
	{ 0 };
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };

	if (pKeyStr == NULL)
	{
		return ERR;
	}
	if (uKeylen > OSPF_MD5_KEY_LEN * 2)
	{
		return ERR;
	}

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}
	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHTYPE, &uTyp) != OK)
	{
		return ERR;
	}
	if (OSPF_AUTH_NONE == uTyp)
	{
		ospf_logx(ospf_debug, "Please set  authentication type first!\n");
		pKeyStr = NULL;
		return ERR;
	}
	if (((OSPF_AUTH_SIMPLE == uTyp) && (uKeylen > 8))
			|| ((OSPF_AUTH_MD5 == uTyp) && (uKeylen > 16)))
	{
		ospf_logx(ospf_debug, "The password key length error!");
		return ERR;
	}
	ret = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_AUTHKEY, pKeyStr);
	ospf_logx(ospf_debug,
			"ospf set Auther Key Interface unit = 0x%x ,ret = 0x%x,Key = %s, Key length = %d\n",
			ulIfunit, ret, pKeyStr, uKeylen);
	if (ret != OK)
	{
		ospf_logx(ospf_debug,
				"failed to set OSPF interface authentication key!");
		return ERR;
	}
	return OK;
}

int ospf_getAuthKeyDis(u_int ulIfunit, u_int *puiKeyDis)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfGetApi(&stOspfIfIndex, OSPF_IF_AUTHDIS, puiKeyDis) != OK)
	{
		return ERR;
	}

	return OK;
}

int ospf_setAuthKeyDis(u_int ulIfunit, u_int *puiKeyDis)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	if (ospfIfSetApi(&stOspfIfIndex, OSPF_IF_AUTHDIS, puiKeyDis) != OK)
	{
		return ERR;
	}

	return OK;
}

int ospf_getHelloInterval(u_int ulIfunit, u_int *uValue)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };
	if (uValue == NULL)
	{
		return ERR;
	}
	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;
	ret = ospfIfGetApi(&stOspfIfIndex, OSPF_IF_HELLOINTERVAL, uValue);
	ospf_logx(ospf_debug,
			"ospf get Hello Interval Interface unit = 0x%x ,ret = 0x%x,Value = 0x%x\n",
			ulIfunit, ret, *uValue);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int ospf_setHelloInterval(u_int ulIfunit, u_int *pValue)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };
	if (pValue == NULL)
	{
		return ERR;
	}

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	ret = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_HELLOINTERVAL, pValue);
	ospf_logx(ospf_debug,
			"ospf set Hello Interval Interface unit = 0x%x ,ret = 0x%x,Value = 0x%x\n",
			ulIfunit, ret, *pValue);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int Mib_ospf_GetInterface(u_int ulIfunit, u_int *uValue, u_int mib_num)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };
	if (uValue == NULL)
	{
		return ERR;
	}
	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;
	ret = ospfIfGetApi(&stOspfIfIndex, mib_num, uValue);
	ospf_logx(ospf_debug,
			"ospf get Hello Interval Interface unit = 0x%x ,ret = 0x%x,Value = 0x%x,mib num = %d\n",
			ulIfunit, ret, *uValue, mib_num);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int Mib_ospf_SetInterface(u_int ulIfunit, u_int *pValue, u_int mib_num)
{
	int ret = OK;
	u_int ulProcessId = 0;
	u_int ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{ 0 };
	if (pValue == NULL)
	{
		return ERR;
	}

	ret = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if (OK != ret)
	{
		ospf_logx(ospf_debug,
				"ospf get process and area by interface Index failed!\n");
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	ret = ospfIfSetApi(&stOspfIfIndex, mib_num, pValue);
	ospf_logx(ospf_debug,
			"Mib ospf set Interface unit = 0x%x ,ret = 0x%x,Value = 0x%x,mib_num = %d\n",
			ulIfunit, ret, *pValue, mib_num);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

int Mib_ospf_NodeCreatArea_Pro(tOSPF_AREA_INDEX *pstIndx)
{
	BOOL value_l = FALSE;
	int valLen = 0, ulStatus = 0;
	tOSPF_AREA_INDEX stOspfAreaIndex =
	{ 0 };
	if (pstIndx == NULL)
	{
		return ERR;
	}

	if (ospfGetApi(pstIndx->process_id, OSPF_GBL_ADMIN, &value_l) != OK)
	{
		ospf_logx(ospf_debug, "failed to get ospf current  state!\n");
		return ERR;
	}

	if (value_l != TRUE)
	{
		value_l = TRUE;
		valLen = sizeof(BOOL);

		if (ospfSetApi(pstIndx->process_id, OSPF_GBL_ADMIN, &value_l) != OK)
		{
			ospf_logx(ospf_debug, "failed to enable ospf !\n");
			return ERR;
		}

#ifdef OSPF_VPN
		value_l = OSPF_NO_VRID;
#else
		zebra_if_ospf_get_api(pstIndx->process_id,ZEBRA_IF_CREATE_VPN_OSPF,&value_l);
		zebra_if_ospf_get_api(pstIndx->process_id,ZEBRA_IF_GET_VRF_BY_OSPF_ID,&value_l);
#endif

		if (ospfSetApi(pstIndx->process_id, OSPF_GBL_VRID, &value_l) != OK)
		{
			ospf_logx(ospf_debug, "failed to set vrid!\n");
			return ERR;
		}

	}

	stOspfAreaIndex.process_id = pstIndx->process_id;
	stOspfAreaIndex.area_id = pstIndx->area_id;

	ulStatus = SNMP_ACTIVE;
	if (OK != ospfAreaSetApi(&stOspfAreaIndex, OSPF_AREA_STATUS, &ulStatus))
	{
		ospf_logx(ospf_debug, "active area failed!\n");
		return ERR;
	}
	return OK;

}

int Mib_ospf_setRowStatus(tOSPF_NETWORK_INDEX *pstIndx, u_int ulData)
{
	int ret = OK;
	u_long value_l = 0;
	tOSPF_NETWORK_INDEX stNetIndex;
	if (pstIndx == NULL)
	{
		return ERR;
	}

	memcpy(&stNetIndex, pstIndx, sizeof(tOSPF_NETWORK_INDEX));
	switch (ulData)
	{
	case 1:      //active
	case 4:      //creat
		value_l = TRUE;
		break;
	case 6:      //destroy Network
		value_l = FALSE;
		ret = ospfNetworkSetApi(&stNetIndex, OSPF_NETWORK_STATUS, &value_l);
		break;
	case 7:      //destroy Process
		ret = Mib_ospf_Nodedelete(stNetIndex.process_id);
		break;
	default:
		return ERR;
	}

	return ret;
}

int Mib_ospf_getRowStatus(tOSPF_NETWORK_INDEX *pstIndx, u_int *ulData)
{
	int ret = OK;
	u_long value_l = 0;

	if ((pstIndx == NULL) || (ulData == NULL))
	{
		return ERR;
	}
	ret = ospfNetworkGetApi(pstIndx, OSPF_NETWORK_STATUS, &value_l);
	ospf_logx(ospf_debug, "Mib ospf get RowStatus ret = 0x%x,Value = 0x%x\n",
			ret, value_l);
	if (ret != OK)
	{
		ospf_logx(ospf_debug, "failed to get network configure \n");
		return ERR;
	}
	switch (value_l)
	{
	case TRUE:      //active
		*ulData = 1;
		break;
	case FALSE:      //destroy
		*ulData = 6;
		break;
	default:
		return ERR;
	}
	return OK;
}

int Mib_ospf_NodeCreatData(tOSPF_NETWORK_INDEX *pstIndx)
{
	u_long value_l = 0;
	tOSPF_AREA_INDEX stOspfAreaIndex =
	{ 0 };
	int ret = OK;
	if (pstIndx == NULL)
	{
		return ERR;
	}
#if 0 /*caoyong delete 2017.9.20*/
	if (CheckMaskAddr(ntohl(pstIndx->mask)) != OK)
	{
		return ERR;
	}
#endif
	stOspfAreaIndex.process_id = pstIndx->process_id;
	stOspfAreaIndex.area_id = pstIndx->area;

	if (OK != Mib_ospf_NodeCreatArea_Pro(&stOspfAreaIndex))
	{
		ospf_logx(ospf_debug, "Creat area or process failed!\n");
		return ERR;
	}
	value_l = TRUE;
	ret = ospfNetworkSetApi(pstIndx, OSPF_NETWORK_STATUS, &value_l);
	if (ret != OK)
	{
		ospf_logx(ospf_debug, "Failed to configure network \n");
		return ERR;
	}
	return OK;

}

int Mib_ospf_Nodedelete(u_int ulPrid)
{
	u_long ulValue = 0;

	ospfGetApi(ulPrid, OSPF_GBL_ADMIN, &ulValue);

	if (ulValue != TRUE)
	{
		return OK;
	}
	ulValue = FALSE;

	if (ospfSetApi(ulPrid, OSPF_GBL_ADMIN, &ulValue) != OK)
	{
		//	ospf_logx(ospf_debug," %%Failed to disable ospf:%d \r\n",__LINE__);
		return ERR;
	}
	return OK;
}

int getFirstNode(tOSPF_NETWORK_INDEX *p_index)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nxtinstance = NULL;
	struct ospf_network *p_network = NULL;
	struct ospf_network *p_next = NULL;

	if (p_index == NULL)
	{
		return ERR;
	}

	for_each_ospf_process(p_process, p_nxtinstance)
	{
		if (
#ifdef OSPF_DCN
		(p_process->process_id == OSPF_DCN_PROCESS) ||
#endif
				(p_process->process_id == 0))
		{
			continue;
		}

		for_each_node(&p_process->network_table, p_network, p_next)
		{
			p_index->process_id = p_network->p_process->process_id;
			p_index->area = p_network->area_id;
			p_index->network = p_network->dest;
			p_index->mask = p_network->mask;
			return OK;
		}

	}
	return ERR;
}

int getNextNode(tOSPF_NETWORK_INDEX *p_index, tOSPF_NETWORK_INDEX *p_Nextindex)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nxtinstance = NULL;
	struct ospf_network *p_network = NULL;
	struct ospf_network *p_next = NULL;
	u_int ulFlag = 0;
	tOSPF_NETWORK_INDEX stCmp;
	if ((p_index == NULL) || (p_Nextindex == NULL))
	{
		return ERR;
	}
	for_each_ospf_process(p_process, p_nxtinstance)
	{
		if (
#ifdef OSPF_DCN
		(p_process->process_id == OSPF_DCN_PROCESS) ||
#endif
				(p_process->process_id == 0))
		{
			continue;
		}

		for_each_node(&p_process->network_table, p_network, p_next)
		{
			stCmp.process_id = p_network->p_process->process_id;
			stCmp.area = p_network->area_id;
			stCmp.network = p_network->dest;
			stCmp.mask = p_network->mask;
			if (0 == memcmp(p_index, &stCmp, sizeof(tOSPF_NETWORK_INDEX)))
			{
				ulFlag = 1;
				continue;
			}
			if (1 == ulFlag)
			{
				p_Nextindex->process_id = p_network->p_process->process_id;
				p_Nextindex->area = p_network->area_id;
				p_Nextindex->network = p_network->dest;
				p_Nextindex->mask = p_network->mask;
				return OK;
			}
		}

	}
	return ERR;
}

int getFirstNeighbor(tOSPFMIB_NBR_INDEX *p_index)
{
	int rc = OK;
	tOSPF_NBR_INDEX st_Index =
	{ 0 }, st_Next =
	{ 0 };

	if (p_index == NULL)
	{
		return ERR;
	}
	for (rc = ospfNbrGetSelect(NULL, &st_Next); rc == OK;
			rc = ospfNbrGetSelect(&st_Index, &st_Next))
	{
		if (
#ifdef OSPF_DCN
		(st_Next.process_id == OSPF_DCN_PROCESS) ||
#endif
				(st_Next.process_id == 0))
		{
			ospf_logx(ospf_debug, " get first neighbor process id =%d,%d,%d.\n",
					st_Next.process_id, st_Next.ipaddr, st_Next.addrlessif);
			memcpy(&st_Index, &st_Next, sizeof(tOSPF_NBR_INDEX));
			memset(&st_Next, 0, sizeof(tOSPF_NBR_INDEX));
			continue;
		}

		p_index->nbr_ip = st_Next.ipaddr;
		p_index->process_id = st_Next.process_id;
		ospf_logx(ospf_debug, " get first Neighbor process id =%d,%d,%d.\n",
				st_Next.process_id, st_Next.ipaddr, st_Next.addrlessif);
		return OK;
	}

	return ERR;
}

int getNextNeighbor(tOSPFMIB_NBR_INDEX *p_index,
		tOSPFMIB_NBR_INDEX *p_Nextindex)
{
	int rc = OK;
	tOSPF_NBR_INDEX st_Index =
	{ 0 }, st_Next =
	{ 0 };
	u_int ulFlag = 0;
	if ((p_index == NULL) || (p_Nextindex == NULL))
	{
		return ERR;
	}

	for (rc = ospfNbrGetSelect(NULL, &st_Next); rc == OK;
			rc = ospfNbrGetSelect(&st_Index, &st_Next))
	{

		if (
#ifdef OSPF_DCN
		(st_Next.process_id == OSPF_DCN_PROCESS) ||
#endif
				(st_Next.process_id == 0))
		{
			ospf_logx(ospf_debug, " get Next Neighbor process id = %d,%d,%d.\n",
					st_Next.process_id, st_Next.ipaddr, st_Next.addrlessif);
			memcpy(&st_Index, &st_Next, sizeof(tOSPF_NBR_INDEX));
			memset(&st_Next, 0, sizeof(tOSPF_NBR_INDEX));
			continue;
		}

		if ((st_Next.process_id == p_index->process_id)
				&& (st_Next.ipaddr == p_index->nbr_ip))
		{
			ulFlag = 1;
			memcpy(&st_Index, &st_Next, sizeof(tOSPF_NBR_INDEX));
			memset(&st_Next, 0, sizeof(tOSPF_NBR_INDEX));
			continue;
		}
		if (ulFlag == 1)
		{
			p_Nextindex->nbr_ip = st_Next.ipaddr;
			p_Nextindex->process_id = st_Next.process_id;
			ospf_logx(ospf_debug, "get Next Neighbor process id = %d,%d,%d.\n",
					st_Next.process_id, st_Next.ipaddr, st_Next.addrlessif);
			return OK;
		}

		memcpy(&st_Index, &st_Next, sizeof(tOSPF_NBR_INDEX));
		memset(&st_Next, 0, sizeof(tOSPF_NBR_INDEX));
	}

	return ERR;
}

int Mib_ospf_GetNbrApi(tOSPFMIB_NBR_INDEX *pstIndx, void *vValue, u_int mib_num)
{
	int ret = OK;
	tOSPF_NBR_INDEX stOspfNbr;
	if ((vValue == NULL) || (pstIndx == NULL))
	{
		return ERR;
	}
	stOspfNbr.addrlessif = 0;
	stOspfNbr.ipaddr = pstIndx->nbr_ip;
	stOspfNbr.process_id = pstIndx->process_id;

	ret = ospfNbrGetApi(&stOspfNbr, mib_num, vValue);
	ospf_logx(ospf_debug, " ret = 0x%x,mib num = %d,process id = %d\n", ret,
			mib_num, stOspfNbr.process_id);
	if (ret != OK)
	{
		return ERR;
	}
	return OK;
}

#if 1
int getFirstRoute(tOSPFMIB_ROUTE_INDEX *p_index)
{
	int ret = 0;
	tOSPF_ROUTE_INDEX stIndex;

	if (p_index == NULL)
	{
		return ERR;
	}

	ret = ospfNetwrokRouteGetFirst(&stIndex);
	if (ret != OK)
	{
		return ERR;
	}

	p_index->process_id = stIndex.process_id;
	p_index->route_dest = stIndex.dest;
	p_index->route_mask = stIndex.mask;
#if 0
	while(ret == OK)
	{
		ret = ospfNetwrokRouteGetNext(&stIndex,&st_Next);
		if((stIndex.process_id == 0)|| (stIndex.process_id== OSPF_DCN_PROCESS))
		{
			stIndex = st_Next;
			continue;
		}
		p_index->process_id = stIndex.process_id;
		p_index->route_dest = stIndex.dest;
		p_index->route_mask = stIndex.mask;
		return OK;
	}
#endif

	return OK;
	//return ERR;
}

int getNextRoute(tOSPFMIB_ROUTE_INDEX *p_index,
		tOSPFMIB_ROUTE_INDEX *p_Nextindex)
{
	int ret = 0;
	tOSPF_ROUTE_INDEX stIndex, st_Next;

	if ((p_index == NULL) || (p_Nextindex == NULL))
	{
		return ERR;
	}

	stIndex.process_id = p_index->process_id;
	stIndex.dest = p_index->route_dest;
	stIndex.mask = p_index->route_mask;
	ret = ospfNetwrokRouteGetNext(&stIndex, &st_Next);
	if (ret != OK)
	{
		return ERR;
	}
	p_Nextindex->process_id = st_Next.process_id;
	p_Nextindex->route_dest = st_Next.dest;
	p_Nextindex->route_mask = st_Next.mask;
	return OK;
#if 0

	ret = ospfNetwrokRouteGetFirst(&stIndex);
	if(ret != OK)
	{
		return ERR;
	}

	while(ret == OK)
	{
		ret = ospfNetwrokRouteGetNext(&stIndex,&st_Next);
		if((stIndex.process_id == 0)|| (stIndex.process_id== OSPF_DCN_PROCESS))
		{
			stIndex = st_Next;
			continue;
		}
		if((stIndex.process_id == p_index->process_id)&&(stIndex.dest== p_index->route_dest)
				&&(stIndex.mask== p_index->route_mask))
		{
			ulFlag =1;
			stIndex = st_Next;
			continue;
		}
		if(ulFlag == 1)
		{
			p_Nextindex->process_id = stIndex.process_id;
			p_Nextindex->route_dest = stIndex.dest;
			p_Nextindex->route_mask = stIndex.mask;
			return OK;
		}
		stIndex = st_Next;

	}

	return ERR;
#endif
}

#endif
int ospfNetwrokRouteGet(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var)
{
	if ((var == NULL) || (p_index == NULL))
	{
		return ERR;
	}

	int ret = OK, j = 0;
	char ucStr[32] =
	{ 0 }, nxtHop[512] =
	{ 0 };
	u_long ulValue = 0, nextHopCnt = 0, ulNexthop = 0, ulAreaid = 0;
	u_long *pVa = (u_long *) var;
	tlv_t octet;
	octet.data = (u_char *) ucStr;
	octet.len = sizeof(ucStr);
	switch (cmd)
	{
	case OSPF_ROUTE_TYPE:
		strcpy(var, "Network");
		break;
	case OSPF_ROUTE_MASK:
		*pVa = p_index->mask;
		break;
	case OSPF_ROUTE_DEST:
		*pVa = p_index->dest;
		break;
	case OSPF_ROUTE_NEXTHOP:
		ret = ospfNetwrokRouteGetApi(p_index, cmd, &octet);
		if (ret == OK)
		{
			memcpy(nxtHop, octet.data, octet.len);
			nextHopCnt = octet.len / 4;
			for (j = 0; j < nextHopCnt; j++)
			{
				memcpy(&ulNexthop, &nxtHop[4 * j], 4);
				*pVa = ulNexthop;
			}
		}
		break;
	case OSPF_ROUTE_PATHTYPE:
		ret = ospfNetwrokRouteGetApi(p_index, cmd, &ulValue);
		if (ret == OK)
		{
			switch (ulValue)
			{
			case OSPF_PATH_INTRA:
				sprintf(ucStr, "INTRA");
				break;
			case OSPF_PATH_INTER:
				sprintf(ucStr, "INTER");
				break;
			case OSPF_PATH_ASE:
				sprintf(ucStr, "ASE");
				break;
			case OSPF_PATH_ASE2:
				sprintf(ucStr, "ASE2");
				break;
			default:
				sprintf(ucStr, "Unkown");
				break;
			}
			strcpy(var, ucStr);
		}
		break;
	case OSPF_ROUTE_BACKUPNEXTHOP:
	case OSPF_ROUTE_COST:
	case OSPF_ROUTE_COST2:
		ret = ospfNetwrokRouteGetApi(p_index, cmd, &ulValue);
		if (ret == OK)
		{
			*pVa = ulValue;
		}
		break;
	case OSPF_ROUTE_AREA:
		ret = ospfNetwrokRouteGetApi(p_index, OSPF_ROUTE_NEXTHOP, &octet);
		if (ret == OK)
		{
			memcpy(nxtHop, octet.data, octet.len);
			nextHopCnt = octet.len / 4;
			for (j = 0; j < nextHopCnt; j++)
			{
				memcpy(&ulNexthop, &nxtHop[4 * j], 4);
				ret = ospf_get_AreaId(p_index->process_id, (u_int) ulNexthop,
						p_index->mask, &ulAreaid);
				*pVa = ulAreaid;
			}
		}
		break;
	default:
		return ERR;
	}
	return ret;

}

int Mib_RouteGetApi(tOSPFMIB_ROUTE_INDEX *p_index, u_int cmd, void *var)
{
	int ret = OK;

	tOSPF_ROUTE_INDEX stIndex;
	if ((var == NULL) || (p_index == NULL))
	{
		return ERR;
	}

	stIndex.process_id = p_index->process_id;
	stIndex.dest = p_index->route_dest;
	stIndex.mask = p_index->route_mask;
	stIndex.type = OSPF_ROUTE_NETWORK;

	ret = ospfNetwrokRouteGet(&stIndex, cmd, var);

	return ERR;
}

/* ͨ��ʵ��ID��ȡ����ID*/

int ospf_get_AreaId(u_int ulProid, u_int ulDest, u_int ulMask, u_int *ulAreaid)
{
	tOSPF_NETWORK_INDEX stIndex, stNextIndex;
	int ret = 0;
	if (ulAreaid == NULL)
	{
		return ERR;
	}

	ret = getFirstNode(&stIndex);
	if (ret != OK)
	{
		return ERR;
	}

	while (ret == OK)
	{
		ret = getNextNode(&stIndex, &stNextIndex);
#if 0
		if((stIndex.process_id == ulProid)&&(stIndex.network== ulDest)
				&&(stIndex.mask== ulMask))
#endif
		if ((stIndex.process_id == ulProid)
				&& ((stIndex.network & stIndex.mask) == (ulDest & stIndex.mask)))
		{
			*ulAreaid = stIndex.area;
			return OK;
		}
		stIndex = stNextIndex;
	}
	return ERR;
}

int ospf_save_InterfaceTab_Api(struct vty *vty, tOSPF_IFINDEX *p_index,
		u_int cmd)
{
	int ret = OK;
	u_long uValue = 0, ulKeyId = 0, ulStatus = 0;
	u_long ulDis = 0;
	u_char ucKey[32] =
	{ 0 };
	char acIfType[20] =
	{ 0 };
	tlv_t stOctet;
	tOSPF_IFMETRIC_INDEX stOspfCostIndex;
	stOctet.data = ucKey;
	stOctet.len = sizeof(ucKey);
	//static u_long hello_interval = OSPF_DEFAULT_HELLO_INTERVAL;
	u_long ulHelloVal = 0;

	if (p_index == NULL)
	{
		return ERR;
	}
	if (cmd == OSPF_IF_COST)
	{
		stOspfCostIndex.process_id = p_index->process_id;
		stOspfCostIndex.ifip = p_index->ipaddr;
		stOspfCostIndex.ifindex = p_index->addrlessif;
		stOspfCostIndex.tos = 0;      //0-7
		ret = ospfIfMetricGetApi(&stOspfCostIndex, OSPF_IFMETRIC_VALUE,
				&uValue);
		if (ret != OK)
		{
			return ERR;
		}
		if ((ospfIfMetricGetApi(&stOspfCostIndex, OSPF_IFMERTRIC_COSTSTATUS,
				&ulStatus) == OK))
		{
			if (ulStatus && (uValue != 1))
			{
				vty_out(vty, " ospf cost %d%s", uValue, VTY_NEWLINE);
			}
		}
#if 0
		if(uValue != 1)
		{
			if((ospfIfMetricGetApi(&stOspfCostIndex,OSPF_IFMETRIC_RATE, &ulRate) == OK) && (ulRate == OSPF_DEFAULT_REF_RATE))
			{
				printf("ulRate = %lu\n",ulRate);
				vty_out(vty, " ospf cost %d%s",uValue, VTY_NEWLINE);
			}
		}
#endif
	}
	else if (cmd == OSPF_IF_SAVE_KEY)
	{
		ret = ospfIfGetApi(p_index, OSPF_IF_AUTHTYPE, &uValue);
		ret |= ospfIfGetApi(p_index, OSPF_IF_AUTHDIS, &ulDis);
		if (ret != OK)
		{
			return ERR;
		}
		if (ulDis == OSPF_AUTHDIS_CIPHER)
		{
			ret = ospfIfGetApi(p_index, OSPF_IF_CIPHERKEY, &stOctet);
		}
		else
		{
			ret = ospfIfGetApi(p_index, OSPF_IF_AUTHKEY, &stOctet);
		}
		if (uValue == OSPF_AUTH_SIMPLE)      //������
		{
			if (strlen((char *) stOctet.data) == 0)
			{
				vty_out(vty, " ospf authentication-mode simple %s",
						VTY_NEWLINE);
			}
			else
			{
				vty_out(vty, " ospf authentication-mode simple %s %s%s",
						(ulDis == OSPF_AUTHDIS_CIPHER) ? "cipher" : "plain",
						stOctet.data, VTY_NEWLINE);
			}
		}
		else if (uValue == OSPF_AUTH_MD5)      //md5����
		{
			if ( OK == ospfIfGetApi(p_index, OSPF_IF_AUTHKEYID, &ulKeyId))
			{
				if (!ulKeyId)
				{
					vty_out(vty, " ospf authentication-mode md5 %s",
							VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, " ospf authentication-mode md5 %d %s %s%s",
							ulKeyId,
							(ulDis == OSPF_AUTHDIS_CIPHER) ? "cipher" : "plain",
							stOctet.data, VTY_NEWLINE);
				}
			}
		}
	}
	else
	{
		ret = ospfIfGetApi(p_index, cmd, &uValue);
		if (ret != OK)
		{
			return ERR;
		}
		switch (cmd)
		{
		case OSPF_IF_HELLOINTERVAL:
			if (uValue != OSPF_DEFAULT_HELLO_INTERVAL)
			{
				//hello_interval = uValue;
				vty_out(vty, " ospf hello-interval %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_PRIORITY:
			if (uValue != OSPF_DEFAULT_PRIORITY)
			{
				vty_out(vty, " ospf priority %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_DEADINTERVAL:
			//if(uValue != OSPF_DEFAULT_ROUTER_DEAD_INTERVAL)
			if (ospfIfGetApi(p_index, OSPF_IF_HELLOINTERVAL, &ulHelloVal) != OK)
			{
				return ERR;
			}
			if (uValue != (4 * ulHelloVal))
			{
				vty_out(vty, " ospf dead-interval %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_TRANSITDELAY:
			if (uValue != OSPF_DEFAULT_TRANSMIT_DELAY)
			{
				vty_out(vty, " ospf transmit-delay %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_RETRANSMITINTERVAL:
			if (uValue != OSPF_DEFAULT_RETRANSMIT_INTERVAL)
			{
				vty_out(vty, " ospf retransmit-interval %d%s", uValue,
						VTY_NEWLINE);
			}
			break;
		case OSPF_IF_POLLINTERVAL:
			if (uValue != OSPF_DEFAULT_POLL_INTERVAL)
			{
				vty_out(vty, " ospf poll-interval %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_MTU:
			if (uValue != OSPF_DEFAULT_IP_MTU)
			{
				vty_out(vty, " mtu %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_MTUIGNORE:
			if (uValue != TRUE)
			{
				vty_out(vty, " ospf mtu-enable%s", VTY_NEWLINE);
			}
			break;
		case OSPF_IF_TEADMINGROUP:
			if (uValue != 0)
			{
				vty_out(vty, " ospf te admin-group %d%s", uValue, VTY_NEWLINE);
			}
			break;

		case OSPF_IF_TECOST:
			if (uValue != 0)
			{
				vty_out(vty, " ospf te cost %d%s", uValue, VTY_NEWLINE);
			}
			break;
		case OSPF_IF_TEENABLE:
			if (uValue == TRUE)
			{
				vty_out(vty, " ospf te enable%s", VTY_NEWLINE);
			}
			break;

		case OSPF_IF_TEMAXBINDWIDTH:
			if (uValue != 0)
			{
				vty_out(vty, " ospf te max-bandwidth %d%s", uValue,
						VTY_NEWLINE);
			}
			break;
		case OSPF_IF_TEMAXRSVDBINDWIDTH:
			if (uValue != 0)
			{
				vty_out(vty, " ospf te max-reserve-bandwidth %d%s", uValue,
						VTY_NEWLINE);
			}
			break;

#if 0
			case OSPF_IF_PASSIVE:
			if(uValue != 0)
			{
				vty_out(vty, " ospf passive-interface%s", VTY_NEWLINE);
			}
			break;
#endif

		case OSPF_IF_TYPE:
		{
			switch (uValue)
			{
			case OSPF_IFT_BCAST:
				strcpy(acIfType, "broadcast");
				break;
			case OSPF_IFT_NBMA:
				strcpy(acIfType, "nbma");
				break;
			case OSPF_IFT_PPP:
				strcpy(acIfType, "p2p");
				break;
			case OSPF_IFT_P2MP:
				strcpy(acIfType, "p2multip");
			default:
				break;
			}
			if (uValue != OSPF_IFT_BCAST)
			{
				vty_out(vty, " ospf if-type %s\n", acIfType);
			}
		}
			break;

#ifdef HAVE_BFD
			case OSPF_IF_BFD:
			if(uValue == OSPF_BFD_IF_ENABLE)
			{
				vty_out(vty, " ospf bfd enable%s", VTY_NEWLINE);
			}
			else if (uValue == OSPF_BFD_BLOCK)
			{
				vty_out(vty, " ospf bfd block%s", VTY_NEWLINE);
			}
			break;

			case OSPF_IF_BFD_MIN_RX_INTERVAL:
			if((uValue > 10)&&(uValue <= 1000))
			{
				vty_out(vty, " ospf bfd min-rx-interval %d%s",uValue, VTY_NEWLINE);
			}
			break;

			case OSPF_IF_BFD_MIN_TX_INTERVAL:
			if((uValue > 10)&&(uValue <= 1000))
			{
				vty_out(vty, " ospf bfd min-tx-interval %d%s",uValue, VTY_NEWLINE);
			}
			break;

			case OSPF_IF_BFD_DETECT_MUL:
			if((uValue > 3)&&(uValue <= 50))
			{
				vty_out(vty, " ospf bfd detect-multiplier %d%s",uValue, VTY_NEWLINE);
			}
			break;
#endif                
		case OSPF_IF_LDP_SYNC:
			if (uValue != FALSE)
			{
				vty_out(vty, " ospf ldp-sync%s", VTY_NEWLINE);
			}
			break;
		case OSPF_IF_HOLD_DOWN:
			if (uValue != OSPF_DEFAULT_HOLD_DOWN_TIME)
			{
				vty_out(vty, " ospf timer ldp-sync hold-down %d%s", uValue,
						VTY_NEWLINE);
			}
			break;
		case OSPF_IF_HOLD_COST:
			if (uValue != OSPF_DEFAULT_HOLD_COST_TIME)
			{
				vty_out(vty, " ospf timer ldp-sync hold-max-cost %d%s", uValue,
						VTY_NEWLINE);
			}
			break;

		default:
			return ERR;
		}
	}

	return OK;
}

void Creat_ospf_IpaddrApi(u_int ulIfIndex, u_int ipaddr)
{
	BOOL cfg_flg = ospf_get_cfg_state();/*�ϵ���ɱ�־*/

	if (TRUE == cfg_flg)/*����ȡ�����ļ�ʱ���������*/
	{
		return;
	}

	//zhurish ospf_updataipaddr(0, ulIfIndex, ipaddr, 1);
}

int ospf_IntfIpAddr_get(u_long ulIfIndex, struct prefix *pstPrefx)
{
	int iRet = ERR;

	if (pstPrefx == NULL)
	{
		return ERR;
	}

	//iRet = zebra_if_get_api(ulIfIndex, ZEBRA_IF_IPADRESS_GET, pstPrefx);

	return iRet;
}

int ospfGetPortLinkStates(u_int ulIfIndex)
{
	u_int ulPort = 0;
	u_int ulIndex = 0;
	u_char ucAdminDown = 0;
	u_char ucLinkState = 0;
#if 0
	if (IFINDEX_TYPE_ETH_TRUNK == IFINDEX_TO_TYPE(ulIfIndex))
	{
		return OK;
	}

	if (IFINDEX_TYPE_ETH_VTRUNK == IFINDEX_TO_TYPE(ulIfIndex))
	{
		return OK;
	}

	if (IFINDEX_TYPE_VPIF != IFINDEX_TO_TYPE(ulIfIndex)) /*�ӿ���������ת��*/
	{
		ulIndex = ulIfIndex;
	}
	else /*�ӽӿ�����ת��Ϊ�ӿ�����*/
	{
		if (ERR == if_vport_ifindex_to_logic_port(ulIfIndex, &ulPort))
		{
			return ERR;
		}
		if (ERR == if_logic_port_to_index(ulPort, &ulIndex))
		{
			return ERR;
		}
	}
	port_get_api(ulIndex, PORT_API_ADMIN_DOWN, &ucAdminDown);
	port_get_api(ulIndex, PORT_API_REAL_LINK, &ucLinkState);

	if (PORT_ADMIN_DOWN == ucAdminDown)
	{
		return ERR;
	}
	else if (PORT_LINK_DOWN == ucLinkState)
	{
		return ERR;
	}
#endif
	return OK;
}

void ospf_lsa_update(u_int uiProcessId)
{
	struct ospf_process *p_process = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;

	p_process = ospf_process_lookup(&ospf, uiProcessId);

	if (p_process != NULL)
	{
		for_each_node(&p_process->area_table, p_area, p_nextarea)
		{
			ospf_router_lsa_originate(p_area);
		}
	}
}

#ifdef HAVE_BFD
int Bfd_ospf_nbr_del_api(struct ospf_nbr *p_nbr)
{
	struct ospf_nbr *p_next = NULL;
	struct ospf_if *p_if = p_nbr->p_if;
	int iRet = OK;

	ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
	ospf_nbr_delete(p_nbr);

	return OK;
}
#endif

#if 0
void ospf_update_route_for_pre_chg(u_int ulProId, u_int ulCmd)
{
	tOSPF_ROUTE_INDEX stOspfAbrIndex =
	{ 0 }, stOspfAbrNextIndex =
	{ 0 };
	int iRetRout = 0;
	int i = 0, j = 0;
	u_long ulTpye = 0;
	struct ospf_iproute stOspfRoute =
	{ 0 };
	struct ospf_process * pstProcess = NULL;
	tlv_t stOctet;
	int iCost = 0;
	u_char aucBuf[256] =
	{ 0 };
	u_char aucNextHop[256] =
	{ 0 };
	u_char aucNextHopIndex[256] =
	{ 0 };
	int iNextHopCnt = 0;
	u_int ulNexthop = 0;
	//ZEBRA_ROUTE_MSG_T stRouteIndex = { 0 };
	u_char ucValue = 0;
	u_int uiDest = 0, uiNextHop = 0, uiNexHopIndex = 0;
	;
	struct in_addr stMask;

	stOctet.data = aucBuf;
	stOctet.len = sizeof(aucBuf);

	for (i = OSPF_ROUTE_ASBR; i <= OSPF_ROUTE_NETWORK; i++)
	{
		memset(&stOspfAbrIndex, 0, sizeof(tOSPF_ROUTE_INDEX));
		iRetRout = ospfRouteGetFirst(&stOspfAbrIndex, i);
		while (OK == iRetRout)
		{
			iRetRout = ospfRouteGetNext(&stOspfAbrIndex, &stOspfAbrNextIndex,
					i);
			if (stOspfAbrIndex.process_id != ulProId)
			{
				stOspfAbrIndex = stOspfAbrNextIndex;
				continue;
			}

			if (OK
					== ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_PATHTYPE,
							&ulTpye, i))
			{
				if (((ulCmd == OSPF_GLB_PREFERENCE)
						&& ((ulTpye == OSPF_PATH_INTRA)
								|| (ulTpye == OSPF_PATH_INTER)))
						|| ((ulCmd == OSPF_GLB_ASE_PREFERENCE)
								&& ((ulTpye == OSPF_PATH_ASE)
										|| (ulTpye == OSPF_PATH_ASE2))))
				{
					stOspfRoute.path_type = ulTpye;
				}
				else
				{
					stOspfAbrIndex = stOspfAbrNextIndex;
					continue;
				}
				pstProcess = ospf_process_lookup(&ospf, ulProId);
				ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST, &iCost, i);
				stOspfRoute.metric = iCost;
				stOspfRoute.dest = stOspfAbrIndex.dest;
				stOspfRoute.mask = stOspfAbrIndex.mask;
				stOspfRoute.p_process = pstProcess;
				ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_NEXTHOP, &stOctet,
						i);
				memcpy(aucNextHop, stOctet.data, stOctet.len);
				iNextHopCnt = stOctet.len / 4;
				memset(stOctet.data, 0, sizeof(stOctet.data));
				stOctet.len = sizeof(stOctet.data);
				ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_NEXTHOP_IFINDEX,
						&stOctet, i);
				memcpy(aucNextHopIndex, stOctet.data, stOctet.len);
				for (j = 0; j < iNextHopCnt; j++)
				{
					memcpy(&ulNexthop, &aucNextHop[4 * j], 4);
					stOspfRoute.fwdaddr = ulNexthop;
					pstProcess->p_master->stat.rt_pre_cnt++;

					memcpy(&uiNexHopIndex, &aucNextHopIndex[4 * j], 4);
					stOspfRoute.if_unit = uiNexHopIndex;

					stRouteIndex.family = AF_INET;
					stRouteIndex.uiVrfId = pstProcess->vrid;
					if (stOspfRoute.path_type == OSPF_PATH_INTRA
							|| stOspfRoute.path_type == OSPF_PATH_INTER)
					{
						stRouteIndex.uiRouteProtocolType =
								ZEBRA_ROUTE_PROTOCOL_OSPF;
					}
					else
					{
						stRouteIndex.uiRouteProtocolType =
								ZEBRA_ROUTE_PROTOCOL_OSPF_ASE;
					}
					stMask.s_addr = (in_addr_t)(ntohl(stOspfRoute.mask));
					stRouteIndex.uiIpMaskLen = ip_masklen(stMask);
					uiDest = htonl(stOspfRoute.dest);
					uiNextHop = htonl(ulNexthop);
					memcpy(&stRouteIndex.stDstIp, &uiDest, 4);
					memcpy(&stRouteIndex.stNexthop, &uiNextHop, 4);
					if (VOS_OK
							== zebra_ip_route_get_api(&stRouteIndex,
									ZEBRA_ROUTE_EXIST, &ucValue))
					{
						ospf_add_route(&stOspfRoute, pstProcess);
					}
				}
			}

			stOspfAbrIndex = stOspfAbrNextIndex;
		}
	}
}
#endif

#ifdef OSPF_MASTER_SLAVE_SYNC
int ospf_master_slave_state_get(u_int *puiState)
{
	*puiState = ospf.syn_work_state;
	// printf("ospf_master_slave_state_get uiState=%d\n",ospf.syn_work_state);

	return OK;
}

int ospf_master_slave_state_set(u_int uiState)
{
	if(uiState != ospf.syn_work_state)
	{
		ospf.syn_work_state = uiState;
		//printf("ospf_master_slave_state_set uiState=%d\n",ospf.syn_work_state);
	}
	return OK;

}

int ospf_dyn_data_send()
{

	//ospf_timer_start(&ospf.sync_check_timer, OSPF_INIT_SYNC_DELAY_TIME);
	ospf_slaver_card_up();
	return OK;
}
#endif

int ospf_mpls_area_select(u_int uiProcessId, u_int *puiAreaid)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_next_area = NULL;
	struct ospf_process *p_process = NULL;

	p_process = ospf_process_lookup(&ospf, uiProcessId);
	p_area = ospf_area_lookup(p_process, OSPF_BACKBONE);
	if (p_area != NULL)
	{
		*puiAreaid = OSPF_BACKBONE;
		return OK;
	}

	for_each_ospf_area(p_process, p_area, p_next_area)
	{
		*puiAreaid = p_area->id;
		return OK;
	}
	return ERR;

}

int ospf_mpls_area_lookup(u_int uiProcessId, u_int uiIpaddr, u_int *puiAreaid)
{
	struct ospf_process *p_process = NULL;
	struct ospf_network *p_network = NULL;

	p_process = ospf_process_lookup(&ospf, uiProcessId);
	if (p_process == NULL)
	{
		return ERR;
	}
#if 0
	p_network = ospf_network_lookup(p_process, uiIpaddr, OSPF_DCNDEF_IPMASK);
	if (p_network != NULL)
	{
		*puiAreaid = p_network->area_id;
		return OK;
	}
#endif
	return ERR;

}

#ifdef OSPF_LSRID
int ospf_mpls_lsr_id_adv(u_int uiProcessId,u_int uiIpaddr,u_int uiCost,u_char ucMode)
{
	u_long ulValue = 0;
	u_int i = 0;
	int iRet = ERR;
	struct ospf_process *p_process = NULL;

	ospf_semtake_try();
	if(TRUE == ucMode)
	{
		if((uiProcessId == 0)
				|| (uiIpaddr == 0))
		{
			iRet = ERR;
			goto END;
		}
	}

	p_process = ospf_process_lookup(&ospf, uiProcessId);
	if((p_process == NULL)
			|| ((ucMode == p_process->mpls_flag)
					&& (uiCost == p_process->mpls_cost)))
	{
		iRet = OK;
		goto END;
	}
	p_process->mpls_lsrid = uiIpaddr;
	p_process->mpls_flag = ucMode;
	p_process->mpls_cost = uiCost;
	/*����mpls te�����ʱʹ��mpls rsvp-teʹ��״̬����*/
	iRet = rsvpGetApi(NULL, RSVP_GLOBAL_STATUS, &ulValue);
	if(iRet == OK)
	{
		if (ulValue == ENABLE)
		{
			ulValue = TRUE;
		}
		else
		{
			ulValue = FALSE;
		}
		p_process->mpls_te = ulValue;
	}
	p_process->mpls_te = TRUE;/*��ʱȡ����mpls rsvp-te�Ĺ���*/
	/*end*/
	ospf_lsa_update(uiProcessId);

	END:
	ospf_semgive();
	return iRet;
}
#endif

#ifdef OSPF_MASTER_SLAVE_SYNC
int ospf_master_slave_dyn_data_sync_recv(int iModid, u_char *pBuf, int iLen)
{
	int iRet = OK;
	int iMytype = 0;
	int iSubModId = 0;
	char *pcTmpVar = NULL;
	octetstring stOctet =
	{	0};
	char cszStr[SYN_CFG_DEF_DATE_LEN] =
	{	0};
	u_int uiIfIndex = 0;
	iMytype = (iModid >> 8);
	iSubModId = (iModid & 0xff);
	//printf("1111 ospf_master_slave_dyn_data_sync_recv iModid:%x,pBuf=%x,iLen=%d\n",iModid,pBuf,iLen);

	switch (iSubModId)
	{
		case MTYPE_OSPF_SYNC_DYN_SUB_MODID:
		stOctet.len = iLen;
		stOctet.pBuf = pBuf;

		vos_pthread_lock(&ospf.lock_sem);
		ospf_syn_recv(&stOctet);
		vos_pthread_unlock(&ospf.lock_sem);
		break;
		default :
		iRet = ERR;
		break;
	}

	return iRet;
}
#endif

#ifdef OSPF_TE
int ospf_te_if_set_api(uint32_t index,int32_t cmd, void* var)
{
	int iRet = OK;
	u_long ulIfunit = 0, ulProcessId = 0, ulAddr = 0;
	tOSPF_IFINDEX stOspfIfIndex =
	{	0};

	ulIfunit = index;
	iRet = ospf_ifindex_to_process(ulIfunit, &ulProcessId, &ulAddr);
	if(OK != iRet)
	{
		return ERR;
	}

	stOspfIfIndex.process_id = ulProcessId;
	stOspfIfIndex.addrlessif = ulIfunit;
	stOspfIfIndex.ipaddr = ulAddr;

	switch(cmd)
	{
		case RSVP_IF_LINK_ADMIN_GROUP:
		{
			iRet = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_TEADMINGROUP, var);
			break;
		}
		case RSVP_IF_MAX_RESERV:
		{
			iRet = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_TEMAXBINDWIDTH, var);
			iRet = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_TEMAXRSVDBINDWIDTH, var);
			break;
		}
		case RSVP_IF_METRIC:
		{
			iRet = ospfIfSetApi(&stOspfIfIndex, OSPF_IF_TECOST, var);
			break;
		}
		default:
		{
			iRet = OK;
			break;
		}

	}

	return iRet;
}
#endif

u_int ospf_router_lsa_lsrid_fill(struct ospf_router_link *p_link,
		struct ospf_area *p_area)
{
	u_int len = 0;
	u_int uiAddr = 0;
	struct ospf_process *p_process = p_area->p_process;

	uiAddr = p_process->mpls_lsrid;

	p_link->id = uiAddr;
	p_link->data = htonl(OSPF_HOST_MASK);
	p_link->type = OSPF_RTRLINK_STUB;
	p_link->tos_count = 0;
	p_link->tos0_metric = htons(p_process->mpls_cost);
	len = ospf_router_link_len(p_link);

	return len;
}

#ifdef OSPF_LSRID
/*mpls teȫ��ʹ�ܱ仯ʱ,ospf����mpls lsr id*/
/*mpls teȫ��ʹ���������ޣ�ʹ��mpls rsvp-teʹ��*/
int ospf_lsrid_refresh(u_char ucMode)
{
	int iRet = OK;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nxtinstance = NULL;
	u_int uiIpaddr = 0;

	iRet = mpls_glb_get_api(MPLS_GLB_API_LSR_ID, &uiIpaddr);
	if(iRet != OK)
	{
		uiIpaddr = 0;
	}
	uiIpaddr = ntohl(uiIpaddr);

	ospf_semtake_try();

	for_each_ospf_process(p_process, p_nxtinstance)
	{
		if((p_process->process_id == OSPF_DCN_PROCESS)||(p_process->process_id == 0))
		{
			continue;
		}
		if(TRUE == p_process->mpls_flag)
		{
			p_process->mpls_te = ucMode;
			p_process->mpls_lsrid = uiIpaddr;
			ospf_lsa_update(p_process->process_id);
		}
	}

	ospf_semgive();
	return iRet;
}
#endif

u_int ospf_vfindextolpindex(u_int ulVfIndex)
{
	u_int ulPort;
	u_int ulIndex = 0;
#if 0
	if (IFINDEX_TYPE_VPIF == IFINDEX_TO_TYPE(ulVfIndex))
	{
		if (ERR == if_vport_ifindex_to_logic_port(ulVfIndex, &ulPort))
		{
			return ERR;
		}
		if (ERR == if_logic_port_to_index(ulPort, &ulIndex))
		{
			return ERR;
		}
	}
	else
	{
		ulIndex = ulVfIndex;
	}
#endif
	return ulIndex;
}

STATUS ospfRouteGetFirst(tOSPF_ROUTE_INDEX *pstRouteIndex, u_int ulIndex)
{
	int ret = ERR;

	if (pstRouteIndex == NULL)
	{
		return ret;
	}

	switch (ulIndex)
	{
	case OSPF_ROUTE_ABR:
		ret = ospAbrRouteGetFirst(pstRouteIndex);
		break;
	case OSPF_ROUTE_ASBR:
		ret = ospAsbrRouteGetFirst(pstRouteIndex);
		break;
	case OSPF_ROUTE_NETWORK:
		ret = ospfNetwrokRouteGetFirst(pstRouteIndex);
		break;
	default:
		ret = ERR;
		break;
	}
	return ret;
}

STATUS ospfRouteGetNext(tOSPF_ROUTE_INDEX *pstRuIndex,
		tOSPF_ROUTE_INDEX *pstNeRuIndex, u_int ulIndex)
{
	int ret = ERR;

	if ((pstRuIndex == NULL) || (pstNeRuIndex == NULL))
	{
		return ret;
	}

	switch (ulIndex)
	{
	case OSPF_ROUTE_ABR:
		ret = ospfAbrRouteGetNext(pstRuIndex, pstNeRuIndex);
		break;
	case OSPF_ROUTE_ASBR:
		ret = ospfAsbrRouteGetNext(pstRuIndex, pstNeRuIndex);
		break;
	case OSPF_ROUTE_NETWORK:
		ret = ospfNetwrokRouteGetNext(pstRuIndex, pstNeRuIndex);
		break;
	default:
		break;
	}
	return ret;
}

STATUS ospfRouteGetApi(tOSPF_ROUTE_INDEX *pstRouteIndex, u_int cmd, void *var,
		u_int ulIndex)
{
	int ret = ERR;

	if (pstRouteIndex == NULL)
	{
		return ret;
	}

	switch (ulIndex)
	{
	case OSPF_ROUTE_ABR:
		ret = ospfAbrRouteGetApi(pstRouteIndex, cmd, var);
		break;
	case OSPF_ROUTE_ASBR:
		ret = ospfAsbrRouteGetApi(pstRouteIndex, cmd, var);
		break;
	case OSPF_ROUTE_NETWORK:
		ret = ospfNetwrokRouteGetApi(pstRouteIndex, cmd, var);
		break;
	default:
		break;
	}
	return ret;
}

#if 0
int ospf_network_getroute_func(struct vty * vty, u_int ulProcessId)
{
	tOSPF_ROUTE_INDEX stOspfAbrIndex =
	{ 0 }, stOspfAbrNextIndex =
	{ 0 };
	int retRout = ERR, ret = 0;
	ULONG value_l = 0;
	char destStr[64] =
	{ 0 }, nxthopStr[16] =
	{ 0 }, path_type[16] =
	{ 0 }, maskStr[64] =
	{ 0 };
	char areaStr[16] =
	{ 0 }, flag = 0, tmpNxthopStr[16] =
	{ 0 }, nxtHop[512] =
	{ 0 };
	int nextHopCnt = 0, i, j, cost = 0, cost2 = 0, type = 0, tag = 0;
	int ifag = 0;
	octetstring octet;
	u_char idxBuf[32] =
	{ 0 };
	char RouTypeStr[3][10] =
	{ "ASBR", "ABR", "Network" };
	int imask;

	octet.pucBuf = idxBuf;
	octet.len = sizeof(idxBuf);
	for (i = OSPF_ROUTE_ASBR; i <= OSPF_ROUTE_NETWORK; i++)
	{
		memset(&stOspfAbrIndex, 0, sizeof(tOSPF_ROUTE_INDEX));
		retRout = ospfRouteGetFirst(&stOspfAbrIndex, i);
		if (VOS_OK != retRout)
		{
			continue;
		}
		while (VOS_OK == retRout)
		{
			retRout = ospfRouteGetNext(&stOspfAbrIndex, &stOspfAbrNextIndex, i);
			if (stOspfAbrIndex.process_id == ulProcessId)
			{
				if (ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_PATHTYPE,
						&value_l, i) == VOS_OK)
				{
					memset(destStr, 0, sizeof(destStr));
					memset(maskStr, 0, sizeof(maskStr));
					memset(areaStr, 0, sizeof(areaStr));
					memset(path_type, 0, sizeof(path_type));
					ospf_inet_ntoa((u_char *) destStr, stOspfAbrIndex.dest);
					ospf_inet_ntoa((u_char *) maskStr, stOspfAbrIndex.mask);
					imask = inet_maskLen(stOspfAbrIndex.mask);

					sprintf(maskStr, "%s/%d", destStr, imask);

					switch (value_l)
					{
					case OSPF_PATH_INTRA:
						sprintf(path_type, "INTRA");
						break;
					case OSPF_PATH_INTER:
						sprintf(path_type, "INTER");
						break;
					case OSPF_PATH_ASE:
						sprintf(path_type, "ASE");
						ifag = 1;
						break;
					case OSPF_PATH_ASE2:
						sprintf(path_type, "ASE2");
						ifag = 1;
						break;
					default:
						sprintf(path_type, "Unkown");
						break;
					}

					if (ifag == 1)
					{
						stOspfAbrIndex = stOspfAbrNextIndex;
						ifag = 0;
						continue;
					}

					if (flag == 0)
					{
						vty_out(vty, " %-10s%-19s%-8s%-12s%-17s%-17s%-8s%s%s",
								"RoutType", "Destination", "Cost", "PathType",
								"NextHop", "AreaId", "Cost2", "BackupNextHop",
								VTY_NEWLINE);
						flag = 1;
					}

					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST, &cost, i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST2, &cost2,
							i);

					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_EXT_TYPE, &type,
							i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_TAG, &tag, i);

					memset(nxtHop, 0, sizeof(nxtHop));
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_NEXTHOP, &octet,
							i);
					memcpy(nxtHop, octet.pucBuf, octet.len);
					nextHopCnt = octet.len / 4;
					for (j = 0; j < nextHopCnt; j++)
					{
						u_int ulNexthop = 0;
						memset(nxthopStr, 0, sizeof(nxthopStr));
						memcpy(&ulNexthop, &nxtHop[4 * j], 4);
						ret = ospf_get_AreaId(stOspfAbrIndex.process_id,
								ulNexthop, stOspfAbrIndex.mask,
								&stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) areaStr, stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) nxthopStr, ulNexthop);
						value_l = 0;
						stOspfAbrIndex.nexthop = ulNexthop;
						/*modified by chend.only network has backup nexthop*/
						if (i == OSPF_ROUTE_NETWORK)
						{
							ospfRouteGetApi(&stOspfAbrIndex,
									OSPF_ROUTE_BACKUPNEXTHOP, &value_l, i);
						}
						ospf_inet_ntoa((u_char *) tmpNxthopStr, value_l);
						vty_out(vty, " %-10s%-19s%-8d%-12s%-17s%-17s%-8d%s%s",
								RouTypeStr[i], maskStr, cost, path_type,
								nxthopStr, areaStr, cost2, tmpNxthopStr,
								VTY_NEWLINE);

					}
				}

			}
			stOspfAbrIndex = stOspfAbrNextIndex;
		}

	}

	return VOS_OK;
}

int ospf_ase_getroute_func(struct vty * vty, u_int ulProcessId)
{
	tOSPF_ROUTE_INDEX stOspfAbrIndex =
	{ 0 }, stOspfAbrNextIndex =
	{ 0 };
	int retRout = ERR, ret = 0;
	ULONG value_l = 0;
	char destStr[64] =
	{ 0 }, nxthopStr[16] =
	{ 0 }, path_type[16] =
	{ 0 }, maskStr[64] =
	{ 0 };
	char areaStr[16] =
	{ 0 }, flag = 0, tmpNxthopStr[16] =
	{ 0 }, nxtHop[512] =
	{ 0 };
	int nextHopCnt = 0, i, j, cost = 0, cost2 = 0, type = 0, tag = 0;
	int ifag = 0;
	char szcExtType[64] =
	{ 0 };
	octetstring octet;
	u_char idxBuf[32] =
	{ 0 };
	char RouTypeStr[3][10] =
	{ "ASBR", "ABR", "Network" };
	int imask;

	octet.pucBuf = idxBuf;
	octet.len = sizeof(idxBuf);
	for (i = OSPF_ROUTE_ASBR; i <= OSPF_ROUTE_NETWORK; i++)
	{
		memset(&stOspfAbrIndex, 0, sizeof(tOSPF_ROUTE_INDEX));
		retRout = ospfRouteGetFirst(&stOspfAbrIndex, i);
		if (VOS_OK != retRout)
		{
			continue;
		}
		while (VOS_OK == retRout)
		{
			retRout = ospfRouteGetNext(&stOspfAbrIndex, &stOspfAbrNextIndex, i);
			if (stOspfAbrIndex.process_id == ulProcessId)
			{
				if (ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_PATHTYPE,
						&value_l, i) == VOS_OK)
				{
					memset(destStr, 0, sizeof(destStr));
					memset(maskStr, 0, sizeof(maskStr));
					memset(areaStr, 0, sizeof(areaStr));
					memset(path_type, 0, sizeof(path_type));
					ospf_inet_ntoa((u_char *) destStr, stOspfAbrIndex.dest);
					ospf_inet_ntoa((u_char *) maskStr, stOspfAbrIndex.mask);
					imask = inet_maskLen(stOspfAbrIndex.mask);

					sprintf(maskStr, "%s/%d", destStr, imask);

					switch (value_l)
					{
					case OSPF_PATH_INTRA:
						sprintf(path_type, "INTRA");
						ifag = 1;
						break;
					case OSPF_PATH_INTER:
						sprintf(path_type, "INTER");
						ifag = 1;
						break;
					case OSPF_PATH_ASE:
						sprintf(path_type, "ASE");
						break;
					case OSPF_PATH_ASE2:
						sprintf(path_type, "ASE2");
						break;
					default:
						sprintf(path_type, "Unkown");
						ifag = 1;
						break;
					}

					if (ifag == 1)
					{
						stOspfAbrIndex = stOspfAbrNextIndex;
						ifag = 0;
						continue;
					}

					if (flag == 0)
					{
						vty_out(vty, " Routing for ASEs\n");
						vty_out(vty,
								" %-10s%-19s%-8s%-12s%-17s%-17s%-8s%-8s%-8s%s%s",
								"RoutType", "Destination", "Cost", "PathType",
								"NextHop", "AreaId", "Cost2", "Type", "Tag",
								"BackupNextHop", VTY_NEWLINE);
						flag = 1;
					}
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST, &cost, i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST2, &cost2,
							i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_TAG, &tag, i);

					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_EXT_TYPE, &type,
							i);
					switch (type)
					{
					case OSPF_PATH_ASE:
					{
						sprintf(szcExtType, "Type1");
						break;
					}
					case OSPF_PATH_ASE2:
					{
						sprintf(szcExtType, "Type2");
						break;
					}
					default:
						break;
					}

					memset(nxtHop, 0, sizeof(nxtHop));
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_NEXTHOP, &octet,
							i);
					memcpy(nxtHop, octet.pucBuf, octet.len);
					nextHopCnt = octet.len / 4;
					for (j = 0; j < nextHopCnt; j++)
					{
						u_int ulNexthop = 0;
						memset(nxthopStr, 0, sizeof(nxthopStr));
						memcpy(&ulNexthop, &nxtHop[4 * j], 4);
						ret = ospf_get_AreaId(stOspfAbrIndex.process_id,
								ulNexthop, stOspfAbrIndex.mask,
								&stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) areaStr, stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) nxthopStr, ulNexthop);
						value_l = 0;
						stOspfAbrIndex.nexthop = ulNexthop;
						/*modified by chend.only network has backup nexthop*/
						if (i == OSPF_ROUTE_NETWORK)
						{
							ospfRouteGetApi(&stOspfAbrIndex,
									OSPF_ROUTE_BACKUPNEXTHOP, &value_l, i);
						}
						ospf_inet_ntoa((u_char *) tmpNxthopStr, value_l);
						vty_out(vty,
								" %-10s%-19s%-8d%-12s%-17s%-17s%-8d%-8s%-8d%s%s",
								RouTypeStr[i], maskStr, cost, path_type,
								nxthopStr, areaStr, cost2, szcExtType, tag,
								tmpNxthopStr, VTY_NEWLINE);

					}
				}

			}
			stOspfAbrIndex = stOspfAbrNextIndex;
		}

	}

	return VOS_OK;
}

STATUS Shell_ospf_GetRoute(struct vty * vty, u_int ulProcessId)
{
	tOSPF_ROUTE_INDEX stOspfAbrIndex =
	{ 0 }, stOspfAbrNextIndex =
	{ 0 };
	int retRout = ERR, ret = 0;
	ULONG value_l = 0;
	char destStr[64] =
	{ 0 }, nxthopStr[16] =
	{ 0 }, path_type[16] =
	{ 0 }, maskStr[64] =
	{ 0 };
	char areaStr[16] =
	{ 0 }, flag = 0, tmpNxthopStr[16] =
	{ 0 }, nxtHop[512] =
	{ 0 };
	int nextHopCnt = 0, i, j, cost = 0, cost2 = 0, type = 0, tag = 0;
	octetstring octet;
	u_char idxBuf[32] =
	{ 0 };
	char RouTypeStr[3][10] =
	{ "ASBR", "ABR", "Network" };
	int imask;

	octet.pucBuf = idxBuf;
	octet.len = sizeof(idxBuf);
	for (i = OSPF_ROUTE_ASBR; i <= OSPF_ROUTE_NETWORK; i++)
	{
		memset(&stOspfAbrIndex, 0, sizeof(tOSPF_ROUTE_INDEX));
		retRout = ospfRouteGetFirst(&stOspfAbrIndex, i);
		if (VOS_OK != retRout)
		{
			continue;
		}
		while (VOS_OK == retRout)
		{
			retRout = ospfRouteGetNext(&stOspfAbrIndex, &stOspfAbrNextIndex, i);
			if (stOspfAbrIndex.process_id == ulProcessId)
			{
				if (ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_PATHTYPE,
						&value_l, i) == VOS_OK)
				{
					if (flag == 0)
					{
						vty_out(vty,
								" %-10s%-19s%-8s%-12s%-17s%-17s%-8s%-12s%-8s%s%s",
								"RoutType", "Destination", "Cost", "PathType",
								"NextHop", "AreaId", "Cost2", "BackupNextHop",
								VTY_NEWLINE);
						flag = 1;
					}
					memset(destStr, 0, sizeof(destStr));
					memset(maskStr, 0, sizeof(maskStr));
					memset(areaStr, 0, sizeof(areaStr));
					memset(path_type, 0, sizeof(path_type));
					ospf_inet_ntoa((u_char *) destStr, stOspfAbrIndex.dest);
					ospf_inet_ntoa((u_char *) maskStr, stOspfAbrIndex.mask);
					imask = inet_maskLen(stOspfAbrIndex.mask);

					sprintf(maskStr, "%s/%d", destStr, imask);

					switch (value_l)
					{
					case OSPF_PATH_INTRA:
						sprintf(path_type, "INTRA");
						break;
					case OSPF_PATH_INTER:
						sprintf(path_type, "INTER");
						break;
					case OSPF_PATH_ASE:
						sprintf(path_type, "ASE");
						break;
					case OSPF_PATH_ASE2:
						sprintf(path_type, "ASE2");
						break;
					default:
						sprintf(path_type, "Unkown");
						break;
					}

					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST, &cost, i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_COST2, &cost2,
							i);

					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_EXT_TYPE, &type,
							i);
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_TAG, &tag, i);

					memset(nxtHop, 0, sizeof(nxtHop));
					ospfRouteGetApi(&stOspfAbrIndex, OSPF_ROUTE_NEXTHOP, &octet,
							i);
					memcpy(nxtHop, octet.pucBuf, octet.len);
					nextHopCnt = octet.len / 4;
					for (j = 0; j < nextHopCnt; j++)
					{
						u_int ulNexthop = 0;
						memset(nxthopStr, 0, sizeof(nxthopStr));
						memcpy(&ulNexthop, &nxtHop[4 * j], 4);
						ret = ospf_get_AreaId(stOspfAbrIndex.process_id,
								ulNexthop, stOspfAbrIndex.mask,
								&stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) areaStr, stOspfAbrIndex.areaid);
						ospf_inet_ntoa((u_char *) nxthopStr, ulNexthop);
						value_l = 0;
						stOspfAbrIndex.nexthop = ulNexthop;
						/*modified by chend.only network has backup nexthop*/
						if (i == OSPF_ROUTE_NETWORK)
						{
							ospfRouteGetApi(&stOspfAbrIndex,
									OSPF_ROUTE_BACKUPNEXTHOP, &value_l, i);
						}
						ospf_inet_ntoa((u_char *) tmpNxthopStr, value_l);
						vty_out(vty,
								" %-10s%-19s%-8d%-12s%-17s%-17s%-8d%-12d%-8d%s%s",
								RouTypeStr[i], maskStr, cost, path_type,
								nxthopStr, areaStr, cost2, tag, type,
								tmpNxthopStr, VTY_NEWLINE);
					}
				}
			}
			stOspfAbrIndex = stOspfAbrNextIndex;
		}
	}
	return OK;
}
#endif
int ospfRoute_lookup(u_int uiDest, stOSPF_ROUTE_NET *pstRoute)
{
	struct ospf_route *p_route = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next = NULL;
	//STATUS rc = ERR;
	struct ospf_route last_route;
	u_int current = 0;
	struct ospf_nexthop nexthop;

	for_each_ospf_process(p_process, p_next)
	{
		//ospf_nm_process_check(p_process);
		for_each_node_greater(&p_process->route_table, p_route, &last_route)
		{
			if (NULL == p_route)
			{
				continue;
			}
			current = p_process->current_route;
			if (p_route->dest == uiDest)
			{
				pstRoute->process_id = p_process->process_id;
				pstRoute->areaid = p_route->path[current].p_area->id;
				pstRoute->dest = uiDest;
				pstRoute->mask = p_route->mask;
				pstRoute->type = p_route->type;
				memset(&nexthop, 0, sizeof(nexthop));
				ospf_nexthop_merge(&nexthop, p_route->path[current].p_nexthop);
				pstRoute->nexthop = nexthop.gateway[0].addr;
				pstRoute->uint = nexthop.gateway[0].if_uint;
				pstRoute->cost = p_route->path[current].cost;

				//pstRoute->priority = p_route->path[current].cost;

				pstRoute->age = p_route->path[current].cost;
#ifdef OSPF_FRR
				struct ospf_backup_route *p_broute = NULL;
				struct ospf_backup_route broute;

				broute.type = OSPF_ROUTE_NETWORK;
				broute.dest = p_route->dest;
				broute.mask = p_route->mask;
				p_broute = ospf_lstlookup(&p_process->backup_route_table,
						&broute);
				if (p_broute)
				{
					pstRoute->bk_nexthop =
							p_broute->path[p_process->backup_current_route].nexthop;
					pstRoute->bk_uint =
							p_broute->path[p_process->backup_current_route].ifunit;
				}
#endif
				return OK;
			}
		}
	}
	return ERR;
}
#if 0
/*asbr-summary��������*/
int ospf_asbr_summary_config_out(struct vty *vty, u_long *pulLine,
		u_long ulProId)
{
	int ret = 0, action = 0;
	int iRet = 0;
	char szcBuffer[256] =
	{ 0 }, acDest[32] =
	{ 0 }, acMask[32] =
	{ 0 };
	tOSPF_DISTRIBUTE_INDEX stDistributeFrist =
	{ 0 };
	tOSPF_DISTRIBUTE_INDEX stDistributeNext =
	{ 0 };

	iRet = ospfRedisRangeGetFirst(&stDistributeFrist);
	while (VOS_OK == iRet)
	{
		if (stDistributeFrist.process_id == ulProId)
		{
			ret = ospfRedisRangeGetApi(&stDistributeFrist,
					OSPF_DISTRIBUTERANGE_TRANSLATE, &action);
			if (VOS_OK != ret)
			{
				return CMD_SUCCESS;
			}
			memset(szcBuffer, 0, sizeof(szcBuffer));
			ip_int_to_ip_str(stDistributeFrist.dest, acDest);
			ip_int_to_ip_str(stDistributeFrist.mask, acMask);

			if (OSPF_MATCH_ADV == action)
			{
				sprintf(szcBuffer, " asbr-summary %s %s\n", acDest, acMask);
			}
			else
			{
				sprintf(szcBuffer, " asbr-summary %s %s no-advertise\n", acDest,
						acMask);
			}
			if (CMD_SUCCESS != vty_out_to_display(vty, pulLine, szcBuffer))
			{
				return CMD_WARNING;
			}

		}
		iRet = ospfRedisRangeGetNext(&stDistributeFrist, &stDistributeNext);

		stDistributeFrist = stDistributeNext;

	}

	return CMD_SUCCESS;
}

int ospf_if_packet_block_status_screen(struct vty *vty, u_long *pulLine,
		u_long ulInstanceId)
{
	u_char szname[32] =
	{ 0 };
	char acNhopw[512] =
	{ 0 };

	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;

	for_each_ospf_process(p_process, p_next_process)
	{
		if (p_process->process_id == ulInstanceId)
		{
			for_each_ospf_if(p_process, p_if, p_next_if)
			{
				if (p_if->passive == 1)
				{
					if (if_name_get_by_index(p_if->ifnet_uint, szname)
							== ERR)
					{
						return CMD_WARNING;
					}
					sprintf(acNhopw, " silent-interface %s \n", szname);
					if (CMD_SUCCESS
							!= vty_out_to_display(vty, pulLine, acNhopw))
					{
						return CMD_WARNING;
					}
				}
			}
		}
	}
	return CMD_SUCCESS;
}
#endif
