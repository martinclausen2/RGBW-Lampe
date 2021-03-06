/*
 * setbrightness.h
 *
 *  Created on: 24.12.2019
 *      Author: Martin
 */

#ifndef SETBRIGHTNESS_H_
#define SETBRIGHTNESS_H_

#include "globals.h"
#include "status_LED.h"
#include <stdbool.h>
#include "stm32l1xx_hal.h"

#define maxBrightness		0x00FF  //avoid overflow with signed numbers, should be filled with 1 from MSB to LSB
#define maxPWM	   			0xFFFF	// =  maxBrigntess^2 = 2^16-1
#define Brightness_steps	20		//number of steps used to execute a brightness change
#define startupfocus		0		//focus channel
#define fadetime			150
#define MaxFocusChannel     3

bool LightOn;

int FocusChannel;

unsigned char Brightness[4];// = {0,0,0,0};	//current value

TIM_HandleTypeDef *htim_PWM;				//handle to address timer

void PWM_Init(TIM_HandleTypeDef *htim_PWM);
void PWM_StepDim();
void Update_PWM_Offset(unsigned char i);
void SwLightOn(unsigned char i, unsigned int relBrightness);
void SwAllLightOn();
void SwLightOff(unsigned char i);
void SwAllLightOff();
void PWM_SetupDim(unsigned char i, signed int PWM_dimsteps, signed int Steps);
void ToggleFocus();
int PreviewToggelFocus();

#endif /* SETBRIGHTNESS_H_ */
