// TicTacToeUdemyCourse.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TicTacToeUdemyCourse.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOEUDEMYCOURSE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOEUDEMYCOURSE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOEUDEMYCOURSE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);	// Changing the background color.
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOEUDEMYCOURSE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//


// My Global variables and functions:
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9];
int winner = 0;
int wins[3];

// Function for receiving the game board rectangle.
BOOL GetGameBoardRect(HWND hwnd, RECT * pRect) {
	// Created pointer rectangle to hold client rect info.
	RECT rc;
	// GetClientRect returns a boolean if found.
	if (GetClientRect(hwnd, &rc)) {
		// (overkill subtracting left and top).
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		// Calculate the center coordinates for game board.
		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;
		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;
		return TRUE;
	}
	SetRectEmpty(pRect);
	return FALSE;
}


// Function to draw a single line, which is used to draw the board.
void DrawLine(HDC hdc, int x1, int y1, int x2, int y2) {
	// Move drawer to specific location.
	MoveToEx(hdc, x1, y1, NULL);
	// Draws a line.
	LineTo(hdc, x2, y2);
}


// Function to determine which cell contains a point.
int GetCellNumberFromPoint(HWND hwnd, int x, int y) {
	// Create point object to check if click is in rectangle.
	POINT pt;
	RECT rc;

	pt.x = x;
	pt.y = y;
	// Gets game board rectangle.
	if (GetGameBoardRect(hwnd, &rc)) {
		if (PtInRect(&rc, pt)) {
			// Normalize (make the gameboard an offset of window so its
			// start is 0, 0.
			// How far from the left edge of game board is the mouse click:
			x = pt.x - rc.left;
			// ... from top edge.
			y = pt.y - rc.top;
			// Calculates column and row sizes.
			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			// Convert to index of 0 to 8 corresponding to which cell 
			// was clicked in.
			return column + row * 3;
		}
	}
	// Outside of game board.
	return -1;
}


// Function to receive the cell's rectangle.
BOOL GetCellRect(HWND hWnd, int index, RECT * pRect) {
	RECT rcBoard;
	
	SetRectEmpty(pRect);

	// Check if outside of game board.
	if (index < 0 || index > 8) {
		return FALSE;
	}

	if (GetGameBoardRect(hWnd, &rcBoard)) {
		// Convert index into x and y pair.
		int y = index / 3;	// Row number.
		int x = index % 3;	// Column number.

		// Set dimensions and adds 1 pixel buffer.
		pRect->left = rcBoard.left + x * CELL_SIZE + 1;
		pRect->top = rcBoard.top + y * CELL_SIZE + 1;
		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;

		return TRUE;
	}
	return FALSE;
}


// Function to check whether there is a winner 
// 0 no winner; 1 player one; 2 player 2; 3 draw
int GetWinner() {
	// Array of all possible cell combinations.
	int cells[] = { 0,1,2, 3,4,5, 6,7,8, 0,3,6, 1,4,7, 2,5,8, 0,4,8, 2,4,6 };

	// Loops through array checking each group of 3 cells.
	for (int i = 0; i < ARRAYSIZE(cells); i += 3) {
		if ((gameBoard[cells[i]] > 0) && 
			(gameBoard[cells[i]] == gameBoard[cells[i + 1]]) && 
			(gameBoard[cells[i]] == gameBoard[cells[i + 2]])) {

			// There is a winner. Return the winning cells.
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}
	// Check for empty cells still.
	for (int i = 0; i < ARRAYSIZE(gameBoard); i++) {
		if (gameBoard[i] < 1) {
			// Still at least one open cell.
			return 0;
		}
	}
	// All cells filled up with no winner. Draw.
	return 3;
}


// Function to write out whos turn it currently is.
void ShowTurn(HWND hwnd, HDC hdc) {
	RECT rc;
	static const WCHAR szTurn1[] = L"Turn: Player 1";
	static const WCHAR szTurn2[] = L"Turn: Player 2";

	const WCHAR * pszTurnText = NULL;

	// winner variable will be zero until someone has won
	// or the board has filled up.
	switch (winner) {
	case 0:
		pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
		break;
	case 1:
		pszTurnText = L"Player 1 is the winner!";
		break;
	case 2:
		pszTurnText = L"Player 2 is the winner!";
		break;
	case 3:
		pszTurnText = L"It is a draw";
		break;
	}

	if (pszTurnText != NULL && GetClientRect(hwnd, &rc)) {
		// Set rectangle to subset of window.
		rc.top = rc.bottom - 48;
		// Colors background to match game board.
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		// Sets the text color and then draws it to the window.
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
	}
}


// Function to draw the player icons in the center of their cell.
void DrawIconCentered(HDC hdc, RECT * pRect, HICON hIcon) {
	// Gets the width and height of the icons.
	const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
	const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);

	if (pRect != NULL) {
		// Calculates the position for the icon and then draws it.
		int left = pRect->left + ((pRect->right - pRect->left) - ICON_WIDTH) / 2;
		int top = pRect->top + ((pRect->bottom - pRect->top) - ICON_HEIGHT) / 2;
		DrawIcon(hdc, left, top, hIcon);
	}
}



// Function for highlighting the winning cells and then adding 
// the icons back on top.
void ShowWinner(HWND hwnd, HDC hdc) {
	RECT rcWin;
	int counter = 0;

	for (int i = 0; i < sizeof(wins); ++i) {
		// Checks if the player has claimed that cell and only 3 cells are highlighted.
		if ((gameBoard[wins[i]] == winner) && (counter < 3)) {
			if (GetCellRect(hwnd, wins[i], &rcWin)) {
				FillRect(hdc, &rcWin, (winner == 1) ? hbr1 : hbr2);
				DrawIconCentered(hdc, &rcWin, (winner == 1) ? hIcon1 : hIcon2);
				counter++;
			}
		}
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			// Assigning brush colors on create.
			hbr1 = CreateSolidBrush(RGB(255, 0, 0));
			hbr2 = CreateSolidBrush(RGB(0, 0, 255));

			// Load player icons if added to references.
			// hInst is global variable made by VS at top of this file.
			// IDI_PLAYERx is custom name given to resouce when added.
			hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER1));
			hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER2));
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_NEWGAME:	// Adds a New Game option in menu and executes the following code.
			{
				// Displays confirmation box for player.
				int ret = MessageBox(hWnd, L"Are you sure you want to start a new game?", L"New Game", MB_YESNO | MB_ICONQUESTION);
				if (ret == IDYES) {
					// Reset and start new game.
					playerTurn = 1;
					winner = 0;
					ZeroMemory(gameBoard, sizeof(gameBoard));
					// Force a paint message. It posts WM_PAINT to windowProc, but doesn't clear.
					// It is queued in the message queue.
					InvalidateRect(hWnd, NULL, TRUE);
					// Forces immediate handling of WM_PAINT.
					UpdateWindow(hWnd);
				}
			}
			break;
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

	case WM_LBUTTONDOWN:	// Actions to take upon a left mouse click.
		{
			// No need to go further if game is already over.
			if (playerTurn == 0) {
				break;
			}
		
			// Gets the x and y coordinates of mouse click.
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			// Gets the cell's index number that is used for identifying the cell.
			int index = GetCellNumberFromPoint(hWnd, xPos, yPos);


			
			HDC hdc = GetDC(hWnd);
			if (NULL != hdc) {
				/*WCHAR temp[100];		// FOR TESTING - unicode printing to screen from "message".
				wsprintf(temp, L"Index = %d", index);
				TextOut(hdc, xPos, yPos, temp, lstrlen(temp));*/

				// Get cell dimension from its index.
				if (index != -1) {
					RECT rcCell;
					if ((gameBoard[index] < 1) && GetCellRect(hWnd, index, &rcCell)) {
						// Assigns the player's number as the value of the gameBoard array
						// to determine who has claimed that cell.
						gameBoard[index] = playerTurn;

						DrawIconCentered(hdc, &rcCell, (playerTurn == 1) ? hIcon1 : hIcon2);
						
						// Checks if there is a winner.
						winner = GetWinner();
						if (winner == 1 || winner == 2) {
							// Displays winner message.
							ShowWinner(hWnd, hdc);
							// Displays pop-up box saying who won.
							MessageBox(hWnd, winner == 1 ? L"Player 1 is the winner!" : L"Player 2 is the winner!", L"You Win!", MB_OK | MB_ICONINFORMATION);
							// Sets the game to gameover.
							playerTurn = 0;
						}
						else if (winner == 3) {
							// The board is full and no winner.
							MessageBox(hWnd, L"No one wins this time!", L"It is a draw!", MB_OK | MB_ICONEXCLAMATION);
							playerTurn = 0;
						}
						else {
							// Change who's turn it is.
							playerTurn = playerTurn == 1 ? 2 : 1;
						}
						//Display which player's turn.
						ShowTurn(hWnd, hdc);
					}
					
				}
				// Release object when done with it.
				ReleaseDC(hWnd, hdc);
			}
		}
		break;

	case WM_GETMINMAXINFO:	// Adding this "message" to get window minimum size.
		{
			MINMAXINFO * pMinMax = (MINMAXINFO*)lParam;
			// Setting the min size that the window can be sized to.
			pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
			pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
			//
			//
			//
			//
			// Created pointer rectangle to hold game board rect info.
			RECT rc;
			
			if (GetGameBoardRect(hWnd, &rc)) {
				// Create pointer rectangle.
				RECT rcClient;
				// Tries to retrieve the window rectangle to place Player 2
				// at the right side.
				if (GetClientRect(hWnd, &rcClient)) {
					const WCHAR szPlayer1[] = L"Player 1";
					const WCHAR szPlayer2[] = L"Player 2";

					// Hide background color of textbox.
					SetBkMode(hdc, TRANSPARENT);

					// Draw Player 1 and 2 text as well as their icons.
					SetTextColor(hdc, RGB(255, 255, 0));
					TextOut(hdc, 16, 16, szPlayer1, lstrlen(szPlayer1));
					DrawIcon(hdc, 24, 40, hIcon1);

					SetTextColor(hdc, RGB(0, 0, 255));
					TextOut(hdc, rcClient.right - 72, 16, szPlayer2, lstrlen(szPlayer2));
					DrawIcon(hdc, rcClient.right - 64, 40, hIcon2);

					//Display which player turn.
					ShowTurn(hWnd, hdc);
				}

				// Draws rectangle for game board.
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
			}
			for (int i = 1; i < 3; i++) {
				// Draw vertical lines.
				DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);
				// Draw horizontal lines.
				DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);	
			}

			RECT rcCell;
			// Draw all occupied cells to prevent losing them when resizing window.
			for (int i = 0; i < ARRAYSIZE(gameBoard); i++) {
				if ((gameBoard[i] > 0) && GetCellRect(hWnd, i, &rcCell)) {
					DrawIconCentered(hdc, &rcCell, (gameBoard[i] == 1) ? hIcon1 : hIcon2);
				}
			}

			// Highlights winning cells if there is a winner.
			if (winner == 1 || winner == 2) {
				// Show winner highlighted cells.
				ShowWinner(hWnd, hdc);
			}
			//
			//
			//
			//
			//
			//
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		// Destroying brushes created earlier.
		DeleteObject(hbr1);
		DeleteObject(hbr2);
		// Destroying icons loaded before (if did).
		DestroyIcon(hIcon1);
		DestroyIcon(hIcon2);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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


// Awesome icons came from http://www.iconarchive.com/show/star-wars-lego-icons-by-iconfactory.html