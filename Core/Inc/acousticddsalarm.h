/*
 * AcousticDDSAlarm.h
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#ifndef INC_ACOUSTICDDSALARM_H_
#define INC_ACOUSTICDDSALARM_H_

#include "stm32l1xx_hal.h"
#include "main.h"
#include <stdbool.h>

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim7;

extern bool AcousticAlarmFlag;

#define dacCacheSize 0x0100u
#define sizesound 5u
#define beepDuration 250

void Beep();
void StartAcousticDDSAlarm();
void StopAcousticDDSAlarm();
void SetVolumeAcousticDDSAlarm(uint32_t volume);
void Acoustic_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);

#endif /* INC_ACOUSTICDDSALARM_H_ */
