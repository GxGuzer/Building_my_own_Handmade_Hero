#define Assert(Expression) if(!(Expression)) { \
	*(int *)0 = 0; \
}

#define KB *(1024)
#define MB *(1024 KB)
#define GB *((ullong)1024 MB)
#define TB *(1024 GB)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#pragma region OS to Game
// TODO: Put here things that are retrieved from the OS.
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
	bool Pressed;
};

struct gamepad_button_state {
	int transition_count;
	bool ended_down;
};

struct gamepad_controller_input {
	union {
		gamepad_button_state gamepad_button[10];
		struct {
			gamepad_button_state dpad_up;
			gamepad_button_state dpad_left;
			gamepad_button_state dpad_down;
			gamepad_button_state dpad_right;
			gamepad_button_state button_y;
			gamepad_button_state button_x;
			gamepad_button_state button_a;
			gamepad_button_state button_b;
			gamepad_button_state left_shoulder;
			gamepad_button_state right_shoulder;
		};
	};

	bool is_analog;

	char left_trigger;
	char right_trigger;

	float stick_start_x;
	float stick_min_x;
	float stick_max_x;
	float stick_end_x;

	float stick_start_y;
	float stick_min_y;
	float stick_max_y;
	float stick_end_y;
};

struct gamepad_input {
	gamepad_controller_input gamepad_controller[4];
};

// TODO: Design the way to pass keyboard inputs.