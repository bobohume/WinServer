#ifndef Lua_TYPE_H
#define Lua_TYPE_H

//-------------------------------------------------------------------------------------

// [LivenHotch]:  ¹Ø±Õ4311¾¯¸æ
#pragma warning( disable : 4311 )
template<typename T>
class CLuaType
{
public:
	static T get(lua_State* L,int Idx)
	{
		return NULL;
	}

	static int push(lua_State* L,T Val)
	{
		return NULL;
	}

	static int set(lua_State* L,const char *Key,T Val)
	{
		if(!push<T>(L,Val))
			return 0;

		lua_setglobal(L, Key);
		return 1;
	}

	static T get(lua_State* L,const char *Key)
	{
		lua_getglobal(L, Key);
		return get(L,-1);
	}
};

//-------------------------------------------------
// get
//-------------------------------------------------

template<>
static const char* CLuaType<const char*>::get(lua_State* L,int Idx)
{
	return lua_tostring(L,Idx);
}

template<>
static int CLuaType<const char*>::push(lua_State* L,const char* Val)
{
	lua_pushstring(L,Val);
	return 1;
}


template<>
static float CLuaType<float>::get(lua_State* L,int Idx)
{
	return (float)lua_tonumber(L,Idx);
}

template<>
static int CLuaType<float>::push(lua_State* L,float Val)
{
	lua_pushnumber(L,Val);
	return 1;
}

template<>
static int CLuaType<int>::get(lua_State* L,int Idx)
{
	return (int)lua_tonumber(L,Idx);
}

template<>
static int CLuaType<int>::push(lua_State* L,int Val)
{
	lua_pushnumber(L,Val);
	return 1;
}

template<>
static double CLuaType<double>::get(lua_State* L,int Idx)
{
	return (double)lua_tonumber(L,Idx);
}

template<>
static int CLuaType<double>::push(lua_State* L,double Val)
{
	lua_pushnumber(L,Val);
	return 1;
}


template<>
static bool CLuaType<bool>::get(lua_State* L,int Idx)
{
	return lua_toboolean(L,Idx) != 0;
}

template<>
static int CLuaType<bool>::push(lua_State* L,bool Val)
{
	lua_pushboolean(L,Val);
	return 1;
}


template<>
static lua_CFunction CLuaType<lua_CFunction>::get(lua_State* L,int Idx)
{
	return (lua_CFunction)lua_tocfunction(L,Idx);
}

template<>
static int CLuaType<lua_CFunction>::push(lua_State* L,lua_CFunction Val)
{
	lua_pushcfunction(L,Val);
	return 1;
}

template<>
static void* CLuaType<void*>::get(lua_State* L,int Idx)
{
	return static_cast<void*>(lua_touserdata(L,Idx));
}

template<>
static int CLuaType<void*>::push(lua_State* L,void* Size)
{
	void* pRet = static_cast<void*>(lua_newuserdata(L,(int)Size));
	return (int)pRet;
}


#endif