struct BitmapBuffer {
  void *Memory;
  int Width;
  int Height;
  int BytePerPixel;
  int Pitch;
};
static void Render(BitmapBuffer *BitmapBuffer, int XOffset, int YOffset);