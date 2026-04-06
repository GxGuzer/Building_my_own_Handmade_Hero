#include "handmade.h"

void RenderGrad(BitmapBuffer *Buffer, int XOffset, int YOffset) {

  uchar *Row = (uchar *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; Y++) {
    uint *Pixel = (uint *)Row;
    for (int X = 0; X < Buffer->Width; X++) {
      
      uchar Red = X + XOffset;
      uchar Green = Y + YOffset;
      uchar Blue = 0;

      *Pixel = ((Red << 16) | (Green << 8) | Blue);
      *Pixel++;

    }
    Row += Buffer->Pitch;
  }
}

static void Render(BitmapBuffer *Buffer, int XOffset, int YOffset) {
  RenderGrad(Buffer, XOffset, YOffset);
}