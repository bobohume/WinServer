#include "BuffData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

CBuffDataRes* g_BuffDataMgr = new CBuffDataRes();
Stats g_Stats;

CBuffDataRes::CBuffDataRes()
{

}
CBuffDataRes::~CBuffDataRes()
{
}

bool CBuffDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Buff.dat"))
	{
		JUST_ASSERT("read itemData error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CBuffData* pdata = new CBuffData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read buffdata id error");
		pdata->m_BuffId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read skilldata des error");
		pdata->m_sDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read buffdata type error");
		pdata->m_BuffType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_F32, "read buffdata val error");
		pdata->m_BuffVal = LineData.m_F32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read buffdata val error");
		pdata->m_BattleFlag = LineData.m_U8;

		addData(pdata->m_BuffId, pdata);
	}
	return true;
}

void CBuffData::getStats(Stats& stats)
{
	F32 *p = &(stats.Bus_Pc);
	S32 iType = mClamp(m_BuffType, 1, (S32)(sizeof(Stats) / sizeof(S32))) ;
	--iType;
	p += iType;

	*p += m_BuffVal;
}


Stats::Stats()
{
	Clear();
}

void Stats::Clear()
{
	dMemset(this, 0, sizeof(Stats));
}

// 做A+=B和A-=B时，注意A::HP和A::HP_Pc没有做运算。所以一般来说A的初始值都为0
Stats& Stats::operator+=(const Stats& _add)
{
	Bus_Pc += _add.Bus_Pc;
	OffBus_gPc += _add.OffBus_gPc;
	DevTime_Pc += _add.DevTime_Pc;
	PvpHP_gPc += _add.PvpHP_gPc;
	Ach_Pc += _add.Ach_Pc;
	LvCost_Pc += _add.LvCost_Pc;
	Queen_gPc += _add.Queen_gPc;
	PvpAtk_gPc += _add.PvpAtk_gPc;
	TecCost_gPc += _add.TecCost_gPc;
	OffTec_gPc += _add.OffTec_gPc;
	Tec_Pc += _add.Tec_Pc;
	TecTime_gPc += _add.TecTime_gPc;
	Mil_Pc += _add.Mil_Pc;
	OffMil_gPc += _add.OffMil_gPc;
	Atk_gPc += _add.Atk_gPc;
	Cri_Pc += _add.Cri_Pc;
	HP_gPC += _add.HP_gPC;
	Dog_gPC += _add.Dog_gPC;
	for (int i = 0; i < 9; ++i)
		Skill[i] += _add.Skill[i];
	Agi += _add.Agi;
	Int += _add.Int;
	Str += _add.Str;
	Bus += _add.Bus;
	Tec += _add.Tec;
	Mil += _add.Mil;
	ClvCost_gPc += _add.ClvCost_gPc;
	SolHp += _add.SolHp;
	SolAtk += _add.SolAtk;
	FlowerSuc_gPc += _add.FlowerSuc_gPc;
	Culture_gPc += _add.Culture_gPc;
	Rite_Pc += _add.Rite_Pc;
	PBattle_Pc += _add.PBattle_Pc;
	FlowerSuc_Pc += _add.FlowerSuc_Pc;
	Charm += _add.Charm;
	PrinceAbi_gPc += _add.PrinceAbi_gPc;
	DevCost_gPc += _add.DevCost_gPc;
	AddMoney_gPc += _add.AddMoney_gPc;
	AddOre_gPc += _add.AddOre_gPc;
	DevNum += _add.DevNum;
	Bus_gPc += _add.Bus_gPc;
	Tec_gPc += _add.Tec_gPc;
	Agi_gPc += _add.Agi_gPc;
	Ach_gPc += _add.Ach_gPc;
	Land_gPc += _add.Land_gPc;
	AddRes_gPc += _add.AddRes_gPc;
	StrongHold_gP += _add.StrongHold_gP;
	CardNum += _add.CardNum;
	Str_gPc += _add.Str_gPc;
	OffRank += _add.OffRank;
	TechNum += _add.TechNum;
	ApplyNum += _add.ApplyNum;
	Rite_gPc += _add.Rite_gPc;
	Int_gPc += _add.Int_gPc;
	PrinceNum += _add.PrinceNum;
	Cri_gPc += _add.Cri_gPc;
	RedAch_gPc += _add.RedAch_gPc;
	CallTroop_gPc += _add.CallTroop_gPc;
	Mil_gPc += _add.Mil_gPc;
	BattleNum += _add.BattleNum;
	PvpAgi_gPc += _add.PvpAgi_gPc;
	PvpInt_gPc += _add.PvpInt_gPc;
	PvpStr_gPc += _add.PvpStr_gPc;
	PvpHp_gPc += _add.PvpHp_gPc;
	PvpStats_gPc += _add.PvpStats_gPc;
	Impose_gPc += _add.Impose_gPc;
	OrgPlayerNum += _add.OrgPlayerNum;
	OrgContribute_gPc += _add.OrgContribute_gPc;
	OrgContributeExp_gPc += _add.OrgContributeExp_gPc;
	OrgCharm += _add.OrgCharm;
	OrgConquest_gPc += _add.OrgConquest_gPc;
	OrgImposeMoney_gPc += _add.OrgImposeMoney_gPc;
	OrgOre_gPc += _add.OrgOre_gPc;
	OrgTroop_gPc += _add.OrgTroop_gPc;
	OrgManagerNum += _add.OrgManagerNum;

	return *this;
}

Stats& Stats::operator-=(const Stats& _add)
{
	Bus_Pc -= _add.Bus_Pc;
	OffBus_gPc -= _add.OffBus_gPc;
	DevTime_Pc -= _add.DevTime_Pc;
	PvpHP_gPc -= _add.PvpHP_gPc;
	Ach_Pc -= _add.Ach_Pc;
	LvCost_Pc -= _add.LvCost_Pc;
	Queen_gPc -= _add.Queen_gPc;
	PvpAtk_gPc -= _add.PvpAtk_gPc;
	TecCost_gPc -= _add.TecCost_gPc;
	OffTec_gPc -= _add.OffTec_gPc;
	Tec_Pc -= _add.Tec_Pc;
	TecTime_gPc -= _add.TecTime_gPc;
	Mil_Pc -= _add.Mil_Pc;
	OffMil_gPc -= _add.OffMil_gPc;
	Atk_gPc -= _add.Atk_gPc;
	Cri_Pc -= _add.Cri_Pc;
	HP_gPC -= _add.HP_gPC;
	Dog_gPC -= _add.Dog_gPC;
	for (int i = 0; i < 9; ++i)
		Skill[i] -= _add.Skill[i];
	Agi -= _add.Agi;
	Int -= _add.Int;
	Str -= _add.Str;
	Bus -= _add.Bus;
	Tec -= _add.Tec;
	Mil -= _add.Mil;
	ClvCost_gPc -= _add.ClvCost_gPc;
	SolHp -= _add.SolHp;
	SolAtk -= _add.SolAtk;
	FlowerSuc_gPc -= _add.FlowerSuc_gPc;
	Culture_gPc -= _add.Culture_gPc;
	Rite_Pc -= _add.Rite_Pc;
	PBattle_Pc -= _add.PBattle_Pc;
	FlowerSuc_Pc -= _add.FlowerSuc_Pc;
	Charm -= _add.Charm;
	PrinceAbi_gPc -= _add.PrinceAbi_gPc;
	DevCost_gPc -= _add.DevCost_gPc;
	AddMoney_gPc -= _add.AddMoney_gPc;
	AddOre_gPc -= _add.AddOre_gPc;
	DevNum -= _add.DevNum;
	Bus_gPc -= _add.Bus_gPc;
	Tec_gPc -= _add.Tec_gPc;
	Agi_gPc -= _add.Agi_gPc;
	Ach_gPc -= _add.Ach_gPc;
	Land_gPc -= _add.Land_gPc;
	AddRes_gPc -= _add.AddRes_gPc;
	StrongHold_gP -= _add.StrongHold_gP;
	CardNum -= _add.CardNum;
	Str_gPc -= _add.Str_gPc;
	OffRank -= _add.OffRank;
	TechNum -= _add.TechNum;
	ApplyNum -= _add.ApplyNum;
	Rite_gPc -= _add.Rite_gPc;
	Int_gPc -= _add.Int_gPc;
	PrinceNum -= _add.PrinceNum;
	Cri_gPc -= _add.Cri_gPc;
	RedAch_gPc -= _add.RedAch_gPc;
	CallTroop_gPc -= _add.CallTroop_gPc;
	Mil_gPc -= _add.Mil_gPc;
	BattleNum -= _add.BattleNum;
	PvpAgi_gPc -= _add.PvpAgi_gPc;
	PvpInt_gPc -= _add.PvpInt_gPc;
	PvpStr_gPc -= _add.PvpStr_gPc;
	PvpHp_gPc -= _add.PvpHp_gPc;
	PvpStats_gPc -= _add.PvpStats_gPc;
	Impose_gPc -= _add.Impose_gPc;
	OrgPlayerNum -= _add.OrgPlayerNum;
	OrgContribute_gPc -= _add.OrgContribute_gPc;
	OrgContributeExp_gPc -= _add.OrgContributeExp_gPc;
	OrgCharm -= _add.OrgCharm;
	OrgConquest_gPc -= _add.OrgConquest_gPc;
	OrgImposeMoney_gPc -= _add.OrgImposeMoney_gPc;
	OrgOre_gPc -= _add.OrgOre_gPc;
	OrgTroop_gPc -= _add.OrgTroop_gPc;
	OrgManagerNum -= _add.OrgManagerNum;

	return *this;
}