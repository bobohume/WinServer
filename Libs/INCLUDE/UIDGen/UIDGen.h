#ifndef _UIDGEN_H_
#define _UIDGEN_H_

/************************************************************************/
/* 计算当前的程序的运行环境的标识
/************************************************************************/

#define UID_VERSION   0x10
#define UID_FILE_MARK 0xaf03102f

//比较key值是否和注册表里的一致
extern bool IsUIDMatch(void);
extern const char* CalUID(void);

#endif /*_UIDGEN_H_*/