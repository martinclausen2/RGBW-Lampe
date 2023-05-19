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

bool AcousticAlarmFlag = false;

bool AcousticAlarmComputeFlag = false;

unsigned int dacCache[dacCacheSize];
unsigned int writePointer;

static uint16_t sound[sizesound][2] = {{32000, 1600},{24000, 3200},{18000, 2400},{9000, 4800},{15000, 0}};

uint16_t accu = 0;
unsigned int soundcounter = 0;
unsigned int stepcounter = 0;

void Beep()
{
	StartAcousticDDSAlarm();
	uint32_t tickstart = HAL_GetTick();
	uint32_t wait = 250;

	while((HAL_GetTick() - tickstart) < wait)
	{
		AcousticDDS_Compute();
	}
	StopAcousticDDSAlarm();
}

void StartAcousticDDSAlarm()
{
	AcousticAlarmFlag = true;
	AcousticAlarmComputeFlag = true;
	soundcounter = 0;
	stepcounter = sound[soundcounter][0];
	writePointer = 0;
	// one call only half fills cache
	AcousticDDS_Compute();
	AcousticAlarmComputeFlag = true;
	AcousticDDS_Compute();
	pDAC_CallbackTypeDef pCallback = *AcousticDDS_ConvHalfCpltCallbackCh1;
	HAL_DAC_RegisterCallback(&hdac, HAL_DAC_CH1_HALF_COMPLETE_CB_ID, pCallback);
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
	HAL_DAC_UnRegisterCallback(&hdac, HAL_DAC_CH1_HALF_COMPLETE_CB_ID);
	HAL_DAC_UnRegisterCallback(&hdac, HAL_DAC_CH1_COMPLETE_CB_ID);
	AcousticAlarmFlag = false;
}

void AcousticDDS_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
	AcousticAlarmComputeFlag = true;
}

void AcousticDDS_Compute()
{
	if (AcousticAlarmComputeFlag)
	{
		uint16_t volume = GLOBAL_settings_ptr->BeepVolume;

		for(int i = (dacCacheSize/2); i; i--)
		{
			accu+=sound[soundcounter][1];
			dacCache[writePointer]=(sinetable[(accu>>8)]*volume)>>8;
			writePointer++;
			if (writePointer >= dacCacheSize)
			{
				writePointer = 0;
			}
			stepcounter--;
			// check if new sound is to be loaded
			if (stepcounter == 0)
			{
				soundcounter++;
				stepcounter = sound[soundcounter][0];
				// check if to start sound sequence over again
				if (soundcounter >= sizesound)
				{
					soundcounter = 0;
				}
			}
		}
		AcousticAlarmComputeFlag = false;
	}
}
