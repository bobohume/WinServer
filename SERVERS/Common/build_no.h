//svn自动生成版本号
#pragma once

// 内部消息版本号，与外网版本号一致

#define BUILD_NO 1,5,1,1
const int SERVER_VERSION = 102008000;

#define LANG_Simplified_Chinese 0
#define LANG_Traditional_Chinese 1
#define LANG_VERSION LANG_Simplified_Chinese
 
#define ITOA_M(NUMBER)		#NUMBER
#define TEXT_M(LINE)		ITOA_M(LINE)
#define G_PVERSION			TEXT_M(BUILD_NO)

#ifndef _DEBUG
#define G_VERSION_BUILD "Build:"##G_PVERSION##" ["##__DATE__##"]-["##__TIME__##"]"
#else
#define G_VERSION_BUILD "Build(D):"##G_PVERSION##" ["##__DATE__##"]-["##__TIME__##"]"
#endif
    
   
                
     