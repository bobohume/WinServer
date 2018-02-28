#ifndef _AUTH_BIGNUMBER_H
#define _AUTH_BIGNUMBER_H

#include <vector>

typedef unsigned char   uint8;
typedef unsigned int    uint32;
typedef unsigned _int64 uint64;

struct bignum_st;

/************************************************************************/
/* 处理大数
/************************************************************************/
class BigNumber
{
public:
    BigNumber();
    BigNumber(const BigNumber &bn);
    BigNumber(uint32);
    ~BigNumber();

    void SetDword(uint32);
    void SetQword(uint64);
    void SetBinary(const uint8 *bytes, int len);
    void SetHexStr(const char *str);

    void SetRand(int numbits);

    BigNumber operator=(const BigNumber &bn);

    BigNumber operator+=(const BigNumber &bn);
    BigNumber operator+(const BigNumber &bn)
    {
        BigNumber t(*this);
        return t += bn;
    }
    BigNumber operator-=(const BigNumber &bn);
    BigNumber operator-(const BigNumber &bn)
    {
        BigNumber t(*this);
        return t -= bn;
    }
    BigNumber operator*=(const BigNumber &bn);
    BigNumber operator*(const BigNumber &bn)
    {
        BigNumber t(*this);
        return t *= bn;
    }
    BigNumber operator/=(const BigNumber &bn);
    BigNumber operator/(const BigNumber &bn)
    {
        BigNumber t(*this);
        return t /= bn;
    }
    BigNumber operator%=(const BigNumber &bn);
    BigNumber operator%(const BigNumber &bn)
    {
        BigNumber t(*this);
        return t %= bn;
    }

    BigNumber ModExp(const BigNumber &bn1, const BigNumber &bn2);
    BigNumber Exp(const BigNumber &);

    int GetNumBytes(void);

    struct bignum_st *BN() { return _bn; }

    uint32 AsDword();
    uint8* AsByteArray(int minSize = 0);
    std::vector<uint8> AsByteVector();

    const char *AsHexStr();
    const char *AsDecStr();

private:
    struct bignum_st *_bn;
    uint8 *_array;
};

#endif
