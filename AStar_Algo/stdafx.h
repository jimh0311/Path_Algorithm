// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//


//--------------------------------------------------------------
//	Carmino 2016.12.21
//
//	stANode* , stList* �� �� ���� ������ ���� �ִµ� �����ѹ� �ϴ°� ���� �� ����.
//	�Ҽ����� �����ϰ� G,H�� ����ϰ����Ѵٸ� �Ѵ� *10�� ����� �Ѵ�. �� �ʸ� ���ָ� ȿ������ �ſ� ������.
//	���� �׽�Ʈ �Ϸ�.
//
//
//--------------------------------------------------------------


#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>
#include <windowsx.h>
// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
struct stANode {
	DWORD dwX;
	DWORD dwY;
	stANode* pParent;	//	�ƹ���~
	int G;				//	Goal ... ������ ������ �Ÿ�.
	int H;				//	Host ... ������������ ī��Ʈ.
	int F;				//	G + H ��, F�� �������� LinkedList�� ������ ����.
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