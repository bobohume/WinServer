#ifndef __TBLSTALLLIST_H__
#define __TBLSTALLLIST_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#pragma once
#include <vector>

struct stFixedStall;
struct stFixedStallItem;
struct stFixedStallPet;

class TBLStallList: public TBLBase
{
public:
	TBLStallList(DataBase* db):TBLBase(db) {}
	virtual ~TBLStallList() {}
	DBError SaveInfo(stFixedStall* info);
	DBError SaveSellItem(U16 stallid, stFixedStallItem* item);
	DBError SaveBuyItem(U16 stallid, stFixedStallItem* item);
	DBError SaveSellPet(U16 stallid, stFixedStallPet* pet);
	DBError LoadAllInfo(stFixedStall* info);
	DBError LoadAllSellItem(stFixedStallItem* pItem);
	DBError LoadAllBuyItem(stFixedStallItem* pItem);
	DBError LoadAllSellPet(stFixedStallPet* pPet);
	DBError UpdateSellItem(stFixedStallItem* pItem);
	DBError UpdateBuyItem(stFixedStallItem* pItem);
	DBError UpdateSellPet(stFixedStallPet* pPet);
};

#endif//__TBLSTALLLIST_H__