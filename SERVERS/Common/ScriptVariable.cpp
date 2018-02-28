#include "Common/ScriptVariable.h"

#if defined(NTJ_SERVER) || defined(NTJ_CLIENT) || defined(NTJ_EDITOR)
#include "console/console.h"

ScriptVariableManager* ScriptVariableManager::GetInstance()
{
	static ScriptVariableManager s_scriptVarMgr;
	return &s_scriptVarMgr;
}

ScriptVariableManager::~ScriptVariableManager()
{
	for (VariablesIter iter=m_variables.begin(); iter!=m_variables.end(); ++iter)
		delete iter->valuePtr;
}

ScriptVariableManager::ScriptVariableManager()
{
}

void ScriptVariableManager::Init()
{
	for (VariablesIter iter=m_variables.begin(); iter!=m_variables.end(); ++iter)
		Con::addVariable(iter->name.c_str(), iter->type, iter->valuePtr);
}

void ScriptVariableManager::AddVariable(const char* name, int type, void* valuePtr)
{
	Variable variable = {name, type, valuePtr};
	m_variables.push_back(variable);
}
#endif