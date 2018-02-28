#pragma  once
#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

int base64_encode(const unsigned char* pSrc, char* pDst, int nSrcLen, int nMaxLineLen);
int base64_decode(const char* pSrc, unsigned char* pDst, int nSrcLen);
