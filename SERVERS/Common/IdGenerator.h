#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include <stack>
#include "base/Locker.h"
#include "Common/CppTypeInfo.h"

class CIdGenerator : public CTI::ClassType< CIdGenerator >
{
public:
	CIdGenerator()
	{
		mIdSeed = 1000;
	}

	int allocId()
	{
		OLD_DO_LOCK( m_cs );

		int ret = 0;

		if( mIdStack.empty() )
		{
			ret = ++mIdSeed;
		}
		else
		{
			ret = mIdStack.top();
			mIdStack.pop();
		}

		return ret;
	}

	void freeId( int id )
	{
		OLD_DO_LOCK( m_cs );
		
		mIdStack.push( id );
	}

private:
	std::stack<int> mIdStack;
	int				mIdSeed;

	CMyCriticalSection m_cs;
};


#endif