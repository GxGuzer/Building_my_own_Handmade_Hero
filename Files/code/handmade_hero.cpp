#include <Windows.h>
#include <iostream>
using namespace std;

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message)
  {
  case WM_SIZE:
    cout << "Resize";
  break;

  case WM_DESTROY:
    PostQuitMessage(0);
    cout << "Destroy";
  break;

  case WM_ACTIVATEAPP:
    cout << "Active/Deactive";
  break;
  
  case WM_PAINT:
    {
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(Window, &paint);
    /*
    int X = paint.rcPaint.left;
    int Y = paint.rcPaint.top;
    int W = paint.rcPaint.right - paint.rcPaint.left;
    int H = paint.rcPaint.bottom - paint.rcPaint.top;
    PatBlt(deviceContext, X, Y, W, H, BLACKNESS);
    */
    FillRect(deviceContext, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
    EndPaint(Window, &paint);
    }
  break;

  case WM_CLOSE:
    if(MessageBox(Window, "Get Out?", "Close the game?" MB_OKCANCEL) == IDOK) {
      DestroyWindow(Window);
    }
    cout << "Close";
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

  WNDCLASS WindowClass = {};
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  RegisterClass(&WindowClass);

  HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
    CW_DEFAULT, CW_DEFAULT, CW_DEFAULT, CW_DEFAULT, 0, 0, Instance, 0);

  if(HandmadeHeroWindow == NULL) {
    return 0;
  }

  ShowWindow(HandmadeHeroWindow, ComandShow);

  MSG message = {};
  while(GetMessage(&message, 0, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  /*
  if(RegisterClass(&WindowClass)) {

    HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
      CW_DEFAULT, CW_DEFAULT, CW_DEFAULT, CW_DEFAULT, 0, 0, Instance, 0);
    
    if(HandmadeHeroWindow) {
      for(;;) {
        MSG message;
        BOOL messageResult = GetMessage(&message, 0, 0, 0);
        if(messageResult > 0) {
          TranslateMessage(&message);
          DispatchMessage(&message);
        }else {

        }
        
      }
    }else {
      //Logging
    }
  }else {
    //Logging
  }*/

  //ShowWindow(HandmadeHeroWindow, ComandShow);

  return 0;
}
