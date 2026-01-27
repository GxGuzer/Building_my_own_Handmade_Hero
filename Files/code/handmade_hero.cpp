#include <Windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include <iostream>
using namespace std;
//TODO: Make a console "catch" for errors.

#define PI 3.14159265359f

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

bool Running = false;

// RENDER

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
      
      uchar Red = X + XOffset;
      uchar Green = Y + YOffset;
      uchar Blue = 0;

      *Pixel = ((Red << 16) | (Green << 8) | Blue);
      *Pixel++;

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

  Buffer->Pitch = Width * Buffer->BytePerPixel;

  // Memory sizing considering a padding for memory alignment.
  int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytePerPixel;
  Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  // Memory allocated and pointer stored.

  // This function have a chance to be cleared.
}

static void DisplayBuffer(HDC DeviceContext, int WindowWidth, int WindowHeight, BitmapBuffer Buffer) {
  StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer.Width, Buffer.Height, Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}

// SOUND

LPDIRECTSOUNDBUFFER GlobalSecondarySoundBuffer;

typedef HRESULT WINAPI MyDirectSoundCreateFunction(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

// Load the sound library and creates sound buffers if succeed.
static void LoadSound(HWND WindowHandle, int BufferSize, int SamplesPerSecond) {
  // TODO: Make the first attempt of sound be with the modern sound API (XAudio2).

  DWORD Error;

  HMODULE DirectSoundLibLoad = LoadLibrary("dsound.dll");
  if(DirectSoundLibLoad) {
    MyDirectSoundCreateFunction *MyDirectSoundCreate = (MyDirectSoundCreateFunction *)GetProcAddress(DirectSoundLibLoad, "DirectSoundCreate");

    LPDIRECTSOUND DirectSoundObject;
    if(SUCCEEDED(MyDirectSoundCreate(0, &DirectSoundObject, 0))) {
      if(SUCCEEDED(DirectSoundObject->SetCooperativeLevel(WindowHandle, DSSCL_PRIORITY))) {

        WAVEFORMATEX WaveFormatex = {};
        WaveFormatex.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormatex.nChannels = 2;
        WaveFormatex.nSamplesPerSec = SamplesPerSecond;
        WaveFormatex.wBitsPerSample = 16;
        WaveFormatex.nBlockAlign = WaveFormatex.nChannels * WaveFormatex.wBitsPerSample / 8;
        WaveFormatex.nAvgBytesPerSec = WaveFormatex.nSamplesPerSec * WaveFormatex.nBlockAlign;
        
        // "Create" a primary sound buffer.
        LPDIRECTSOUNDBUFFER PrimarySoundBuffer;
        DSBUFFERDESC PrimarySoundBufferDescription = {};
        PrimarySoundBufferDescription.dwSize = sizeof(PrimarySoundBufferDescription);
        PrimarySoundBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;   
        PrimarySoundBufferDescription.guid3DAlgorithm = GUID_NULL;
        // the members dwBufferBytes and lpwfxFormat are 0.
        if(SUCCEEDED(DirectSoundObject->CreateSoundBuffer(&PrimarySoundBufferDescription, &PrimarySoundBuffer, 0))) {
          
          if(SUCCEEDED(PrimarySoundBuffer->SetFormat(&WaveFormatex))) {
            // Buffer format set.
          }else {
            // Error catch.
            Error = GetLastError();
          }
        }else {
          // Error catch.
          Error = GetLastError();
        }

        // "Create" a secondary sound buffer. 
        DSBUFFERDESC SecondarySoundBufferDescription = {};
        SecondarySoundBufferDescription.dwSize = sizeof(SecondarySoundBufferDescription);
        SecondarySoundBufferDescription.dwFlags = 0; // maybe put the flag DSBCAPS_GETCURRENTPOSITION2.
        SecondarySoundBufferDescription.dwBufferBytes = BufferSize;
        SecondarySoundBufferDescription.lpwfxFormat = &WaveFormatex;
        SecondarySoundBufferDescription.guid3DAlgorithm = GUID_NULL;
        if(SUCCEEDED(DirectSoundObject->CreateSoundBuffer(&SecondarySoundBufferDescription, &GlobalSecondarySoundBuffer, 0))) {
          // Secondary buffer created.
        }else {
          // Error catch.
          Error = GetLastError();
        }
      }else {
        // Error catch.
        Error = GetLastError();
      }

    }else {
      // Error catch.
      Error = GetLastError();
    }
  }else {
    // Error catch.
    Error = GetLastError();
  }

  // BufferSize goes on secondary buffer.
  
  // Start playing.
}

// Stuff for audio.
  struct SoundOutputConfig {
    int SamplePerSeconds;
    int BytesPerSample;
    int SoundBufferSize;
    uint RunningSampleIndex;
    int ToneHertz;
    short ToneVolume;
    int WavePeriod;
    bool SoundIsPlaying;
  };

static void FillSoundBuffer(SoundOutputConfig *SoundOutputConfig, DWORD WriteRegionOffset, DWORD WriteRegionLength) {

  void *FirstWriteRegionPointer;
  DWORD FirstWriteRegionLength;
  void *SecondWriteRegionPointer;
  DWORD SecondWriteRegionLength;
  HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(WriteRegionOffset, WriteRegionLength, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0); // Lock is returning an error: E_INVALIDARG
  if(SUCCEEDED(LockResult)) {
    short *SampleOutput = (short *)FirstWriteRegionPointer;
    DWORD FirstRegionSampleCounter = FirstWriteRegionLength / SoundOutputConfig->BytesPerSample;
    for(DWORD SampleIndex = 0; SampleIndex < FirstRegionSampleCounter; SampleIndex++) {
      float t = 2.0f*PI * (float)SoundOutputConfig->RunningSampleIndex / (float)SoundOutputConfig->WavePeriod;
      float SineValue = sinf(t);
      short SampleValue = (short)(SineValue * SoundOutputConfig->ToneVolume);
      *SampleOutput++ = SampleValue;
      *SampleOutput++ = SampleValue;

      SoundOutputConfig->RunningSampleIndex++;
    }
    DWORD SecondRegionSampleCounter = SecondWriteRegionLength / SoundOutputConfig->BytesPerSample;
    SampleOutput = (short *)SecondWriteRegionPointer;
    for(DWORD SampleIndex = 0; SampleIndex < SecondWriteRegionLength; SampleIndex++) {
      float t = 2.0f*PI * (float)SoundOutputConfig->RunningSampleIndex / (float)SoundOutputConfig->WavePeriod;
      float SineValue = sinf(t);
      short SampleValue = (short)(SineValue * SoundOutputConfig->ToneVolume);
      *SampleOutput++ = SampleValue;
      *SampleOutput++ = SampleValue;
      
      SoundOutputConfig->RunningSampleIndex++;
    }
    GlobalSecondarySoundBuffer->Unlock(&FirstWriteRegionPointer, FirstWriteRegionLength, &SecondWriteRegionPointer, SecondWriteRegionLength);
  }
}

// INPUT

DWORD WINAPI ThereAreNoXInputLib(DWORD dwUserIndex, XINPUT_STATE* pState) {
  return ERROR_DEVICE_NOT_CONNECTED;
}

typedef DWORD WINAPI XInputGetStateFunction(DWORD dwUserIndex, XINPUT_STATE* pState);
static XInputGetStateFunction *XInputGetStatePointer = ThereAreNoXInputLib;
#define XInputGetState XInputGetStatePointer

typedef DWORD WINAPI XInputSetStateFunction(DWORD dwUserIndex, XINPUT_STATE* pState);
static XInputSetStateFunction *XInputSetStatePointer = ThereAreNoXInputLib;
#define XInputSetState XInputSetStatePointer

static void LoadXInputLib(void) {
  HMODULE XInputLibLoad = LoadLibrary("xinput1_4.dll");
  if(XInputLibLoad) {
    XInputGetStatePointer = (XInputGetStateFunction *)GetProcAddress(XInputLibLoad, "XInputGetState");
    XInputSetStatePointer = (XInputSetStateFunction *)GetProcAddress(XInputLibLoad, "XInputSetState");
  }else {
    XInputLibLoad = LoadLibrary("xinput1_3.dll");
    if(XInputLibLoad) {
      XInputGetStatePointer = (XInputGetStateFunction *)GetProcAddress(XInputLibLoad, "XInputGetState");
      XInputSetStatePointer = (XInputSetStateFunction *)GetProcAddress(XInputLibLoad, "XInputSetState");
    }
  }
}

// Struct with the "player state".
struct {
  bool Up;
  bool Left;
  bool Down;
  bool Right;
} ScreenState;

// WINDOW

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
  
  case WM_PAINT:
  {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);

    ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(Window);
    DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer);

    EndPaint(Window, &Paint);
  }
  break;

  case WM_CLOSE:
  {
     /*
    if(MessageBox(Window, "Get Out?", "Close the game?", MB_OKCANCEL) == IDOK) {
      DestroyWindow(Window);
    }*/ 
    DestroyWindow(Window);
    //cout << "Close" << endl;
  }
  break;

  case WM_KEYDOWN: case WM_KEYUP:
  {
    uint VirtualKeyCode = WParam;
    bool IsPressed = !(LParam & (1 << 31));
    if(IsPressed) {
      switch (VirtualKeyCode) {
      case 'W':
      {
        ScreenState.Up = true;
      }
      break;
      
      case 'A':
      {
        ScreenState.Left = true;
      }
      break;
      
      case 'S':
      {
        ScreenState.Down = true;
      }
      break;
      
      case 'D':
      {
        ScreenState.Right = true;
      }
      break;

      case VK_ESCAPE:
      {
        Running = false;
      }
      break;
      }
    }
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
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(!RegisterClass(&WindowClass)) {
    DWORD Error = GetLastError();
    //cout << "Register failed with: " << Error << endl;
  }
  
  HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 640, 480, 0, 0, Instance, 0);
  
  if(HandmadeHeroWindow == NULL) {
    DWORD Error = GetLastError();
    //cout << "Window created null or with error: " << Error << endl;
    return 0;
  }else {
    Running = true;
  }

  // Stuff for a simple animation.
  int XOffset = 0;
  int YOffset = 0;

	// Stuff for audio.
	DWORD CurrentSoundPlayCursor;
	DWORD CurrentSoundWriteCursor;
	SoundOutputConfig SoundOutput = {};
	SoundOutput.SamplePerSeconds = 48000;
	SoundOutput.BytesPerSample = sizeof(short) * 2;
	SoundOutput.SoundBufferSize = SoundOutput.SamplePerSeconds * SoundOutput.BytesPerSample;
	SoundOutput.RunningSampleIndex = 0;
	SoundOutput.ToneHertz = 261;
	SoundOutput.ToneVolume = 4000;
	SoundOutput.WavePeriod = SoundOutput.SamplePerSeconds / SoundOutput.ToneHertz;
	SoundOutput.SoundIsPlaying = false;

  // Library loads.
  LoadXInputLib();
  LoadSound(HandmadeHeroWindow, SoundOutput.SoundBufferSize, SoundOutput.SamplePerSeconds);

  // Show window.
  ResizeDIBSection(&GlobalBackbuffer, 1208, 720);
  ShowWindow(HandmadeHeroWindow, ComandShow);

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
    
    // Input handling procedure.
    for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) {
      XINPUT_STATE ControllerState;
      if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
        // Controller connected.
        // Controller input collection.
        XINPUT_GAMEPAD *GamepadState = &ControllerState.Gamepad;
        bool DpadUp = (GamepadState->wButtons & XINPUT_GAMEPAD_DPAD_UP);
        bool DpadLeft = (GamepadState->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
        bool DpadDown = (GamepadState->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
        bool DpadRight = (GamepadState->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
        bool StartButton = (GamepadState->wButtons & XINPUT_GAMEPAD_START);
        bool SelectButton = (GamepadState->wButtons & XINPUT_GAMEPAD_BACK);
        bool LeftShoulder = (GamepadState->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
        bool RightShoulder = (GamepadState->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
        bool AButton = (GamepadState->wButtons & XINPUT_GAMEPAD_A);
        bool BButton = (GamepadState->wButtons & XINPUT_GAMEPAD_B);
        bool XButton = (GamepadState->wButtons & XINPUT_GAMEPAD_X);
        bool YButton = (GamepadState->wButtons & XINPUT_GAMEPAD_Y);
        short LeftJoystickX = GamepadState->sThumbLX;
        short LeftJoystickY = GamepadState->sThumbLY;
        
      }else {
        // Controller not connected or error.
      }
    }
    
    // Procedure to output the program.
		HRESULT GetBufferPositionResult = GlobalSecondarySoundBuffer->GetCurrentPosition(&CurrentSoundPlayCursor, &CurrentSoundWriteCursor);
    if(SUCCEEDED(GetBufferPositionResult)) {
      DWORD WriteRegionOffset = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SoundBufferSize;
      DWORD WriteRegionLength;
			
      if(WriteRegionOffset == CurrentSoundPlayCursor) {
        if(SoundOutput.SoundIsPlaying) {
          WriteRegionLength = 0;
        }else {
          WriteRegionLength = SoundOutput.SoundBufferSize;
        }
      }else if(WriteRegionOffset >= CurrentSoundPlayCursor) {
				WriteRegionLength = SoundOutput.SoundBufferSize - WriteRegionOffset;
				WriteRegionLength += CurrentSoundPlayCursor;
			}else {
				WriteRegionLength = CurrentSoundPlayCursor - WriteRegionOffset;
			}
			FillSoundBuffer(&SoundOutput, WriteRegionOffset, WriteRegionLength);
    }

    if(!SoundOutput.SoundIsPlaying) {
      GlobalSecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
      SoundOutput.SoundIsPlaying = true;
		}

    RenderGrad(GlobalBackbuffer, XOffset, YOffset);
    HDC DeviceContext = GetDC(HandmadeHeroWindow);
    ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(HandmadeHeroWindow);
    DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer);

    if(ScreenState.Up) {
      YOffset += 4;
    }
    if(ScreenState.Left) {
      XOffset += 4;
    }
    if(ScreenState.Down) {
      YOffset -= 4;
    }
    if(ScreenState.Right) {
      XOffset -= 4;
    }

    ScreenState.Up = false;
    ScreenState.Left = false;
    ScreenState.Down = false;
    ScreenState.Right = false;
  }

  return 0;
}
