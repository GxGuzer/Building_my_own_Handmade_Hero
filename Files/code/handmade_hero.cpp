#include <Windows.h>
#include <iostream>
using namespace std;

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message)
  {
  case WM_SIZE:
    cout << "Resize" << endl;
  break;

  case WM_DESTROY:
    cout << "Destroy" << endl;
    PostQuitMessage(0);
  break;

  case WM_ACTIVATEAPP:
    cout << "Active/Deactive" << endl;
  break;
  
  case WM_PAINT:
    {
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(Window, &paint);
    FillRect(deviceContext, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
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

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance,
  PSTR ComandLine, int ComandShow)
{
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
