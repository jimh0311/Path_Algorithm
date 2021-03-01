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
//	클래스 초기화 관련 함수
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
	//	2차원 배열 완성
	//--------------------------------------------------------------
	_pArrMap = new stTile*[_dwHeight];
	for (DWORD i = 0; i < _dwHeight; i++)
	{
		_pArrMap[i] = new stTile[_dwWidth];
	}

	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	타일 구조체의 값 셋팅
	//
	//	이렇게 까지 해야하나 자괴감 들어...
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
			//	JumpPointSearch GDI를 위한 변수들 추가...
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
//	삭제 중...
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
//	유저 인터페이스 부분
//	타일변환
//	출발, 도착지점 셋팅
//	그래픽 출력
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
			//	여러가지 추가되느라 더럽다...
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
//	GDI = 예술
//	예술 = 노가다
//	GDI = 노가다
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
	//	각 타일의 타입별 색상 지정...
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
	//	점프 포인트 서치 GDI를 위한 Bru 셋업
	//------------------------------------------------------------------
	HBRUSH hJumpVisited[7], hJumpOpen, hJumpClose;
	hJumpVisited[0] = CreateSolidBrush(0xcccccc);
	hJumpVisited[1] = CreateSolidBrush(0xFF9999);
	hJumpVisited[2] = CreateSolidBrush(0x99FF99);
	hJumpVisited[3] = CreateSolidBrush(0xFF9933);
	hJumpVisited[4] = CreateSolidBrush(0xcccccc);
	hJumpVisited[5] = CreateSolidBrush(0xcccccc);
	hJumpVisited[6] = CreateSolidBrush(0xcccccc);


	hJumpOpen = CreateSolidBrush(0xFF00FF);		//	분홍
	hJumpClose = CreateSolidBrush(0xFF0000);	//	보라색



	HFONT hFont2, hFont, hOldFont;
	hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"맑은 고딕");
	hFont2 = CreateFont(10, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"맑은 고딕");
	hOldFont = (HFONT)SelectObject(hMemDC, hFont2);
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	모든 타일을 돌면서 메모리DC에 그린다.
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
			//	물은... 흘러야... 물이다...
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
			//	Jump Point Search 일 때만 해당 색상으로 칠한다.
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
			//	bVisted 이면 검은색 동그라미를 찍겠어요 ^ㅇ^	(뭔 개소리야;;)
			//------------------------------------------------------------------



			//--------------------------------------------------------------
			//	Carmino 2016.12.21
			//	F값 확인용
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
	//	브러쉬 삭제
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
	//	텍스트 그리는 부분
	//--------------------------------------------------------------
	
	SelectObject(hMemDC, hFont);
	SetTextColor(hMemDC, 0xffffff);
	TextOut(hMemDC, 1180, 50, L"A-Star 알고리즘 테스트 툴", wcslen(L"A-Star 알고리즘 테스트 툴"));
	TextOut(hMemDC, 1180, 80, L"왼클릭 : 타일 타입변경", wcslen(L"왼클릭 : 타일 타입변경"));
	TextOut(hMemDC, 1180, 110, L"우클릭 : Start, Goal 지점 셋팅", wcslen(L"우클릭 : Start, Goal 지점 셋팅"));
	TextOut(hMemDC, 1180, 140, L"SHIFT : A* Search  /  TAB : Jumping Point Search", wcslen(L"SHIFT : A* Search  /  TAB : Jumping Point Search"));
	TextOut(hMemDC, 1180, 170, L"SPACE : 초기화", wcslen(L"SPACE : 초기화"));
	TextOut(hMemDC, 1180, 200, L"시작, 도착지점은 무조건 하나만 지정해주세요!",
		wcslen(L"시작, 도착지점은 무조건 하나만 지정해주세요!"));
	TextOut(hMemDC, 1180, 230, L"길 : 노란색, 벽 : 검정색, 물 : 파란색, 시작점 : 초록색, 도착점 : 빨간색"
	,wcslen(L"길 : 노란색, 벽 : 검정색, 물 : 파란색, 시작점 : 초록색, 도착점 : 빨간색"));
	
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	폰트 삭제
	//--------------------------------------------------------------
	SelectObject(hMemDC, hOldFont);
	DeleteObject(hFont);


	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//
	//	도착점으로 부터 출발점까지 역으로 되돌아가면서 선을 그어준다.
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
	//	메모리DC -> DC 출력부분
	//--------------------------------------------------------------
	BitBlt(hdc, 0, 0, 1600, 900, hMemDC, 0, 0, SRCCOPY);	// BitBlt 호출하면 InvalidRect가 먹는건가?
	SelectObject(hMemDC, hOldBit);
	DeleteObject(hBit);
	DeleteDC(hMemDC);
}

//------------------------------------------------------------------
//  Carmino  16.12.26
//
//	Jump-Search 알고리즘을 위한 GDI 드로우
//------------------------------------------------------------------

//------------------------------------------------------------------
//  Carmino  16.12.19
//	
//	출발점을 행렬 맵에서 찾아 인자로 내보내준다.
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
//	도착점을 행렬 맵에서 찾아 인자로 내보내준다.
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
//	그전에 코드들 너무 병맛이라 모두 삭제한 버전.
//	A* 알고리즘 추가하면 된다.
//------------------------------------------------------------------

void CTileMap::FindPath(DWORD dwStartX, DWORD dwStartY, DWORD dwGoalX, DWORD dwGoalY)
{
	//------------------------------------------------------------------
	//  Carmino  16.12.21
	//	
	//	bool Push_ANode(stANode *p) = *p를 가지는 ListNode를 생성하여 최소값 Heap로 넣는다. 성공 true
	//	stANode* Pop_ANode(void) = 성공 시 stANode의 포인터를, 실패 시 NULL을 뱉는다.
	//	bool SearchANode(stANode *p) = *p의 dwX, dwY 값을 가진 리스트 노드가 있는지 찾는다. 있으면 true, 없으면 false;
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
	//	Node를 생성하는 함수를 하나 만들어두자
	//--------------------------------------------------------------
	H_Open.Push_ANode(pStartNode);

	while (1)
	{
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	Case 1. Open Heap 에서 F값이 가장 작은 노드를 가지고 온다.
		//	성공적으로 꺼내왔다면, 해당 값을 Close Heap 에 넣는다.
		//--------------------------------------------------------------
 		stANode *pNow = H_Open.Pop_ANode();
		if (pNow == NULL)
			break;
		
		H_Close.Push_ANode(pNow);
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	Case 2. pNow 를 기준으로 8방향으로 노드들을 생성시킨다.
		//	먼저 약식으로 4방향으로 구현
		//--------------------------------------------------------------
		//상하좌우
		int iX, iY;
		iX = pNow->dwX;
		iY = pNow->dwY;
	
		// 좌, 좌상, 좌하
		PushToOpenList(pNow, iX - 1, iY, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX - 1, iY - 1, dwGoalX, dwGoalY, true);
		PushToOpenList(pNow, iX - 1, iY + 1, dwGoalX, dwGoalY, true);
		// 우, 우상, 우하
		PushToOpenList(pNow, iX + 1, iY, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX + 1, iY - 1, dwGoalX, dwGoalY, true);
		PushToOpenList(pNow, iX + 1, iY + 1, dwGoalX, dwGoalY, true);
		// 위 아래 위위 아래 위 아래 위위 아래 위 아래? 위 아래래래래래래래해ㅗㅋ
		PushToOpenList(pNow, iX, iY - 1, dwGoalX, dwGoalY);
		PushToOpenList(pNow, iX, iY + 1, dwGoalX, dwGoalY);
		
		//--------------------------------------------------------------
		//	Carmino 2016.12.21
		//	다시 지울 필요가 없는게, 어짜피 마지막에 H_Close.Clear() 에서 삭제될 예정
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
//	ㅋㅋㅋ
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
//	라고 생각해서 만들어두었다.
//--------------------------------------------------------------
stANode* CTileMap::CreateANode(DWORD dwX, DWORD dwY, DWORD dwGoalX, DWORD dwGoalY, stANode* pParent, bool bIsDiag)
{
	
	// 클리핑 처리 및 WALL 이면 생성을 안할 것 이다.
	if (dwX >= 0 && dwX < _dwWidth && dwY >= 0 && dwY < _dwHeight && _pArrMap[dwY][dwX].enType != WALL)
	{
		// 생성 후 반환할 노드
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
	
	return NULL; // 실패 바깥에서 NULL 을 체크한다.
}
//--------------------------------------------------------------
//	Carmino 2016.12.26
//	JumpSearch Node 만들기
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
//	Jump-Search 알고리즘 함수
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
	//	진행방향은 부모를 알고있으니, 부모로부터 구하는 걸로 진행
	//	색 칠하는 거는 뒤에 알아서 처리
	//	stANode 로 진행
	//------------------------------------------------------------------
	//while (1)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	H_Open 에 도착점의 노드가 들어있으면, 종료
		//------------------------------------------------------------------
		_pBackTracking = H_Open.Search_ANode(dwGoalX, dwGoalY);
		if (_pBackTracking != NULL)
		{
			DrawMapGDI(g_hdc, true);
			return false;
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	H_Open 에서 F 가 가장 작은 값을 꺼내고, 없으면 길 없음 = 종료
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
		//	부모가 없으면, 시작노드 이므로 전방향에 대한 조사 들어간다.
		//------------------------------------------------------------------
		if (pNowNode->pParent == NULL)
		{
			//------------------------------------------------------------------
			//  Carmino  16.12.26
			//	JumpProcess 에는 나아갈 방향을 수정해서 호출해줘야한다.
			//	
			//	Jump 를 통해 노드들을 확인한 후
			//	내부에서 Jump가 True 라면 노드를 생성해서 Open 리스트에 넣고
			//	빠져나올 것이다.
			//------------------------------------------------------------------
			// 좌, 좌상, 좌하
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_LL);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);
			// 우, 우상, 우하
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY, dwGoalX, dwGoalY, dfDIR_RR);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);
			// 위 , 아래
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_UU);
			JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
		}
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	부모가 있으면, 부모와 나의 진행방향을 체크하고 그 방향에 대한 조사를 들어간다.
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
				//	0,0,true 이면 벽의 유무만 뱉어내줌... true 면 벽...
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
				//	왼쪽이 막혀있다, LD 진행, 위에가 막혀있다, RU 진행
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
				
				//	왼쪽이 막혀있다, LU 진행, 아래가 막혀있다, RD 진행
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
				

				// 꺽이는 부분 체크...
				if (CheckCorner(pNowNode->dwX, pNowNode->dwY + 1, 0, 0, true) == true)	//	우아래
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_RD);

				if (CheckCorner(pNowNode->dwX, pNowNode->dwY - 1, 0, 0, true) == true)	//	우위에
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
				
				//	오른쪽이 RD 위에가 막혔다 LU
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

				//아래가 막혀있다 = LD , 오른쪽이 막혀있다 = RU
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
				
				// 꺽이는 부분 체크...
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true) == true)	//	우아래
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_LU);

				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true) == true)	//	우위에
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX + 1, pNowNode->dwY - 1, dwGoalX, dwGoalY, dfDIR_RU);

				break;


				//------------------------------------------------------------------
				//  Carmino  16.12.26
				//
				//------------------------------------------------------------------
			case dfDIR_DD:
				JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_DD);
				
				// 꺽이는 부분 체크...
				if (CheckCorner(pNowNode->dwX - 1, pNowNode->dwY, 0, 0, true) == true)	//	우아래
					JumpProcess(pNowNode, dwStartX, dwStartY, pNowNode->dwX - 1, pNowNode->dwY + 1, dwGoalX, dwGoalY, dfDIR_LD);

				if (CheckCorner(pNowNode->dwX + 1, pNowNode->dwY, 0, 0, true) == true)	//	우위에
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
//	아 뭔가 분기가 너무 많아서 일단 이렇게 가봅시다.
//------------------------------------------------------------------
void CTileMap::JumpProcess(stANode *pNow, const DWORD dwStartX, const DWORD dwStartY, DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, int dfDir)
{
	DWORD pX = 0, pY = 0;
	if (Jump(dwX, dwY, dwGoalX, dwGoalY, &pX, &pY, dfDir) == true)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	Open 혹은 Close에 들어있는 지 검사하는 로직 추가함.
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
//	기준점 노드에서, 꺽어진 곳이 있는 지 찾아볼 함수.
//	재귀로 들어간다.
//------------------------------------------------------------------
bool CTileMap::Jump(DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, DWORD *pX, DWORD *pY, int dfDir)
{
	
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	클리핑 처리
	//------------------------------------------------------------------
	if (dwX < 0 || dwX >= _dwWidth || dwY < 0 || dwY >= _dwHeight || _pArrMap[dwY][dwX].enType == WALL)
		return false;
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	도착점 처리
	//------------------------------------------------------------------
	if (dwX == dwGoalX && dwY == dwGoalY)
	{
		*pX = dwGoalX;
		*pY = dwGoalY;
		return true;
	}
	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	방향별로 정상 로직 처리
	//------------------------------------------------------------------
	switch (dfDir)
	{
		//------------------------------------------------------------------
		//  Carmino  16.12.26
		//	위로 가는 경우.
		//	내 오른쪽이 막혀있고, 오른쪽위에가 뚫려있는 경우
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
		//	아래로 가는 경우
		//	내 오른쪽이 막혀있고, 오른쪽 아래가 뚫려있는 경우
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
		//	왼쪽으로 가는 경우
		//	내 위에가 막혀있고, 내 위에 왼쪽이 뚫려있는 경우
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
		//	좌 위로 가는 경우
		//	왼쪽이 막혀있고, 왼쪽 아래에가 열려있는 경우
		//	위에가 막혀있고, 아래 오른쪽이 열려있는 경우
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
		//	왼쪽, 아래쪽을 일자로 검사해나간다.
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
		//	좌 아래 로 가는 경우
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
		//	왼쪽, 아래쪽을 일자로 검사해나간다.
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
		//	오른쪽으로 가는 경우
		//	내 위에가 막혀있고 , 내 위에 오른쪽이 뚫려있는 경우
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
		//	우 위로 가는 경우
		//	오른쪽이 막혀있고 그 아래가 뚫림
		//	위에가 막혀있고 그 왼쪽이 뚫림
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
		//	오른쪽, 위쪽을 검사한다.
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
		//	우 아래로 가는 경우
		//	오른쪽이 막혀있고 그 위가 뚫림
		//	아래가 막혀있고 그 왼쪽이 뚫림
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
		//	왼쪽, 아래쪽을 일자로 검사해나간다.
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
//	해당 위치에서 꺽여질 만한 곳인지 체크하는 함수
//	
//	꺽여질 곳이다  = true
//	택도 없다  = false
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
//	노드 내에 따로 방향정보를 저장하지 않고 있어, 부모의 좌표로 방향정보를 추측해낸다.
//------------------------------------------------------------------
DWORD CTileMap::CalcDIR_FromParent(DWORD dwX, DWORD dwY, DWORD parentX, DWORD parentY)
{
	int DIR_X = (int)parentX - (int)dwX;
	int DIR_Y = (int)parentY - (int)dwY;

	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	X좌표의 변화가 없다 = 위 또는 아래로 이동했다.
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
	//	부모로 부터 내가 오른쪽에 있다.
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
	//	부모로 부터 내가 왼쪽에 있다.
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