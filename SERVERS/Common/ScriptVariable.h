//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------
#ifndef __SCRIPTVARIABLE_H__
#define __SCRIPTVARIABLE_H__

#if defined(NTJ_SERVER) || defined(NTJ_CLIENT) || defined(NTJ_EDITOR)
#include <string>
#include <vector>
#include "console/consoleTypes.h"

class ScriptVariableManager
{
public:
	static ScriptVariableManager* GetInstance();
	~ScriptVariableManager();
	void Init();

private:
	ScriptVariableManager();
	template <typename T>
	friend class ScriptVariable;
	void AddVariable(const char* name, int type, void* valuePtr);
	struct Variable
	{
		std::string name;
		int type;
		void* valuePtr;
	};
	typedef std::vector<Variable> Variables;
	typedef Variables::const_iterator VariablesIter;
	Variables m_variables;
};

#define SCRIPTVARMGR ScriptVariableManager::GetInstance()

template <typename T>
class ScriptVariable
{
public:
	ScriptVariable(const char* name, int type, T value)
	{
		SCRIPTVARMGR->AddVariable(name, type, new T(value));
	}
};

#define SCRIPTVARTYPES8(name, script, value) const S8 name = value; const ScriptVariable<S8> name##S8(script, TypeS8, value)
#else
#define SCRIPTVARTYPES8(name, script, value) const S8 name = value
#endif

#endif //__SCRIPTVARIABLE_H__