#ifndef SAFE_MEM_POOL_H
#define SAFE_MEM_POOL_H

template< typename _Ty >
class SafeAllcation
{
public:
	SafeAllcation();
	virtual ~SafeAllcation();
};

template< int __size >
class SafeMemPool
{
	
};

class SafeMemManager
{
public:
	SafeMemManager();
	virtual ~SafeMemManager();

private:
	static void CALLBACK exceptionHandler()
	{
		
	}
};

#endif