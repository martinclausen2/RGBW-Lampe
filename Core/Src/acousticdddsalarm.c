/*
 * acousticdddsalarm.c
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#include "acousticddsalarm.h"

#define dacCacheSize 0x0FFF

bool AcousticAlarmFlag = false;

uint16_t DACcache[dacCacheSize];

void Beep()
{

}

void StartAcousticDDSAlarm()
{
	AcousticAlarmFlag = true;
	HAL_TIM_Base_Start(&htim7);
}

void AcousticDDSAlarm()
{
	//reload ram for signal generation
	//
}

void StopAcousticDDSAlarm()
{
	HAL_TIM_Base_Stop(&htim7);
	AcousticAlarmFlag = false;
}
