#include "Common.h"
#include <regex>

#ifndef NTJ_CLIENT
#include "CfgBuilder.h"
#endif

CConfig* CConfig::GetInstance()
{
	static CConfig s_config;
	return &s_config;
}

int CConfig::GetGameType()
{
	return m_gametype;
}

void CConfig::SetGameType(int type)
{
	m_gametype = type;
}

CConfig::CConfig()
{
#ifndef NTJ_CLIENT
	m_gametype = atoi(CFG_BUIILDER->Get("Game_Type").c_str());
#else
	m_gametype = 0;
#endif
}

const char* LOGIN_TYPE_STR[] =
{
	"normal",
	"tweibo",
	"yy",
	"other"
};

LOGIN_TYPE GetLoginType(const char* loginTypeStr)
{
	for (int i=LOGIN_Normal; i<LOGIN_TYPE_Count; ++i)
	{
		if (strcmp(loginTypeStr, LOGIN_TYPE_STR[i]) == 0)
			return (LOGIN_TYPE)i;
	}
	return LOGIN_Normal;
}

const char* GetLoginTypeStr(LOGIN_TYPE loginType)
{
	if (loginType>=LOGIN_Normal && loginType<LOGIN_TYPE_Count)
		return LOGIN_TYPE_STR[loginType];
	return LOGIN_TYPE_STR[LOGIN_Normal];
}

bool IsTelePhone(const char* telePhone) {
	std::regex e("^1(3\\d|47|5([0-3]|[5-9])|8(0|2|[5-9]))\\d{8}$");
	if (std::regex_match(telePhone, e))
	{
		return true;
	}
	
	return false;
}

#ifdef NTJ_SERVER 
U32 getCreateFace(U32 sex, U32 face)
{
	const int MAX_FACE_NUM = 6;
	static int playerMaleFaceShape[MAX_FACE_NUM + 1] = {21091, 21092, 21093, 21094, 21095, 21096, 111012};
	static int playerFeMaleFaceShape[MAX_FACE_NUM + 1] = {21591, 21592, 21593, 21594, 21594, 21596, 111013};
	if(face < 0 || face > MAX_FACE_NUM)
	{
		face = 0;
	}

	if(sex == 1)
	{
		return playerMaleFaceShape[face];
	}
	else
	{
		return playerFeMaleFaceShape[face];
	}
}

U32 getCreateHair(U32 sex, U32 hair, U32 hairColor)
{
	const int MAX_HAIR_NUM = 6;
	static int playerMaleHairShape[MAX_HAIR_NUM + 1] = {310910, 310920, 310930, 310940, 310950, 310960, 111014};
	static int playerFeMaleHairShape[MAX_HAIR_NUM + 1] = {315910, 315920, 315930, 315940, 315950, 315960, 111015};
	if(hair < 0 || hair > MAX_HAIR_NUM)
	{
		hair = 0;
	}

	if(hairColor < 0 || hairColor > MAX_HAIR_NUM - 1)
	{
		hairColor = 0;
	}

	if(sex == 1)
	{
		if(hair == MAX_HAIR_NUM)
		{
			return playerMaleHairShape[hair];
		}
		else
		{
			return playerMaleHairShape[hair] + hairColor;
		}
	}
	else
	{
		if(hair == MAX_HAIR_NUM)
		{
			return playerFeMaleHairShape[hair];
		}
		else
		{
			return playerFeMaleHairShape[hair] + hairColor;
		}
	}
}
#endif // NTJ_SERVER

#ifdef NTJ_CLIENT 
U32 getCreateFace(U32 sex, U32 face)
{
	const int MAX_FACE_NUM = 6;
	static int playerMaleFaceShape[MAX_FACE_NUM + 1] = {21091, 21092, 21093, 21094, 21095, 21096, 111012};
	static int playerFeMaleFaceShape[MAX_FACE_NUM + 1] = {21591, 21592, 21593, 21594, 21594, 21596, 111013};
	if(face < 0 || face > MAX_FACE_NUM)
	{
		face = 0;
	}

	if(sex == 1)
	{
		return playerMaleFaceShape[face];
	}
	else
	{
		return playerFeMaleFaceShape[face];
	}
}

U32 getCreateHair(U32 sex, U32 hair, U32 hairColor)
{
	const int MAX_HAIR_NUM = 6;
	static int playerMaleHairShape[MAX_HAIR_NUM + 1] = {310910, 310920, 310930, 310940, 310950, 310960, 111014};
	static int playerFeMaleHairShape[MAX_HAIR_NUM + 1] = {315910, 315920, 315930, 315940, 315950, 315960, 111015};
	if(hair < 0 || hair > MAX_HAIR_NUM)
	{
		hair = 0;
	}

	if(hairColor < 0 || hairColor > MAX_HAIR_NUM - 1)
	{
		hairColor = 0;
	}

	if(sex == 1)
	{
		if(hair == MAX_HAIR_NUM)
		{
			return playerMaleHairShape[hair];
		}
		else
		{
			return playerMaleHairShape[hair] + hairColor;
		}
	}
	else
	{
		if(hair == MAX_HAIR_NUM)
		{
			return playerFeMaleHairShape[hair];
		}
		else
		{
			return playerFeMaleHairShape[hair] + hairColor;
		}
	}
}
#endif //NTJ_CLIENT