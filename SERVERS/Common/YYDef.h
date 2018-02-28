#pragma once
enum EM_YYERROR
{
	EM_YYERROR_START = -1,
	EM_YYERROR_NONE,
	EM_YYERROR_NOTARGETONLINE,//目标玩家不在线
	EM_YYERROR_END,
};
#define YYPROCESSNAME "yyhzh.exe"
#define MAXWAITTIME 30000    //超出此时间上限，即判断为DLL内部错误，停止初始化部分，初始化错误
enum EM_YYINITTYPE
{
	EM_YYINIT_NONE = 0,
	EM_YYINIT_PRIVATE,  //初始化完以后调用私人会话窗口
	EM_YYINIT_TEAM,			//。。。。队伍
	EM_YYINIT_MAIN,		//YY界面窗口的主界面窗口
	EM_YYINIT_FAMILY,   //。。。。家族
	EM_YYINIT_END,
};