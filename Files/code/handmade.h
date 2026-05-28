
#pragma region OS to Game
// TODO: Put here things that are retrieved from the OS.
#pragma endregion

#pragma region Game to OS
// TODO: Put here things that are sent to the OS.
#pragma endregion

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