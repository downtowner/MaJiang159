#include "stdafx.h"
#include "ReasoningGuide.h"
#include "LDebug.h"

CReasoningGuide::CReasoningGuide()
{
	m_nExtraScore = -1;
	m_nMaxValidComb = 0;
	m_nWinMode = 0;
	m_pCallInfoList = nullptr;
}


CReasoningGuide::~CReasoningGuide()
{
}

void CReasoningGuide::EnumRelRelationCard(STONE* pArray, int nCount, std::vector<STONE>& out)
{
	if (2 == nCount)
	{
		if (pArray[0].eColor == pArray[1].eColor && pArray[0].eWhat == pArray[1].eWhat)
		{
			out.push_back(OP_IGNORE(STONE(pArray[0].eColor, (MJ_VALUE)pArray[0].eWhat)));
		}
		else
		{
			out.push_back(OP_IGNORE(STONE(pArray[0].eColor, (MJ_VALUE)pArray[0].eWhat)));
			out.push_back(OP_IGNORE(STONE(pArray[1].eColor, (MJ_VALUE)pArray[1].eWhat)));
		}
		
		return;
	}

	//因为嘴子成牌的情况很多，所以暂时按花色来吧
	auto bHaveBing(false);
	auto bHaveWan(false);
	auto bHanveTiao(false);
	auto bHaveFeng(false);
	auto bHaveJian(false);
	for (auto i(0); i<nCount; i++)
	{
		if (false == bHaveBing && pArray[i].eColor == MJ_COLOR::CC_BING)
		{
			bHaveBing = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_BING, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveWan && pArray[i].eColor == MJ_COLOR::CC_WAN)
		{
			bHaveWan = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_WAN, (MJ_VALUE)i)));
			}
		}
		else if (false == bHanveTiao &&  pArray[i].eColor == MJ_COLOR::CC_TIAO)
		{
			bHanveTiao = true;
			for (int i = 0; i < 9; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_TIAO, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveFeng &&  pArray[i].eColor == MJ_COLOR::CC_WIND)
		{
			bHaveFeng = true;
			for (int i = 0; i < 4; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_WIND, (MJ_VALUE)i)));
			}
		}
		else if (false == bHaveJian &&  pArray[i].eColor == MJ_COLOR::CC_JIAN)
		{
			bHaveJian = true;
			for (int i = 0; i < 3; i++)
			{
				out.push_back(OP_IGNORE(STONE(MJ_COLOR::CC_JIAN, (MJ_VALUE)i)));
			}
		}
	}
}


int CReasoningGuide::ReconstructionData(VCTItem** pGroup, int cn, bool bSevenPairs, int cnSG, bool bShiSanYao)
{
	auto copy = m_vFanTypeList;
	copy.clear();
	
	bool bDY(false);

	if (bSevenPairs || bShiSanYao)
	{
		if (bSevenPairs)
		{
			m_nExtraScore = m_pUser->FiltAcceptableType(pGroup, cn, m_nCardID, 0, m_pVoidParam, (void*)&copy, bDY, true, cnSG, m_nWinMode);
		}
		if (bShiSanYao)
		{
			m_nExtraScore = m_pUser->FiltAcceptableType(pGroup, cn, m_nCardID, 0, m_pVoidParam, (void*)&copy, bDY, false, 0, true, m_nWinMode);
		}
		m_vFanTypeList = copy;
		return 0;
	}

	int score = m_pUser->FiltAcceptableType(pGroup, cn, m_nCardID, 0, m_pVoidParam, (void*)&copy, bDY, false, 0, false, m_nWinMode);
	if (-1 == score)
	{
		return -1;
	}
	
	if (false == bDY)
	{
		m_nDYFlag = 3;
	}
	else
	{
		if (3 != m_nDYFlag)
		{
			m_nDYFlag = 2;
		}	
	}

	if (score > m_nExtraScore)
	{
		m_nGroupCount = cn;
		m_nExtraScore = score;
		m_vFanTypeList = copy;

		for (auto i = 0; i < m_nGroupCount; i++)
		{
			m_asCardGroup[i].clr = pGroup[i]->clr;
			m_asCardGroup[i].eCGType = pGroup[i]->eCGType;
			m_asCardGroup[i].v1 = pGroup[i]->v1;
			m_asCardGroup[i].v2 = pGroup[i]->v2;
			m_asCardGroup[i].v3 = pGroup[i]->v3;

		}
	}

	if (score > -1)
	{
		m_nMaxValidComb++;
	}

	return score;
}

void CReasoningGuide::OperationComplete(bool bSevenPairs , int cnSG , bool bShiSanYao)
{
	if (bSevenPairs || bShiSanYao)
	{
		m_pUser->SpecialCombination(m_nExtraScore, (void*)&m_vFanTypeList, m_nCardID, bSevenPairs, cnSG, bShiSanYao);
		return;
	}

	bool bDY(false);
	if (2 == m_nDYFlag)
	{
		bDY = true;
	}

	if (Check_Mode::CM_VICTORY == m_eCheckMode)
	{
		m_pUser->BestCombination(m_asCardGroup, m_nGroupCount, m_nCardID, m_nExtraScore, bDY, m_pVoidParam, (void*)&m_vFanTypeList);
	}

	else if (m_eCheckMode == Check_Mode::CM_BEINORDER)
	{
		m_nExtraScore++;
	}
}

bool CReasoningGuide::CheckVirctoy(STONE* pArray, int nCount, int nMask, int nCardID, IResoningUser* pUser, void* LPParam, int nWinMode)
{
	m_nDYFlag = 1;
	m_pUser = pUser;
	m_nCardID = nCardID;
	m_pVoidParam = LPParam;
	m_nExtraScore = -1;
	m_nMaxValidComb = 0;
	m_nWinMode = nWinMode;
	m_eCheckMode = Check_Mode::CM_VICTORY;

	return CReasoningControl::CheckVictory(pArray, nCount, nMask, this);
}

bool CReasoningGuide::CheckBeInOrder(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo, int* pFilterList, int cnFilt)
{
#ifdef _DEBUG_
	LDCN("传入的牌:\n");
	for (int i(0); i < nCount; i++)
	{
		LDCW("%s ", CReasoningControl::Println(pArray[i].eColor, pArray[i].eWhat).c_str());
	}
	LDCN("\n");
#endif
	if (1 == nCount || 4 == nCount || 7 == nCount || 10 == nCount || 13 == nCount)
	{
		return false;
	}

	m_nDYFlag = 1;
	m_pUser = pUser;
	m_pVoidParam = LPParam;
	m_nExtraScore = -1;
	m_nMaxValidComb = 0;
	m_eCheckMode = Check_Mode::CM_BEINORDER;
	m_pCallInfoList = (CALLINFOVECTOREX*)pCallInfo;
	m_pCallInfoList->clear();
	
	//需要尝试打出的列表,本来是合并后检测效率搞，现在没办法，队友太牛逼
	/*
	std::vector<ItemStatistics> DisCardCardList;
	for (int i = 0; i < nCount; i++)
	{
		if (pFilterList)
		{
			bool bFInd(false);
			for (int ij = 0; ij < cnFilt; ij++)
			{
				if (pFilterList[ij] == pArray[i].nID)
				{
					bFInd = true;
					break;
				}
			}

			if (false == bFInd)
			{
				DisCardCardList.push_back(OP_IGNORE(ItemStatistics(pArray[i].eColor, pArray[i].eWhat, pArray[i].nID)));
			}
		}
		else
		{
			DisCardCardList.push_back(OP_IGNORE(ItemStatistics(pArray[i].eColor, pArray[i].eWhat, pArray[i].nID)));
		}
		
	}
	*/
	//CReasoningControl::StaitisticItemCount(pArray, nCount, DisCardCardList);

	//需要尝试胡牌的列表
	std::vector<STONE> outTryWinList;
	EnumRelRelationCard(pArray, nCount, outTryWinList);

	//其实也就是用尝试能胡的替换掉尝试打出的
	for (int i=0; i<nCount; i++)
	{
		for (auto& vv : outTryWinList)
		{
			STONE sArray[14];
			memmove(sArray, pArray, sizeof(STONE)*nCount);


			STONE sDis;
			sDis.eColor = sArray[i].eColor;
			sDis.eWhat = sArray[i].eWhat;
			sDis.nID = sArray[i].nID;

			if (pFilterList)
			{
				bool bFInd(false);
				for (int ij = 0; ij < cnFilt; ij++)
				{
					if (pFilterList[ij] == sDis.nID)
					{
						bFInd = true;
						break;
					}
				}

				if (bFInd)
				{
					break;
				}
			}

			sArray[i].eColor = vv.eColor;
			sArray[i].eWhat = vv.eWhat;
#ifdef _DEBUG_
			static int cnv(0);
			cnv++;
			LDCI("尝试:%d ,关键key:%d * %d=(%d)\n", cnv, DisCardCardList.size(), outTryWinList.size(), (int)DisCardCardList.size() * (int)outTryWinList.size());
#endif
#ifdef _DEBUG_
			LDCN("尝试:%d\n", i);
			for (int i(0); i < nCount; i++)
			{
				LDCW("%s ", CReasoningControl::Println(sArray[i].eColor, sArray[i].eWhat).c_str());
			}
			LDCN("\n");
#endif
			auto ret = CReasoningControl::CheckVictory(sArray, nCount, nMask, this);
			if (ret)
			{
				auto IsHaveNode = [&](int nDisCard)->bool {//返回节点的个数

					int nSize = (int)m_pCallInfoList->size();
					if (0 == nSize)
						return false;

					//LDCN("查找:%d, 找到:%d, 最后一个:%d\n", nDisCard, nSize, (*m_pCallInfoList)[nSize - 1].nDiscardTileID);
					if ((*m_pCallInfoList)[nSize - 1].nDiscardTileID == nDisCard)
					{
						
						return true;
					}

					return false;
				};

				if (IsHaveNode(sDis.nID))
				{
					int cur = ++((*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].cnCallTile);
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[cur - 1].nCallTileID = MAKEID(vv.eColor, vv.eWhat);
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[cur - 1].nFans = m_nExtraScore;
				}
				else
				{
					m_pCallInfoList->push_back(OP_IGNORE(CALLINFOEx()));
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].nDiscardTileID = sDis.nID;//MAKEID(v.clr, v.vla);
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].cnCallTile = 1;
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[0].nCallTileID = MAKEID(vv.eColor, vv.eWhat);
					(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[0].nFans = m_nExtraScore;
				}
			}
		}
				
	}

	return m_pCallInfoList->empty() ? false : true;
}

bool CReasoningGuide::CheckJianTing(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo)
{
	

	return true;
}

bool CReasoningGuide::CheckZhiTing(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo)
{
	return false;
	/*
	@brief
	下面的暂时没用，没有直听这一说
	*/
	STONE sArray[14];//手上的牌最多14张
	memmove(sArray, pArray, sizeof(STONE)*nCount);

	if (1 == nCount)
	{
		sArray[nCount].eColor = pArray[0].eColor;
		sArray[nCount].eWhat = pArray[0].eWhat;
		
		
		auto ret = CReasoningControl::CheckVictory(pArray, nCount, nMask, this);
		
		if (ret)
		{
			m_pCallInfoList->push_back(OP_IGNORE(CALLINFOEx()));
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].cnCallTile = 1;
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[0].nCallTileID = MAKEID(pArray[0].eColor, pArray[0].eWhat);
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[0].nFans = m_nExtraScore;
		}
	}

	std::vector<STONE> checkData;
	EnumRelRelationCard(pArray, nCount, checkData);

	int nStart(0);
	for (auto i(0); i<(int)checkData.size(); i++)
	{
		sArray[nCount] = checkData[i];

		auto ret = CReasoningControl::CheckVictory(pArray, nCount, nMask, this);

		if (ret)
		{
			m_pCallInfoList->push_back(OP_IGNORE(CALLINFOEx()));
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].cnCallTile = nStart;
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[nStart].nCallTileID = MAKEID(checkData[i].eColor, checkData[i].eWhat);
			(*m_pCallInfoList)[(int)m_pCallInfoList->size() - 1].asCallTileInfo[nStart].nFans = m_nExtraScore;
			nStart++;
		}
	}


	return true;
}