#include <math.h>
#include <stdio.h>

#define PI 3.14159265359f

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

// Stuff for audio.
struct SoundOutputConfig {
	int SamplePerSeconds;
	int BytesPerSample;
	int BytesPerSeconds;
	int BufferSeconds;
	int BufferSize;
	int ChunkSize;
	int ChunkCount;
	int ChunkIndex;
	uint LastChunk;
	uint RunningSampleIndex; // maybe useless.
	bool SoundIsPlaying;
};

static void ClearSoundBuffer(SoundOutputConfig *SoundOutputConfig) {
	void *FirstWriteRegionPointer;
	DWORD FirstWriteRegionLength;
	void *SecondWriteRegionPointer;
	DWORD SecondWriteRegionLength;
	HRESULT LockResult = GlobalSecondarySoundBuffer->Lock(0, SoundOutputConfig->BufferSize, &FirstWriteRegionPointer, &FirstWriteRegionLength, &SecondWriteRegionPointer, &SecondWriteRegionLength, 0);
	if(SUCCEEDED(LockResult)) {
		char *SampleOutput = (char *)FirstWriteRegionPointer;
		for(uint ByteIndex = 0; ByteIndex < FirstWriteRegionLength; ByteIndex++) {
			*SampleOutput = 0;
		}
		SampleOutput = (char *)SecondWriteRegionPointer;
		for(uint ByteIndex = 0; ByteIndex < SecondWriteRegionLength; ByteIndex++) {
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

/* GLOBAL WINDOWS KEYBOARD STRUCT
struct KeyboardInputInfo {
	uint VirtualKeycode;
	bool WithAlt;
	bool WasPressed;
	bool IsPressed;
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

static void ProcessKeyboardButton(GamepadButtonState *NewButtonState, bool IsPressed) {
	NewButtonState->EndedDown = IsPressed;
	++NewButtonState->TransitionCount;
}

static void process_digital_button(WORD buttons_state, ushort button_bitmask, GamepadButtonState *old_button_state, GamepadButtonState *new_button_state) {
	new_button_state->EndedDown = ((buttons_state & button_bitmask) == button_bitmask);
	new_button_state->TransitionCount = (old_button_state->EndedDown != new_button_state->EndedDown) ? 1 : 0;
}

static float process_analogic_stick(SHORT hardware_stick_value, SHORT dead_zone_constant) {
	float stick_value = 0;
	
	if(hardware_stick_value < -dead_zone_constant) {
		stick_value = (float)hardware_stick_value / 32768.0f;
	}else if(hardware_stick_value > dead_zone_constant) {
		stick_value = (float)hardware_stick_value / 32767.0f;
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
	int GetSizeSuccess = GetFileSizeEx(FileHandle, &FileSize);
	if(!GetSizeSuccess) {
		// ERROR CATCH.
		return Result;
	}
	
	Result.FileSize = Truncate64bitsTo32bits(FileSize.QuadPart);
	DWORD BytesRead;
	Result.FileContent = VirtualAlloc(0, Result.FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	bool ReadFileSuccess = (ReadFile(FileHandle, Result.FileContent, Result.FileSize, &BytesRead, 0) && (BytesRead == Result.FileSize));
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

static bool DEBUG_WriteFile(char *FileName, uint MemorySize, void *Memory) {
	bool Result = false;
	HANDLE FileHandle = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE) {
		// ERROR CATCH.
		Result = false;
		return Result;
	}

	DWORD BytesWritten;
	bool WriteFileSuccess = (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0) && (BytesWritten == MemorySize));
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
			KeyInput.VirtualKeycode = (uint)WParam;
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
				KeyInput.VirtualKeycode = (uint)Message.wParam;
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

	HWND HandmadeHeroWindow = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 640, 480, 0, 0, Instance, 0);

	if(HandmadeHeroWindow) {
		Running = true;
	}else {
		// Window not created, ERROR CATCH.
		return 0;
	}

	// Performance counter.
	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	llong CountFrequency = PerformanceFrequency.QuadPart;

	ullong LastCycleCount = __rdtsc();

	LARGE_INTEGER LastCount;
	QueryPerformanceCounter(&LastCount);
	// "CycleCount" refers to exact CPU cycles, while performance count is more about real time.

	// Memory setup.
	LPVOID BaseAddress = (LPVOID)(2 TB); // TODO: Remove on final build.
	GameMemory GameMemory = {};
	GameMemory.PermanentSize = 64 MB;
	GameMemory.VolatileSize = 4 GB;
	ullong TotalSize = GameMemory.PermanentSize + GameMemory.VolatileSize;
	GameMemory.PermanentPtr = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	GameMemory.VolatilePtr = ((uchar *)GameMemory.PermanentPtr + GameMemory.PermanentSize);
	
	// Render setup.
	ResizeDIBSection(&GlobalBackbuffer, 1208, 720);
	ShowWindow(HandmadeHeroWindow, ComandShow);

	// Sound setup.
	SoundOutputConfig SoundConfig = {};
	SoundConfig.SamplePerSeconds = 48000;
	SoundConfig.BytesPerSample = sizeof(short) * 2;
	SoundConfig.BytesPerSeconds = SoundConfig.SamplePerSeconds * SoundConfig.BytesPerSample; // 192000 bytes
	SoundConfig.BufferSeconds = 1;
	SoundConfig.BufferSize = SoundConfig.BytesPerSeconds * SoundConfig.BufferSeconds;
	SoundConfig.ChunkSize = SoundConfig.BytesPerSeconds / 20;
	SoundConfig.ChunkCount = SoundConfig.BufferSize / SoundConfig.ChunkSize;
	SoundConfig.ChunkIndex = 0;
	SoundConfig.LastChunk = 0;
	SoundConfig.RunningSampleIndex = 0;
	SoundConfig.SoundIsPlaying = false;

	short *SoundBufferPointer = (short *)VirtualAlloc(NULL, SoundConfig.BufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

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

	#pragma region RUNNING
	// While loop controled by a bool to keep the program running, because `PeekMessage` gets outta the loop when there are no messages.
	while(Running) {
		gamepad_controller_input *NewKeyboardController = &new_input->gamepad_controller[0];
		gamepad_controller_input *OldKeyboardController = &old_input->gamepad_controller[0];
		*NewKeyboardController = {};
		NewKeyboardController->IsConnected = true;
		for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->GamepadButton); ButtonIndex++) {
			NewKeyboardController->GamepadButton[ButtonIndex].EndedDown = OldKeyboardController->GamepadButton[ButtonIndex].EndedDown;
		}

		WindowMsg(KeyInput, NewKeyboardController);
		
		BitmapBuffer GameBuffer = {};
		GameBuffer.Memory = GlobalBackbuffer.Memory;
		GameBuffer.Width = GlobalBackbuffer.Width;
		GameBuffer.Height = GlobalBackbuffer.Height;
		GameBuffer.BytePerPixel = GlobalBackbuffer.BytePerPixel;
		GameBuffer.Pitch = GlobalBackbuffer.Pitch;
		
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
		
		DWORD CurrentSoundPlayCursor = 0;
		DWORD CurrentSoundWriteCursor = 0;
		DWORD WriteRegionOffset = 0;
		DWORD WriteRegionLength = 0;
		bool ValidSound = false;
		// TODO: Have a system to presume how far ahead of the result we are at the GameMain time.
		HRESULT GetBufferPositionResult = GlobalSecondarySoundBuffer->GetCurrentPosition(&CurrentSoundPlayCursor, &CurrentSoundWriteCursor);
		if(SUCCEEDED(GetBufferPositionResult)) {
			SoundConfig.ChunkIndex = CurrentSoundWriteCursor / SoundConfig.ChunkSize;
			WriteRegionLength = SoundConfig.ChunkSize;

			DWORD ChunkToWrite = (SoundConfig.ChunkIndex + 1) % SoundConfig.ChunkCount;
			
			if(ChunkToWrite != SoundConfig.LastChunk) {
				WriteRegionOffset = ChunkToWrite * SoundConfig.ChunkSize;
				SoundConfig.LastChunk = ChunkToWrite;
				ValidSound = true;
			}
		}
		
		SoundBuffer GameSoundBuffer = {};
		GameSoundBuffer.SamplesPerSecond = SoundConfig.SamplePerSeconds;
		GameSoundBuffer.SampleCount = WriteRegionLength / SoundConfig.BytesPerSample;
		GameSoundBuffer.SampleOut = SoundBufferPointer;
		GameSoundBuffer.ReadyToWrite = ValidSound;
		
		uint max_controller_count = XUSER_MAX_COUNT;
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

				new_controller->left_stick_average_x = process_analogic_stick(gamepad_state->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				new_controller->left_stick_average_y = process_analogic_stick(gamepad_state->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				new_controller->right_stick_average_x = process_analogic_stick(gamepad_state->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				new_controller->right_stick_average_y = process_analogic_stick(gamepad_state->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				
				float stick_action_threshold = 0.6f;
				
				process_digital_button((new_controller->left_stick_average_y > stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickUp, &new_controller->LeftStickUp);
				process_digital_button((new_controller->left_stick_average_x < -stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickLeft, &new_controller->LeftStickLeft);
				process_digital_button((new_controller->left_stick_average_y < -stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickDown, &new_controller->LeftStickDown);
				process_digital_button((new_controller->left_stick_average_x > stick_action_threshold) ? 1 : 0, 1, &old_controller->LeftStickRight, &new_controller->LeftStickRight);
			}else {
				// Controller not connected or error.
				new_controller->IsConnected = false;
			}
		}
		
		GameMain(&GameMemory, &GameBuffer, &GameSoundBuffer, &KeyInput, new_input);
		
		HDC DeviceContext = GetDC(HandmadeHeroWindow);
		ClientWindowDimension ClientWindowDimension = GetClientWindowDimension(HandmadeHeroWindow);
		DisplayBuffer(DeviceContext, ClientWindowDimension.Width, ClientWindowDimension.Height, GlobalBackbuffer);

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

		// END PROCESS ####################################################################################################

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
		
		/*
		char StringBuffer[256];
		sprintf(StringBuffer, "Render Time: %.03fms. FPS: %d. CPU Cycles: %.03fM.\n", MSPerFrame, FPS, MegaCyclesPerFrame); // WARNNG: This type of string outputting is problematic, it assumes a long enough buffer and the formats may access what it shouldn't on the stack.
		OutputDebugString(StringBuffer);
		*/
		
		LastCycleCount = EndCycleCount;
		LastCount = EndCount;
	}

	return 0;
}
#pragma endregion
#pragma endregion