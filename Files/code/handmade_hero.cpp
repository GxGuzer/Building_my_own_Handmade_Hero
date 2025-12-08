#include <Windows.h>
#include <iostream>
using namespace std;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

bool running = false;

// Bitmap variables and functions.
static BITMAPINFO bitmapInfo;
static void *bitmapMemory;
static int bitmapWidth; // Temporarily global
static int bitmapHeight; // Temporarily global
int bytePerPixel = 4;

void renderGrad(int xOffset, int yOffset) {
  int width = bitmapWidth;
  int height = bitmapHeight;

  int pitch = width*bytePerPixel;
  uchar *row = (uchar *)bitmapMemory;
  for (int Y = 0; Y < bitmapHeight; Y++) {
    uint *pixel = (uint *)row;
    for (int X = 0; X < bitmapWidth; X++) {
      /*
      Logic for pixels on memory:
      00 00 00 00
      RR GG BB xx

      Based on this RGB estimative the code follows:
      - FF 00 00 00 Whole screen should be red ended being blue.
      - 00 FF 00 00 Whole screen should be green ended as expected.
      - 00 00 FF 00 Whole screen should be blue ended being red.
      - 00 00 00 FF Nothing special is expected.
      Conclusion: RGB scheme is actually BGR.
      */

      uchar Blue = (X + xOffset);
      uchar Green = (Y + yOffset);

      *pixel = ((Green << 8) | Blue);
      pixel++;

      /*
      Little endian on pixel explained:
      Little endian makes the bigger significants values the last, so when working with 8 bits we have to make the buffer pull the variables accordingly.
      
      With 32 bits since it's one variable, everything we have to do is to make the channel values fall into their respectives pads, we do that through bitshifts, but considering that on registers the bits are already on the RGB pattern instead of the BGR.
      */

    }
    row += pitch;
  }  
}

// Bitmap creation and manipulation.
static void resizeDIBsection(int width, int height) {

  if(bitmapMemory) {
    VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;

  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = bitmapWidth;
  bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  // Memory sizing considering a padding for memory alignment.
  int bitmapMemorySize = (bitmapWidth*bitmapHeight)*bytePerPixel;
  bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  // Memory allocated and pointer stored.

  // This function have a chance to be cleared.
}

static void updateClientWindow(HDC DeviceContext, RECT *WindowRect, int x, int y, int width, int height) {
  int windowWidth = WindowRect->right - WindowRect->left;
  int windowHeight = WindowRect->bottom - WindowRect->top;
  StretchDIBits(DeviceContext, /*x, y, width, height, x, y, width, height*/ 0, 0, bitmapWidth, bitmapHeight, 0, 0, windowWidth, windowHeight, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

// Window procedure to messages.
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message)
  {
  case WM_SIZE: 
  {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int ClientRectWidth = ClientRect.right - ClientRect.left;
    int ClientRectHeight = ClientRect.bottom - ClientRect.top;
    resizeDIBsection(ClientRectWidth, ClientRectHeight);
    InvalidateRect(Window, NULL, true);
  }
  break;

  case WM_DESTROY:
  {
    // TODO: error handle with window recreation?
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
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(Window, &paint);

    int x = paint.rcPaint.left;
    int y = paint.rcPaint.top;
    int width = paint.rcPaint.right - paint.rcPaint.left;
    int height = paint.rcPaint.bottom - paint.rcPaint.top;

    COLORREF white = RGB(255, 255, 255);
    HBRUSH whiteBrush = CreateSolidBrush(white);
    FillRect(deviceContext, &paint.rcPaint, whiteBrush);
    DeleteObject(whiteBrush);

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    updateClientWindow(deviceContext, &ClientRect, x, y, width, height);

    EndPaint(Window, &paint);
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
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(!RegisterClass(&WindowClass)) {
    DWORD error = GetLastError();
    cout << "Register failed with: " << error << endl;
  }
  
  HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 100, 100, 640, 480, 0, 0, Instance, 0);
  
  if(HandmadeHeroWindow == NULL) {
    DWORD error = GetLastError();
    cout << "Window created null or with error: " << error << endl;
    return 0;
  }else {
    running = true;
  }


  // Show window and get messages.
  ShowWindow(HandmadeHeroWindow, ComandShow);
  
  // Stuff for a simple animation.
  int xOffset = 0;
  int yOffset = 0;

  // While loop controled by a bool to keep the program running, because `PeekMessage` gets outta the loop when there are no messages.
  while(running) {
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
      // Check for a quit message to guarantee the program will terminate when desired.
      if(message.message == WM_QUIT) {
        running = false;
      }
      // Message handling
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    renderGrad(xOffset, yOffset);
    
    // Procedure to update the window to animate properly (better turn this into a function).
    HDC deviceContext = GetDC(HandmadeHeroWindow);
    RECT ClientRect;
    GetClientRect(HandmadeHeroWindow, &ClientRect);
    int ClientRectWidth = ClientRect.right - ClientRect.left;
    int ClientRectHeight = ClientRect.bottom - ClientRect.top;
    updateClientWindow(deviceContext, &ClientRect, 0, 0, ClientRectWidth, ClientRectHeight);
    ReleaseDC(HandmadeHeroWindow, deviceContext);

    xOffset++;
  }

  return 0;
}
