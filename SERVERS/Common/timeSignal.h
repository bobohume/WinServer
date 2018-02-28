//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FairRain.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _UTIL_TIMESIGNAL_H_
#define _UTIL_TIMESIGNAL_H_
#include "base/types.h"
#include "base/tVector.h"

template <class A = EmptyType, class B = EmptyType, class C = EmptyType, class D = EmptyType, 
class E = EmptyType, class F = EmptyType, class G = EmptyType, class H = EmptyType>
class TimeSignal
{
private:
	// Delegate base class
	struct DelegateLink 
	{
		U32 timeStep;
		U32 timeStepRT;
		U32 timeStepExDelay;
		U32 timeStepExMod;
		U32 destTime;
		S32 runTimes;
		bool triggerOnce;
		bool needremove;
		virtual ~DelegateLink() {}
	};

	// Base class with virtual operator(), no other way to deal with
	// multiple arguments except to specialize. Don't really need the
	// class RR return type, but specializing the class without it
	// was causing problems with gcc 3.3.1
	template <class RR, class AA, class BB, class CC, class DD,
	class EE, class FF, class GG, class HH>
	struct DelegateBase: public DelegateLink {
		virtual RR operator()(AA,BB,CC,DD,EE,FF,GG,HH) = 0;
	};

	template <class RR, class AA, class BB, class CC, class DD,
	class EE, class FF, class GG>
	struct DelegateBase<RR,AA,BB,CC,DD,EE,FF,GG,
		EmptyType>: public DelegateLink {
			virtual RR operator()(AA,BB,CC,DD,EE,FF,GG) = 0;
	};

	template <class RR, class AA, class BB, class CC, class DD, 
	class EE, class FF>
	struct DelegateBase<RR,AA,BB,CC,DD,EE,FF,
		EmptyType,EmptyType>: public DelegateLink {
			virtual RR operator()(AA,BB,CC,DD,EE,FF) = 0;
	};

	template <class RR, class AA, class BB, class CC, class DD,
	class EE>
	struct DelegateBase<RR,AA,BB,CC,DD,EE,
		EmptyType,EmptyType,EmptyType> : public DelegateLink {
			virtual RR operator()(AA,BB,CC,DD,EE) = 0;
	};

	template <class RR, class AA, class BB, class CC, class DD>
	struct DelegateBase<RR,AA,BB,CC,DD,EmptyType,EmptyType,
		EmptyType,EmptyType>: public DelegateLink {
			virtual RR operator()(AA,BB,CC,DD) = 0;
	};

	template <class RR, class AA, class BB, class CC>
	struct DelegateBase<RR,AA,BB,CC,EmptyType,EmptyType,
		EmptyType,EmptyType,EmptyType>: public DelegateLink 
	{
		virtual RR operator()(AA,BB,CC) = 0;
	};

	template <class RR, class AA, class BB>
	struct DelegateBase<RR,AA,BB,EmptyType,EmptyType,EmptyType,EmptyType,
		EmptyType,EmptyType>: public DelegateLink 
	{
		virtual RR operator()(AA,BB) = 0;
	};

	template <class RR, class AA>
	struct DelegateBase<RR,AA,EmptyType,EmptyType,EmptyType,
		EmptyType,EmptyType,EmptyType,EmptyType>: public DelegateLink 
	{
		virtual RR operator()(AA) = 0;
	};

	template <class RR>
	struct DelegateBase<RR,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,
		EmptyType,EmptyType,EmptyType> : public DelegateLink {
			virtual RR operator()() = 0;
	};

	// Member function pointers
	template <class T, class U>
	struct DelegateMethod: public DelegateBase<void,A,B,C,D,E,F,G,H> {
		T object;
		U func;
		DelegateMethod(T ptr,U ff): object(ptr),func(ff) {}
		void operator()(A a,B b,C c,D d,E e,F f,G g,H h) {  (object->*func)(a,b,c,d,e,f,g,h); }
		void operator()(A a,B b,C c,D d,E e,F f,G g) {  (object->*func)(a,b,c,d,e,f,g); }
		void operator()(A a,B b,C c,D d,E e,F f) {  (object->*func)(a,b,c,d,e,f); }
		void operator()(A a,B b,C c,D d,E e) {  (object->*func)(a,b,c,d,e); }
		void operator()(A a,B b,C c,D d) {  (object->*func)(a,b,c,d); }
		void operator()(A a,B b,C c) {  (object->*func)(a,b,c); }
		void operator()(A a,B b) {  (object->*func)(a,b); }
		void operator()(A a) {  (object->*func)(a); }
		void operator()() {  (object->*func)(); }
	};

	// Static function pointers
	template <class T>
	struct DelegateFunction: public DelegateBase<void,A,B,C,D,E,F,G,H> {
		T func;
		DelegateFunction(T ff): func(ff) {}
		void operator()(A a,B b,C c,D d,E e,F f,G g,H h) {  (*func)(a,b,c,d,e,f,g,h); }
		void operator()(A a,B b,C c,D d,E e,F f,G g) {  (*func)(a,b,c,d,e,f,g); }
		void operator()(A a,B b,C c,D d,E e,F f) {  (*func)(a,b,c,d,e,f); }
		void operator()(A a,B b,C c,D d,E e) {  (*func)(a,b,c,d,e); }
		void operator()(A a,B b,C c,D d) {  (*func)(a,b,c,d); }
		void operator()(A a,B b,C c) {  (*func)(a,b,c); }
		void operator()(A a,B b) {  (*func)(a,b); }
		void operator()(A a) {  (*func)(a); }
		void operator()() {  (*func)(); }
	};

private:
	static S32 timeSort(const void *a, const void *b)
	{
		const DelegateLink* mse1 = *((const DelegateLink**) a);
		const DelegateLink* mse2 = *((const DelegateLink**) b);

		return S32(mse1->destTime) - S32(mse2->destTime);  //Ë³Ðò
	}

public:
	Vector<DelegateLink *> mNodeList;
	U32 mNextSortTime;
	U32 mTargetTime;
	S32 mTriggerSequence;
	CMyCriticalSection m_cs;

public:

	TimeSignal() 
	{
		mNextSortTime = U32_MAX;
		mTriggerSequence = 0;
		mTargetTime = 0;
	}

	~TimeSignal() 
	{
		for(int i=mNodeList.size()-1;i>=0;i--)
		{
			delete mNodeList[i];
		}
	}

	inline bool isEmpty() 
	{
		return mNodeList.size()==0;
	}

	template <class T,class U>
	void notify(T obj,U func, U32 timeStep, bool triggerOnce=false, S32 timeCount=-1, U32 timeStepExDelay=0, U32 timeStepExMod=0) 
	{
		CLocker lock(m_cs);
		for(int i=mNodeList.size()-1;i>=0;i--)
		{
			if (DelegateMethod<T,U>* fd = dynamic_cast<DelegateMethod<T,U>*>(mNodeList[i]))
			{
				if (fd->object == obj && fd->func == func && !fd->needremove) 
				{
					return;
				}
			}
		}

		DelegateLink *pNode = new DelegateMethod<T,U>(obj,func);
		pNode->timeStep = pNode->timeStepRT = timeStep;
		pNode->timeStepExDelay = timeStepExDelay;
		pNode->timeStepExMod = timeStepExMod;
		pNode->destTime = GetTickCount()+timeStep;
		pNode->triggerOnce = triggerOnce;
		pNode->needremove = false;
		pNode->runTimes = timeCount;
		mNodeList.push_back(pNode);
		mNextSortTime = getMin(mNextSortTime, pNode->destTime);
	}

	template <class T>
	void notify(T func, U32 timeStep, bool triggerOnce=false, S32 timeCount=-1, U32 timeStepExDelay=1000, U32 timeStepExMod=0) 
	{
		CLocker lock(m_cs);
		for(int i=mNodeList.size()-1;i>=0;i--)
		{
			if (DelegateFunction<T>* fd = dynamic_cast<DelegateFunction<T>*>(mNodeList[i]))
			{
				if (fd->func == func && !fd->needremove) 
				{
					return;
				}
			}
		}

		DelegateLink *pNode = new DelegateFunction<T>(func);
		pNode->timeStep = pNode->timeStepRT = timeStep;
		pNode->timeStepExDelay = timeStepExDelay;
		pNode->timeStepExMod = timeStepExMod;
		pNode->destTime = GetTickCount()+timeStep;
		pNode->triggerOnce = triggerOnce;
		pNode->needremove = false;
		pNode->runTimes = timeCount;
		mNodeList.push_back(pNode);
		mNextSortTime = getMin(mNextSortTime, pNode->destTime);
	}

	template <class T>
	void remove(T func) 
	{
		CLocker lock(m_cs);
		for(int i=mNodeList.size()-1;i>=0;i--)
		{
			if (DelegateFunction<T>* fd = dynamic_cast<DelegateFunction<T>*>(mNodeList[i]))
			{
				if (fd->func == func && !fd->needremove) 
				{
					fd->needremove = true;
					return;
				}
			}
		}
	}

	template <class T,class U>
	void remove(T obj,U func) 
	{
		CLocker lock(m_cs);
		for(int i=mNodeList.size()-1;i>=0;i--)
		{
			if (DelegateMethod<T,U>* fd = dynamic_cast<DelegateMethod<T,U>*>(mNodeList[i]))
			{
				if (fd->object == obj && fd->func == func && !fd->needremove) 
				{
					fd->needremove = true;
					return;
				}
			}
		}
	}

	void trigger() 
	{
		CLocker lock(m_cs);
		U32 timeCount = GetTickCount();
		if(mNextSortTime <= timeCount)
		{
			qsort(mNodeList.address(),mNodeList.size(),sizeof(DelegateLink *),timeSort);
			mNextSortTime = U32_MAX;
			mTriggerSequence=0;
		}

		DelegateLink *pNode = NULL;
		int size = mNodeList.size();

		for(; mTriggerSequence<mNodeList.size(); ++mTriggerSequence)
		{
			pNode = mNodeList[mTriggerSequence];

			if (pNode->needremove)
			{
				delete pNode;
				mNodeList.erase(mTriggerSequence);
				--mTriggerSequence;
				continue;
			}
			if(pNode->destTime > timeCount)
				break;

			if(!pNode->triggerOnce)
			{
				U32 delta = pNode->timeStep;

				//while(pNode->destTime <= timeCount)
				if(pNode->destTime <= timeCount)
				{
					(*(DelegateBase<void,A,B,C,D,E,F,G,H>*)pNode)(delta);

					//pNode->destTime += pNode->timeStep;
					pNode->destTime = timeCount + pNode->timeStep;
					if(pNode->runTimes > 0 && pNode->needremove == false)
					{
						--pNode->runTimes;
						if(pNode->runTimes == 0)
						{
							pNode->needremove = true;
						}
					}
				}
			}
			else
			{
				U32 delta = timeCount-pNode->destTime+pNode->timeStepRT;

				(*(DelegateBase<void,A,B,C,D,E,F,G,H>*)pNode)(delta);

				if (pNode->timeStepExMod)
				{
					delta = (delta > pNode->timeStepRT + pNode->timeStepExDelay) ? (delta - pNode->timeStepRT - pNode->timeStepExDelay) : 0;
					pNode->timeStepRT = pNode->timeStep + (mTriggerSequence%pNode->timeStepExMod) + delta;
				}
				pNode->destTime = timeCount + pNode->timeStepRT;

				if(pNode->runTimes > 0 && pNode->needremove == false)
				{
					--pNode->runTimes;
					if(pNode->runTimes == 0)
					{
						pNode->needremove = true;
					}
				}
			}
			mNextSortTime = getMin(mNextSortTime, pNode->destTime);
		}
		if(mNextSortTime != U32_MAX)
		{
			mTargetTime = mNextSortTime;
		}
		else
		{
			mTargetTime = timeCount;
		}
	}
};

#endif