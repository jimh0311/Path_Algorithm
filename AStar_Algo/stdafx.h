// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//


//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//	stANode* , stList* 두 개 지금 얽혀서 쓰고 있는데 정리한번 하는게 좋을 것 같다.
//	소수점을 배제하고 G,H를 사용하고자한다면 둘다 *10을 해줘야 한다. 한 쪽만 해주면 효율성이 매우 떨어짐.
//	얼추 테스트 완료.
//
//
//--------------------------------------------------------------


#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <windowsx.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
struct stANode {
	DWORD dwX;
	DWORD dwY;
	stANode* pParent;	//	아버지~
	int G;				//	Goal ... 도착점 까지의 거리.
	int H;				//	Host ... 시작점까지의 카운트.
	int F;				//	G + H 값, F를 기준으로 LinkedList를 정렬할 것임.
};

struct stList
{
	stANode *value;
	stList *pNext;
	stList *pPrev;
};

#define dfDIR_LL	4
#define dfDIR_LU	7
#define dfDIR_LD	1
#define dfDIR_RR	6
#define dfDIR_RD	3
#define dfDIR_RU	9
#define dfDIR_UU	8
#define dfDIR_DD	2