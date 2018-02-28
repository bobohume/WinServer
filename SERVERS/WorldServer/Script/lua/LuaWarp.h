#ifndef Lua_WARP_H
#define Lua_WARP_H

extern "C"
{
    #include "Lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #include "luaconf.h"
};

#include "LuaType.h"
#include <string>
#include <vector>

struct  stLuaFuncList
{
	stLuaFuncList()
	{
		pNext = NULL;
	}

	std::string name;
	lua_CFunction func;
	stLuaFuncList *pNext;
};

//---------------------------------------------------------------------------
class  CLuaWarp
{
	lua_State* L;

public:

	CLuaWarp();
	virtual ~CLuaWarp();

	void Initialize();

	_inline lua_State* GetLuaState(){return L;}
	_inline bool IsValid(){return L != NULL;}
	
	int DoString(const char *str);
	int DoFile(const char* FileName);

	void RegisterCFunction(const char *FuncName, lua_CFunction Func);
	void RegisterCFunction(stLuaFuncList *Funcs);

	bool CallLuaFunction(const char * cFuncName);

public:
	template<typename T>
	T Get(int Idx)
	{
		return CLuaType<T>::get(L,Idx);
	}

	template<typename T>
	T Get(const char*Key)
	{
		return CLuaType<T>::get(L,Key);
	}
};



#endif


















