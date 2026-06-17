#include "handmade.h"

struct {
	short ToneVolume = 4000;
	int ToneHertz = 261;
} SineParam;

void SoundOutput(SoundBuffer *SoundBuffer) {
	short *SampleOut = SoundBuffer->SampleOut;

	static float t = 0;
	int WavePeriod = SoundBuffer->SamplesPerSecond / SineParam.ToneHertz;

	if(SoundBuffer->ReadyToWrite) {
		for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
			float SineValue = sinf(t);
			short SampleValue = (short)(SineValue * SineParam.ToneVolume);
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

static void GameMain(BitmapBuffer *Buffer, SoundBuffer *SoundBuffer, gamepad_input *input_) {
	static int XOffset = 0;
	static int YOffset = 0;
	
	gamepad_controller_input *input_0 = &input_->gamepad_controller[0];
	if(input_0->is_analog) {
		// Analog tuning.
		SineParam.ToneHertz = 261 + (int)(128.0f * input_0->stick_end_x);
		YOffset += (int)(4.0f * input_0->stick_end_y);
	}else {
		// Digital tuning.
	}

	// Input.AButtonEndedDown;
	// Input.AButtonTransitionCount;
	if(input_0->button_a.ended_down) {
		XOffset++;
	}

	SoundOutput(SoundBuffer);
	RenderGrad(Buffer, XOffset, YOffset);
}