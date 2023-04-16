/*
 * acousticdddsalarm.c
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#include "acousticddsalarm.h"

// 2048 entries => 4 Bytes each 8kB SRAM cache
#define dacCacheSize 0x07FF

bool AcousticAlarmFlag = false;

bool AcousticAlarmComputeFlag = false;

uint32_t dacCache[dacCacheSize];

void Beep()
{
	//TODO output beep via DAC
}

void StartAcousticDDSAlarm()
{
	AcousticAlarmFlag = true;
	pDAC_CallbackTypeDef pCallback = *AcousticDDS_ConvHalfCpltCallbackCh1;
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, &dacCache[0], dacCacheSize, DAC_ALIGN_12B_L);
	HAL_DAC_RegisterCallback(&hdac, HAL_DAC_CH1_HALF_COMPLETE_CB_ID, pCallback);
	HAL_TIM_Base_Start(&htim7);
}

void StopAcousticDDSAlarm()
{
	HAL_TIM_Base_Stop(&htim7);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_UnRegisterCallback(&hdac, HAL_DAC_CH1_HALF_COMPLETE_CB_ID);
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
		//TODO compute signal
		AcousticAlarmComputeFlag = false;
	}
}
