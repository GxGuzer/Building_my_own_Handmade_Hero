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

#define Assert(Expression) if(!(Expression)) { \
	*(int *)0 = 0; \
}

#define KB *(1024)
#define MB *(1024 KB)
#define GB *(1024LL MB)
#define TB *(1024 GB)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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
static DEBUG_FileRead DEBUG_ReadFile(char *FileName);
static bool32 DEBUG_WriteFile(char *FileName, nat32 MemorySize, void *Memory);
static void DEBUG_FreeFileMemory(void *Memory);
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
};

struct GameState {
	struct {
		int32 XOffset;
		int32 YOffset;
		int32 Speed;
	} Render;
	struct {
		int16 ToneVolume;
		int32 ToneHertz;
	} Sound;
};

struct SoundBuffer {
	int32 SamplesPerSecond;
	int32 SampleCount;
	int16 *SampleOut;
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
	bool32 IsConnected;
	
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