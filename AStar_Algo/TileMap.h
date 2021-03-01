#ifndef _TILE_MAP_H_
#define _TILE_MAP_H_
//--------------------------------------------------------------
//	Carmino 2016.12.15
//
//	타일맵의 구성은 어떻게 할 것인가??
//	1. 맵이라는 구조체로 내부에 동적인 2x2 타일의 값을 가지게 한다...
//--------------------------------------------------------------

#include <Windows.h>
#include <cstdio>
class CTileMap
{
private:
	enum enTileType {
		ROAD = 0,
		WALL,
		WATER
	};

	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	Tile Structure;
	//	정사각형 모양을 기준으로한다.
	//--------------------------------------------------------------
	struct stTile {
		enTileType	enType;
		DWORD	dwLength;
		DWORD	dwSpeed;
		bool	bVisited;
		bool	bStart;
		bool	bGoal;
		bool	bEnd;
		int		iF;

		int		bJumpVisited;
		bool	bJumpOpenNode;
		bool	bJumpCloseNode;
	};
public:
	//--------------------------------------------------------------
	//	Carmino 2016.12.15
	//	생성 및 파괴자...
	//--------------------------------------------------------------
	CTileMap(DWORD, DWORD, DWORD);
	~CTileMap();
	//------------------------------------------------------------------
	//  Carmino  16.12.16
	//	멤버변수 호출 함수들
	//------------------------------------------------------------------
	DWORD GetTileLength() { return _dwLength; };
	//------------------------------------------------------------------
	//  Carmino  16.12.16
	//	유저 인터페이스를 위한 함수.
	//	ChangeTileType = 길, 물, 벽 세가지로 변경.
	//	SettingGoal	= 출발점, 도착점을 설정.
	//	Clear =  상태 초기화.
	//	DrawMapGDI = 그래픽 출력
	//------------------------------------------------------------------
	void DrawMapGDI(HDC, bool isJump = false);
	
	void ChangeTileType(DWORD, DWORD);
	void SettingGoal(DWORD, DWORD);
	void Clear();
	void GetStartPos(DWORD *, DWORD *);
	void GetGoalPos(DWORD *, DWORD *);
	//------------------------------------------------------------------
	//  Carmino  16.12.21
	//	A-Star 알고리즘 함수
	//------------------------------------------------------------------
	void FindPath(DWORD dwStartX, DWORD dwStartY, const DWORD dwGoalX, const DWORD dwGoalY);

	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	Jump-Search 알고리즘 함수
	//------------------------------------------------------------------
	bool FindJumpingPath(DWORD dwStartX, DWORD dwStartY, const DWORD dwGoalX, const DWORD dwGoalY, bool isFirst = true);

protected:
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	노드를 편리하게 만들어봅시다.
	//	** stdafx.h 에 struct 구현.
	//--------------------------------------------------------------
	stANode* CreateANode(DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, stANode* pParent, bool bIsDiag = false);
	stANode* CreateJumpNode(DWORD, DWORD, DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, stANode* pParent);
	void PushToOpenList(stANode *pNow, DWORD dwX, DWORD dwY, DWORD dwGoalX, DWORD dwGoalY, bool bIsDiag = false);
	DWORD CalcDIR_FromParent(DWORD dwX, DWORD dwY, DWORD parentX, DWORD parentY);
	bool Jump(DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, DWORD *pX, DWORD *pY, int dfDir);
	void JumpProcess(stANode *pNow,const DWORD dwStartX,const DWORD dwStartY, DWORD dwX, DWORD dwY, const DWORD dwGoalX, const DWORD dwGoalY, int dfDir);
	bool CheckCorner(DWORD dwWallX , DWORD dwWallY , DWORD dwRoadX, DWORD dwRoadY, bool isWallCheck = false);

protected:
	DWORD	_dwWidth;		//
	DWORD	_dwHeight;		//
	DWORD	_dwLength;		//
	stTile **_pArrMap;		//	행렬 맵
	stList* _pBackTracking;
	bool _JumpIsFirst;
};


#endif