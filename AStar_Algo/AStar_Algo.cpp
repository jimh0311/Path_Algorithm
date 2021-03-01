// AStar_Algo.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
//#include "AStar_Algo.h"
#include "TileMap.h"
#include "Resource.h"
//--------------------------------------------------------------
//	Carmino 2016.12.15
//	�����Լ� ����
//--------------------------------------------------------------
HINSTANCE	hInst;                                // ���� �ν��Ͻ��Դϴ�.
HWND		g_hWnd;
CTileMap	g_TileMap(48,27,24);
HDC			g_hdc;

DWORD	g_dwMouseX = 0;
DWORD	g_dwMouseY = 0;
DWORD	g_dwOldMouseX = 0;
DWORD	g_dwOldMouseY = 0;
//--------------------------------------------------------------
//	Carmino 2016.12.15
//	�Լ� ������Ÿ�� ����
//--------------------------------------------------------------
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


//--------------------------------------------------------------
//	Carmino 2016.12.15
//	Window Main
//--------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASTAR_ALGO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASTAR_ALGO);
	wcex.lpszClassName = L"lpszClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

	g_hWnd = CreateWindowW(L"lpszClass", L"A Star Algorithm tool", ( WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ),
		100, 100, 1600, 750, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	g_hdc = GetDC(g_hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASTAR_ALGO));

    MSG msg;
	g_TileMap.DrawMapGDI(g_hdc);
    // �⺻ �޽��� �����Դϴ�.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	ReleaseDC(g_hWnd, g_hdc);
    return (int) msg.wParam;
}

//--------------------------------------------------------------
//	Carmino 2016.12.15
//	Window Proc
//--------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 1:
			{
				DWORD	StartX, StartY, GoalX, GoalY;
				g_TileMap.GetStartPos(&StartX, &StartY);
				g_TileMap.GetGoalPos(&GoalX, &GoalY);
				if (g_TileMap.FindJumpingPath(StartX, StartY, GoalX, GoalY) == false)
				{
					KillTimer(g_hWnd, 1);
				}
			}
			break;
		}
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_SPACE:
			{
				g_TileMap.Clear();
				g_TileMap.DrawMapGDI(g_hdc);
				break;
			}
		case VK_TAB:
			{
				DWORD	StartX, StartY, GoalX, GoalY;
				g_TileMap.GetStartPos(&StartX, &StartY);
				g_TileMap.GetGoalPos(&GoalX, &GoalY);
				g_TileMap.FindJumpingPath(StartX, StartY, GoalX, GoalY);
				SetTimer(g_hWnd, 1, 500, NULL);
				break;
			}
		case VK_SHIFT:
			{
				DWORD	StartX, StartY, GoalX, GoalY;
				g_TileMap.GetStartPos(&StartX, &StartY);
				g_TileMap.GetGoalPos(&GoalX, &GoalY);
				g_TileMap.FindPath(StartX, StartY, GoalX, GoalY);
				MessageBox(g_hWnd, L"Complete SearchRoad", L"System", MB_OK);
				break;
			}
		}
	}
	break;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//
	//	�巡�� �ؼ� ������ �ܾ� ���� ����� �� ����
	//--------------------------------------------------------------
	case WM_MOUSEMOVE:
	{
		switch (wParam)
		{
		case MK_LBUTTON:
			g_dwOldMouseX = g_dwMouseX;
			g_dwOldMouseY = g_dwMouseY;
			g_dwMouseX = GET_X_LPARAM(lParam);
			g_dwMouseY = GET_Y_LPARAM(lParam);
			DWORD L = g_TileMap.GetTileLength();
			if ( (g_dwOldMouseX / L) != (g_dwMouseX / L) || (g_dwOldMouseY / L) != ( g_dwMouseY / L))
			{
				g_TileMap.ChangeTileType(g_dwMouseX / L, g_dwMouseY / L);
				g_TileMap.DrawMapGDI(g_hdc);
			}
			break;
		}
	}
	break;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//	
	//	�� �� ��ŸƮ ���� ����
	//--------------------------------------------------------------
	case WM_RBUTTONDOWN:
	{
		g_dwMouseX = GET_X_LPARAM(lParam);
		g_dwMouseY = GET_Y_LPARAM(lParam);

		DWORD L = g_TileMap.GetTileLength();
		g_TileMap.SettingGoal(g_dwMouseX / L, g_dwMouseY / L);
		//InvalidateRect(g_hWnd, NULL, TRUE);
		g_TileMap.DrawMapGDI(g_hdc);
	}
	break;
	//--------------------------------------------------------------
	//	Carmino 2016.12.21
	//
	//	��� �巡�� ������ üũ�ϸ� ���ڸ����� ������ Ŭ���� ������ ������
	//--------------------------------------------------------------
	case WM_LBUTTONDOWN:
	{
		g_dwOldMouseX = g_dwMouseX;
		g_dwOldMouseY = g_dwMouseY;
		g_dwMouseX = GET_X_LPARAM(lParam);
		g_dwMouseY = GET_Y_LPARAM(lParam);
		DWORD L = g_TileMap.GetTileLength();	
		g_TileMap.ChangeTileType(g_dwMouseX / L, g_dwMouseY / L);
		g_TileMap.DrawMapGDI(g_hdc);
	}
	break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // �޴� ������ ���� �м��մϴ�.
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
		
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			//--------------------------------------------------------------
			//	Carmino 2016.12.15
			//	Ÿ�ϸ��� ��ο츦 �߰��մϴ�...
			//
			//	2016.12.21 ������Ʈ ����...
			//--------------------------------------------------------------
			g_TileMap.DrawMapGDI(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
		
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
