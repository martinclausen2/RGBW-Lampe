/*
 * fadelight.c
 *
 *  Created on: May 20, 2023
 *      Author: Martin
 */

#include "fadelight.h"

#define colorCount 11

unsigned int FadeDim_Cnt[maxChannel] = {0,0,0,0};
unsigned int FadeDim_Cnt_Reload[maxChannel] = {0,0,0,0};
signed int FadeDimStep[maxChannel] = {0,0,0,0};

unsigned int fadeColor = colorCount;

bool fadeLightFlag = false;

// array with meaning full color combinations

static unsigned int colors[colorCount][maxChannel]=
{
		{255, 000, 000, 000},

		{ 95, 255, 000, 000},
		{000, 255, 000, 000},
		{000, 255, 000, 180},
		{000, 255,  52, 000},

		{ 40, 000,  52, 000},
		{000, 000,  52, 000},
		{000, 000,  52, 110},
		{000, 123,  52, 000},

		{000, 000, 000, 255},
		{120, 000, 000, 180}
};

void FadeLight()
{
	bool fading = false;
	if (fadeLightFlag)
	{
		for (int i = 0; i < maxChannel;	i++)
		{
			FadeLight_StepDim(i);
			fading = fading || DeltaBrightness(i);
		}
	}
	if (!fadeLightFlag || !fading)
	{
		fadeLightFlag = true;

		fadeColor++;
		if (fadeColor >= colorCount)
		{
			fadeColor = 0;
		}

		for (int i = 0; i < maxChannel;	i++)
		{
			// fade to next brightness
			int deltaBrightness = DeltaBrightness(i);
			if (deltaBrightness > 0)
			{
				FadeDim_Cnt_Reload[i]=Calc_FadeDim_Cnt_Reload(deltaBrightness);
				FadeDimStep[i] = 1;
			}
			else if (deltaBrightness < 0)
			{
				FadeDim_Cnt_Reload[i]=Calc_FadeDim_Cnt_Reload(-deltaBrightness);
				FadeDimStep[i] = -1;
			}
			else
			{
				FadeDim_Cnt_Reload[i]=1;		//no change
				FadeDimStep[i] = 0;
			}
			FadeDim_Cnt[i]=0; 					//schedule immediate calculation
		}
	}
}

void ResetFadeLight()
{
	fadeLightFlag = false;
}

int DeltaBrightness(unsigned int i)
{
	int temp = ((Brightness[FadeLightChannel]*colors[fadeColor][i]) >> 8) - Brightness[i];
	return temp;
}

int Calc_FadeDim_Cnt_Reload(int deltaBrightness)
{
	return GLOBAL_settings_ptr->FadingTime*callsinsecond/deltaBrightness;
}


//fade light dimming
void FadeLight_StepDim(unsigned char i)
{
	if (FadeDim_Cnt[i])
	{
		--FadeDim_Cnt[i];							//count down step
	}
	else if (DeltaBrightness(i))					//dimming step
	{
		int dimsteps = Brightness[i] + FadeDimStep[i];
		dimsteps = dimsteps * dimsteps - Brightness[i] * Brightness[i];

		FadeDim_Cnt[i]=FadeDim_Cnt_Reload[i];		//reload count down
		if (dimsteps)								//avoid div zero if nothing is to do
		{
			PWM_SetupDim(i, dimsteps, FadeDimStep[i], FadeDim_Cnt_Reload[i]*cases/dimsteps);	//setup brightness
		}
	}
}

