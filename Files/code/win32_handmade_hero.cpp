#include <math.h>
#include <stdio.h>

#define PI 3.14159265359f

#include "handmade_hero.cpp"

#include "win32_handmade.h"
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

static Win32BitmapBuffer GlobalBackbuffer;

ClientWindowDimension GetClientWindowDimension(HWND WindowHandle) {
	ClientWindowDimension Result;

	RECT ClientRect;
	GetClientRect(WindowHandle, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

/*void RenderGrad(BitmapBuffer Buffer, int32 XOffset, int32 YOffset) {

	nat8 *Row = (nat8 *)Buffer.Memory;
	for (int32 Y = 0; Y < Buffer.Height; Y++) {
		nat32 *Pixel = (nat32 *)Row;
		for (int32 X = 0; X < Buffer.Width; X++) {
			
			nat8 Red = X + XOffset;
			nat8 Green = Y + YOffset;
			nat8 Blue = 0;

			*Pixel = ((Red << 16) | (Green << 8) | Blue);
			*Pixel++;

		}
		Row += Buffer.Pitch;
	}
}*/

// Bitmap creation and manipulation.
static void ResizeDIBSection(Win32BitmapBuffer *Buffer, int32 Width, int32 Height) {

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
	int32 BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytePerPixel;
	Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	// Memory allocated and pointer stored.

	// This function have a chance to be cleared.
}

static void DisplayBuffer(HDC DeviceContext, int32 WindowWidth, int32 WindowHeight, Win32BitmapBuffer Buffer) {
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

// Load the sound library and creates sound buffers if succeed.
static void LoadSoundLib(HWND WindowHandle, int32 BufferSize, int32 SamplesPerSecond) {
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
						// ERROR CATCH.
						Error = GetLastError();
					}
				}else {
					// ERROR CATCH.
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
					// ERROR CATCH.
					Error = GetLastError();
				}
			}else {
			// ERROR CATCH.
			Error = GetLastError();
			}

		}else {
			// ERROR CATCH.
			Error = GetLastError();
		}
	}else {
		// ERROR CATCH.
		Error = GetLastError();
	}

	// BufferSize goes on secondary buffer.

	// Start playing.
}

static void ClearSoundBuffer(SoundOutputConfig *SoundOutputConfig) {
	void *FirstWriteRegionPointer;
	DWORD FirstWriteRegionLength;
	void *SecondWriteRegionPointer;
	DWORD SecondWriteRegionLength;
	HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(0, SoundOutputConfig->BufferSize, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0);
	if(SUCCEEDED(LockResult)) {
		int8 *SampleOutput = (int8 *)FirstWriteRegionPointer;
		for(nat32 ByteIndex = 0; ByteIndex < FirstWriteRegionLength; ByteIndex++) {
			*SampleOutput = 0;
		}
		SampleOutput = (int8 *)SecondWriteRegionPointer;
		for(nat32 ByteIndex = 0; ByteIndex < SecondWriteRegionLength; ByteIndex++) {
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
	HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(WriteRegionOffset, WriteRegionLength, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0);
	if(SUCCEEDED(LockResult)) {
		
		DWORD FirstRegionSampleCounter = FirstWriteRegionLength / SoundOutputConfig->BytesPerSample;
		int16 *SampleOutput = (int16 *)FirstWriteRegionPointer;
		int16 *SampleSource = SourceBuffer->SampleOut;

		for(DWORD SampleIndex = 0; SampleIndex < FirstRegionSampleCounter; SampleIndex++) {
			*SampleOutput++ = *SampleSource++;
			*SampleOutput++ = *SampleSource++;
			
			SoundOutputConfig->RunningSampleIndex++;
		}

		DWORD SecondRegionSampleCounter = SecondWriteRegionLength / SoundOutputConfig->BytesPerSample;
		SampleOutput = (int16 *)SecondWriteRegionPointer;
		
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

/* GLOBAL WINDOWS KEYBOARD STRUCT
struct KeyboardInputInfo {
	nat32 VirtualKeycode;
	bool32 WithAlt;
	bool32 WasPressed;
	bool32 IsPressed;
};

static KeyboardInputInfo KeyInput;
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

static void ProcessKeyboardButton(GamepadButtonState *NewButtonState, bool32 IsPressed) {
	NewButtonState->EndedDown = IsPressed;
	++NewButtonState->TransitionCount;
}

static void process_digital_button(WORD buttons_state, nat16 button_bitmask, GamepadButtonState *old_button_state, GamepadButtonState *new_button_state) {
	new_button_state->EndedDown = ((buttons_state & button_bitmask) == button_bitmask);
	new_button_state->TransitionCount = (old_button_state->EndedDown != new_button_state->EndedDown) ? 1 : 0;
}

static rat32 process_analogic_stick(SHORT hardware_stick_value, SHORT dead_zone_constant) {
	rat32 stick_value = 0;
	
	if(hardware_stick_value < -dead_zone_constant) {
		stick_value = ((rat32)hardware_stick_value + (rat32)dead_zone_constant) / (32768.0f - (rat32)dead_zone_constant);
	}else if(hardware_stick_value > dead_zone_constant) {
		stick_value = ((rat32)hardware_stick_value - (rat32)dead_zone_constant) / (32768.0f - (rat32)dead_zone_constant);
	}
	
	return stick_value;
}

#pragma endregion

#pragma region FILE I/O
/*
###################################################################################################
#
# FILE I/O
#
###################################################################################################
*/

static DEBUG_FileRead DEBUG_ReadFile(char *FileName) {
	DEBUG_FileRead Result = {};
	HANDLE FileHandle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE) {
		// ERROR CATCH.
		return Result;
	}

	LARGE_INTEGER FileSize;
	int32 GetSizeSuccess = GetFileSizeEx(FileHandle, &FileSize);
	if(!GetSizeSuccess) {
		// ERROR CATCH.
		return Result;
	}
	
	Result.FileSize = Truncate64bitsTo32bits(FileSize.QuadPart);
	DWORD BytesRead;
	Result.FileContent = VirtualAlloc(0, Result.FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	bool32 ReadFileSuccess = (ReadFile(FileHandle, Result.FileContent, Result.FileSize, &BytesRead, 0) && (BytesRead == Result.FileSize));
	if(ReadFileSuccess) {
		// Successfully read the file.
	}else {
		// ERROR CATCH.
		DEBUG_FreeFileMemory(Result.FileContent);
		Result = {};
	}

	CloseHandle(FileHandle);
	return Result;
}

static bool32 DEBUG_WriteFile(char *FileName, nat32 MemorySize, void *Memory) {
	bool32 Result = false;
	HANDLE FileHandle = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE) {
		// ERROR CATCH.
		Result = false;
		return Result;
	}

	DWORD BytesWritten;
	bool32 WriteFileSuccess = (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0) && (BytesWritten == MemorySize));
	if(WriteFileSuccess) {
		// Written a file successfully.
		Result = true;
	}else {
		// ERROR CATCH.
		Result = false;
	}
	CloseHandle(FileHandle);
	return Result;
}

static void DEBUG_FreeFileMemory(void *Memory) {
	VirtualFree(Memory, 0, MEM_RELEASE);
}

#pragma endregion

#pragma region WINDOW
/*
###################################################################################################
#
# WINDOW
#
###################################################################################################
*/

static bool32 DEBUG_GlobalPause;

// Window procedure to messages.
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	LRESULT Result = 0;
	switch (Message) {
		case WM_SIZE: {
			InvalidateRect(Window, NULL, true);
		}break;

		case WM_DESTROY: {
			// TODO: Error handle with window recreation?
			PostQuitMessage(0); 
		}break;
		
		case WM_PAINT: {
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);

			ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(Window);
			DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer);

			EndPaint(Window, &Paint);
		}break;

		case WM_CLOSE: {
			/*
			if(MessageBox(Window, "Get Out?", "Close the game?", MB_OKCANCEL) == IDOK) {
			DestroyWindow(Window);
			}*/ 
			DestroyWindow(Window);
			//cout << "Close" << endl;
		}break;

		case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYUP: case WM_SYSKEYDOWN: {
			Assert(!"Keyboard input passed through message dispatch.");
			/*
			KeyInput.VirtualKeycode = (nat32)WParam;
			KeyInput.WithAlt = (LParam & (1 << 29));
			KeyInput.WasPressed = (LParam & (1 << 30));
			KeyInput.IsPressed = !(LParam & (1 << 31));
			if(KeyInput.IsPressed && (KeyInput.VirtualKeycode == VK_ESCAPE)) {
				Running = false;
			}
			*/
		}break;

		default: {
			Result = DefWindowProc(Window, Message, WParam, LParam);
		}break;
	}
	return Result;
}

static void WindowMsg(GameKeyboardState KeyInput, gamepad_controller_input *KeyboardController) {
	MSG Message;
	while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
		// Message handling
		switch(Message.message) {
			case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYUP: case WM_SYSKEYDOWN: {
				KeyInput.VirtualKeycode = (nat32)Message.wParam;
				KeyInput.WithAlt = (Message.lParam & (1 << 29));
				KeyInput.WasPressed = (Message.lParam & (1 << 30));
				KeyInput.IsPressed = !(Message.lParam & (1 << 31));
				
				if(KeyInput.WasPressed != KeyInput.IsPressed) {
					switch(KeyInput.VirtualKeycode) {
						case 'W': {
							ProcessKeyboardButton(&KeyboardController->LeftStickUp, KeyInput.IsPressed);
						}break;
						
						case 'A': {
							ProcessKeyboardButton(&KeyboardController->LeftStickLeft, KeyInput.IsPressed);
						}break;
						
						case 'S': {
							ProcessKeyboardButton(&KeyboardController->LeftStickDown, KeyInput.IsPressed);
						}break;
						
						case 'D': {
							ProcessKeyboardButton(&KeyboardController->LeftStickRight, KeyInput.IsPressed);
						}break;
						
						case 'J': {
							ProcessKeyboardButton(&KeyboardController->AButton, KeyInput.IsPressed);
						}break;
	
						case 'K': {
							ProcessKeyboardButton(&KeyboardController->BButton, KeyInput.IsPressed);
						}break;
	
						case 'U': {
							ProcessKeyboardButton(&KeyboardController->XButton, KeyInput.IsPressed);
						}break;
	
						case 'I': {
							ProcessKeyboardButton(&KeyboardController->YButton, KeyInput.IsPressed);
						}break;
						
						case VK_UP: {
							ProcessKeyboardButton(&KeyboardController->DpadUp, KeyInput.IsPressed);
						}break;
						
						case VK_LEFT: {
							ProcessKeyboardButton(&KeyboardController->DpadLeft, KeyInput.IsPressed);
						}break;
						
						case VK_DOWN: {
							ProcessKeyboardButton(&KeyboardController->DpadDown, KeyInput.IsPressed);
						}break;
						
						case VK_RIGHT: {
							ProcessKeyboardButton(&KeyboardController->DpadRight, KeyInput.IsPressed);
						}break;
	
						case VK_ESCAPE: {
							if(KeyInput.IsPressed) {
								Running = false;
							}
						}break;
					}
				}
				
				if(KeyInput.VirtualKeycode == 'P') {
					if(KeyInput.IsPressed) {
						DEBUG_GlobalPause = !DEBUG_GlobalPause;
					}
				}
			}break;

			case WM_QUIT: {
				Running = false;
			}break;

			default: {
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}break;
		}
	}
}

static int64 CountFrequency;

inline LARGE_INTEGER GetSystemTimeStamp() {
	LARGE_INTEGER TimeStamp;
	QueryPerformanceCounter(&TimeStamp);
	return TimeStamp;
}

inline rat32 GetMilisecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End) {
	return (rat32)((1000.0f * (End.QuadPart - Start.QuadPart)) / CountFrequency);
}

static void DEBUG_DrawVerticalLine(Win32BitmapBuffer *Backbuffer, int X, int Top, int Bottom, nat32 Color) {
	
	nat8 *Pixel = ((nat8 *)(Backbuffer->Memory) + (X * Backbuffer->BytePerPixel) + (Top * Backbuffer->Pitch));
	
	for(int Y = Top; Y < Bottom; Y++) {
		*(nat32 *)Pixel = Color;
		Pixel += Backbuffer->Pitch;
	}
}

static void DEBUG_DisplayAudioImageSync(Win32BitmapBuffer *Backbuffer, DEBUG_SoundCursorMarkers *CursorMarkers, int CurrentMarkerIndex, int CursorMarkerArraySize, SoundOutputConfig *SoundConfig) {
	#define AssertLess(DEBUG_VAR) Assert(CursorMarkers[MarkerIndex].DEBUG_VAR < (DWORD)(SoundConfig->BufferSize))
	
	int XPadding = 16;
	int YPadding = 16;
	int LineHeight = 64;
	
	rat32 Coefficient = ((rat32)(Backbuffer->Width - (XPadding * 2)) / (rat32)(SoundConfig->BufferSize));
	
	for(int MarkerIndex = 0; MarkerIndex < CursorMarkerArraySize; MarkerIndex++) {
		AssertLess(DEBUG_FlipPlayCursor);
		AssertLess(DEBUG_FlipWriteCursor);
		AssertLess(DEBUG_OutputPlayCursor);
		AssertLess(DEBUG_OutputWriteCursor);
		AssertLess(DEBUG_OffsetLocked);
		AssertLess(DEBUG_TargetCursor);
		
		int FlipPlayX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_FlipPlayCursor)) + XPadding);
		int FlipWriteX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_FlipWriteCursor)) + XPadding);
		
		nat32 PlayColor = 0xFFFFFFFF;
		nat32 WriteColor = 0xFF00FFFF;
		nat32 JitterColor = 0xFFFF00FF;
		nat32 FrameBoundaryColor = 0xFF000000;
		
		int Top = YPadding;
		int Bottom = YPadding + LineHeight; // (Backbuffer->Height - YPadding);
		
		DEBUG_DrawVerticalLine(Backbuffer, FlipPlayX, Top, Bottom, PlayColor);
		DEBUG_DrawVerticalLine(Backbuffer, FlipWriteX, Top, Bottom, WriteColor);
		if(MarkerIndex == CurrentMarkerIndex) {
			int OutPlayX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_OutputPlayCursor)) + XPadding);
			int OutWriteX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_OutputWriteCursor)) + XPadding);
			Top += LineHeight + YPadding;
			Bottom += LineHeight + YPadding;
			int FirstTop = Top;
			DEBUG_DrawVerticalLine(Backbuffer, OutPlayX, Top, Bottom, PlayColor);
			DEBUG_DrawVerticalLine(Backbuffer, OutWriteX, Top, Bottom, WriteColor);
			
			int LockX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_OffsetLocked)) + XPadding);
			int TargetX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_TargetCursor)) + XPadding);
			Top += LineHeight + YPadding;
			Bottom += LineHeight + YPadding;
			DEBUG_DrawVerticalLine(Backbuffer, LockX, Top, Bottom, PlayColor);
			DEBUG_DrawVerticalLine(Backbuffer, TargetX, Top, Bottom, WriteColor);
			
			int JitterX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_FlipPlayCursor + (480 * SoundConfig->BytesPerSample))) + XPadding);
			Top += LineHeight + YPadding;
			Bottom += LineHeight + YPadding;
			DEBUG_DrawVerticalLine(Backbuffer, FlipPlayX, Top, Bottom, PlayColor);
			DEBUG_DrawVerticalLine(Backbuffer, FlipWriteX, Top, Bottom, WriteColor);
			DEBUG_DrawVerticalLine(Backbuffer, JitterX, Top, Bottom, JitterColor);
			
			int FrameX = ((int)(Coefficient * (rat32)(CursorMarkers[MarkerIndex].DEBUG_FrameBoundaryTarget)) + XPadding);
			DEBUG_DrawVerticalLine(Backbuffer, FrameX, FirstTop, Bottom, FrameBoundaryColor);
		}
	}
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
		// Class not initialized, ERROR CATCH.
	}

	HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1472, 828, 0, 0, Instance, 0);

	if(HandmadeHeroWindow) {
		Running = true;
	}else {
		// Window not created, ERROR CATCH.
		return 0;
	}

	// Performance counter.
	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	CountFrequency = PerformanceFrequency.QuadPart;
	LARGE_INTEGER LastCount = GetSystemTimeStamp();
	#define DisplayRefreshRate 120 // in HZ. TODO: Actually get the device refresh rate.
	#define GameRefreshRate (DisplayRefreshRate / 2)
	rat32 TargetMSPerFrame = 1000.0f / GameRefreshRate;
	
	nat64 LastCycleCount = __rdtsc();
	// "CycleCount" refers to exact CPU cycles, while performance count is more about real time.
	
	// Setting the CPU scheduler granularity to 1ms.
	nat32 TargetSchedulerGranularity = 1;
	bool32 SleepIsPrecise = (timeBeginPeriod(TargetSchedulerGranularity) == TIMERR_NOERROR);
	
	// Memory setup.
	LPVOID BaseAddress = (LPVOID)(2 TB); // TODO: Remove on final build.
	GameMemory GameMemory = {};
	GameMemory.PermanentSize = 64 MB;
	GameMemory.VolatileSize = 4 GB;
	nat64 TotalSize = GameMemory.PermanentSize + GameMemory.VolatileSize;
	GameMemory.PermanentPtr = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	GameMemory.VolatilePtr = ((nat8 *)GameMemory.PermanentPtr + GameMemory.PermanentSize);
	
	// Render setup.
	ResizeDIBSection(&GlobalBackbuffer, 1208, 720);
	ShowWindow(HandmadeHeroWindow, ComandShow);

	// Sound setup.
	SoundOutputConfig SoundConfig = {};
	SoundConfig.SamplePerSeconds = 48000;
	SoundConfig.BytesPerSample = sizeof(int16) * 2;
	SoundConfig.BytesPerSeconds = SoundConfig.SamplePerSeconds * SoundConfig.BytesPerSample; // 192000 bytes
	SoundConfig.BufferSeconds = 1;
	SoundConfig.BufferSize = SoundConfig.BytesPerSeconds * SoundConfig.BufferSeconds;
	SoundConfig.SafetyMargin = (SoundConfig.BytesPerSeconds / GameRefreshRate) * 8 / 10; // TODO: Compute the actual safety margin is, that is the lowest reasonable value in which sound and frame jitter.
	SoundConfig.SampleCount = SoundConfig.BufferSize / SoundConfig.BytesPerSample; // This should be 48000 samples
	SoundConfig.ChunkSize = SoundConfig.BytesPerSeconds / 20;
	SoundConfig.ChunkCount = SoundConfig.BufferSize / SoundConfig.ChunkSize;
	SoundConfig.ChunkIndex = 0;
	SoundConfig.LastChunk = 0;
	SoundConfig.RunningSampleIndex = 0;
	SoundConfig.SoundIsPlaying = false;
	DWORD SoundBytesPerFrame = SoundConfig.BytesPerSeconds / GameRefreshRate;
	bool32 ValidSound = false;

	int16 *SoundBufferPointer = (int16 *)VirtualAlloc(NULL, SoundConfig.BufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	LoadSoundLib(HandmadeHeroWindow, SoundConfig.BufferSize, SoundConfig.SamplePerSeconds);
	ClearSoundBuffer(&SoundConfig);

	// Input setup.
	LoadXInputLib();

	GameKeyboardState KeyInput = {};
	
	gamepad_input input_[2] = {0, 0};
	gamepad_input *new_input = &input_[0];
	gamepad_input *old_input = &input_[1];
	
	// Allocation safety check (perhaps put memory allocation here?).
	if(SoundBufferPointer && GameMemory.PermanentPtr && GameMemory.VolatilePtr) {
		Running = true;
	}else {
		// TODO: ERROR CATCH?
	}
	
	int DEBUG_MarkerIndex = 0;
	DEBUG_SoundCursorMarkers DEBUG_LastSCMarkers[GameRefreshRate / 2] = {};
	
	#pragma region RUNNING
	// While loop controled by a bool to keep the program running, because `PeekMessage` gets outta the loop when there are no messages.
	while(Running) {
		
		gamepad_controller_input *NewKeyboardController = &new_input->gamepad_controller[0];
		gamepad_controller_input *OldKeyboardController = &old_input->gamepad_controller[0];
		*NewKeyboardController = {};
		NewKeyboardController->IsConnected = true;
		for(int32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->GamepadButton); ButtonIndex++) {
			NewKeyboardController->GamepadButton[ButtonIndex].EndedDown = OldKeyboardController->GamepadButton[ButtonIndex].EndedDown;
		}
		
		WindowMsg(KeyInput, NewKeyboardController);
		
		nat32 max_controller_count = XUSER_MAX_COUNT;
		if(max_controller_count > ArrayCount(new_input->gamepad_controller) - 1) {
			max_controller_count = ArrayCount(new_input->gamepad_controller) - 1;
		}
		for(DWORD controller_index = 0; controller_index < max_controller_count; controller_index++) {
			XINPUT_STATE controller_state;
			DWORD TrueControllerIndex = controller_index + 1;
			gamepad_controller_input *new_controller = GetController(new_input, TrueControllerIndex);
			gamepad_controller_input *old_controller = GetController(old_input, TrueControllerIndex);

			if(XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
				// Colntroller connected.
				new_controller->IsConnected = true;
				// Controller input collection.
				XINPUT_GAMEPAD *gamepad_state = &controller_state.Gamepad;
				
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_DPAD_UP, &old_controller->DpadUp, &new_controller->DpadUp);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_DPAD_LEFT, &old_controller->DpadLeft, &new_controller->DpadLeft);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &old_controller->DpadDown, &new_controller->DpadDown);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &old_controller->DpadRight, &new_controller->DpadRight);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_A, &old_controller->AButton, &new_controller->AButton);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_B, &old_controller->BButton, &new_controller->BButton);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_X, &old_controller->XButton, &new_controller->XButton);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_Y, &old_controller->YButton, &new_controller->YButton);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &old_controller->LeftShoulder, &new_controller->LeftShoulder);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &old_controller->RightShoulder, &new_controller->RightShoulder);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_START, &old_controller->StartButton, &new_controller->StartButton);
				process_digital_button(gamepad_state->wButtons, XINPUT_GAMEPAD_BACK, &old_controller->SelectButton, &new_controller->SelectButton);
				
				new_controller->is_analog = true;
				// TODO: Confirm round dead zone, and prepare to do a vectoring algorithm if so.
				new_controller->left_stick_average_x = process_analogic_stick(gamepad_state->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				new_controller->left_stick_average_y = process_analogic_stick(gamepad_state->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				new_controller->right_stick_average_x = process_analogic_stick(gamepad_state->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				new_controller->right_stick_average_y = process_analogic_stick(gamepad_state->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				
				rat32 stick_action_threshold = 0.6f;
				
				process_digital_button((new_controller->left_stick_average_y > stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickUp, &new_controller->LeftStickUp);
				process_digital_button((new_controller->left_stick_average_x < -stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickLeft, &new_controller->LeftStickLeft);
				process_digital_button((new_controller->left_stick_average_y < -stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickDown, &new_controller->LeftStickDown);
				process_digital_button((new_controller->left_stick_average_x > stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickRight, &new_controller->LeftStickRight);
				
				/*
				NOTE: Casey has a strat for the systematic design of the game.
				DPad and analogic stick relates to the same thing, so DPad should deactivate the analogic state and let a smoothing algorithm to be performed. So the analogic state should not be active until the analogic is in use.
				The problem the consideration for purely control gameplay, with Casey passing actions, but i pass buttons instead.
				*/
			}else {
				// Controller not connected or error.
				new_controller->IsConnected = false;
			}
		}
		
		if(!DEBUG_GlobalPause) {
		BitmapBuffer GameBuffer = {};
		GameBuffer.Memory = GlobalBackbuffer.Memory;
		GameBuffer.Width = GlobalBackbuffer.Width;
		GameBuffer.Height = GlobalBackbuffer.Height;
		GameBuffer.BytePerPixel = GlobalBackbuffer.BytePerPixel;
		GameBuffer.Pitch = GlobalBackbuffer.Pitch;
		
		GameUpdate(&GameMemory, &GameBuffer, &KeyInput, new_input);
		
		/* NOTE:
		SOUND BUFFER PLAN:
		- Get the cursors position.
		- From the play cursor, estimate where the frame flip will happen relative to the sound buffer.
		- The frame flip margin is a safety value before the actual frame flip, that value is a estimative of how much the audio varies relative to the game loop.
		- Compare the write cursor with the margin.
		- If the write cursor is before the margin:
			- Calculate the target cursor to be on the next margin.
			- Write from the write cursor up to the target cursor.
		- If the write cursor is after the margin:
			- Calculate the target cursor to be one frame worth of samples ahead from the write cursor, plus the safety margin.
			- Write from the write cursor up to the target cursor.
		*/
		
		// WARNING: Sound logic will be remade for the new frame loop.
		
		DWORD SoundPlayCursor = 0;
		DWORD SoundWriteCursor = 0;
		DWORD WriteRegionOffset = 0;
		DWORD WriteRegionLength = 0;
		LARGE_INTEGER TimeStampUpToAudio = GetSystemTimeStamp();
		HRESULT GetBufferPositionResult = GlobalSecondarySoundBuffer->GetCurrentPosition(&SoundPlayCursor, &SoundWriteCursor);
		if(GetBufferPositionResult == DS_OK) {
			/*
			SoundConfig.ChunkIndex = SoundWriteCursor / SoundConfig.ChunkSize;
			WriteRegionLength = SoundConfig.ChunkSize;
			
			DWORD ChunkToWrite = (SoundConfig.ChunkIndex + 1) % SoundConfig.ChunkCount;
			
			if(ChunkToWrite != SoundConfig.LastChunk) {
				WriteRegionOffset = ChunkToWrite * SoundConfig.ChunkSize;
				SoundConfig.LastChunk = ChunkToWrite;
				ValidSound = true;
			}
			*/
			
			// Calculate WriteRegionOffset.
			if(!ValidSound) {
				SoundConfig.RunningSampleIndex = SoundWriteCursor / SoundConfig.BytesPerSample;
				ValidSound = true;
			}
			WriteRegionOffset = (SoundConfig.RunningSampleIndex * SoundConfig.BytesPerSample) % SoundConfig.BufferSize;
			
			// Calculate latency.
			rat32 TimeElapsedUpToAudio = GetMilisecondsElapsed(LastCount, TimeStampUpToAudio);
			rat32 TimeRemainingForThisFrame = TargetMSPerFrame - TimeElapsedUpToAudio;
			DWORD SoundBytesRemainingForThisFrame = (DWORD)((TimeRemainingForThisFrame / TargetMSPerFrame) * (rat32)(SoundBytesPerFrame));
			DWORD FrameBoundaryTarget = SoundPlayCursor + SoundBytesRemainingForThisFrame;
			DWORD UnwrappedWriteCursor = SoundWriteCursor;
			if(UnwrappedWriteCursor < SoundPlayCursor) {
				UnwrappedWriteCursor += SoundConfig.BufferSize;
			}
			bool32 SoundHasLowLatency = (UnwrappedWriteCursor < FrameBoundaryTarget);
			
			// Calculate TargetCursor.
			nat32 TargetCursor = 0;
			if(SoundHasLowLatency) {
				// Frame synced audio.
				TargetCursor = FrameBoundaryTarget + SoundBytesPerFrame;
			}else {
				// Output sound ASAP.
				TargetCursor = SoundWriteCursor + SoundBytesPerFrame + SoundConfig.SafetyMargin;
			}
			TargetCursor = TargetCursor % SoundConfig.BufferSize;
			
			// Calculate WriteRegionLength.
			if(WriteRegionOffset > TargetCursor) {
				WriteRegionLength = (SoundConfig.BufferSize - WriteRegionOffset) + TargetCursor;
			}else {
				WriteRegionLength = TargetCursor - WriteRegionOffset;
			}
			
			// DEBUG DRAWING SOUND MARKERS
			DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_OutputPlayCursor = SoundPlayCursor;
			DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_OutputWriteCursor = SoundWriteCursor;
			DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_OffsetLocked = WriteRegionOffset;
			DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_TargetCursor = TargetCursor;
			DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_FrameBoundaryTarget = FrameBoundaryTarget % SoundConfig.BufferSize;
			/*
			The frame flip prediction seems to be roughly correct, with only sometimes the flip mark being considerably before the after flip play cursor. Most of the time they are somewhat together, with the flip mark just a little bit behind, but the flip marker is never ahead the after flip play cursor.
			*/
		}else {
			ValidSound = false;
		}
		
		rat32 DEBUG_CursorDifference;
		if(SoundWriteCursor < SoundPlayCursor) {
			DEBUG_CursorDifference = (rat32)(SoundWriteCursor + SoundConfig.BufferSize) - (rat32)(SoundPlayCursor);
		}else {
			DEBUG_CursorDifference = (rat32)(SoundWriteCursor) - (rat32)(SoundPlayCursor);
		}
		rat32 DEBUG_SoundLatency = ((DEBUG_CursorDifference / (rat32)(SoundConfig.BytesPerSeconds)) * 1000.0f);
		int32 DEBUG_SampleLatency = (int32)(DEBUG_CursorDifference / (rat32)(SoundConfig.BytesPerSample));
		
		SoundBuffer GameSoundBuffer = {};
		GameSoundBuffer.SamplesPerSecond = SoundConfig.SamplePerSeconds;
		GameSoundBuffer.SampleCount = WriteRegionLength / SoundConfig.BytesPerSample;
		GameSoundBuffer.SampleOut = SoundBufferPointer;
		GameSoundBuffer.LastByte = SoundBufferPointer + (int16)SoundConfig.BufferSize;
		GameSoundBuffer.SampleIndex = SoundConfig.RunningSampleIndex;
		GameSoundBuffer.ReadyToWrite = ValidSound;
		
		GameSoundOutput(&GameMemory, &GameSoundBuffer);
		
		rat32 TimeComputingInMiliseconds = GetMilisecondsElapsed(LastCount, GetSystemTimeStamp());
		
		rat32 TotalTimeElapsedInMiliseconds = TimeComputingInMiliseconds;
		if(TotalTimeElapsedInMiliseconds < TargetMSPerFrame) {
			while(TotalTimeElapsedInMiliseconds < TargetMSPerFrame) {
				if(SleepIsPrecise) {
					DWORD SleepTime = (DWORD)(TargetMSPerFrame - TotalTimeElapsedInMiliseconds - 1.0f);
					if(SleepTime > 0) {
						Sleep(SleepTime);
					}
				}
				TotalTimeElapsedInMiliseconds = GetMilisecondsElapsed(LastCount, GetSystemTimeStamp());
			}
		}else {
			// FRAME MISSED!
			// ERROR CATCH!
		}
		
		LastCount = GetSystemTimeStamp();
		rat32 FPS = 1.0f / (TotalTimeElapsedInMiliseconds / 1000.0f);
		
		HDC DeviceContext = GetDC(HandmadeHeroWindow);
		ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(HandmadeHeroWindow);
		int DEBUG_CurrentMarkerIndex = DEBUG_MarkerIndex - 1; // This is wrong for the index zero.
		DEBUG_DisplayAudioImageSync(&GlobalBackbuffer, DEBUG_LastSCMarkers, DEBUG_CurrentMarkerIndex, ArrayCount(DEBUG_LastSCMarkers), &SoundConfig);
		DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer);
		
		// DEBUG AUDIO MARKERS
		GlobalSecondarySoundBuffer->GetCurrentPosition(&DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_FlipPlayCursor, &DEBUG_LastSCMarkers[DEBUG_MarkerIndex].DEBUG_FlipWriteCursor);
		DEBUG_MarkerIndex++;
		if(DEBUG_MarkerIndex >= ArrayCount(DEBUG_LastSCMarkers)) {
			DEBUG_MarkerIndex = 0;
		}
		Assert(DEBUG_MarkerIndex < ArrayCount(DEBUG_LastSCMarkers));

		if(ValidSound) {
			FillSoundBuffer(&SoundConfig, WriteRegionOffset, WriteRegionLength, &GameSoundBuffer);
		}

		if(!SoundConfig.SoundIsPlaying) {
			GlobalSecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
			SoundConfig.SoundIsPlaying = true;
		}
		
		gamepad_input *temp_ = new_input;
		new_input = old_input;
		old_input = temp_;

		// END PROCESS ########################################################################################

		// Performance counting and display.
		
		nat64 EndCycleCount = __rdtsc();
		nat64 CyclesPassed = EndCycleCount - LastCycleCount;
		rat32 MegaCyclesPerFrame = (rat32)(CyclesPassed) / (1000.0f * 1000.0f);
		LastCycleCount = EndCycleCount;
		
		DEBUG_AUDIO
		}
	}
	timeEndPeriod(TargetSchedulerGranularity);
	return 0;
}
#pragma endregion
#pragma endregion