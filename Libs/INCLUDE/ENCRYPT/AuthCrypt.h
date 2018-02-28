#ifndef _AUTHCRYPT_H
#define _AUTHCRYPT_H

#include <vector>
#include <string>

typedef unsigned char uint8;
class BigNumber;

#define SHA_DIGEST_LENGTH 20

/************************************************************************/
/* 轻量级的字符加密和解密
/************************************************************************/
class AuthCrypt
{
public:

    static AuthCrypt* Instance(void)
    {
        static AuthCrypt local;
        return &local;
    }
    
    void SetKey(BigNumber *bn) {}

    void DecryptRecv(uint8 *, size_t);
    void EncryptSend(uint8 *, size_t);
};

#endif
