typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

#define Assert(Expression) if(!(Expression)) { \
	*(int *)0 = 0; \
}

#define KB *(1024)
#define MB *(1024 KB)
#define GB *(1024LL MB)
#define TB *(1024 GB)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

uint Truncate64bitsTo32bits(ullong UInt64) {
	Assert(UInt64 <= 0xFFFFFFFF);
	return (uint)UInt64;
}

#pragma region OS to Game
// TODO: Put here things that are retrieved from the OS.
/*
IMPORTANT: These functions aren't protective and should not be present on the end code.
	For example, the write may be interrupted mid writing thus leading to a corrupt file.
*/
struct DEBUG_FileRead {
	uint FileSize;
	void *FileContent;
};
static DEBUG_FileRead DEBUG_ReadFile(char *FileName);
static bool DEBUG_WriteFile(char *FileName, uint MemorySize, void *Memory);
static void DEBUG_FreeFileMemory(void *Memory);
#pragma endregion

#pragma region Game to OS
// TODO: Put here things that are sent to the OS.
#pragma endregion

struct GameMemory {
	bool Initialized;
	ullong PermanentSize;
	void *PermanentPtr; // Memory is required to be initialized to zero, it must be done if the platform layer doesn't do it.
	ullong VolatileSize;
	void *VolatilePtr;
};

struct GameState {
	struct {
		int XOffset;
		int YOffset;
		int Speed;
	} Render;
	struct {
		short ToneVolume;
		int ToneHertz;
	} Sound;
};

struct SoundBuffer {
	int SamplesPerSecond;
	int SampleCount;
	short *SampleOut;
	bool ReadyToWrite;
};

struct BitmapBuffer {
	void *Memory;
	int Width;
	int Height;
	int BytePerPixel;
	int Pitch;
};

struct GameKeyboardState {
	uint VirtualKeycode;
	bool WithAlt;
	bool WasPressed;
	bool IsPressed;
};

struct GamepadButtonState {
	int TransitionCount;
	bool EndedDown;
};

struct gamepad_controller_input {
	bool IsConnected;
	
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
			union {
				GamepadButtonState RightStickRight;
				GamepadButtonState LastButton;
			};
		};
	};

	bool is_analog;
	
	float left_stick_average_x;
	float left_stick_average_y;
	float right_stick_average_x;
	float right_stick_average_y;

	char left_trigger;
	char right_trigger;
};

struct gamepad_input {
	gamepad_controller_input gamepad_controller[5];
};

inline gamepad_controller_input *GetController(gamepad_input *GamepadInput, int GamepadIndex) {
	Assert(GamepadIndex < ArrayCount(GamepadInput->gamepad_controller));
	return &GamepadInput->gamepad_controller[GamepadIndex];
}