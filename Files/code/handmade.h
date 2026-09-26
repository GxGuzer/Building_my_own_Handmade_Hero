#pragma once

#include <math.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t  nat8;
typedef uint16_t nat16;
typedef uint32_t nat32;
typedef uint64_t nat64;

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef float    rat32;
typedef double   rat64;

typedef int32    bool32;

typedef char xs_string[64];
typedef char  s_string[128];
typedef char    string[256];
typedef char  l_string[512];
typedef char xl_string[1024];

#define PI 3.14159265359f

#define Assert(Expression) if(!(Expression)) { \
	*(int *)0 = 0; \
}

#define KB *(1024)
#define MB *(1024 KB)
#define GB *(1024LL MB)
#define TB *(1024 GB)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define StringLength(String) ArraryCount(String)

nat32 Truncate64bitsTo32bits(nat64 UInt64) {
	Assert(UInt64 <= 0xFFFFFFFF);
	return (nat32)UInt64;
}

#pragma region OS to Game
// TODO: Put here things that are retrieved from the OS.
/*
IMPORTANT: These functions aren't protective and should not be present on the end code.
	For example, the write may be interrupted mid writing thus leading to a corrupt file.
*/
struct DEBUG_FileRead {
	nat32 FileSize;
	void *FileContent;
};
typedef DEBUG_FileRead DEBUG_ReadFileFunction(char *FileName);
typedef bool32 DEBUG_WriteFileFunction(char *FileName, nat32 MemorySize, void *Memory);
typedef void DEBUG_FreeFileMemoryFunction(void *Memory);
#pragma endregion

#pragma region Game to OS
// TODO: Put here things that are sent to the OS.
#pragma endregion

struct GameMemory {
	bool32 Initialized;
	nat64 PermanentSize;
	void *PermanentPtr; // Memory is required to be initialized to zero, it must be done if the platform layer doesn't do it.
	nat64 VolatileSize;
	void *VolatilePtr;
	
	DEBUG_ReadFileFunction *DEBUG_ReadFile;
	DEBUG_WriteFileFunction *DEBUG_WriteFile;
	DEBUG_FreeFileMemoryFunction *DEBUG_FreeFileMemory;
};

struct RenderState {
	int32 XOffset;
	int32 YOffset;
	int32 Speed;
};

struct SoundState {
	int32 ToneHertz;
	nat16 ToneVolume;
	rat32 T;
};

struct GameState {
	RenderState Render;
	SoundState Sound;
};

struct SoundBuffer {
	int32 SamplesPerSecond;
	int32 SampleCount;
	int16 *SampleOut;
	int16 *LastByte;
	nat32 SampleIndex;
	bool32 ReadyToWrite;
};

struct BitmapBuffer {
	void *Memory;
	int32 Width;
	int32 Height;
	int32 BytePerPixel;
	int32 Pitch;
};

struct GameKeyboardState {
	nat32 VirtualKeycode;
	bool32 WithAlt;
	bool32 WasPressed;
	bool32 IsPressed;
};

struct GamepadButtonState {
	int32 TransitionCount;
	bool32 EndedDown;
};

struct gamepad_controller_input {
	union {
		GamepadButtonState GamepadButton[20];
		struct {
			GamepadButtonState StartButton;
			GamepadButtonState SelectButton;
			GamepadButtonState DpadUp;
			GamepadButtonState DpadLeft;
			GamepadButtonState DpadDown;
			GamepadButtonState DpadRight;
			GamepadButtonState AButton;
			GamepadButtonState BButton;
			GamepadButtonState XButton;
			GamepadButtonState YButton;
			GamepadButtonState LeftShoulder;
			GamepadButtonState RightShoulder;
			GamepadButtonState LeftStickUp;
			GamepadButtonState LeftStickLeft;
			GamepadButtonState LeftStickDown;
			GamepadButtonState LeftStickRight;
			GamepadButtonState RightStickUp;
			GamepadButtonState RightStickLeft;
			GamepadButtonState RightStickDown;
			GamepadButtonState RightStickRight;
			
			// All buttons must be above Terminator.
			
			GamepadButtonState Terminator;
		};
	};
	
	bool32 IsConnected;
	bool32 is_analog;
	
	rat32 left_stick_average_x;
	rat32 left_stick_average_y;
	rat32 right_stick_average_x;
	rat32 right_stick_average_y;

	int8 left_trigger;
	int8 right_trigger;
};

struct gamepad_input {
	gamepad_controller_input gamepad_controller[5];
};

inline gamepad_controller_input *GetController(gamepad_input *GamepadInput, int32 GamepadIndex) {
	Assert(GamepadIndex < ArrayCount(GamepadInput->gamepad_controller));
	return &GamepadInput->gamepad_controller[GamepadIndex];
}

void GameCodeNotFound() {
	return;
}

typedef void GameUpdateFunction(GameMemory *Memory, BitmapBuffer *Buffer, GameKeyboardState *KeyState, gamepad_input *input_);

typedef void GameSoundFunction(GameMemory *Memory, SoundBuffer *Buffer);