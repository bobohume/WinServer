#ifndef __BASE64_H
#define __BASE64_H

class dBase64
{
public:
	static int encode(const unsigned char* pSrc, char* pDst, int nSrcLen, int nMaxLineLen);
	static int decode(const char* pSrc, unsigned char* pDst, int nSrcLen);

private:
	static const char EnBase64Tab[];
	static const char DeBase64Tab[];
};

#endif /*__BASE64_H*/