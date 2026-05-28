#include "handmade.h"

void SoundOutput(SoundBuffer *SoundBuffer) {
  short *SampleOut = SoundBuffer->SampleOut;

  static float t = 0;
  short ToneVolume = 4000;
  int ToneHertz = 261;
  int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHertz;
  
  if(SoundBuffer->ReadyToWrite) {
    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
      float SineValue = sinf(t);
      short SampleValue = (short)(SineValue * ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;

      t += 2.0f*PI / (float)WavePeriod;
      if(t > 2.0f*PI) {
        t -= 2.0f*PI;
      }
    }
  }
}

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

static void GameMain(BitmapBuffer *Buffer, int XOffset, int YOffset, SoundBuffer *SoundBuffer) {
  SoundOutput(SoundBuffer);
  RenderGrad(Buffer, XOffset, YOffset);
}