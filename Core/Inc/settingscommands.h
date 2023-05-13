#ifndef _SETTINGSCOMMANDS_H_
#define _SETTINGSCOMMANDS_H_

#include "terminal_helper.h"
#include <stdint.h>
#include "settings.h"
#include "rtc.h"
#include "extbrightness.h"
#include "setbrightness.h"
#include "acousticddsalarm.h"

void SettingsCommands_Init();

uint8_t BrightnessCmd();
uint8_t GetExtBrightCmd();
uint8_t RemoteControlCmd();
uint8_t SetDateCmd();
uint8_t SetTimeCmd();
uint8_t GetTimestampCmd();
uint8_t AlarmSettingsCmd();
uint8_t AlarmScheduleCmd();
uint8_t AlarmCmd();
uint8_t SetBeepVolumeCmd();
uint8_t AlarmCmd();
uint8_t PowerCmd();
uint8_t ResetSettingsCmd();
uint8_t StatusLEDCmd();

#endif // _SETTINGSCOMMANDS_H_
