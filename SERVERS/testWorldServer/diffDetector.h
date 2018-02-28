//
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <map>


struct Diff//比较之后需要更新的数据
{
	std::vector<int> m_vecNeedUpdate;//需要更新的index 此指新数组中的Index
	std::vector<int> m_vecNeedInsert;//需要添加的index 此指新数组中的Index
	std::vector<int> m_vecNeedDelete;//需要删除的index 此指老数组中的Index
};

template<typename ElementStruct,int MaxCount,typename keytype>
class DiffDetector
{
public:
	typedef std::map<keytype,int> MapKey2Index;
public:
	static bool IsMemorySame(ElementStruct& a, ElementStruct& b) //判断两个结构是否完全相等
	{
		return (memcmp(&a,&b,sizeof(ElementStruct) == 0));
	}
	//比较两个结构数组，以pKey字段为0作为终结符，得到哪些记录需要更新/删除/添加
	static Diff GetDiff(ElementStruct (&oldData)[MaxCount], 
					    ElementStruct (&newData)[MaxCount],
						keytype ElementStruct::*pKey)
	{
		Diff res;
		MapKey2Index t_MapOldKey2Index;
		MapKey2Index t_MapNewKey2Index;
		for (int i=0; i<MaxCount; i++)
		{
			if (oldData[i].*pKey ==0 ) break; //0是终结符
			t_MapOldKey2Index.insert(std::make_pair(oldData[i].*pKey,i));
			
		}
		MapKey2Index::iterator oldset_end = t_MapOldKey2Index.end();
		
		for (int i=0; i<MaxCount; i++)//newkeyset中没有出现在oldkeyset中的需要insert
		{
			if (newData[i].*pKey==0) break; //终结符
			if (t_MapOldKey2Index.find(newData[i].*pKey) == oldset_end)
			{
				res.m_vecNeedInsert.push_back(i);//需要insert
			}			
			t_MapNewKey2Index.insert(std::make_pair(newData[i].*pKey,i));			
		}

		MapKey2Index::iterator newset_end = t_MapNewKey2Index.end();

		for (int i=0; i<MaxCount; i++)//oldkeyset中没有出现在newkeyset中的需要delete
		{
			if (oldData[i].*pKey==0) break; //终结符
			if (t_MapNewKey2Index.find(oldData[i].*pKey) == newset_end)
			{
				res.m_vecNeedDelete.push_back(i);//需要删除
			}			
		}

		MapKey2Index::iterator itNewMap = t_MapNewKey2Index.begin();
		MapKey2Index::iterator itNewMapEnd = t_MapNewKey2Index.end();
		MapKey2Index::iterator itOldMapEnd = t_MapOldKey2Index.end();
		for (; itNewMap!= itNewMapEnd; itNewMap++)
		{
			MapKey2Index::iterator itOldMap = t_MapOldKey2Index.find(itNewMap->first);
			if (itOldMap != itOldMapEnd)//存在newMap的同时也存在于OldMap 可能需要更新
			{
				if (!IsMemorySame(newData[itNewMap->second], oldData[itOldMap->second]))//判断两块内存是否相等
				{
					res.m_vecNeedUpdate.push_back(itNewMap->second);//需要更新
				}
			}
		}


		return res;

		
	}
};