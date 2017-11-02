#pragma once

#include "ShareDefine08.h"
#include "IControl.h"
#include <vector>
#include <string>

struct ItemStatistics {
	MJ_COLOR clr;
	MJ_VALUE vla;
	int		 cn;
	ItemStatistics(MJ_COLOR c, MJ_VALUE v) :clr(c), vla(v)
	{
		cn = 1;
	}

	ItemStatistics(MJ_COLOR c, MJ_VALUE v, int nID) :clr(c), vla(v), cn(nID)
	{
	}
};

#define EQUAL_CARD(x1, x2) (x1.eColor == x2.eColor) && (x1.eWhat == x2.eWhat) ? true : false

/*
===================================================================
author:	xiaob
date:	2017/9/8
失败之处:	本来为了代码的高效和无数据维护的初衷设计为静态类，导致部分数据需要通过栈传递
===================================================================
*/
class CReasoningControl
{
public:
	/*
	@brief
	nCheckMask:EXTRA_CHECK_CARD_LEVEL的组合,默认检测顺子和刻字
	约定:这个接口传进来的值是胡牌数,2,5,8,11,14
	*/
	static bool CheckVictory(STONE* pStoneArray, int nCount, int nCheckMask, IHijack* pUser);
	
	/*
	@breif
	*/
	static void StaitisticItemCount(STONE* pStoneArray, int nCount, std::vector<ItemStatistics>& result);

private:

	static bool CheckSevenPairs(STONE* pStoneArray, int nCount, IHijack* pUser);

	static bool CheckShiSanYao(STONE* pStoneArray, int nCount, IHijack* pUser);

	static void EnumValidCardType(STONE* pStoneArray, int nCount, std::vector<VCTItem>& vResult, int nCheckMask);
	static void JudgeCardType(STONE& s1, STONE& s2, STONE& s3, int i1, int i2, int i3, std::vector<VCTItem>& vResult, int nCheckMask);

	static void Mathematical11(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet);
	static void Mathematical21(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet);
	static void Mathematical22(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, IHijack*& pUser, bool& bRet);
	static void Mathematical31(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, int nIndex, IHijack*& pUser, bool& bRet);
	static void Mathematical4(int nDeeps, int nCurrentDeep, std::vector<VCTItem>& vResult, int nEleCount, VCTItem** pBuffer, std::vector<ItemStatistics>& contrastData, IHijack*& pUser, bool& bRet);

	static void ExactFiltration(VCTItem** pBuffer, int nCount, std::vector<ItemStatistics> contrastData, IHijack*& pUser, bool& bRet);

	

	static void EnumRelRelationCard(std::vector<ItemStatistics>& dataOrigin, std::vector<STONE>& out);
public:
	//测试接口
	static std::string Println(MJ_COLOR eClr, MJ_VALUE eVal);
};

