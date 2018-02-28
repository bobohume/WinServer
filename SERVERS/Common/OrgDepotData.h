//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------
#ifndef	_ORGDEPOTDATA_H
#define _ORGDEPOTDATA_H

#if defined(WORLDSERVER) || defined(NTJ_LOGSERVER)
#include "Common/DataFile.h"
#else
#include "platform/platform.h"
#include "Gameplay/Data/readDataFile.h"
#endif

#include "Common/OrgBase.h"
#include <map>
#include <vector>

//Common Data Begin
class TypeSize
{
public:
	enum Size
	{
		Type_String = 256,
		Type_8		= 1,
		Type_16		= 2,
		Type_32		= 4,
		Type_64		= 8
	};
	static Size GetTypeSize(DataType type)
	{
		static Size s_TypeSize[] = {Type_String, Type_8, Type_16, Type_8, Type_16, Type_32, Type_8, Type_16, Type_32, Type_32, Type_64};
		IF_ASSERT(!(type>0 && type<=_countof(s_TypeSize)))
			type = DType_U32;
		return s_TypeSize[type-1];
	}
};

class ResultProxy
{
public:
	ResultProxy(const char* data, DataType type)
		:m_data(NULL), m_type(type), m_size(0)
	{
		m_size = TypeSize::GetTypeSize(m_type);
		m_data = new char[m_size];
		memcpy(m_data, data, m_size);
	}

	~ResultProxy()
	{
		delete[] m_data;
		m_data = NULL;
	}

	ResultProxy(const ResultProxy& other)
		:m_data(NULL), m_type(DType_none), m_size(0)
	{
		*this = other;
	}

	ResultProxy& operator=(const ResultProxy& other)
	{
		if (this != &other)
		{
			if (m_size != other.m_size)
			{
				delete[] m_data;
				m_size = other.m_size;
				m_data = new char[m_size];
			}
			m_type = other.m_type;
			memcpy(m_data, other.m_data, m_size);
		}
		return *this;
	}

	operator std::string() const
	{
		IF_ASSERT(m_type != DType_string)
			return std::string();
		return std::string(m_data);
	}

	operator U8() const
	{ 
		IF_ASSERT(!(m_type==DType_enum8 || m_type==DType_U8)) 
			return 0; 
		return *(U8*)m_data; 
	}	

	operator U16() const
	{ 
		IF_ASSERT(!(m_type==DType_enum16 || m_type==DType_U16)) 
			return 0; 
		return *(U16*)m_data; 
	}	


#define OPERATOR(type) \
	operator type() const \
	{ \
		IF_ASSERT(m_type != DType_##type) \
			return 0; \
		return *(type*)m_data; \
	}	

	OPERATOR(S8)
	OPERATOR(S16)
	OPERATOR(S32)
	OPERATOR(U32)	
	OPERATOR(F32)	
	OPERATOR(F64)	

#undef OPERATOR

private:
	char* m_data;
	DataType m_type;
	int m_size;
};

template <typename T>
class TData
{
public:
	TData()
		:m_data(NULL), m_size(0)
	{
		memset(&m_offset, 0, sizeof(m_offset));
		memset(&m_type, 0, sizeof(m_type));
	}

	bool Init(const CDataFile& file)
	{
		IF_ASSERT(file.ColumNum != T::Column_Count)
			return false;
		IF_ASSERT(file.m_DataTypes.size() != T::Column_Count)
			return false;
		for (int i=0; i<T::Column_Count; ++i)
		{
			m_offset[i] = m_size;
			m_type[i] = (DataType)file.m_DataTypes[i];
			m_size += TypeSize::GetTypeSize(m_type[i]);
		}
		m_data = new char[m_size];
		memset(m_data, 0, m_size);
		return true;
	}

	~TData()
	{
		delete[] m_data;
		m_data = NULL;
	}

	TData(const TData& other)
		:m_data(NULL), m_size(0)
	{
		memset(&m_offset, 0, sizeof(m_offset));
		memset(&m_type, 0, sizeof(m_type));
		*this = other;
	}

	TData& operator=(const TData& other)
	{
		if (this != &other)
		{
			if (m_size != other.m_size)
			{
				delete[] m_data;
				m_size = other.m_size;
				m_data = new char[m_size];
			}
			memcpy(m_data, other.m_data, m_size);
			memcpy(m_offset, other.m_offset, sizeof(m_offset));
			memcpy(m_type, other.m_type, sizeof(m_type));
		}
		return *this;
	}

	typedef std::tr1::shared_ptr< TData<T> > TDataPtr;
	TDataPtr Copy() const
	{
		return TDataPtr(new TData(*this));
	}

	void Read(CDataFile& file)
	{
		for (int i=0; i<T::Column_Count; ++i)
		{
			RData lineData;
			file.GetData(lineData);
			IF_ASSERT(m_type[i] != lineData.m_Type)
				continue;

#define CPYDATA(data) memcpy(&m_data[m_offset[i]], &lineData.##data, TypeSize::GetTypeSize(m_type[i]));

			switch (m_type[i])
			{
			case DType_string: 
				CPYDATA(m_string)
				m_data[m_offset[i]+TypeSize::GetTypeSize(m_type[i])-1] = 0;
				break;
			case DType_U8:
				CPYDATA(m_U8)
				break;
			case DType_S8:
				CPYDATA(m_S8)
				break;
			case DType_U16:
				CPYDATA(m_U16)
				break;
			case DType_S16:
				CPYDATA(m_S16)
				break;
			case DType_U32:
				CPYDATA(m_U32)
				break;
			case DType_S32:
				CPYDATA(m_S32)
				break;
			case DType_enum8:
				CPYDATA(m_Enum8)
				break;
			case DType_enum16:
				CPYDATA(m_Enum16)
				break;
			case DType_F32:
				CPYDATA(m_F32)
				break;
			case DType_F64:
				CPYDATA(m_F64)
				break;
			default:
				g_Log.WriteError("Error type[%d]!", m_type[i]);
				break;
			}

#undef CPYDATA
		}
	}

	ResultProxy GetData(typename T::Column column) const
	{
		return ResultProxy(&m_data[m_offset[column]], m_type[column]);
	}

private:
	int m_offset[T::Column_Count];
	DataType m_type[T::Column_Count];
	char* m_data;
	int m_size;
};

template <typename T>
class TDataManager
{
public:
	typedef TData<T> Data;
	typedef typename Data::TDataPtr DataPtr;
	bool Init(const char* fileName)
	{
        m_depots.clear();
        m_queues.clear();

		CDataFile file;
		file.ReadDataInit();
		IF_ASSERT(!file.readDataFile(fileName))
		{
			file.ReadDataClose();
			return false;
		}
		Data data;
		IF_ASSERT(!data.Init(file))
			return false;
		for (int i=0; i<file.RecordNum; i++)
		{
			DataPtr dataPtr = data.Copy();
			dataPtr->Read(file);
			IF_ASSERT(!m_depots.insert(Depots::value_type(dataPtr->GetData((typename T::Column)0), dataPtr)).second)
				continue;
			m_queues.push_back(dataPtr);
		}
		m_iterator = m_queues.begin();
		return true;
	}

	DataPtr GetData(U32 itemID) const
	{
		DepotsIter iter = m_depots.find(itemID);
		if (iter != m_depots.end())
			return iter->second;
		return DataPtr();
	}

	DataPtr Begin() const
	{
		m_iterator = m_queues.begin();
		if (m_iterator != m_queues.end())
			return *m_iterator;
		return DataPtr();
	}

	DataPtr Next() const
	{
		if (m_iterator == m_queues.end())
			return DataPtr();
		++m_iterator;
		if (m_iterator != m_queues.end())
			return *m_iterator;
		return DataPtr();
	}

	U32 Size() const
	{
		return m_depots.size();
	}

private:
	typedef std::map<U32, DataPtr> Depots;
	typedef typename Depots::const_iterator DepotsIter;
	typedef std::vector<DataPtr> Queues;
	typedef typename Queues::const_iterator QueuesIter;

	Depots m_depots; 
	Queues m_queues;
	mutable QueuesIter m_iterator;
};

template <typename T>
class TDataInstance
{
public:
	typedef TDataManager<T> DataManager;
	DataManager m_dataManager;
	TDataInstance(const char* file)
	{
		char fileName[1024];
#if defined(WORLDSERVER) || defined(NTJ_LOGSERVER)
		sprintf_s(fileName, _countof(fileName), "%s", file);
#else
		Platform::makeFullPathName(file, fileName, _countof(fileName));
#endif
		JUST_ASSERT(!m_dataManager.Init(fileName));
	}
};
//Common Data End
//Manual: class T must define enum Column and Column_Count. that is all.

class COrgDepot
{
public:
	enum Column
	{
		Column_ItemID,
		Column_Category,
		Study_PreStudy,
		Study_OrgMoney,
		Study_OrgLevel,
		Study_LeagueLevel,
		Study_BossCumulate,
		Make_PreItem,
		Make_OrgMoney,
		Make_Infinity,
		Make_Condition4,
		Make_Condition5,
		Buy_Class,
		Buy_OrgMoney,
		Buy_MemberLevel,
		Buy_ConsumeItem,
		Buy_Condition5,
		Column_FitClass,
		Column_Count
	};

	typedef TDataInstance<COrgDepot>::DataManager::DataPtr Data;
	typedef std::vector<Data> Depots;
	typedef Depots::const_iterator DepotsIter;

	static COrgDepot* GetInstance();
	Data GetData(U32 itemID) const;
	const Depots& GetCategory(U32 category, U32 nClass) const;
	Data Begin() const;
	Data Next() const;
	U32 Size() const;
    
    void Reload();
private:
	COrgDepot();

	TDataInstance<COrgDepot> m_dataInstance;
	typedef std::map<U32, Depots> Categorys;
	typedef Categorys::const_iterator CategorysIter;
	Categorys m_categorys;
	Depots m_null;
	mutable Depots m_class;
};

#define ORGDEPOT COrgDepot::GetInstance()

class COrgBoss
{
public:
	enum Column
	{
		Column_BossID,
		Column_Nimbus,
		Column_Skill1,
		Column_Skill2,
		Column_Skill3,
		Column_Skill4,
		Column_Skill5,
		Column_Skill6,
		Column_Skill7,
		Column_Skill8,
		Column_Skill9,
		Column_Skill10,
		Column_Skill11,
		Column_Skill12,
		Column_DropItem1,
		Column_DropItem2,
		Column_DropItem3,
		Column_DropItem4,
		Column_DropItem5,
		Column_DropItem6,
		Column_DropItem7,
		Column_DropItem8,
		Column_DropItem9,
		Column_DropItem10,
		Column_DropItem11,
		Column_DropItem12,
		Column_Level,
		Column_Count
	};

	typedef TDataInstance<COrgBoss>::DataManager::DataPtr Data;

	static COrgBoss* GetInstance();
	Data GetData(U32 bossID) const;
	Data Begin() const;
	Data Next() const;
    
    void Reload();
private:
	COrgBoss();

	TDataInstance<COrgBoss> m_dataInstance;
};

#define ORGBOSS COrgBoss::GetInstance()

#endif //_ORGDEPOTDATA_H