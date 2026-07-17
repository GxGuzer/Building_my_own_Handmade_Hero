#include "handmade.h"

void SoundOutput(SoundBuffer *SoundBuffer, int ToneHertz, short ToneVolume) {
	short *SampleOut = SoundBuffer->SampleOut;

	static float t = 0;
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
			
			uchar Red = (uchar)(X + XOffset);
			uchar Green = (uchar)(Y + YOffset);
			uchar Blue = 0;

			*Pixel = ((Red << 16) | (Green << 8) | Blue);
			*Pixel++;

		}
		Row += Buffer->Pitch;
	}
}

static void GameMain(GameMemory *Memory, BitmapBuffer *Buffer, SoundBuffer *SoundBuffer, GameKeyboardState *KeyState, gamepad_input *input_) {
	Assert(sizeof(GameState) <= Memory->PermanentSize);

	GameState *State = (GameState *)Memory->PermanentPtr;
	if(!Memory->Initialized) {
		char *FileName = __FILE__;
		DEBUG_FileRead File = DEBUG_ReadFile(FileName);
		if(File.FileContent) {
			DEBUG_WriteFile("test.out", File.FileSize, File.FileContent);
			DEBUG_FreeFileMemory(File.FileContent);
		}

		State->Render.Speed = 4;
		State->Sound.ToneVolume = 4000;
		State->Sound.ToneHertz = 261;
		Memory->Initialized = true;
	}

	if(KeyState->IsPressed) {
		switch(KeyState->VirtualKeycode) {
			case 'W':
			{
				State->Render.YOffset -= State->Render.Speed;
				if(State->Sound.ToneVolume < 40000) {
					State->Sound.ToneVolume++;
				}
			}
			break;

			case 'A':
			{
				State->Render.XOffset -= State->Render.Speed;
				if(State->Sound.ToneHertz > 65) {
					State->Sound.ToneHertz--;
				}
			}
			break;

			case 'S':
			{
				State->Render.YOffset += State->Render.Speed;
				if(State->Sound.ToneVolume > 400) {
					State->Sound.ToneVolume--;
				}
			}
			break;

			case 'D':
			{
				State->Render.XOffset += State->Render.Speed;
				if(State->Sound.ToneHertz < 1046) {
					State->Sound.ToneHertz++;
				}
			}
			break;
		}
	}
	
	gamepad_controller_input *input_0 = &input_->gamepad_controller[0];
	if(input_0->is_analog) {
		// Analog tuning.
		State->Sound.ToneHertz = 261 + (int)(128.0f * input_0->stick_end_x);
		State->Render.YOffset += (int)(4.0f * input_0->stick_end_y);
	}else {
		// Digital tuning.
	}

	// Input.AButtonEndedDown;
	// Input.AButtonTransitionCount;
	if(input_0->a_button.ended_down) {
		State->Render.XOffset++;
	}

	RenderGrad(Buffer, State->Render.XOffset, State->Render.YOffset);
	SoundOutput(SoundBuffer, State->Sound.ToneHertz, State->Sound.ToneVolume);
}