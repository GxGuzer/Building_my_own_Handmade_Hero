#include "handmade.h"

void SoundOutput(SoundBuffer *SoundBuffer, int32 ToneHertz, int16 ToneVolume) {
	int16 *SampleOut = SoundBuffer->SampleOut;

	static rat32 t = 0;
	int32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHertz;

	if(SoundBuffer->ReadyToWrite) {
		for(int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
			rat32 SineValue = sinf(t);
			int16 SampleValue = (int16)(SineValue * ToneVolume);
			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;

			t += 2.0f*PI / (rat32)WavePeriod;
			if(t > 2.0f*PI) {
				t -= 2.0f*PI;
			}
		}
	}
}

void RenderGrad(BitmapBuffer *Buffer, int32 XOffset, int32 YOffset) {

	nat8 *Row = (nat8 *)Buffer->Memory;
	for (int32 Y = 0; Y < Buffer->Height; Y++) {
		nat32 *Pixel = (nat32 *)Row;
		for (int32 X = 0; X < Buffer->Width; X++) {
			
			nat8 Red = (nat8)(X + XOffset);
			nat8 Green = (nat8)(Y + YOffset);
			nat8 Blue = 0;

			*Pixel = ((Red << 16) | (Green << 8) | Blue);
			*Pixel++;

		}
		Row += Buffer->Pitch;
	}
}

static void GameMain(GameMemory *Memory, BitmapBuffer *Buffer, SoundBuffer *SoundBuffer, GameKeyboardState *KeyState, gamepad_input *input_) {
	Assert(sizeof(GameState) <= Memory->PermanentSize);
	Assert((&input_->gamepad_controller[0].Terminator - &input_->gamepad_controller[0].GamepadButton[0]) == ArrayCount(input_->gamepad_controller[0].GamepadButton));

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
	
	for(int32 ControllerIndex = 0; ControllerIndex < ArrayCount(input_->gamepad_controller); ControllerIndex++) {
		gamepad_controller_input *CurrentController = GetController(input_, ControllerIndex);
		bool32 UpAction = (CurrentController->DpadUp.EndedDown || CurrentController->AButton.EndedDown || CurrentController->LeftStickUp.EndedDown);
		bool32 LeftAction = (CurrentController->DpadLeft.EndedDown || CurrentController->XButton.EndedDown || CurrentController->LeftStickLeft.EndedDown);
		bool32 DownAction = (CurrentController->DpadDown.EndedDown || CurrentController->BButton.EndedDown || CurrentController->LeftStickDown.EndedDown);
		bool32 RightAction = (CurrentController->DpadRight.EndedDown || CurrentController->YButton.EndedDown || CurrentController->LeftStickRight.EndedDown);
		
		if(CurrentController->is_analog) {
			// Analog tuning.
			State->Sound.ToneHertz = 261 + (int32)(128.0f * CurrentController->left_stick_average_x);
			State->Render.YOffset += (int32)(4.0f * CurrentController->left_stick_average_y);
		}else {
			// Digital tuning.
			if(UpAction) {
				State->Render.YOffset -= State->Render.Speed;
				if(State->Sound.ToneVolume < 40000) {
					State->Sound.ToneVolume++;
				}
			}
			if(LeftAction) {
				State->Render.XOffset -= State->Render.Speed;
				if(State->Sound.ToneHertz > 65) {
					State->Sound.ToneHertz--;
				}
			}
			if(DownAction) {
				State->Render.YOffset += State->Render.Speed;
				if(State->Sound.ToneVolume > 400) {
					State->Sound.ToneVolume--;
				}
			}
			if(RightAction) {
				State->Render.XOffset += State->Render.Speed;
				if(State->Sound.ToneHertz < 1046) {
					State->Sound.ToneHertz++;
				}
			}
		}
	}

	RenderGrad(Buffer, State->Render.XOffset, State->Render.YOffset);
	SoundOutput(SoundBuffer, State->Sound.ToneHertz, State->Sound.ToneVolume);
}