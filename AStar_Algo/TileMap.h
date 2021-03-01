#ifndef _TILE_MAP_H_
#define _TILE_MAP_H_
//--------------------------------------------------------------
//	Carmino 2016.12.15
//
//	Ÿ�ϸ��� ������ ��� �� ���ΰ�??
//	1. ���̶�� ����ü�� ���ο� ������ 2x2 Ÿ���� ���� ������ �Ѵ�...
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
	//	���簢�� ����� ���������Ѵ�.
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
	//	���� �� �ı���...
	//--------------------------------------------------------------
	CTileMap(DWORD, DWORD, DWORD);
	~CTileMap();
	//------------------------------------------------------------------
	//  Carmino  16.12.16
	//	������� ȣ�� �Լ���
	//------------------------------------------------------------------
	DWORD GetTileLength() { return _dwLength; };
	//------------------------------------------------------------------
	//  Carmino  16.12.16
	//	���� �������̽��� ���� �Լ�.
	//	ChangeTileType = ��, ��, �� �������� ����.
	//	SettingGoal	= �����, �������� ����.
	//	Clear =  ���� �ʱ�ȭ.
	//	DrawMapGDI = �׷��� ���
	//------------------------------------------------------------------
	void DrawMapGDI(HDC, bool isJump = false);
	
	void ChangeTileType(DWORD, DWORD);
	void SettingGoal(DWORD, DWORD);
	void Clear();
	void GetStartPos(DWORD *, DWORD *);
	void GetGoalPos(DWORD *, DWORD *);
	//------------------------------------------------------------------
	//  Carmino  16.12.21
	//	A-Star �˰��� �Լ�
	//------------------------------------------------------------------
	void FindPath(DWORD dwStartX, DWORD dwStartY, const DWORD dwGoalX, const DWORD dwGoalY);

	//------------------------------------------------------------------
	//  Carmino  16.12.26
	//	Jump-Search �˰��� �Լ�
	//------------------------------------------------------------------
	bool FindJumpingPath(DWORD dwStartX, DWORD dwStartY, const DWORD dwGoalX, const DWORD dwGoalY, bool isFirst = true);

protected:
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	��带 ���ϰ� �����ô�.
	//	** stdafx.h �� struct ����.
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
	stTile **_pArrMap;		//	��� ��
	stList* _pBackTracking;
	bool _JumpIsFirst;
};


#endif