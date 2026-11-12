#include "handmade.h"

void SineOutput(SoundBuffer *SoundBuffer, int32 ToneHertz, nat16 ToneVolume, rat32 *T) {
	
	int16 *SampleOut = SoundBuffer->SampleOut;
	int32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHertz;
	
	for(int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
		rat32 SineValue = sinf(*T);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
		
		*T += 2.0f*PI / (rat32)(WavePeriod);
		if(*T > 2.0f*PI) {
			*T -= 2.0f*PI;
		}
	}
}

void RenderGrad(BitmapBuffer *Buffer, int32 XOffset, int32 YOffset) {

	nat8 *Row = (nat8 *)(Buffer->Memory);
	for (int32 Y = 0; Y < Buffer->Height; Y++) {
		nat32 *Pixel = (nat32 *)(Row);
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

void RenderSquare(BitmapBuffer *Buffer, int32 XCoord, int32 YCoord, nat32 Color) {
	int32 Size = 16;
	int32 HalfSize = Size / 2;
	int32 XMin = 0;
	int32 XMax = 0;
	if(XCoord < HalfSize) {
		XMin = 0;
		XMax = Size;
	}else if(XCoord > (Buffer->Width - HalfSize)) {
		XMin = Buffer->Width - Size;
		XMax = Buffer->Width;
	}else {
		XMin = XCoord - HalfSize;
		XMax = XCoord + HalfSize;
	}
	int32 YMin = 0;
	int32 YMax = 0;
	if(YCoord < HalfSize) {
		YMin = 0;
		YMax = Size;
	}else if(YCoord > (Buffer->Height - HalfSize)) {
		YMin = Buffer->Height - Size;
		YMax = Buffer->Height;
	}else {
		YMin = YCoord - HalfSize;
		YMax = YCoord + HalfSize;
	}
	
	nat8 *Row = (nat8 *)(Buffer->Memory) + (YMin * Buffer->Pitch);
	for(int32 Y = YMin; Y < YMax; Y++) {
		nat32 *Pixel = (nat32 *)(Row) + XMin;
		for(int32 X = XMin; X < XMax; X++) {
			*Pixel = Color;
			Pixel++;
		}
		Row += Buffer->Pitch;
	}
}

extern "C" void GameUpdate(ThreadContext *Thread, GameMemory *Memory, BitmapBuffer *Buffer, GameKeyboardState *KeyState, gamepad_input *input_) {
	Assert(sizeof(GameState) <= Memory->PermanentSize);
	Assert((&input_->gamepad_controller[0].Terminator - &input_->gamepad_controller[0].GamepadButton[0]) == ArrayCount(input_->gamepad_controller[0].GamepadButton));

	GameState *State = (GameState *)Memory->PermanentPtr;
	if(!Memory->Initialized) {
		char *FileName = __FILE__;
		DEBUG_FileRead File = Memory->DEBUG_ReadFile(FileName);
		if(File.FileContent) {
			Memory->DEBUG_WriteFile("test.out", File.FileSize, File.FileContent);
			Memory->DEBUG_FreeFileMemory(File.FileContent);
		}

		State->Render.Speed = 4;
		State->Sound.ToneVolume = 4000;
		State->Sound.ToneHertz = 261;
		State->Sound.T = 0.0f;
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
				if(State->Sound.ToneVolume < 60000) {
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
				if(State->Sound.ToneVolume > 600) {
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
	nat8 Red = (input_->MouseButtons[0].EndedDown) ? 0xFF : 0;
	nat8 Green = (input_->MouseButtons[1].EndedDown) ? 0xFF : 0;
	nat8 Blue = (input_->MouseButtons[2].EndedDown) ? 0xFF : 0;
	nat32 Color = ((Red << 16) | (Green << 8) | Blue);
	if(Color == 0) {
		Color = 0xFFFFFFFF;
	}
	RenderSquare(Buffer, input_->MouseX, input_->MouseY, Color);
}

extern "C" void GameSoundOutput(ThreadContext *Thread, GameMemory *Memory, SoundBuffer *Buffer) {
	GameState *State = (GameState *)Memory->PermanentPtr;
	if(!Memory->Initialized) {
		State->Sound.ToneVolume = 4000;
		State->Sound.ToneHertz = 261;
		State->Sound.T = 0.0f;
	}
	
	SineOutput(Buffer, State->Sound.ToneHertz, State->Sound.ToneVolume, &State->Sound.T);
}