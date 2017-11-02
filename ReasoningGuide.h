#pragma once
#include "ReasoningControl.h"
#include "IControl.h"

#define MAKEID(clr, vla) (int)(clr << 8 | vla << 4 | 0x01)

enum class Check_Mode : int {

	CM_VICTORY,
	CM_BEINORDER
};

class CReasoningGuide : public IResoningServant, public IHijack
{
public:
	CReasoningGuide();
	~CReasoningGuide();

	virtual int ReconstructionData(VCTItem** pGroup, int cn, bool bSevenPairs = false, int cnSG = 0,  bool bShiSanYao = false);

	virtual void OperationComplete(bool bSevenPairs = false, int cnSG = 0, bool bShiSanYao = false);

	virtual bool CheckVirctoy(STONE* pArray, int nCount, int nMask, int nCardID, IResoningUser* pUser, void* LPParam, int nWinMode = 0);

	virtual bool CheckBeInOrder(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo, int* pFilterList = nullptr, int cnFilt = 0);

private:

	void EnumRelRelationCard(STONE* pArray, int nCount, std::vector<STONE>& out);

	bool CheckZhiTing(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo);

	bool CheckJianTing(STONE* pArray, int nCount, int nMask, IResoningUser* pUser, void* LPParam, void* pCallInfo);
protected:
	IResoningUser*		m_pUser;												//用户的接口
	int					m_nCardID;												//胡牌的id
	void*				m_pVoidParam;											//附加参数
	VCTItem				m_asCardGroup[5];										//最优秀的一组
	int					m_nGroupCount;											//具体组数
	int                 m_nMaxValidComb;										//当前这副牌最大的组合数
	int					m_nDYFlag;
	std::vector<int>	m_vFanTypeList;											//最优秀那组的翻(嘴子)列表
	int					m_nExtraScore;											//记录当前牌可赢的基本分数
	Check_Mode			m_eCheckMode;
	int					m_nWinMode;
	CALLINFOVECTOREX*	m_pCallInfoList;
};

