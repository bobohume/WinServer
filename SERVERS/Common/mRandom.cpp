//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FairRain.com, Inc.
//-----------------------------------------------------------------------------

#include "mRandom.h"
#include <time.h>


MRandomLCG gRandGen;


static U32 msSeed = 1376312589;

inline U32 generateSeed()
{
   // A very, VERY crude LCG but good enough to generate
   // a nice range of seed values
   msSeed = (U32)time(NULL);
   msSeed = (msSeed * 0x015a4e35L) + 1;
   msSeed = (msSeed>>16)&0x7fff;
   return (msSeed);
}

void MRandomLCG::setGlobalRandSeed()
{
	//msSeed = (S32)time(NULL);
	gRandGen.setSeed(msSeed);
}

//--------------------------------------
void MRandomGenerator::setSeed()
{
   setSeed(generateSeed());
}


//--------------------------------------
const S32 MRandomLCG::msQuotient  = S32_MAX / 16807L;
const S32 MRandomLCG::msRemainder = S32_MAX % 16807L;


//--------------------------------------
MRandomLCG::MRandomLCG()
{
   setSeed(generateSeed());
}

MRandomLCG::MRandomLCG(S32 s)
{
   setSeed(s);
}


//--------------------------------------
void MRandomLCG::setSeed(S32 s)
{
   mSeed = s;
}


//--------------------------------------
U32 MRandomLCG::randI()
{
   if ( mSeed <= msQuotient )
      mSeed = (mSeed * 16807L) % S32_MAX;
   else
   {
      S32 high_part = mSeed / msQuotient;
      S32 low_part  = mSeed % msQuotient;

      S32 test = (16807L * low_part) - (msRemainder * high_part);

      if ( test > 0 )
         mSeed = test;
      else
         mSeed = test + S32_MAX;

   }
   return mSeed;
}
