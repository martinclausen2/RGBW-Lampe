/*
 * acousticdddsalarm.c
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#include "acousticddsalarm.h"

bool AcousticAlarmFlag = false;

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
}

void StopAcousticDDSAlarm()
{
	HAL_TIM_Base_Stop(&htim7);
	AcousticAlarmFlag = false;
}
