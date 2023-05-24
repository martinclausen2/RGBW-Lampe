/*
 * fadelight.h
 *
 *  Created on: May 20, 2023
 *      Author: Martin
 */

#ifndef INC_FADELIGHT_H_
#define INC_FADELIGHT_H_

#include "setbrightness.h"
#include "settings.h"

void FadeLight();
void ResetFadeLight();

void FadeLight_StepDim(unsigned char i);
int DeltaBrightness(unsigned int i);
int Calc_FadeDim_Cnt_Reload(int deltaBrightness);

#endif /* INC_FADELIGHT_H_ */
