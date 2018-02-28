#ifndef SCRIPT_H
#define SCRIPT_H

#include "lua/LuaWarp.h"

#define CStr const char *
#define SCRIPT (&g_TccWapper)
#define RETURN CLuaType

class CLuaCReg
{
	static stLuaFuncList *pAutoRegList;

	friend class CScript;
public:
	CLuaCReg(const char *Name,lua_CFunction Func)
	{
		stLuaFuncList *reg = new stLuaFuncList;
		reg->name = Name;
		reg->func = Func;
		reg->pNext = pAutoRegList;

		pAutoRegList = reg;
	}
};

class CScript : public CLuaWarp
{
	static CScript *m_pInstance;

	CScript()
	{
		m_pInstance = NULL;
	}

public:
	~CScript()
	{
		m_pInstance = NULL;

		stLuaFuncList *pFuncs  = NULL;

		while(CLuaCReg::pAutoRegList)
		{
			pFuncs = CLuaCReg::pAutoRegList;
			CLuaCReg::pAutoRegList = CLuaCReg::pAutoRegList->pNext;
			delete pFuncs;
		}
	}

	static CScript *GetInstance()
	{
		if(!m_pInstance)
			m_pInstance = new CScript;

		return m_pInstance;
	}

	void Initialize()
	{
		RegisterCFunction(CLuaCReg::pAutoRegList);
	}

	//从本地配置文件Config.ini读取数据
	bool Load(const char *Path)
	{
		if(DoFile(Path)==0)
			return true;
		else
			return false;
	}
};


//---------------------------------------------------------------------------

#define ScriptFunction(name) \
	static int Warp_##name(lua_State *L); \
	CLuaCReg reg_##name(#name,Warp_##name); \
	static int Warp_##name(lua_State *L) \


#endif

