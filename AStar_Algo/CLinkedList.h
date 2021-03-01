#pragma once
#include "stdafx.h"


//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//	귀찮아서 새로만듬...
//	A-Star 알고리즘 노드 전용의 LinkedList 입니다.
//--------------------------------------------------------------
class CLinkedHeap
{	
public:
	CLinkedHeap();
	~CLinkedHeap();

	bool Push_ANode(stANode *pValue);
	stANode* Pop_ANode(void);
	stList* Search_ANode(DWORD, DWORD);
	void Clear();
	void Pop_Push_Sort(stANode *p);
private:
	stList *_ptrHead;
	DWORD	_dwNumOfList;
};

//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//--------------------------------------------------------------
CLinkedHeap::CLinkedHeap()
{
	_ptrHead = NULL;
	_dwNumOfList = 0;
}

//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//--------------------------------------------------------------
CLinkedHeap::~CLinkedHeap()
{
	//삭제할 건데 이건 나중에만들래 ~
	Clear();
}


//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//--------------------------------------------------------------
bool CLinkedHeap::Push_ANode(stANode *pValue)
{
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	예외처리
	//--------------------------------------------------------------
	if (pValue == NULL)
		return false;

	stList *pNewList = new stList;
	pNewList->pNext = NULL;
	pNewList->pPrev = NULL;
	pNewList->value = pValue;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	아무것도 없다? 그냥 넣고 꺼졍 ^ㅇ^
	//--------------------------------------------------------------
	if (_ptrHead == NULL)
	{
		_ptrHead = pNewList;
		_dwNumOfList++;
		return true;
	}
	//	Loop 를 돌기위한 pNow;
	stList *pNow = _ptrHead;
	while (1)
	{
		// 내가 pNow보다 작으면 pNow 앞에 갖다 붙는다.
		if (pNow->value->F >= pNewList->value->F)
		{
			pNewList->pPrev = pNow->pPrev;
			pNewList->pNext = pNow;
			if (pNow->pPrev != NULL)
			{
				pNow->pPrev->pNext = pNewList;
			}

			pNow->pPrev = pNewList;

			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	Root 처리해준다.
			//--------------------------------------------------------------
			if (pNow == _ptrHead)
			{
				_ptrHead = pNewList;
			}
			_dwNumOfList++;
			return true;
		}
		// 아니면 pNow를 전진시키고, 다시 루프한다. 마지막까지 왔다는 것은 내가 제일 큰 놈이다.
		// 따라서 맨 뒤에 그냥 넣고 종료.
		else
		{
			if (pNow->pNext != NULL)
				pNow = pNow->pNext;
			else
			{
				pNow->pNext = pNewList;
				pNewList->pPrev = pNow;
				_dwNumOfList++;
				return true;
			}
		}
	}

	return false;
}


//--------------------------------------------------------------
//	Carmino 2016.12.21
//	맨 앞에 놈 빼준다. stANode 는 Clear()에서 일괄로 해줘야함.
//	어짜피 Open 을 빠져나가서 Close로 들어가기 때문에, 중간에 지우면 데이터 손실이 일어나버림.
//--------------------------------------------------------------
stANode* CLinkedHeap::Pop_ANode(void)
{
	if (_ptrHead == NULL)
	{
		return NULL;
	}
	stANode *pReturn = _ptrHead->value;
	stList* pDel = _ptrHead;

	_ptrHead = _ptrHead->pNext;
	if (_ptrHead != NULL)
		_ptrHead->pPrev = NULL;
	
	delete pDel;
	_dwNumOfList--;
	return pReturn;
}


//--------------------------------------------------------------
//	Carmino 2016.12.21
//	찾는다. stList => stANode 로 나중에 바꿔줄 것.
//	외부에서 변환, 변수 낭비하고 있음.
//--------------------------------------------------------------
stList* CLinkedHeap::Search_ANode(DWORD dwX, DWORD dwY)
{
	stList *pNow = _ptrHead;
	while (1)
	{
		if (pNow == NULL)
			return NULL;

		if (dwX == pNow->value->dwX && dwY == pNow->value->dwY)
			return pNow;
		else
			pNow = pNow->pNext;
	}

	return false;
}


//--------------------------------------------------------------
//	Carmino 2016.12.21
//	귀찮아서 넣었다가 뺐다가...
//--------------------------------------------------------------
void CLinkedHeap::Pop_Push_Sort(stANode *p)
{
	stList *pList = Search_ANode(p->dwX, p->dwY);
	
	if(pList->pPrev != NULL)
		pList->pPrev->pNext = pList->pNext;
	if(pList->pNext != NULL)
		pList->pNext->pPrev = pList->pPrev;
	if (pList == _ptrHead)
	{
		_ptrHead = pList->pNext;
	}
	delete pList;

	Push_ANode(p);
}



//--------------------------------------------------------------
//	Carmino 2016.12.21
//	스페이스는 초기화 모든 걸 초기화...
//--------------------------------------------------------------
void CLinkedHeap::Clear()
{
	stList *pDel, *pNow;
	pNow = _ptrHead;
	while (1)
	{
		if (pNow == NULL)
			break;

		pDel = pNow;
		pNow = pNow->pNext;
		delete pDel->value;
		delete pDel;
	}
	_dwNumOfList = 0;
	_ptrHead = NULL;
}
