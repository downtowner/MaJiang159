#include "stdafx.h"
#include "ReasoningControl.h"
#include <algorithm>
#include "LDebug.h"

bool CReasoningControl::CheckVictory(STONE* pStoneArray, int nCount, int nCheckMask, IHijack* pUser)
{
#ifdef _DEBUG_
	LDCN("传入的牌:\n");
	for (int i(0); i < nCount; i++)
	{
		LDCW("%s ", Println(pStoneArray[i].eColor, pStoneArray[i].eWhat).c_str());
	}
	LDCN("\n");
#endif
	if (2 == nCount)
	{
		if (EQUAL_CARD(pStoneArray[0], pStoneArray[1]))
		{
			VCTItem groups;
			groups.clr = pStoneArray[0].eColor;
			groups.eCGType = CARD_GROUP_TYPE::CGT_DUI_ZI;
			groups.v1 = pStoneArray[0].eWhat;
			groups.v2 = pStoneArray[1].eWhat;

			VCTItem* tmp[1];
			tmp[0] = &groups;
			if (-1 == pUser->ReconstructionData(tmp, 1))
			{
				return false;
			}

			return true;
		}

		return false;
	}

	//牌必须是能胡的，2，5，8，11，14
	if (nCount % 3 != 2)
	{
		return false;
	}

	if (EXTRA_CHECK_CARD_LEVEL::ECCL_SEVEN_PAIR == (EXTRA_CHECK_CARD_LEVEL::ECCL_SEVEN_PAIR & nCheckMask) && CheckSevenPairs(pStoneArray, nCount, pUser))
	{
		
		return true;
	}

	if (EXTRA_CHECK_CARD_LEVEL::ECCL_SHI_SAN_YAO == (EXTRA_CHECK_CARD_LEVEL::ECCL_SHI_SAN_YAO & nCheckMask) && CheckShiSanYao(pStoneArray, nCount, pUser))
	{
	
		return true;
	}

	
	std::sort(pStoneArray, pStoneArray + nCount, [](STONE& s1, STONE& s2)->bool {
		
		if (s1.eColor == s2.eColor)
		{
			if (s1.eWhat < s2.eWhat)
				return true;
		}
		else 
		{
			if (s1.eColor < s2.eColor)
			{
				return true;
			}
		}

		return false;
	});

#ifdef _DEBUG_
	LDCN("排序后:\n");
	for (int i(0); i < nCount; i++)
	{
		LDCW("%s ", Println(pStoneArray[i].eColor, pStoneArray[i].eWhat).c_str());
	}
	LDCN("\n");
#endif
	
	std::vector<ItemStatistics> listStati;
	StaitisticItemCount(pStoneArray, nCount, listStati);

#ifdef _DEBUG_

	LDCN("统计:\n");
	for (auto& v : listStati)
	{
		LDCW("%s, 个数:%d \n", Println(v.clr, v.vla).c_str(), v.cn);
	}

#endif

	std::vector<VCTItem>	validGroup;
	EnumValidCardType(pStoneArray, nCount, validGroup, nCheckMask);

	std::sort(validGroup.begin(), validGroup.end(), [](VCTItem& item1, VCTItem& item2)->bool {

		if (item1.nMaxGroups > item2.nMaxGroups)
			return true;
		return false;
	});

#ifdef _DEBUG_
	LDCN("可用组合:\n");
	for (auto& v : validGroup)
	{
		LDCW("颜色:%d, 最大组数:%d, 类型:%3d, 牌:%s %s %s ", v.clr, v.nMaxGroups, (int)v.eCGType, Println(v.clr, v.v1).c_str(), Println(v.clr, v.v2).c_str(), Println(v.clr, v.v3).c_str());
		LDCW("[");
		for (auto v1 : v.indexSet1)
		{
			LDCW("%d ", v1);
		}
		LDCW("] ");

		LDCW("[");
		for (auto v1 : v.indexSet2)
		{
			LDCW("%d ", v1);
		}
		LDCW("] ");

		LDCW("[");
		for (auto v1 : v.indexSet3)
		{
			LDCW("%d ", v1);
		}
		LDCW("] ");
		LDCN("\n");
	}

#endif

	bool victory(false);
	auto groups = nCount / 3;
	VCTItem* buffer[5] = { nullptr };

	Mathematical11(groups, 0, validGroup, (int)validGroup.size(), buffer, listStati, 0, pUser, victory);
	Mathematical21(groups-1, 0, validGroup, (int)validGroup.size(), buffer, listStati, 0, pUser, victory);
	Mathematical22(4 == groups ? 2 : 0, 0, validGroup, (int)validGroup.size(), buffer, listStati, pUser, victory);
	Mathematical31(3 == groups ? 1 : (4 == groups ? 2 : 0), 0, validGroup, (int)validGroup.size(), buffer, listStati, 0, pUser, victory);
	Mathematical4(4 == groups ? 1 : 0, 0, validGroup, (int)validGroup.size(), buffer, listStati, pUser, victory);
	
	if (victory)
		pUser->OperationComplete();

	return victory;
}

void CReasoningControl::StaitisticItemCount(STONE* pStoneArray, int nCount, std::vector<ItemStatistics>& result)
{
	
	for (int i(0); i < nCount; i++)
	{
		auto iter = std::find_if(result.begin(), result.end(), [&](ItemStatistics& item)->bool {
			
			if (pStoneArray[i].eColor == item.clr && pStoneArray[i].eWhat == item.vla)
			{
				item.cn++;
				return true;
			}
			return false;
		});
		if (iter == result.end())
		{
			result.push_back(OP_IGNORE(ItemStatistics(pStoneArray[i].eColor, pStoneArray[i].eWhat)));
		}
	}
}

void CReasoningControl::EnumValidCardType(STONE* pStoneArray, int nCount, std::vector<VCTItem>& vResult, int nCheckMask)
{
	for (int i(0); i < nCount; i++)
	{
		for (int j(i + 1); j < nCount; j++)
		{
			if (pStoneArray[i].eColor != pStoneArray[j].eColor)
				break;

			for (int k(j + 1); k < nCount; k++)
			{
				if (pStoneArray[j].eColor != pStoneArray[k].eColor)
					break;

				JudgeCardType(pStoneArray[i], pStoneArray[j], pStoneArray[k], i, j, k, vResult, nCheckMask);
			}
		}
	}
}

void CReasoningControl::JudgeCardType(STONE& s1, STONE& s2, STONE& s3, int i1, int i2, int i3, std::vector<VCTItem>& vResult, int nCheckMask)
{
	CARD_GROUP_TYPE eType(CARD_GROUP_TYPE::CGTT_INVALID);//CARD_GROUP_TYPE::CGTT_INVALID == eType
	MJ_COLOR eClr = s1.eColor;

	if (s1.eWhat == s2.eWhat && s2.eWhat == s3.eWhat)
	{
		eType = CARD_GROUP_TYPE::CGT_KE_ZI;
	}
	else if (eClr == MJ_COLOR::CC_BING || eClr == MJ_COLOR::CC_WAN || eClr == MJ_COLOR::CC_TIAO)
	{
		if (1 == ((int)s2.eWhat - (int)s1.eWhat) && 1 == ((int)s3.eWhat - (int)s2.eWhat))
		{
			eType = CARD_GROUP_TYPE::CGT_SHUN_ZI;
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_135 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_135 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_135;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_137 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_137 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 4 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_137;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_159 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_159 & nCheckMask))
		{
			if (4 == ((int)s2.eWhat - (int)s1.eWhat) && 4 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_159;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_179 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_179 & nCheckMask))
		{
			if (6 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_179;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_139 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_139 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 6 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_139;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_157 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_157 & nCheckMask))
		{
			if (4 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_1 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_157;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_359 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_359 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 4 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_3 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_359;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_357 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_357 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_3 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_357;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_379 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_379 & nCheckMask))
		{
			if (4 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_3 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_379;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_579 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_579 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_5 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_579;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_246 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_246 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_2 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_246;
			}
		}
		if (CARD_GROUP_TYPE::CGTT_INVALID == eType && (int)EXTRA_CHECK_CARD_LEVEL::ECCL_468 == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_468 & nCheckMask))
		{
			if (2 == ((int)s2.eWhat - (int)s1.eWhat) && 2 == ((int)s3.eWhat - (int)s2.eWhat) && MJ_VALUE::CV_4 == s1.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_468;
			}
		}
	}
	else if ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_LUAN_FENG == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_LUAN_FENG & nCheckMask) && eClr == MJ_COLOR::CC_WIND)
	{
		if (s1.eWhat != s2.eWhat && s2.eWhat != s3.eWhat)
		{
			eType = CARD_GROUP_TYPE::CGT_LUAN_FENG;
		}
	}
	else
	{
		if ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_LUAN_JIAN == ((int)EXTRA_CHECK_CARD_LEVEL::ECCL_LUAN_JIAN & nCheckMask) && eClr == MJ_COLOR::CC_JIAN)
		{
			if (s1.eWhat != s2.eWhat && s2.eWhat != s3.eWhat)
			{
				eType = CARD_GROUP_TYPE::CGT_LUAN_JIAN;
			}
		}
	}
	if (CARD_GROUP_TYPE::CGTT_INVALID != eType)
	{
		auto re = std::find_if(vResult.begin(), vResult.end(), [=](VCTItem& item)->bool {
			
			if (eClr == item.clr)
			{
				if (item.v1 == s1.eWhat && item.v2 == s2.eWhat && item.v3 == s3.eWhat)
				{
					auto iter = std::find_if(item.indexSet1.begin(), item.indexSet1.end(), [=](int nIndex)->bool {
						if (nIndex == i1)
						{
							return true;
						}
						return false;
					});
					if (iter == item.indexSet1.end())
					{
						item.indexSet1.push_back(i1);
					}
					item.nMaxGroups = (int)item.indexSet1.size();

					iter = std::find_if(item.indexSet2.begin(), item.indexSet2.end(), [=](int nIndex)->bool {
						if (nIndex == i2)
						{
							return true;
						}
						return false;
					});
					if (iter == item.indexSet2.end())
					{
						item.indexSet2.push_back(i2);
					}
					if (item.nMaxGroups > (int)item.indexSet2.size())
					{
						item.nMaxGroups = (int)item.indexSet2.size();
					}

					iter = std::find_if(item.indexSet3.begin(), item.indexSet3.end(), [=](int nIndex)->bool {
						if (nIndex == i3)
						{
							return true;
						}
						return false;
					});
					if (iter == item.indexSet3.end())
					{
						item.indexSet3.push_back(i3);
					}
					if (item.nMaxGroups > (int)item.indexSet3.size())
					{
						item.nMaxGroups = (int)item.indexSet3.size();
					}

					return true;
				}
			}
			return false;
		});

		if (re == vResult.end())
		{
			vResult.push_back(OP_IGNORE(VCTItem(eClr, eType, s1.eWhat, s2.eWhat, s3.eWhat, i1, i2, i3)));
		}
	}
}

void CReasoningControl::Mathematical11(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet)
{
	if (nCurrentDeep == nDeeps)
		return;

	for (int i=nIndex; i<nEleCount; i++)
	{
		pBuffer[nCurrentDeep] = &vResult[i];
		Mathematical11(nDeeps, nCurrentDeep + 1, vResult, nEleCount, pBuffer, contrastData, i+1, pUser, bRet);
		
		if (nDeeps == (nCurrentDeep + 1))
			ExactFiltration(pBuffer, nDeeps, contrastData, pUser, bRet);
	}
	
}

void CReasoningControl::ExactFiltration(VCTItem** pBuffer, int nCount, std::vector<ItemStatistics> contrastData, IHijack*& pUser, bool& bRet)
{
	auto iter = std::find_if(contrastData.begin(), contrastData.end(), [&](ItemStatistics& item)->bool {
		
		for (int i = 0; i < nCount; i++)
		{
			if (pBuffer[i]->clr == item.clr)
			{
				if (pBuffer[i]->v1 == item.vla)
				{
					item.cn--;
					if (item.cn < 0)
						return true;
				}
				if (pBuffer[i]->v2 == item.vla)
				{
					item.cn--;
					if (item.cn < 0)
						return true;
				}
				if (pBuffer[i]->v3 == item.vla)
				{
					item.cn--;
					if (item.cn < 0)
						return true;
				}
			}
		}

		return false;
	});

	

	if (iter == contrastData.end())
	{
		STONE s1;
		s1.nID = 0;

		STONE s2;
		s2.nID = 0;

		for (auto& v : contrastData)
		{
			if (v.cn == 1)
			{
				if (0 == s1.nID)
				{
					s1.eWhat	= v.vla;
					s1.eColor	= v.clr;
					s1.nID		= 99;
				}
				else {
					s2.eWhat  = v.vla;
					s2.eColor = v.clr;
					s2.nID = 99;
					break;
				}
			}
			else if (v.cn == 2)
			{
				s1.eWhat = v.vla;
				s1.eColor = v.clr;

				s2.eWhat = v.vla;
				s2.eColor = v.clr;

				break;
			}
			else  if ( 0 != v.cn)
			{
				LDCI("重要错误,在校验的组后发现张数不合法牌!%d\n", v.cn);
			}
		}

		if (s1.eColor == s2.eColor && s1.eWhat == s2.eWhat)
		{
			

			VCTItem groups;
			groups.clr = s1.eColor;
			groups.eCGType = CARD_GROUP_TYPE::CGT_DUI_ZI;
			groups.v1 = s1.eWhat;
			groups.v2 = s2.eWhat;
			groups.v3 = MJ_VALUE::CV_INVALID;
			pBuffer[nCount++] = &groups;
			if (pUser->ReconstructionData(pBuffer, nCount) > -1)
			{
				bRet = true;
			}

#ifdef _DEBUG_
			static int nc(0);
			nc++;
			LDCW("........................................................%d\n", nc);
			for (int i = 0; i < nCount; i++)
			{
				LDCW("类型:%d, 颜色:%d, 牌:%s %s %s\n",pBuffer[i]->eCGType, pBuffer[i]->clr, Println(pBuffer[i]->clr, pBuffer[i]->v1).c_str(), Println(pBuffer[i]->clr, pBuffer[i]->v2).c_str(), Println(pBuffer[i]->clr, pBuffer[i]->v3).c_str());
			}
			LDCW("类型:将, 颜色:%d, 牌:%s %s\n", s1.eColor, Println(s1.eColor, s1.eWhat).c_str(), Println(s2.eColor, s2.eWhat).c_str());
			LDCW("........................................................\n");
#endif
		}
	}
}

void CReasoningControl::Mathematical21(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet)
{
	if (nCurrentDeep == nDeeps)
		return;


	for (int i=nIndex; i<nEleCount; i++)
	{
		if (0 == nCurrentDeep)
		{
			if (vResult[i].nMaxGroups < 2)
				break;

			pBuffer[0] = &vResult[i];
			pBuffer[1] = &vResult[i];

			Mathematical21(nDeeps, nCurrentDeep+1, vResult, nEleCount, pBuffer, contrastData, 0, pUser, bRet);
		}

		else if (1 == nCurrentDeep)
		{
			if (pBuffer[0] == &vResult[i])
			{
				continue;
			}

			pBuffer[2] = &vResult[i];
			
			Mathematical21(nDeeps, nCurrentDeep+1, vResult, nEleCount, pBuffer, contrastData, 0, pUser, bRet);
		}

		else if (2 == nCurrentDeep)
		{
			if (pBuffer[0] == &vResult[i] || pBuffer[2] == &vResult[i])
			{
				continue;
			}

			pBuffer[3] = &vResult[i];
		}

		if (nDeeps != (nCurrentDeep + 1))
			continue;
		
		ExactFiltration(pBuffer, nDeeps+1, contrastData, pUser, bRet);
	}
}
//固定参数2
void CReasoningControl::Mathematical22(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, IHijack*& pUser, bool& bRet)
{
	if (2 != nDeeps)
		return;

	if (nCurrentDeep == nDeeps)
		return;

	for (int i = nCurrentDeep; i < nEleCount; i++)
	{
		if (0 == nCurrentDeep)
		{
			if (vResult[i].nMaxGroups < 2)
				break;

			pBuffer[0] = &vResult[i];
			pBuffer[1] = &vResult[i];

			Mathematical22(nDeeps, nCurrentDeep+1, vResult, nEleCount, pBuffer, contrastData, pUser, bRet);
		}

		if (1 == nCurrentDeep)
		{
			if (vResult[i].nMaxGroups < 2)
				break;

			pBuffer[2] = &vResult[i];
			pBuffer[3] = &vResult[i];

			ExactFiltration(pBuffer, nDeeps*2, contrastData, pUser, bRet);
		}
	}
}
//<=2
void CReasoningControl::Mathematical31(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet)
{
	if (nCurrentDeep == nDeeps)
		return;

	for (int i = nCurrentDeep; i < nEleCount; i++)
	{
		if (0 == nCurrentDeep)
		{
			if (vResult[i].nMaxGroups < 3)
				break;

			pBuffer[0] = &vResult[i];
			pBuffer[1] = &vResult[i];
			pBuffer[2] = &vResult[i];

			Mathematical31(nDeeps, nCurrentDeep+1, vResult, nEleCount, pBuffer, contrastData,0, pUser, bRet);
		}
		if (1 == nCurrentDeep)
		{
			if (pBuffer[0] == &vResult[i])
			{
				continue;
			}

			pBuffer[3] = &vResult[i];
		}

		if (nDeeps != (nCurrentDeep + 1))
			continue;

		ExactFiltration(pBuffer, 3+(nDeeps-1), contrastData, pUser, bRet);
	}
}
//=1
void CReasoningControl::Mathematical4(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, IHijack*& pUser, bool& bRet)
{
	if (nCurrentDeep == nDeeps)
		return;

	for (int i = nCurrentDeep; i < nEleCount; i++)
	{

		if (vResult[i].nMaxGroups < 4)
			break;

		pBuffer[0] = &vResult[i];
		pBuffer[1] = &vResult[i];
		pBuffer[2] = &vResult[i];
		pBuffer[3] = &vResult[i];

		ExactFiltration(pBuffer, 4, contrastData, pUser, bRet);
	}
}

void CReasoningControl::EnumRelRelationCard(std::vector<ItemStatistics>& dataOrigin, std::vector<STONE>& out)
{
	//因为嘴子成牌的情况很多，所以暂时按花色来吧
	auto bHaveBing(false);
	auto bHaveWan(false);
	auto bHanveTiao(false);
	auto bHaveFeng(false);
	auto bHaveJian(false);
	for (auto& v : dataOrigin)
	{
		if (false == bHaveBing && v.clr == MJ_COLOR::CC_BING)
		{
			bHaveBing = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_BING, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveWan && v.clr == MJ_COLOR::CC_WAN)
		{
			bHaveWan = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_WAN, (MJ_VALUE)i)));
			}
		}
		else if (false == bHanveTiao && v.clr == MJ_COLOR::CC_TIAO)
		{
			bHanveTiao = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_TIAO, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveFeng && v.clr == MJ_COLOR::CC_WIND)
		{
			bHaveFeng = true;
			for (int i = 0; i < 4; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_WIND, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveJian && v.clr == MJ_COLOR::CC_JIAN)
		{
			bHaveJian = true;
			for (int i = 0; i < 3; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_JIAN, (MJ_VALUE)i)));
			}
		}
	}
}

std::string CReasoningControl::Println(MJ_COLOR eClr, MJ_VALUE eVal)
{
	char szName[10] = { 0 };
	if (eClr == MJ_COLOR::CC_BING)
	{
		sprintf_s(szName, "%d饼", (int)eVal + 1);
	}
	else if (eClr == MJ_COLOR::CC_TIAO)
	{
		sprintf_s(szName, "%d条", (int)eVal + 1);
	}
	else if (eClr == MJ_COLOR::CC_WAN)
	{
		sprintf_s(szName, "%d万", (int)eVal + 1);
	}
	else if (eClr == MJ_COLOR::CC_WIND)
	{
		switch (eVal)
		{
		case MJ_VALUE::CV_1:
		{
			sprintf_s(szName, "东");
			break;
		}
		case MJ_VALUE::CV_2:
		{
			sprintf_s(szName, "南");
			break;
		}
		case MJ_VALUE::CV_3:
		{
			sprintf_s(szName, "西");
			break;
		}
		case MJ_VALUE::CV_4:
		{
			sprintf_s(szName, "北");
			break;
		}
		}
	}
	else if (eClr == MJ_COLOR::CC_JIAN)
	{
		switch (eVal)
		{
		case MJ_VALUE::CV_1:
		{
			sprintf_s(szName, "中");
			break;
		}
		case MJ_VALUE::CV_2:
		{
			sprintf_s(szName, "发");
			break;
		}
		case MJ_VALUE::CV_3:
		{
			sprintf_s(szName, "白");
			break;
		}
		}
	}
	return OP_IGNORE(std::string(szName));
}

bool CReasoningControl::CheckSevenPairs(STONE* pStoneArray, int nCount, IHijack* pUser)
{
	if (14 != nCount)
		return false;

	int nSiGeCount(0);

	std::vector<ItemStatistics> listStati;
	StaitisticItemCount(pStoneArray, nCount, listStati);

	VCTItem* buffer[7] = { nullptr };
	VCTItem varray[7];
	int nStartIndex(0);

	for (auto& v : listStati)
	{
		if (v.cn != 2 && v.cn != 4)
		{
			return false;
		}

		varray[nStartIndex].clr = v.clr;
		varray[nStartIndex].v1  = v.vla;
		varray[nStartIndex].nMaxGroups = v.cn;
		buffer[nStartIndex] = &varray[nStartIndex];
		if (4 == v.cn)
		{
			nSiGeCount++;
		}
		
		nStartIndex++;
	}

	pUser->ReconstructionData(buffer, nStartIndex, true, nSiGeCount);
	
	pUser->OperationComplete(true, nSiGeCount);
	return true;
}

bool CReasoningControl::CheckShiSanYao(STONE* pStoneArray, int nCount, IHijack* pUser)
{
	if (14 != nCount)
		return false;

	std::vector<STONE*> RecordList;

	auto PushFunc = [&](STONE* s) {

		auto iter = std::find_if(RecordList.begin(), RecordList.end(), [&](STONE* item) {
			
			if (s->eWhat == item->eWhat && s->eColor == item->eColor)
			{
				return true;
			}

			return false;
		});

		if (iter == RecordList.end())
		{
			RecordList.push_back(s);
		}

	};

	for (int i = 0; i < nCount; i++)
	{
		if (MJ_COLOR::CC_BING == pStoneArray[i].eColor || MJ_COLOR::CC_WAN == pStoneArray[i].eColor || MJ_COLOR::CC_TIAO == pStoneArray[i].eColor)
		{
			if (MJ_VALUE::CV_1 != pStoneArray[i].eWhat && MJ_VALUE::CV_9 != pStoneArray[i].eWhat)
			{
				return false;
			}
			else
			{
				PushFunc(&pStoneArray[i]);
			}
		}
		else
		{
			PushFunc(&pStoneArray[i]);
		}
	}

	if (RecordList.size() == 13)
	{
		VCTItem* buffer[14] = { nullptr };
		VCTItem varray[14];
		
		for (int i = 0; i < 14; i++)
		{
			varray[i].clr = pStoneArray[i].eColor;
			varray[i].v1  = pStoneArray[i].eWhat;
			buffer[i]	  = &varray[i];
		}

		pUser->ReconstructionData(buffer, 14, false, 0, true);
		pUser->OperationComplete(false, 0, true);
		return true;
	}

	return false;
}