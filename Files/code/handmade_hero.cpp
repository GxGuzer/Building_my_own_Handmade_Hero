#include <Windows.h>
#include <iostream>
using namespace std;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

bool Running = false;

// Bitmap variables and functions.
struct BitmapBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int BytePerPixel;
  int Pitch;
};

static BitmapBuffer GlobalBackbuffer;

struct ClientWindowDimension {
  int Width;
  int Height;
};

ClientWindowDimension GetClientWindowDimension(HWND WindowHandle) {
  ClientWindowDimension Result;

  RECT ClientRect;
  GetClientRect(WindowHandle, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return Result;
}

void RenderGrad(BitmapBuffer Buffer, int XOffset, int YOffset) {

  uchar *Row = (uchar *)Buffer.Memory;
  for (int Y = 0; Y < Buffer.Height; Y++) {
    uint *Pixel = (uint *)Row;
    for (int X = 0; X < Buffer.Width; X++) {
      
      uchar Blue = (X + XOffset);
      uchar Green = (Y + YOffset);

      *Pixel = ((Green << 8) | Blue);
      Pixel++;

    }
    Row += Buffer.Pitch;
  }  
}

// Bitmap creation and manipulation.
static void ResizeDIBSection(BitmapBuffer *Buffer, int Width, int Height) {

  if(Buffer->Memory) {
    VirtualFree(Buffer->Memory, NULL, MEM_RELEASE);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytePerPixel = 4;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  Buffer->Pitch = Width*Buffer->BytePerPixel;

  // Memory sizing considering a padding for memory alignment.
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytePerPixel;
  Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  // Memory allocated and pointer stored.

  // This function have a chance to be cleared.
}

static void DisplayBuffer(HDC DeviceContext, int WindowWidth, int WindowHeight, BitmapBuffer Buffer, int X, int Y, int Width, int Height) {
  StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer.Width, Buffer.Height, Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}

// Window procedure to messages.
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message)
  {
  case WM_SIZE: 
  {
    InvalidateRect(Window, NULL, true);
  }
  break;

  case WM_DESTROY:
  {
    // TODO: Error handle with window recreation?
    PostQuitMessage(0); 
  }
  break;

  case WM_ACTIVATEAPP:
  {
    cout << "Active/Deactive" << endl;
  }
  break;
  
  case WM_PAINT:
  {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);

    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

    COLORREF White = RGB(255, 255, 255);
    HBRUSH WhiteBrush = CreateSolidBrush(White);
    FillRect(DeviceContext, &Paint.rcPaint, WhiteBrush);
    DeleteObject(WhiteBrush);

    ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(Window);
    DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer, X, Y, Width, Height);

    EndPaint(Window, &Paint);
  }
  break;

  case WM_CLOSE:
  {
    if(MessageBox(Window, "Get Out?", "Close the game?", MB_OKCANCEL) == IDOK) {
      DestroyWindow(Window);
    }
    cout << "Close" << endl;
  }
  break;

  default:
  {
    Result = DefWindowProc(Window, Message, WParam, LParam);
  }
  break;
  }
  return Result;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR ComandLine, int ComandShow) {
  
  // Class and window creation.
  WNDCLASS WindowClass = {};
  WindowClass.style = CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(!RegisterClass(&WindowClass)) {
    DWORD Error = GetLastError();
    cout << "Register failed with: " << Error << endl;
  }
  
  HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 100, 100, 640, 480, 0, 0, Instance, 0);
  
  if(HandmadeHeroWindow == NULL) {
    DWORD Error = GetLastError();
    cout << "Window created null or with error: " << Error << endl;
    return 0;
  }else {
    Running = true;
  }

  ResizeDIBSection(&GlobalBackbuffer, 1208, 720);

  // Show window and get messages.
  ShowWindow(HandmadeHeroWindow, ComandShow);
  
  // Stuff for a simple animation.
  int XOffset = 0;
  int YOffset = 0;

  // While loop controled by a bool to keep the program running, because `PeekMessage` gets outta the loop when there are no messages.
  while(Running) {
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
      // Check for a quit message to guarantee the program will terminate when desired.
      if(Message.message == WM_QUIT) {
        Running = false;
      }
      // Message handling
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    }
    RenderGrad(GlobalBackbuffer, XOffset, YOffset);
    
    // Procedure to update the window to animate properly (better turn this into a function).
    HDC DeviceContext = GetDC(HandmadeHeroWindow);
    ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(HandmadeHeroWindow);
    DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer, 0, 0, ClientWindowDimension.Width, ClientWindowDimension.Height);
    ReleaseDC(HandmadeHeroWindow, DeviceContext);

    XOffset++;
  }

  return 0;
}
