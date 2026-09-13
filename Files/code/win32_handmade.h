#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <dsound.h>

// WARNNG: This type of string outputting is problematic, it assumes a long enough buffer and the formats may access what it shouldn't on the stack.
#define DEBUG_TIME char StringBuffer[1 KB]; \
sprintf(StringBuffer, "Time per frame: %.03fms Time computing: %.03fms\nFPS: %.03f MCPF: %.03f\n---\n", TotalTimeElapsedInMiliseconds, TimeComputingInMiliseconds, FPS, MegaCyclesPerFrame); \
OutputDebugString(StringBuffer);
#define DEBUG_AUDIO char StringBuffer[1 KB]; \
sprintf(StringBuffer, "Frame time: %.03fms\nPC: %u WC: %u Lock: %u Length: %u\nLatency: %.03fms SD: %d\n---\n", TotalTimeElapsedInMiliseconds, SoundPlayCursor, SoundWriteCursor, WriteRegionOffset, WriteRegionLength, DEBUG_SoundLatency, DEBUG_SampleLatency); \
OutputDebugString(StringBuffer);

struct Win32BitmapBuffer {
	BITMAPINFO Info;
	void *Memory;
	int32 Width;
	int32 Height;
	int32 BytePerPixel;
	int32 Pitch;
};

struct ClientWindowDimension {
	int32 Width;
	int32 Height;
};

typedef HRESULT WINAPI MyDirectSoundCreateFunction(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

struct SoundOutputConfig {
	int32 SamplePerSeconds;
	int32 BytesPerSample;
	int32 BytesPerSeconds;
	int32 BufferSeconds;
	int32 BufferSize;
	nat32 SafetyMargin;
	int32 SampleCount;
	int32 ChunkSize;
	int32 ChunkCount;
	int32 ChunkIndex;
	nat32 LastChunk;
	nat32 RunningSampleIndex;
	bool32 SoundIsPlaying;
};

struct DEBUG_SoundCursorMarkers {
	DWORD DEBUG_OutputPlayCursor;
	DWORD DEBUG_OutputWriteCursor;
	DWORD DEBUG_OffsetLocked;
	DWORD DEBUG_TargetCursor;
	DWORD DEBUG_FrameBoundaryTarget;
	DWORD DEBUG_FlipPlayCursor;
	DWORD DEBUG_FlipWriteCursor;
};
