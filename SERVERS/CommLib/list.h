#pragma once


template <class T>
struct stNode 
{
	stNode<T>* prev;
	stNode<T>* next;
	T value;

	stNode():value(0), prev(0), next(0){
	}
};

template<class T>
class List
{
public:
	stNode<T>* m_pFrontNode;
	stNode<T>* m_pBackNode;

	List():m_pFrontNode(0), m_pBackNode(0)
	{
	}

	void PushBack(T val)
	{
		stNode<T>* pNode = new stNode<T>();
		pNode->value = val;
		if(m_pBackNode == NULL)
		{
			m_pBackNode = m_pFrontNode = pNode;
		}
		else 
		{
			pNode->next = m_pBackNode;
			m_pBackNode->prev = pNode;
			m_pBackNode = pNode;
		}
	}

	void PopBack()
	{
		if (m_pBackNode != NULL) 
		{
			stNode<T>* pNode = m_pBackNode;
			m_pBackNode = m_pBackNode->next;
			delete(pNode);
			if (m_pBackNode != NULL)
			{
				m_pBackNode->prev = NULL;
			}
			else
			{
				m_pFrontNode = NULL;
			}
		}
	}

	T Back()
	{
		if (m_pBackNode != NULL)
		{
			return m_pBackNode->value;
		}
		
		return NULL;
	}


	void PushFront(T val)
	{
		stNode<T>* pNode = new stNode<T>();
		pNode->value = val;
		if (m_pFrontNode == NULL)
		{
			m_pBackNode = m_pFrontNode = pNode;
		}
		else
		{
			pNode->prev = m_pFrontNode;
			m_pFrontNode->next = pNode;
			m_pFrontNode = pNode;
		}
	}


	void PopFront()
	{
		if (m_pFrontNode != NULL)
		{
			stNode<T>* pNode = m_pFrontNode;
			m_pFrontNode = m_pFrontNode->prev;
			delete(pNode);
			if (m_pFrontNode != NULL)
			{
				m_pFrontNode->next = NULL;
			}
			else
			{
				m_pBackNode = NULL;
			}
		}
	}

	T Front()
	{
		if (m_pFrontNode != NULL)
		{
			return m_pFrontNode->value;
		}

		return NULL;
	}

	bool Empty() 
	{
		return m_pBackNode == NULL;
	}
};

