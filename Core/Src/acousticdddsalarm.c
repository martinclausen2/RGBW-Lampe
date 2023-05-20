/*
 * acousticdddsalarm.c
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#include "acousticddsalarm.h"

#include "sinetable.c"

#define dacCacheSize 0x0100u
#define sizesound 5u
#define beepDuration 250

bool AcousticAlarmFlag = false;

unsigned int dacCache[dacCacheSize];

static unsigned int sound[sizesound][2] = {{128, 60},{96, 40},{72, 80},{48, 60},{64, 100}};

unsigned int soundcounter = 0;
unsigned int stepcounter = 0;

void Beep()
{
	if (!AcousticAlarmFlag)
	{
		StartAcousticDDSAlarm();
		HAL_Delay(beepDuration);
		StopAcousticDDSAlarm();
	}
}

void StartAcousticDDSAlarm()
{
	AcousticAlarmFlag = true;
	soundcounter = 0;
	stepcounter = sound[soundcounter][0];
	htim7.Instance->ARR = sound[soundcounter][1];
	uint16_t volume = GLOBAL_settings_ptr->BeepVolume;
	for(int i = dacCacheSize; i; i--)
	{
		dacCache[i]=(sinetable[i]*volume)>>8;
	}
	pDAC_CallbackTypeDef pCallback = *Acoustic_ConvCpltCallbackCh1;
	HAL_DAC_RegisterCallback(&hdac, HAL_DAC_CH1_COMPLETE_CB_ID, pCallback);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *) &dacCache, dacCacheSize, DAC_ALIGN_12B_L);
	HAL_TIM_Base_Start(&htim7);
	HAL_GPIO_WritePin(AMP_SHTDN_GPIO_Port, AMP_SHTDN_Pin, GPIO_PIN_RESET);
}

void StopAcousticDDSAlarm()
{
	HAL_GPIO_WritePin(AMP_SHTDN_GPIO_Port, AMP_SHTDN_Pin, GPIO_PIN_SET);
	HAL_TIM_Base_Stop(&htim7);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_UnRegisterCallback(&hdac, HAL_DAC_CH1_COMPLETE_CB_ID);
	AcousticAlarmFlag = false;
}

void Acoustic_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
	stepcounter--;
	if (!stepcounter)
	{
		soundcounter++;
		if (soundcounter >= sizesound)
		{
			soundcounter = 0;
		}
		stepcounter = sound[soundcounter][0];
		htim7.Instance->ARR = sound[soundcounter][1];
	}
}
