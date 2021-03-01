#include "stdafx.h"
#include "TileMap.h"
#include "CLinkedList.h"
#include <cstdio>
#include <cstdlib>
extern HDC g_hdc;
CLinkedHeap H_Open, H_Close;
static int iVisit = 1;
//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	Ŭ���� �ʱ�ȭ ���� �Լ�
//------------------------------------------------------------------
CTileMap::CTileMap(DWORD width, DWORD height, DWORD length)
{
	_dwWidth = width;
	_dwHeight = height;
	_dwLength = length;
	_pBackTracking = NULL;
	_JumpIsFirst = true;
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	2���� �迭 �ϼ�
	//--------------------------------------------------------------
	_pArrMap = new stTile*[_dwHeight];
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		_pArrMap[i] = new stTile[_dwWidth];
	}

	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	Ÿ�� ����ü�� �� ����
	//
	//	�̷��� ���� �ؾ��ϳ� �ڱ��� ���...
	//--------------------------------------------------------------
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		for (DWORD j = 0; j < _dwWidth; j++)
		{
			_pArrMap[i][j].enType = ROAD;
			_pArrMap[i][j].dwLength = length;
			_pArrMap[i][j].dwSpeed = 1;
			_pArrMap[i][j].bVisited = false;
			_pArrMap[i][j].bStart = false;
			_pArrMap[i][j].bGoal = false;
			_pArrMap[i][j].iF = 0;
			_pArrMap[i][j].bEnd = false;

			//------------------------------------------------------------------
			//  Carmino  16.12.26
			//	JumpPointSearch GDI�� ���� ������ �߰�...
			//------------------------------------------------------------------
			_pArrMap[i][j].bJumpVisited = 99;
			_pArrMap[i][j].bJumpOpenNode = false;
			_pArrMap[i][j].bJumpCloseNode = false;
		}
	}
}

//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//	���� ��...
//--------------------------------------------------------------
CTileMap::~CTileMap()
{
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		delete[] _pArrMap[i];
	}
	delete[] _pArrMap;
}

//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	���� �������̽� �κ�
//	Ÿ�Ϻ�ȯ
//	���, �������� ����
//	�׷��� ���
//------------------------------------------------------------------
void CTileMap::Clear()
{
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		for (DWORD j = 0; j < _dwWidth; j++)
		{
			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//
			//	�������� �߰��Ǵ��� ������...
			//--------------------------------------------------------------
			_pArrMap[i][j].enType = ROAD;
			//_pArrMap[i][j].dwLength = length;
			_pArrMap[i][j].dwSpeed = 1;
			_pArrMap[i][j].bVisited = FALSE;
			_pArrMap[i][j].bStart = FALSE;
			_pArrMap[i][j].bGoal = FALSE;
			_pArrMap[i][j].iF = 0;
			_pArrMap[i][j].bEnd = false;

			_pArrMap[i][j].bJumpVisited = 99;
			_pArrMap[i][j].bJumpOpenNode = false;
			_pArrMap[i][j].bJumpCloseNode = false;
		}
	}
	_pBackTracking = NULL;
	H_Open.Clear();
	H_Close.Clear();
	_JumpIsFirst = true;
}

//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//--------------------------------------------------------------
void CTileMap::ChangeTileType(DWORD width, DWORD height)
{
	if (width < 0 || height < 0 || width >= _dwWidth || height >= _dwHeight)
	{
		return;
	}

	switch (_pArrMap[height][width].enType)
	{
	case ROAD:
		_pArrMap[height][width].enType = WALL;
		break;
	case WALL:
		_pArrMap[height][width].enType = WATER;
		break;
	case WATER:
		_pArrMap[height][width].enType = ROAD;
		break;
	}
}

//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//--------------------------------------------------------------
void CTileMap::SettingGoal(DWORD width, DWORD height)
{
	if (width < 0 || height < 0 || width >= _dwWidth || height >= _dwHeight)
	{
		return;
	}
	if (_pArrMap[height][width].bGoal == TRUE)
	{
		_pArrMap[height][width].bStart = FALSE;
		_pArrMap[height][width].bGoal = FALSE;
		return;
	}
	if (_pArrMap[height][width].bStart == FALSE)
	{
		_pArrMap[height][width].bStart = TRUE;
		_pArrMap[height][width].bGoal = FALSE;
	}
	else
	{
		_pArrMap[height][width].bStart = FALSE;
		_pArrMap[height][width].bGoal = TRUE;
	}
}


//--------------------------------------------------------------
//	Carmino 2016.12.21
//	
//	GDI = ����
//	���� = �밡��
//	GDI = �밡��
//--------------------------------------------------------------
void CTileMap::DrawMapGDI(HDC hdc, bool isJump)
{
	RECT	crt;
	crt.top = 0;
	crt.bottom = 900;
	crt.left = 0;
	crt.right = 1600;

	HBITMAP hBit, hOldBit;
	HDC hMemDC = CreateCompatibleDC(hdc);
	hBit = CreateCompatibleBitmap(hdc, 1600, 900);
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);

	HBRUSH	hBgColor = CreateSolidBrush(0x000000);
	FillRect(hMemDC, &crt, hBgColor);
	
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	�� Ÿ���� Ÿ�Ժ� ���� ����...
	//--------------------------------------------------------------
	HBRUSH	hRoadBru, hWallBru, hWaterBru[3], hOldBru, hStartBru, hGoalBru, hVisitedBru;
	hRoadBru = CreateSolidBrush(RGB(255, 255, 255));
	hWallBru = CreateSolidBrush(RGB(0, 0, 0));
	static int WaterIdx = 0;
	hWaterBru[0] = CreateSolidBrush(0xCC0000);
	hWaterBru[1] = CreateSolidBrush(0xFF0000);
	hWaterBru[2] = CreateSolidBrush(0xFF3333);
	hStartBru = CreateSolidBrush(RGB(51, 255, 51));
	hGoalBru = CreateSolidBrush(RGB(255, 51, 51));
	hOldBru = (HBRUSH)SelectObject(hMemDC, hRoadBru);
	hVisitedBru = CreateSolidBrush(0xFFCCFF);
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	���� ����Ʈ ��ġ GDI�� ���� Bru �¾�
	//------------------------------------------------------------------
	HBRUSH hJumpVisited[7], hJumpOpen, hJumpClose;
	hJumpVisited[0] = CreateSolidBrush(0xcccccc);
	hJumpVisited[1] = CreateSolidBrush(0xFF9999);
	hJumpVisited[2] = CreateSolidBrush(0x99FF99);
	hJumpVisited[3] = CreateSolidBrush(0xFF9933);
	hJumpVisited[4] = CreateSolidBrush(0xcccccc);
	hJumpVisited[5] = CreateSolidBrush(0xcccccc);
	hJumpVisited[6] = CreateSolidBrush(0xcccccc);


	hJumpOpen = CreateSolidBrush(0xFF00FF);		//	��ȫ
	hJumpClose = CreateSolidBrush(0xFF0000);	//	�����



	HFONT hFont2, hFont, hOldFont;
	hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"���� ���");
	hFont2 = CreateFont(10, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"���� ���");
	hOldFont = (HFONT)SelectObject(hMemDC, hFont2);
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	��� Ÿ���� ���鼭 �޸�DC�� �׸���.
	//--------------------------------------------------------------
	SetBkMode(hMemDC, TRANSPARENT);
	DWORD dwYpos = 0;
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		DWORD dwXpos = 0;
		for (DWORD j = 0; j < _dwWidth; j++)
		{

			switch (_pArrMap[i][j].enType)
			{
			case WALL:
			{
				SelectObject(hMemDC, hWallBru);
			}
			break;
			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	����... �귯��... ���̴�...
			//--------------------------------------------------------------
			case WATER:
			{
				SelectObject(hMemDC, hWaterBru[WaterIdx % 3]);
				WaterIdx += 1;
				if (WaterIdx >= 1000)
				{
					WaterIdx = 0;
				}
			}
			break;
			}
			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	
			//--------------------------------------------------------------
			if (_pArrMap[i][j].bStart == TRUE)
			{
				SelectObject(hMemDC, hStartBru);
			}
			if (_pArrMap[i][j].bGoal == TRUE)
			{
				SelectObject(hMemDC, hGoalBru);
			}
			if (_pArrMap[i][j].bVisited == true)
			{
				if (_pArrMap[i][j].enType != WATER)
					SelectObject(hMemDC, hVisitedBru);
			}

			//------------------------------------------------------------------
			//  Carmino  16.12.26
			//	Jump Point Search �� ���� �ش� �������� ĥ�Ѵ�.
			//------------------------------------------------------------------
			if (isJump == true && _pArrMap[i][j].bStart == false && _pArrMap[i][j].bGoal == false)
			{

				if (_pArrMap[i][j].bJumpVisited != 99)
				{
					SelectObject(hMemDC, hJumpVisited[_pArrMap[i][j].bJumpVisited]);
				}
				if (_pArrMap[i][j].bJumpOpenNode)
				{
					SelectObject(hMemDC, hJumpOpen);
				}
				if (_pArrMap[i][j].bJumpCloseNode)
				{
					SelectObject(hMemDC, hJumpClose);
				}

			}

			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	
			//--------------------------------------------------------------
			Rectangle(hMemDC, dwXpos, dwYpos, dwXpos + _dwLength, dwYpos + _dwLength);

			//------------------------------------------------------------------
			//  Carmino  16.12.16
			//	bVisted �̸� ������ ���׶�̸� ��ھ�� ^��^	(�� ���Ҹ���;;)
			//------------------------------------------------------------------



			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	F�� Ȯ�ο�
			//--------------------------------------------------------------
			if (_pArrMap[i][j].iF != 0)
			{
				WCHAR wzF[32];
				_itow_s(_pArrMap[i][j].iF, wzF, 32, 10);
				TextOut(hMemDC, dwXpos, dwYpos, wzF, wcslen(wzF));
			}

			dwXpos += _dwLength;
			SelectObject(hMemDC, hRoadBru);
		}
		dwYpos += _dwLength;
	}
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	�귯�� ����
	//--------------------------------------------------------------
	SelectObject(hMemDC, hOldBru);
	DeleteObject(hVisitedBru);
	DeleteObject(hRoadBru);
	DeleteObject(hWallBru);
	DeleteObject(hWaterBru[0]);
	DeleteObject(hWaterBru[1]);
	DeleteObject(hWaterBru[2]);
	DeleteObject(hStartBru);
	DeleteObject(hGoalBru);
	DeleteObject(hBgColor);

	DeleteObject(hJumpOpen);
	DeleteObject(hJumpClose);
	DeleteObject(hJumpVisited[0]);
	DeleteObject(hJumpVisited[1]);
	DeleteObject(hJumpVisited[2]);
	DeleteObject(hJumpVisited[3]);
	DeleteObject(hJumpVisited[4]);
	DeleteObject(hJumpVisited[5]);
	DeleteObject(hJumpVisited[6]);
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	�ؽ�Ʈ �׸��� �κ�
	//--------------------------------------------------------------
	
	SelectObject(hMemDC, hFont);
	SetTextColor(hMemDC, 0xffffff);
	TextOut(hMemDC, 1180, 50, L"A-Star �˰��� �׽�Ʈ ��", wcslen(L"A-Star �˰��� �׽�Ʈ ��"));
	TextOut(hMemDC, 1180, 80, L"��Ŭ�� : Ÿ�� Ÿ�Ժ���", wcslen(L"��Ŭ�� : Ÿ�� Ÿ�Ժ���"));
	TextOut(hMemDC, 1180, 110, L"��Ŭ�� : Start, Goal ���� ����", wcslen(L"��Ŭ�� : Start, Goal ���� ����"));
	TextOut(hMemDC, 1180, 140, L"SHIFT : A* Search  /  TAB : Jumping Point Search", wcslen(L"SHIFT : A* Search  /  TAB : Jumping Point Search"));
	TextOut(hMemDC, 1180, 170, L"SPACE : �ʱ�ȭ", wcslen(L"SPACE : �ʱ�ȭ"));
	TextOut(hMemDC, 1180, 200, L"����, ���������� ������ �ϳ��� �������ּ���!",
		wcslen(L"����, ���������� ������ �ϳ��� �������ּ���!"));
	TextOut(hMemDC, 1180, 230, L"�� : �����, �� : ������, �� : �Ķ���, ������ : �ʷϻ�, ������ : ������"
	,wcslen(L"�� : �����, �� : ������, �� : �Ķ���, ������ : �ʷϻ�, ������ : ������"));
	
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	��Ʈ ����
	//--------------------------------------------------------------
	SelectObject(hMemDC, hOldFont);
	DeleteObject(hFont);


	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//
	//	���������� ���� ��������� ������ �ǵ��ư��鼭 ���� �׾��ش�.
	//--------------------------------------------------------------
	if (_pBackTracking != NULL)
	{
		HPEN hPen, hOldPen;
		hPen = CreatePen(PS_SOLID, 2, RGB(255,0,0));
		hOldPen = (HPEN)SelectObject(hMemDC, hPen);
		stANode *pNow = _pBackTracking->value;
		while (1)
		{
			MoveToEx(hMemDC, ((pNow->dwX * _dwLength) + (_dwLength / 2)), ((pNow->dwY * _dwLength) + (_dwLength / 2)), NULL);
			if (pNow->pParent != NULL)
			{
				LineTo(hMemDC, ((pNow->pParent->dwX * _dwLength) + (_dwLength / 2)), ((pNow->pParent->dwY * _dwLength) + (_dwLength / 2)));
				pNow = pNow->pParent;
				continue;
			}

			break;
		}

		SelectObject(hMemDC, hOldPen);
		DeleteObject(hPen);
	}
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	�޸�DC -> DC ��ºκ�
	//--------------------------------------------------------------
	BitBlt(hdc, 0, 0, 1600, 900, hMemDC, 0, 0, SRCCOPY);	// BitBlt ȣ���ϸ� InvalidRect�� �Դ°ǰ�?
	SelectObject(hMemDC, hOldBit);
	DeleteObject(hBit);
	DeleteDC(hMemDC);
}

//------------------------------------------------------------------
//  Carmino  16.12.26
//
//	Jump-Search �˰����� ���� GDI ��ο�
//------------------------------------------------------------------

//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	������� ��� �ʿ��� ã�� ���ڷ� �������ش�.
//------------------------------------------------------------------
void CTileMap::GetStartPos(DWORD *piX, DWORD *piY)
{
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		for (DWORD j = 0; j < _dwWidth; j++)
		{
			if (_pArrMap[i][j].bStart == true)
			{
				*piX = j;
				*piY = i;
				return;
			}
		}
	}
}

//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	�������� ��� �ʿ��� ã�� ���ڷ� �������ش�.
//------------------------------------------------------------------
void CTileMap::GetGoalPos(DWORD *piX, DWORD *piY)
{
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		for (DWORD j = 0; j < _dwWidth; j++)
		{
			if (_pArrMap[i][j].bGoal == true)
			{
				*piX = j;
				*piY = i;
				return;
			}
		}
	}
}


//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	������ �ڵ�� �ʹ� �����̶� ��� ������ ����.
//	A* �˰��� �߰��ϸ� �ȴ�.
//------------------------------------------------------------------

void CTileMap::FindPath(DWORD dwStartX, DWORD dwStartY, DWORD dwGoalX, DWORD dwGoalY)
{
	//------------------------------------------------------------------
	//  Carmino  16.12.21
	//	
	//	bool Push_ANode(stANode *p) = *p�� ������ ListNode�� �����Ͽ� �ּҰ� Heap�� �ִ´�. ���� true
	//	stANode* Pop_ANode(void) = ���� �� stANode�� �����͸�, ���� �� NULL�� ��´�.
	//	bool SearchANode(stANode *p) = *p�� dwX, dwY ���� ���� ����Ʈ ��尡 �ִ��� ã�´�. ������ true, ������ false;
	//------------------------------------------------------------------

	stANode *pStartNode = new stANode;
	pStartNode->dwX = dwStartX;
	pStartNode->dwY = dwStartY;
	pStartNode->H = 0;
	pStartNode->G = abs((int)dwGoalX - (int)dwStartX) + abs((int)dwGoalY - (int)dwStartY);
	pStartNode->F = pStartNode->H + pStartNode->G;
	pStartNode->pParent = NULL;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	Node�� �����ϴ� �Լ��� �ϳ� ��������
	//--------------------------------------------------------------
	H_Open.Push_ANode(pStartNode);

	while (1)
	{
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	Case 1. Open Heap ���� F���� ���� ���� ��带 ������ �´�.
		//	���������� �����Դٸ�, �ش� ���� Close Heap �� �ִ´�.
		//--------------------------------------------------------------
 		stANode *pNow = H_Open.Pop_ANode();
		if (pNow == NULL)
			break;
		
		H_Close.Push_ANode(pNow);
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	Case 2. pNow �� �������� 8�������� ������ ������Ų��.
		//	���� ������� 4�������� ����
		//--------------------------------------------------------------
		//�����¿�
		int iX, iY;
		iX = pNow->dwX;
		iY = pNow->dwY;
	
		// ��, �»�, ����
		PushToOpenList(pNow, iX - 1, iY, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX - 1, iY - 1, dwGoalX, dwGoalY, true);
		PushToOpenList(pNow, iX - 1, iY + 1, dwGoalX, dwGoalY, true);
		// ��, ���, ����
		PushToOpenList(pNow, iX + 1, iY, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX + 1, iY - 1, dwGoalX, dwGoalY, true);
		PushToOpenList(pNow, iX + 1, iY + 1, dwGoalX, dwGoalY, true);
		// �� �Ʒ� ���� �Ʒ� �� �Ʒ� ���� �Ʒ� �� �Ʒ�? �� �Ʒ��������������ؤǤ�
		PushToOpenList(pNow, iX, iY - 1, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX, iY + 1, dwGoalX, dwGoalY);
		
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	�ٽ� ���� �ʿ䰡 ���°�, ��¥�� �������� H_Close.Clear() ���� ������ ����
		//--------------------------------------------------------------
		DrawMapGDI(g_hdc);
		Sleep(10);
		_pBackTracking = H_Open.Search_ANode(dwGoalX, dwGoalY);
		if (_pBackTracking != NULL)
		{
			_pArrMap[dwGoalY][dwGoalX].bVisited = false;
			DrawMapGDI(g_hdc);
			break;
		}
	}
}
//--------------------------------------------------------------
//	Carmino 2016.12.21
//	������
//--------------------------------------------------------------
void CTileMap::PushToOpenList(stANode *pNow, DWORD dwX, DWORD dwY, DWORD dwGoalX, DWORD dwGoalY, bool bIsDiag)
{
	stList *pOpen, *pClose;
	pOpen = H_Open.Search_ANode(dwX, dwY);
	pClose = H_Close.Search_ANode(dwX, dwY);
	if ( pOpen== NULL)
	{
		if (pClose == NULL)
		{
			stANode *pTemp = CreateANode(dwX, dwY, dwGoalX, dwGoalY, pNow, bIsDiag);
			if (pTemp != NULL)
			{
				H_Open.Push_ANode(pTemp);
			}
		}
		else
		{
			if (_pArrMap[pClose->value->dwY][pClose->value->dwX].enType == WATER)
			{
				if (pClose->value->H > pNow->H + 14)
				{
					pClose->value->H = pNow->H + 14;
					pClose->value->pParent = pNow;
					//pOpen->value->F = pOpen->value->G + pOpen->value->H;
				}
			}
			else
			{
				if (pClose->value->H > pNow->H + 10)
				{
					pClose->value->H = pNow->H + 10;
					pClose->value->pParent = pNow;
					//pOpen->value->F = pOpen->value->G + pOpen->value->H;
				}
			}
		}
	}
	else
	{
		if (_pArrMap[pOpen->value->dwY][pOpen->value->dwX].enType == WATER)
		{
			if (pOpen->value->H > pNow->H + 14)
			{
				pOpen->value->H = pNow->H + 14;
				pOpen->value->pParent = pNow;
				pOpen->value->F = pOpen->value->G + pOpen->value->H;
				H_Open.Pop_Push_Sort(pOpen->value);

			}
		}
		else
		{
			if (pOpen->value->H > pNow->H + 10)
			{
				pOpen->value->H = pNow->H + 10;
				pOpen->value->pParent = pNow;
				pOpen->value->F = pOpen->value->G + pOpen->value->H;
				H_Open.Pop_Push_Sort(pOpen->value);
			}
		}
	}
}
//--------------------------------------------------------------
//	Carmino 2016.12.21
//	��� �����ؼ� �����ξ���.
//--------------------------------------------------------------
stANode* CTileMap::CreateANode(DWORD dwX, DWORD dwY, DWORD dwGoalX, DWORD dwGoalY, stANode* pParent, bool bIsDiag)
{
	
	// Ŭ���� ó�� �� WALL �̸� ������ ���� �� �̴�.
	if (dwX >= 0 && dwX < _dwWidth && dwY >= 0 && dwY < _dwHeight && _pArrMap[dwY][dwX].enType != WALL)
	{
		// ���� �� ��ȯ�� ���
		stANode *pNode = new stANode;
		pNode->dwX = dwX;
		pNode->dwY = dwY;
		pNode->G = (abs((int)dwGoalX - (int)dwX) + abs((int)dwGoalY - (int)dwY))*10;
		
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	
		//--------------------------------------------------------------
		if (bIsDiag == true || _pArrMap[dwY][dwX].enType == WATER)
		{
			pNode->H = pParent->H + 14;
		}
		else
		{
			pNode->H = (pParent->H) + 10;
		}
		pNode->F = pNode->G + (pNode->H);
		pNode->pParent = pParent;
		_pArrMap[dwY][dwX].bVisited = true;
		_pArrMap[dwY][dwX].iF = pNode->F;
		return pNode;
	}
	
	return NULL; // ���� �ٱ����� NULL �� üũ�Ѵ�.
}
//--------------------------------------------------------------
//	Carmino 2016.12.26
//	JumpSearch Node �����
//--------------------------------------------------------------
stANode* CTileMap::CreateJumpNode(DWORD dwStartX, DWORD dwStartY, DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, stANode* pParent)
{

	stList *pList = H_Open.Search_ANode(dwX, dwY);
	stList *pList2 = H_Close.Search_ANode(dwX, dwY);

	if (pList != NULL)
	{
		DWORD G = (abs((int)dwGoalX - (int)dwX) + abs((int)dwGoalY - (int)dwY)) * 10;
		if (G < pList->value->G)
		{
			pList->value->G = G;
			pList->value->F = pList->value->G + pList->value->H;
			pList->value->pParent = pParent;
			H_Open.Pop_Push_Sort(pList->value);
			return NULL;
		}
	}

	if (pList2 != NULL)
	{
		DWORD G = (abs((int)dwGoalX - (int)dwX) + abs((int)dwGoalY - (int)dwY)) * 10;
		if (G < pList2->value->G)
		{
			pList2->value->G = G;
			pList2->value->pParent = pParent;
			return NULL;
		}
	}

	if (pList == NULL && pList2 == NULL)
	{
		stANode *pNode = new stANode;
		pNode->dwX = dwX;
		pNode->dwY = dwY;
		pNode->G = (abs((int)dwGoalX - (int)dwX) + abs((int)dwGoalY - (int)dwY)) * 10;

		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	
		//--------------------------------------------------------------
		pNode->H = (abs((int)dwStartX - (int)dwX) + abs((int)dwStartY - (int)dwY)) * 10;

		pNode->F = pNode->G + (pNode->H);
		pNode->pParent = pParent;

		_pArrMap[dwY][dwX].bJumpOpenNode = true;
		return pNode;
	}
	
	return NULL;
}



//------------------------------------------------------------------
//  Carmino  16.12.26
//	Jump-Search �˰��� �Լ�
//------------------------------------------------------------------
bool CTileMap::FindJumpingPath(DWORD dwStartX, DWORD dwStartY, const DWORD dwGoalX, const DWORD dwGoalY, bool isFirst)
{
	if (_JumpIsFirst == true)
	{
		stANode *pStartNode = new stANode;

		pStartNode->dwX = dwStartX;
		pStartNode->dwY = dwStartY;
		pStartNode->G = abs((int)dwGoalX - (int)dwStartX) + abs((int)dwGoalY - (int)dwStartY);
		pStartNode->H = 0;
		pStartNode->F = pStartNode->G + pStartNode->H;
		pStartNode->pParent = NULL;

		H_Open.Push_ANode(pStartNode);
		_JumpIsFirst = false;
	}
	
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	��������� �θ� �˰�������, �θ�κ��� ���ϴ� �ɷ� ����
	//	�� ĥ�ϴ� �Ŵ� �ڿ� �˾Ƽ� ó��
	//	stANode �� ����
	//------------------------------------------------------------------
	//while (1)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	H_Open �� �������� ��尡 ���������, ����
		//------------------------------------------------------------------
		_pBackTracking = H_Open.Search_ANode(dwGoalX, dwGoalY);
		if (_pBackTracking != NULL)
		{
			DrawMapGDI(g_hdc, true);
			return false;
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	H_Open ���� F �� ���� ���� ���� ������, ������ �� ���� = ����
		//------------------------------------------------------------------
		stANode *pNowNode = H_Open.Pop_ANode();
		if (pNowNode == NULL)
		{
			return false;
		}
		
		H_Close.Push_ANode(pNowNode);
		_pArrMap[pNowNode->dwY][pNowNode->dwX].bJumpCloseNode = true;

		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�θ� ������, ���۳�� �̹Ƿ� �����⿡ ���� ���� ����.
		//------------------------------------------------------------------
		if (pNowNode->pParent == NULL)
		{
			//------------------------------------------------------------------
			//  Carmino  16.12.26
			//	JumpProcess ���� ���ư� ������ �����ؼ� ȣ��������Ѵ�.
			//	
			//	Jump �� ���� ������ Ȯ���� ��
			//	���ο��� Jump�� True ��� ��带 �����ؼ� Open ����Ʈ�� �ְ�
			//	�������� ���̴�.
			//------------------------------------------------------------------
			// ��, �»�, ����
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_LL);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
			// ��, ���, ����
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_RR);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
			// �� , �Ʒ�
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_UU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�θ� ������, �θ�� ���� ��������� üũ�ϰ� �� ���⿡ ���� ���縦 ����.
		//------------------------------------------------------------------
		else
		{
			switch (CalcDIR_FromParent(pNowNode->dwX, pNowNode->dwY, pNowNode->pParent->dwX, pNowNode->pParent->dwY))
			{
				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_LL:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_LL);
				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//	0,0,true �̸� ���� ������ ����... true �� ��...
				//------------------------------------------------------------------
				if(CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true) == true)
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true) == true)
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);

				break;
				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_LU:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_LL);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_UU);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
				

				//  P
				//	  P W P
				//	 W P
				//   P   P
				//        P
				//	������ �����ִ�, LD ����, ������ �����ִ�, RU ����
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				}
				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
				}
				break;

				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_LD:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_LL);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				
				//	������ �����ִ�, LU ����, �Ʒ��� �����ִ�, RD ����
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
				}
				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
				}
				break;


				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_RR:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_RR);
				

				// ���̴� �κ� üũ...
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true) == true)	//	��Ʒ�
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);

				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true) == true)	//	������
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);

				break;
				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_RU:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_RR);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_UU);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
				
				//	�������� RD ������ ������ LU
				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
				}
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
				}
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
				}
				
				break;


				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_RD:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_RR);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);

				//�Ʒ��� �����ִ� = LD , �������� �����ִ� = RU
				if (CheckCorner(pNowNode->dwX , pNowNode->dwY + 1, 0, 0, true) == true)
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				}

				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true) == true)
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
				}

				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
				}

				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0 ,true))
				{
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
				}

				break;


				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_UU:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_UU);
				
				// ���̴� �κ� üũ...
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true) == true)	//	��Ʒ�
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);

				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true) == true)	//	������
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);

				break;


				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_DD:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
				
				// ���̴� �κ� üũ...
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true) == true)	//	��Ʒ�
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);

				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true) == true)	//	������
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
				
				break;
			}
		}
	}

	
	return true;
}
//------------------------------------------------------------------
//  Carmino  16.12.26
//	
//	�� ���� �бⰡ �ʹ� ���Ƽ� �ϴ� �̷��� �����ô�.
//------------------------------------------------------------------
void CTileMap::JumpProcess(stANode *pNow, const DWORD dwStartX, const DWORD dwStartY, DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, int dfDir)
{
	DWORD pX = 0, pY = 0;
	if (Jump(dwX, dwY, dwGoalX, dwGoalY, &pX, &pY, dfDir) == true)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	Open Ȥ�� Close�� ����ִ� �� �˻��ϴ� ���� �߰���.
		//------------------------------------------------------------------
		stANode *pNode = CreateJumpNode(dwStartX, dwStartY, pX, pY, dwGoalX, dwGoalY, pNow);
		
		if(pNode != NULL)
			H_Open.Push_ANode(pNode);
	}
	DrawMapGDI(g_hdc, true);
	iVisit = (iVisit + 1) % 4;
	//Sleep(50);
}
//------------------------------------------------------------------
//  Carmino  16.12.26
//	������ ��忡��, ������ ���� �ִ� �� ã�ƺ� �Լ�.
//	��ͷ� ����.
//------------------------------------------------------------------
bool CTileMap::Jump(DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, DWORD *pX, DWORD *pY, int dfDir)
{
	
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	Ŭ���� ó��
	//------------------------------------------------------------------
	if (dwX < 0 || dwX >= _dwWidth || dwY < 0 || dwY >= _dwHeight || _pArrMap[dwY][dwX].enType == WALL)
		return false;
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	������ ó��
	//------------------------------------------------------------------
	if (dwX == dwGoalX && dwY == dwGoalY)
	{
		*pX = dwGoalX;
		*pY = dwGoalY;
		return true;
	}
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	���⺰�� ���� ���� ó��
	//------------------------------------------------------------------
	switch (dfDir)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	���� ���� ���.
		//	�� �������� �����ְ�, ������������ �շ��ִ� ���
		//------------------------------------------------------------------
	case dfDIR_UU:
		{
			_pArrMap[dwY][dwX].bJumpVisited = iVisit;
			if (CheckCorner(dwX + 1, dwY, dwX + 1, dwY - 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			if (CheckCorner(dwX - 1, dwY, dwX - 1, dwY - 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			//Sleep(20);
			//DrawMapGDI(g_hdc, true);
			return Jump(dwX, dwY - 1, dwGoalX, dwGoalY, pX, pY, dfDIR_UU);
		}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�Ʒ��� ���� ���
		//	�� �������� �����ְ�, ������ �Ʒ��� �շ��ִ� ���
		//------------------------------------------------------------------
	case dfDIR_DD:
		{
			_pArrMap[dwY][dwX].bJumpVisited = iVisit;
			if (CheckCorner(dwX + 1, dwY, dwX + 1, dwY + 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			if (CheckCorner(dwX - 1, dwY, dwX - 1, dwY + 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			//Sleep(20);
			//DrawMapGDI(g_hdc, true);
			return Jump(dwX, dwY + 1, dwGoalX, dwGoalY, pX, pY, dfDIR_DD);
		}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�������� ���� ���
		//	�� ������ �����ְ�, �� ���� ������ �շ��ִ� ���
		//------------------------------------------------------------------
	case dfDIR_LL:
		{
			_pArrMap[dwY][dwX].bJumpVisited = iVisit;
			if (CheckCorner(dwX, dwY + 1, dwX - 1, dwY + 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			if (CheckCorner(dwX, dwY - 1, dwX - 1, dwY - 1))
			{
				*pX = dwX;
				*pY = dwY;
				return true;
			}
			//Sleep(20);
			//DrawMapGDI(g_hdc, true);
			return Jump(dwX - 1, dwY, dwGoalX, dwGoalY, pX, pY, dfDIR_LL);
		}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�� ���� ���� ���
		//	������ �����ְ�, ���� �Ʒ����� �����ִ� ���
		//	������ �����ְ�, �Ʒ� �������� �����ִ� ���
		//------------------------------------------------------------------
	case dfDIR_LU:
	{
		_pArrMap[dwY][dwX].bJumpVisited = iVisit;
		if (CheckCorner(dwX - 1, dwY , dwX - 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX , dwY - 1, dwX + 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY + 1, dwX - 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX + 1, dwY, dwX + 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	����, �Ʒ����� ���ڷ� �˻��س�����.
		//------------------------------------------------------------------
		DWORD ppX, ppY;	//	Dummy Data
		if (Jump(dwX - 1, dwY, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_LL) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (Jump(dwX, dwY - 1, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_UU) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		//Sleep(20);
		//DrawMapGDI(g_hdc, true);
		return Jump(dwX - 1, dwY - 1, dwGoalX, dwGoalY, pX, pY, dfDIR_LU);
	}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�� �Ʒ� �� ���� ���
		//	
		//------------------------------------------------------------------
	case dfDIR_LD:
	{
		_pArrMap[dwY][dwX].bJumpVisited = iVisit;
		if (CheckCorner(dwX - 1, dwY, dwX - 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY + 1, dwX + 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY - 1, dwX - 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX + 1, dwY, dwX + 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	����, �Ʒ����� ���ڷ� �˻��س�����.
		//------------------------------------------------------------------
		DWORD ppX, ppY;	//	Dummy Data
		if (Jump(dwX - 1, dwY, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_LL) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (Jump(dwX, dwY + 1, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_DD) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
	//	Sleep(20);
	//	DrawMapGDI(g_hdc, true);
		return Jump(dwX - 1, dwY + 1, dwGoalX, dwGoalY, pX, pY, dfDIR_LD);
	}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	���������� ���� ���
		//	�� ������ �����ְ� , �� ���� �������� �շ��ִ� ���
		//------------------------------------------------------------------
	case dfDIR_RR:
	{
		_pArrMap[dwY][dwX].bJumpVisited = iVisit;
		if (CheckCorner(dwX, dwY - 1, dwX + 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY + 1, dwX + 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
	//	Sleep(20);
	//	DrawMapGDI(g_hdc, true);
		return Jump(dwX + 1, dwY, dwGoalX, dwGoalY, pX, pY, dfDIR_RR);
	}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�� ���� ���� ���
		//	�������� �����ְ� �� �Ʒ��� �ո�
		//	������ �����ְ� �� ������ �ո�
		//------------------------------------------------------------------
	case dfDIR_RU:
	{
		_pArrMap[dwY][dwX].bJumpVisited = iVisit;
		if (CheckCorner(dwX+1, dwY, dwX + 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY - 1, dwX - 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY + 1, dwX + 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX - 1, dwY, dwX - 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	������, ������ �˻��Ѵ�.
		//------------------------------------------------------------------
		DWORD ppX, ppY;	//	Dummy Data
		if (Jump(dwX + 1, dwY, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_RR) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (Jump(dwX, dwY - 1, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_UU) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}

	//	Sleep(20);
	//	DrawMapGDI(g_hdc, true);
		return Jump(dwX + 1, dwY - 1, dwGoalX, dwGoalY, pX, pY, dfDIR_RU);
	}
		break;
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	�� �Ʒ��� ���� ���
		//	�������� �����ְ� �� ���� �ո�
		//	�Ʒ��� �����ְ� �� ������ �ո�
		//------------------------------------------------------------------
	case dfDIR_RD:
	{
		_pArrMap[dwY][dwX].bJumpVisited = iVisit;
		if (CheckCorner(dwX + 1, dwY, dwX + 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY + 1, dwX - 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX - 1, dwY, dwX - 1, dwY + 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (CheckCorner(dwX, dwY - 1, dwX + 1, dwY - 1))
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}

		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	����, �Ʒ����� ���ڷ� �˻��س�����.
		//------------------------------------------------------------------
		DWORD ppX, ppY;	//	Dummy Data
		if (Jump(dwX + 1, dwY, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_RR) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}
		if (Jump(dwX, dwY + 1, dwGoalX, dwGoalY, &ppX, &ppY, dfDIR_DD) == true)
		{
			*pX = dwX;
			*pY = dwY;
			return true;
		}

	//	Sleep(20);
	//	DrawMapGDI(g_hdc, true);
		return Jump(dwX + 1, dwY + 1, dwGoalX, dwGoalY, pX, pY, dfDIR_RD);
	}
		break;
	}

	return false;
}
//------------------------------------------------------------------
//  Carmino  16.12.26
//	�ش� ��ġ���� ������ ���� ������ üũ�ϴ� �Լ�
//	
//	������ ���̴�  = true
//	�õ� ����  = false
//------------------------------------------------------------------
bool CTileMap::CheckCorner(DWORD dwWallX, DWORD dwWallY, DWORD dwRoadX, DWORD dwRoadY, bool isWallCheck)
{
	if (dwWallX < 0 || dwWallX >= _dwWidth || dwWallY < 0 || dwWallY >= _dwHeight || _pArrMap[dwWallY][dwWallX].enType != WALL)
		return false;
	if (isWallCheck == false)
	{
		if (dwRoadX < 0 || dwRoadX >= _dwWidth || dwRoadY < 0 || dwRoadY >= _dwHeight || _pArrMap[dwRoadY][dwRoadX].enType == WALL)
			return false;
	}
	return true;
}
//------------------------------------------------------------------
//  Carmino  16.12.26
//	��� ���� ���� ���������� �������� �ʰ� �־�, �θ��� ��ǥ�� ���������� �����س���.
//------------------------------------------------------------------
DWORD CTileMap::CalcDIR_FromParent(DWORD dwX, DWORD dwY, DWORD parentX, DWORD parentY)
{
	int DIR_X = (int)parentX - (int)dwX;
	int DIR_Y = (int)parentY - (int)dwY;

	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	X��ǥ�� ��ȭ�� ���� = �� �Ǵ� �Ʒ��� �̵��ߴ�.
	//------------------------------------------------------------------
	if (DIR_X == 0)
	{
		if (DIR_Y > 0)
		{
			return dfDIR_UU;
		}
		else
			return dfDIR_DD;
	}
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	�θ�� ���� ���� �����ʿ� �ִ�.
	//------------------------------------------------------------------
	else if (DIR_X < 0)
	{
		if (DIR_Y == 0)
			return dfDIR_RR;
		else if (DIR_Y > 0)
			return dfDIR_RU;
		else if (DIR_Y < 0)
			return dfDIR_RD;
	}
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	�θ�� ���� ���� ���ʿ� �ִ�.
	//------------------------------------------------------------------
	else if (DIR_X > 0)
	{
		if (DIR_Y == 0)
			return dfDIR_LL;
		else if (DIR_Y > 0)
			return dfDIR_LU;
		else if (DIR_Y < 0)
			return dfDIR_LD;
	}

	return 0;
}