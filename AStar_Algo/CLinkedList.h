#pragma once
#include "stdafx.h"


//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//	�����Ƽ� ���θ���...
//	A-Star �˰��� ��� ������ LinkedList �Դϴ�.
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
	//������ �ǵ� �̰� ���߿����鷡 ~
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
	//	����ó��
	//--------------------------------------------------------------
	if (pValue == NULL)
		return false;

	stList *pNewList = new stList;
	pNewList->pNext = NULL;
	pNewList->pPrev = NULL;
	pNewList->value = pValue;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	�ƹ��͵� ����? �׳� �ְ� ���� ^��^
	//--------------------------------------------------------------
	if (_ptrHead == NULL)
	{
		_ptrHead = pNewList;
		_dwNumOfList++;
		return true;
	}
	//	Loop �� �������� pNow;
	stList *pNow = _ptrHead;
	while (1)
	{
		// ���� pNow���� ������ pNow �տ� ���� �ٴ´�.
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
			//	Root ó�����ش�.
			//--------------------------------------------------------------
			if (pNow == _ptrHead)
			{
				_ptrHead = pNewList;
			}
			_dwNumOfList++;
			return true;
		}
		// �ƴϸ� pNow�� ������Ű��, �ٽ� �����Ѵ�. ���������� �Դٴ� ���� ���� ���� ū ���̴�.
		// ���� �� �ڿ� �׳� �ְ� ����.
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
//	�� �տ� �� ���ش�. stANode �� Clear()���� �ϰ��� �������.
//	��¥�� Open �� ���������� Close�� ���� ������, �߰��� ����� ������ �ս��� �Ͼ����.
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
//	ã�´�. stList => stANode �� ���߿� �ٲ��� ��.
//	�ܺο��� ��ȯ, ���� �����ϰ� ����.
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
//	�����Ƽ� �־��ٰ� ���ٰ�...
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
//	�����̽��� �ʱ�ȭ ��� �� �ʱ�ȭ...
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
