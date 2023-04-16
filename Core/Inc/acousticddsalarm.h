/*
 * AcousticDDSAlarm.h
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#ifndef INC_ACOUSTICDDSALARM_H_
#define INC_ACOUSTICDDSALARM_H_

#include "stm32l1xx_hal.h"
#include <stdbool.h>

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim7;

extern bool AcousticAlarmFlag;

void Beep();
void StartAcousticDDSAlarm();
void StopAcousticDDSAlarm();
void AcousticDDS_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void AcousticDDS_Compute();

#endif /* INC_ACOUSTICDDSALARM_H_ */
