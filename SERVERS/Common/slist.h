#pragma once

#include <memory>
template< typename _Ty >
struct SList
{
	typedef std::tr1::shared_ptr< _Ty > NodeValueRef;

	struct Node;

	typedef std::tr1::shared_ptr< Node > NodeRef;

	struct Node
	{
		NodeValueRef		mValue;
		NodeRef				mNext;
		NodeRef				mPrev;
	};

	NodeRef mHead;
	NodeRef mTail;

	unsigned int mCount;

	SList()
	{
		mCount = 0;
	}

	unsigned int size()
	{
		return mCount;
	}

	NodeRef push_front( NodeValueRef ref )
	{
		NodeRef node = NodeRef( new Node );

		if( mHead )
		{
			mHead->mPrev			= node;
			mHead->mPrev->mNext		= mHead;
			mHead					= mHead->mPrev;
			mHead->mValue			= ref;
		}
		else
		{
			mHead = mTail			= node;
			mHead->mValue			= ref;
		}

		++mCount;

		return node;
	}

	NodeRef push_front(	NodeRef node )
	{
		if( mHead )
		{
			mHead->mPrev			= node;
			mHead->mPrev->mNext		= mHead;
			mHead					= mHead->mPrev;
		}
		else
		{
			mHead = mTail = node;
		}

		++mCount;

		return node;
	}

	NodeRef push_back( NodeRef node )
	{
		if( mTail )
		{
			mTail->mNext			= node;
			mTail->mNext->mPrev		= mTail;
			mTail					= mTail->mNext;
		}
		else
		{
			mHead = mTail = node;
		}

		++mCount;

		return node;
	}

	NodeRef push_back( NodeValueRef ref )
	{
		NodeRef node = NodeRef( new Node );

		if( mTail )
		{
			mTail->mNext			= node;
			mTail->mNext->mPrev		= mTail;
			mTail					= mTail->mNext;
			mTail->mValue			= ref;
		}
		else
		{
			mHead = mTail = node;
			mTail->mValue = ref;
		}

		++mCount;

		return node;
	}

	void remove( NodeRef ref )
	{
		if( !ref )
			return ;

		NodeRef nodeP = ref->mPrev;
		NodeRef nodeN = ref->mNext;

		if( ref == mHead )
			mHead = mHead->mNext;

		if( ref == mTail )
			mTail = mTail->mPrev;

		if( nodeN )
			nodeN->mPrev = nodeP;

		if( nodeP )
			nodeP->mNext = nodeN;
		
		if (nodeP)
			ref->mPrev.reset();

		if (nodeN)
			ref->mNext.reset();

		--mCount;
	}

	NodeRef getHead()
	{
		return mHead;
	}

	NodeRef getTail()
	{
		return mTail;
	}
};