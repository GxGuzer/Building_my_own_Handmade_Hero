#include <math.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#define PI 3.14159265359f

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

#include "handmade_hero.cpp"

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <dsound.h>
//TODO: Make a console "catch" for errors.
//TODO: Make a debug console with iostream.

bool Running = false;

#pragma region RENDER
/*
###################################################################################################
#
# RENDER
#
###################################################################################################
*/

// Bitmap variables and functions.
struct Win32BitmapBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int BytePerPixel;
  int Pitch;
};

static Win32BitmapBuffer GlobalBackbuffer;

struct ClientWindowDimension {
  int Width;
  int Height;
};

short PixelSoundSample;

ClientWindowDimension GetClientWindowDimension(HWND WindowHandle) {
  ClientWindowDimension Result;

  RECT ClientRect;
  GetClientRect(WindowHandle, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return Result;
}

/*void RenderGrad(BitmapBuffer Buffer, int XOffset, int YOffset) {

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
}*/

// Bitmap creation and manipulation.
static void ResizeDIBSection(Win32BitmapBuffer *Buffer, int Width, int Height) {

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

static void DisplayBuffer(HDC DeviceContext, int WindowWidth, int WindowHeight, Win32BitmapBuffer Buffer) {
  StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer.Width, Buffer.Height, Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}
#pragma endregion

#pragma region SOUND
/*
###################################################################################################
#
# SOUND
#
###################################################################################################
*/

LPDIRECTSOUNDBUFFER GlobalSecondarySoundBuffer;

typedef HRESULT WINAPI MyDirectSoundCreateFunction(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

// Load the sound library and creates sound buffers if succeed.
static void LoadSoundLib(HWND WindowHandle, int BufferSize, int SamplesPerSecond) {
  // TODO: Make the first attempt of sound be with the modern sound API (XAudio2).

  DWORD Error;

  HMODULE SoundLibLoad = LoadLibrary("dsound.dll");
  if(SoundLibLoad) {
    MyDirectSoundCreateFunction *MyDirectSoundCreate = (MyDirectSoundCreateFunction *)GetProcAddress(SoundLibLoad, "DirectSoundCreate");

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
  int BytesPerSeconds;
  int SoundBufferSeconds;
  int SoundBufferSize;
  uint RunningSampleIndex;
  int ToneHertz;
  short ToneVolume;
  int WavePeriod;
  bool SoundIsPlaying;
};

static void ClearSoundBuffer(SoundOutputConfig *SoundOutputConfig) {
  void *FirstWriteRegionPointer;
  DWORD FirstWriteRegionLength;
  void *SecondWriteRegionPointer;
  DWORD SecondWriteRegionLength;
  HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(0, SoundOutputConfig->SoundBufferSize, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0);
  if(SUCCEEDED(LockResult)) {
    char *SampleOutput = (char *)FirstWriteRegionPointer;
    for(int ByteIndex = 0; ByteIndex < FirstWriteRegionLength; ByteIndex++) {
      *SampleOutput = 0;
    }
    SampleOutput = (char *)SecondWriteRegionPointer;
    for(int ByteIndex = 0; ByteIndex < SecondWriteRegionLength; ByteIndex++) {
      *SampleOutput = 0;
    }
    GlobalSecondarySoundBuffer->Unlock(FirstWriteRegionPointer, FirstWriteRegionLength, SecondWriteRegionPointer, SecondWriteRegionLength);
  }
}

static void FillSoundBuffer(SoundOutputConfig *SoundOutputConfig, DWORD WriteRegionOffset, DWORD WriteRegionLength, SoundBuffer *SourceBuffer) {

  void *FirstWriteRegionPointer;
  DWORD FirstWriteRegionLength;
  void *SecondWriteRegionPointer;
  DWORD SecondWriteRegionLength;
  HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(WriteRegionOffset, WriteRegionLength, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0); // Lock is returning an error: E_INVALIDARG
  if(SUCCEEDED(LockResult)) {
    
    DWORD FirstRegionSampleCounter = FirstWriteRegionLength / SoundOutputConfig->BytesPerSample;
    short *SampleOutput = (short *)FirstWriteRegionPointer;
    short *SampleSource = SourceBuffer->SampleOut;

    for(DWORD SampleIndex = 0; SampleIndex < FirstRegionSampleCounter; SampleIndex++) {
      *SampleOutput++ = *SampleSource++;
      *SampleOutput++ = *SampleSource++;
      
      SoundOutputConfig->RunningSampleIndex++;
    }

    DWORD SecondRegionSampleCounter = SecondWriteRegionLength / SoundOutputConfig->BytesPerSample;
    SampleOutput = (short *)SecondWriteRegionPointer;
    
    for(DWORD SampleIndex = 0; SampleIndex < SecondRegionSampleCounter; SampleIndex++) {
      *SampleOutput++ = *SampleSource++;
      *SampleOutput++ = *SampleSource++;
      
      SoundOutputConfig->RunningSampleIndex++;
    }
    
    GlobalSecondarySoundBuffer->Unlock(FirstWriteRegionPointer, FirstWriteRegionLength, SecondWriteRegionPointer, SecondWriteRegionLength);
    // OutputDebugString("Buffer filled.\n");
  }else {
    // OutputDebugString("Buffer failed.\n");
  }
}
#pragma endregion

#pragma region INPUT
/*
###################################################################################################
#
# INPUT
#
###################################################################################################
*/

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

void *LoadFile(char *FileName) {
  return 0;
}

// Struct with the "player state".
struct {
  bool Up;
  bool Left;
  bool Down;
  bool Right;
} ScreenState;
#pragma endregion

#pragma region WINDOW
/*
###################################################################################################
#
# WINDOW
#
###################################################################################################
*/

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
#pragma endregion

#pragma region MAIN
/*
###################################################################################################
#
# MAIN
#
###################################################################################################
*/

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
  
  // Performance counter.
  LARGE_INTEGER PerformanceFrequency;
  QueryPerformanceFrequency(&PerformanceFrequency);
  llong CountFrequency = PerformanceFrequency.QuadPart;

  ullong LastCycleCount = __rdtsc();

  LARGE_INTEGER LastCount;
  QueryPerformanceCounter(&LastCount);
  // "CycleCount" refers to exact CPU cycles, while performance count is more about real time.

  // Stuff for a simple animation.
  int XOffset = 0;
  int YOffset = 0;

	// Stuff for audio.
	DWORD CurrentSoundPlayCursor;
	DWORD CurrentSoundWriteCursor;
	SoundOutputConfig SoundOutput = {};
	SoundOutput.SamplePerSeconds = 48000;
	SoundOutput.BytesPerSample = sizeof(short) * 2;
  SoundOutput.BytesPerSeconds = SoundOutput.SamplePerSeconds * SoundOutput.BytesPerSample; // 192000 bytes
  SoundOutput.SoundBufferSeconds = 1;
	SoundOutput.SoundBufferSize = SoundOutput.BytesPerSeconds * SoundOutput.SoundBufferSeconds;
	SoundOutput.RunningSampleIndex = 0;
	SoundOutput.ToneHertz = 261;
	SoundOutput.ToneVolume = 4000;
	SoundOutput.WavePeriod = SoundOutput.SamplePerSeconds / SoundOutput.ToneHertz;
	SoundOutput.SoundIsPlaying = false;
  int SoundBufferChunkSize = SoundOutput.BytesPerSeconds / 20;
  int SoundChunkCount = SoundOutput.SoundBufferSize / SoundBufferChunkSize;
  int CurrentChunkIndex;
  int LastWrittenChunk = 0;

  short *SoundBufferPointer = (short *)VirtualAlloc(NULL, SoundOutput.SoundBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  // Library loads.
  LoadXInputLib();
  LoadSoundLib(HandmadeHeroWindow, SoundOutput.SoundBufferSize, SoundOutput.SamplePerSeconds);
  ClearSoundBuffer(&SoundOutput);

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
    
    /*
    SOUND BUFFER PLAN:
    To write a chunk from the WRITE CURSOR or maybe a offset from it, have two variables to hold the last written boundary.
    Check if the WRITE CURSOR is before the lower boundary AND after the higher boundary, all in modulo with BUFFER SIZE to gurantee the wrap around.
    On writing, the lower boundary should be set to WRITE CURSOR (or an offset from it),
    and the higher boundary should be set to WRITE CURSOR (or an offset from it) = WRITE SIZE
    ```cpp
    if(WRITE_CURSOR < LOW_BOUNDARY && WRITE_CURSOR > HIGH_BOUNDARY) {
      ready_to_write = true;
      LOW_BOUNDARY = (WRITE_CURSOR + offset) % BUFFER_SIZE;
      HIGH_BOUNDARY = (WRITE_CURSOR + WRITE_SIZE) % BUFFER_SIZE;
    }
    // Probably put LOW_BOUNDARY = BUFFER_SIZE and HIGH_BOUNDARY = 0 to make the whole buffer valid at first.
    ```
    */
    
    // Procedure to output the program.
    
    bool ValidSound = false;
    DWORD WriteRegionOffset = 0;
    DWORD WriteRegionLength = 0;
    // TODO: Have a system to presume how far ahead of the result we are at the GameMain time.
    HRESULT GetBufferPositionResult = GlobalSecondarySoundBuffer->GetCurrentPosition(&CurrentSoundPlayCursor, &CurrentSoundWriteCursor);
    if(SUCCEEDED(GetBufferPositionResult)) {
      CurrentChunkIndex = CurrentSoundWriteCursor / SoundBufferChunkSize;
      WriteRegionLength = SoundBufferChunkSize;

      DWORD ChunkToWrite = (CurrentChunkIndex + 1) % SoundChunkCount;
      
      if(ChunkToWrite != LastWrittenChunk) {
        WriteRegionOffset = ChunkToWrite * SoundBufferChunkSize;
        LastWrittenChunk = ChunkToWrite;
        ValidSound = true;
      }
    }
    
    SoundBuffer GameSoundBuffer = {};
    GameSoundBuffer.SamplesPerSecond = SoundOutput.SamplePerSeconds;
    GameSoundBuffer.SampleCount = WriteRegionLength / SoundOutput.BytesPerSample;
    GameSoundBuffer.SampleOut = SoundBufferPointer;
    GameSoundBuffer.ReadyToWrite = ValidSound;
    
    BitmapBuffer GameBuffer = {};
    GameBuffer.Memory = GlobalBackbuffer.Memory;
    GameBuffer.Width = GlobalBackbuffer.Width;
    GameBuffer.Height = GlobalBackbuffer.Height;
    GameBuffer.BytePerPixel = GlobalBackbuffer.BytePerPixel;
    GameBuffer.Pitch = GlobalBackbuffer.Pitch;
    
    GameMain(&GameBuffer, XOffset, YOffset, &GameSoundBuffer);
    
    if(ValidSound) {
      FillSoundBuffer(&SoundOutput, WriteRegionOffset, WriteRegionLength, &GameSoundBuffer);
    }

    if(!SoundOutput.SoundIsPlaying) {
      GlobalSecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
      SoundOutput.SoundIsPlaying = true;
		}
    
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

    // Performance counting and display.

    // TODO: Log the timing of each critical progress (render, audio, input).

    ullong EndCycleCount = __rdtsc();
    ullong CyclesPassed = EndCycleCount - LastCycleCount;
    float MegaCyclesPerFrame = (float)(CyclesPassed) / (1000.0f * 1000.0f);

    LARGE_INTEGER EndCount;
    QueryPerformanceCounter(&EndCount);
    llong CountPassed = EndCount.QuadPart - LastCount.QuadPart;
    float MSPerFrame = (1000.0f * (float)(CountPassed)) / (float)(CountFrequency);
    int FPS = (int)(CountFrequency / CountPassed);
    
    char StringBuffer[256];
    sprintf(StringBuffer, "Render Time: %.03fms. FPS: %d. CPU Cycles: %.03fM.\n", MSPerFrame, FPS, MegaCyclesPerFrame); // WARNNG: This type of string outputting is problematic, it assumes a long enough buffer and the formats may access what it shouldn't on the stack.
    // OutputDebugString(StringBuffer);
    
    LastCycleCount = EndCycleCount;
    LastCount = EndCount;
  }

  return 0;
}
#pragma endregion