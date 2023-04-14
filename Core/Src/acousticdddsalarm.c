/*
 * acousticdddsalarm.c
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#include "acousticddsalarm.h"

bool AcousticAlarmFlag = false;

void StartAcousticDDSAlarm()
{
	AcousticAlarmFlag = true;
}

void AcousticDDSAlarm()
{

}

void StopAcousticDDSAlarm()
{
	AcousticAlarmFlag = false;
}
