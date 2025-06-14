#include <Windows.h>
#include <iostream>
using namespace std;

static void resizeDIBsection(int width, int height) {
    HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset);
}

static void updateClientWindow(HDC DeviceContext, int x, int y, int width, int height) {
  StretchDIBits(DeviceContext, x, y, width, height, x, y, width, height, const VOID *lpBits, const BITMAPINFO *lpbmi, UINT iUsage, DWORD rop);
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message)
  {
  case WM_SIZE:
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int ClientRectWidth = ClientRect.right - ClientRect.left;
    int ClientRectHeight = ClientRect.bottom - ClientRect.top;
    resizeDIBsection(ClientRectWidth, ClientRectHeight);
    InvalidateRect(Window, NULL, true);
  break;

  case WM_DESTROY:
    // TODO: vai ter um handle de erro aqui com recriação de janela?
    PostQuitMessage(0); 
  break;

  case WM_ACTIVATEAPP:
    cout << "Active/Deactive" << endl;
  break;
  
  case WM_PAINT:
  {
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(Window, &paint);

    COLORREF white = RGB(255, 255, 255);
    HBRUSH whiteBrush = CreateSolidBrush(white);

    int x = paint.rcPaint.left;
    int y = paint.rcPaint.top;
    int width = paint.rcPaint.right - paint.rcPaint.left;
    int height = paint.rcPaint.bottom - paint.rcPaint.top;
    updateClientWindow(deviceContext, x, y, width, height);

    FillRect(deviceContext, &paint.rcPaint, whiteBrush);
    DeleteObject(whiteBrush);
    /*COLORREF red = RGB(255, 0, 0);
    COLORREF blue = RGB(0, 0, 255);
    COLORREF green = RGB(0, 255, 0);
    COLORREF yellow = RGB(255, 255, 0);

    RECT redArea;
    redArea.left = paint.rcPaint.left;
    redArea.top = paint.rcPaint.top;
    redArea.right = paint.rcPaint.right/2;
    redArea.bottom = paint.rcPaint.bottom/2;

    RECT blueArea;
    blueArea.left = paint.rcPaint.left+(paint.rcPaint.right/2);
    blueArea.top = paint.rcPaint.top;
    blueArea.right = paint.rcPaint.right;
    blueArea.bottom = paint.rcPaint.bottom/2;

    RECT greenArea;
    greenArea.left = paint.rcPaint.left;
    greenArea.top = paint.rcPaint.top+(paint.rcPaint.bottom/2);
    greenArea.right = paint.rcPaint.right/2;
    greenArea.bottom = paint.rcPaint.bottom;

    RECT yellowArea;
    yellowArea.left = paint.rcPaint.left+(paint.rcPaint.right/2);
    yellowArea.top = paint.rcPaint.top+(paint.rcPaint.bottom/2);
    yellowArea.right = paint.rcPaint.right;
    yellowArea.bottom = paint.rcPaint.bottom;

    
    HBRUSH redBrush = CreateSolidBrush(red);
    HBRUSH blueBrush = CreateSolidBrush(blue);
    HBRUSH greenBrush = CreateSolidBrush(green);
    HBRUSH yellowBrush = CreateSolidBrush(yellow);

    
    FillRect(deviceContext, &redArea, redBrush);
    FillRect(deviceContext, &blueArea, blueBrush);
    FillRect(deviceContext, &greenArea, greenBrush);
    FillRect(deviceContext, &yellowArea, yellowBrush);

    
    DeleteObject(redBrush);
    DeleteObject(blueBrush);
    DeleteObject(greenBrush);
    DeleteObject(yellowBrush);*/

    EndPaint(Window, &paint);
  }
  break;

  case WM_CLOSE:
    if(MessageBox(Window, "Get Out?", "Close the game?", MB_OKCANCEL) == IDOK) {
      DestroyWindow(Window);
    }
    cout << "Close" << endl;
  break;

  default:
    Result = DefWindowProc(Window, Message, WParam, LParam);
  break;
  }
  return Result;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR ComandLine, int ComandShow) {
  /*
  if(AllocConsole()) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    cout << "Console allocated" << endl;
  }
  */

  WNDCLASS WindowClass = {};
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(!RegisterClass(&WindowClass)) {
    DWORD error = GetLastError();
    cout << "Register failed with: " << error << endl;
  }
  
  HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
    100, 100, 640, 480, 0, 0, Instance, 0);
  
  if(HandmadeHeroWindow == NULL) {
    DWORD error = GetLastError();
    cout << "Window created null or with error: " << error << endl;
    return 0;
  }

  ShowWindow(HandmadeHeroWindow, ComandShow);
  
  MSG message = {};
  while(GetMessage(&message, 0, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  /*
  if(GetConsoleWindow() != NULL) {
    FreeConsole();
  }
  */

  return 0;
}
