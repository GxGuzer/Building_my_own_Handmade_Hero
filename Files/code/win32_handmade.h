#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <dsound.h>

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
	int32 ChunkSize;
	int32 ChunkCount;
	int32 ChunkIndex;
	nat32 LastChunk;
	nat32 RunningSampleIndex; // maybe useless.
	bool SoundIsPlaying;
};

struct DEBUG_SoundCursorMarkers {
	DWORD DEBUG_PlayCursor;
	DWORD DEBUG_WriteCursor;
};
