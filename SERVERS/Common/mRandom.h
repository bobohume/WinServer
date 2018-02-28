//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FairRain.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _MRANDOM_H_
#define _MRANDOM_H_

#include "Base/types.h"
#include <assert.h>

//--------------------------------------
/// Base class for random number generators
class MRandomGenerator
{
protected:
   MRandomGenerator() {}
   S32  mSeed;

public:
   void setSeed();
   S32  getSeed() { return mSeed; }
   virtual void setSeed(S32 s) = 0;

   virtual U32 randI( void ) = 0;      ///< 0..2^31 random number generator
   virtual F32  randF( void );         ///< 0.0 .. 1.0 F32 random number generator
   S32 randI(S32 i, S32 n);            ///< i..n integer random number generator
   F32 randF(F32 i, F32 n);            ///< i..n F32 random number generator

   //为了支持模板，添加下面的函数
   S32 randM(S32 i, S32 n) { return randI(i, n);}
   F32 randM(F32 i, F32 n) { return randF(i, n);}
};


//--------------------------------------
inline F32 MRandomGenerator::randF()
{
   // default: multiply by 1/(2^31)
   return  F32(randI()) * (1.0f/2147483647.0f);
}

inline S32 MRandomGenerator::randI(S32 i, S32 n)
{
   if(i	> n)
	   assert("MRandomGenerator::randi: inverted range.");

   return (S32)(i + (randI() % (n - i + 1)) );
}

inline F32 MRandomGenerator::randF(F32 i, F32 n)
{
   //AssertFatal(i<=n, "MRandomGenerator::randf: inverted range.");
   return (i + (n - i) * randF());
}
//--------------------------------------
/// Linear Congruential Method, the "minimal standard generator"
///
/// Fast, farly good random numbers (better than using rand)
///
/// @author Park & Miller, 1988, Comm of the ACM, 31(10), pp. 1192-1201
class MRandomLCG : public MRandomGenerator
{
protected:
   static const S32 msQuotient;
   static const S32 msRemainder;

public:
   MRandomLCG();
   MRandomLCG(S32 s);

   static void setGlobalRandSeed();

   void setSeed(S32 s);
//   using MRandomGenerator::randI;
    S32 randI(S32 i, S32 n);            ///< i..n integer generator

   U32 randI( void );

};

// Solution to "using" problem.
inline S32 MRandomLCG::randI(S32 i, S32 n)
{
    return( MRandomGenerator::randI(i,n) );
}

typedef MRandomLCG MRandom;

extern MRandomLCG gRandGen;


#endif //_MRANDOM_H_
