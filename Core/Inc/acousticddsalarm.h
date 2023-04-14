/*
 * AcousticDDSAlarm.h
 *
 *  Created on: Apr 14, 2023
 *      Author: Martin
 */

#ifndef INC_ACOUSTICDDSALARM_H_
#define INC_ACOUSTICDDSALARM_H_

#include <stdbool.h>

extern bool AcousticAlarmFlag;

void StartAcousticDDSAlarm();
void AcousticDDSAlarm();
void StopAcousticDDSAlarm();

#endif /* INC_ACOUSTICDDSALARM_H_ */
