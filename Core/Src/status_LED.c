/*
 * status_LED.c
 *
 *  Created on: 04.06.2021
 *      Author: Martin
 */


/*
Controls Status LED
*/

#include "status_LED.h"

TIM_HandleTypeDef *htim_StatusPWM;				//handle to address timer

unsigned char LEDFlashCount;		//Number of flashes currently required, should not exceed 0x7F ...
unsigned char LEDFlashSeqCounter;	//Number of flashes * 2 (on & off) to be produced in this sequence
unsigned char LEDLimitFlashTimer;	//Software timer to decouple flashing frequency from calling frequency
unsigned char LEDFlashTimer;		//Software timer to decouple flashing frequency from calling frequency
unsigned char LEDStandbyTimer;

unsigned char LEDCurrentColor;
unsigned char LEDColorBackup;

unsigned char limit;

void Status_LED_Init(TIM_HandleTypeDef *handle_tim)
{
	htim_StatusPWM = handle_tim;
	HAL_TIM_PWM_Start(htim_StatusPWM, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(htim_StatusPWM, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(htim_StatusPWM, TIM_CHANNEL_3);
}

void LEDOff()
{
	LEDSetColor(LEDBlank);
}

void LEDOn()
{
	LEDSetColor(LEDWhite);
	LEDColorBackup = LEDCurrentColor;
}

void LEDCancel()
{
	LEDSetColor(LEDGreen);
	LEDColorBackup = LEDCurrentColor;
}

void LEDFadeLightOut()
{
	LEDSetColor(LEDGreen);
	LEDColorBackup = LEDCurrentColor;
}

// indicate value by color
void LEDValue(unsigned char i)		//i should not exceed colorTable length - 5
{
	LEDSetColor(i+5);		//add offset reserved for other status
}

// functions to indicate current option
void LEDSetupOptions(unsigned char i)	//i should not exceed LEDmax Flash
{
	if (i)
	{
		LEDColorBackup = LEDCurrentColor;
		LEDSetColor(flashColorTable[0]);
		LEDFlashSeqCounter = 0;
		LEDFlashCount = i*2-1;
		LEDFlashTimer = LEDmaxFlashTimer;
	}
	else
	{
		LEDFlashTimer = 0;					//stop flashing
											// backup of color setting required in case function is called for first time with i == 0
		LEDSetColorTemp(LEDColorBackup);	//restore LED color
	}
}

void LEDOptions()		//flashes LED to indicate current option, must be called frequently
{
	if (LEDFlashTimer == 1)
		{
		LEDFlashTimer = LEDmaxFlashTimer;
		if (LEDFlashSeqCounter <= (LEDFlashCount))
			{
			LEDSetColor(flashColorTable[LEDFlashSeqCounter & flashColorMask]);
			}
		if (LEDFlashSeqCounter == LEDFlashMaxSeqSteps)
			{
			LEDFlashSeqCounter = 0;
			}
		else
			{
			++LEDFlashSeqCounter;		//upcounting required for proper reading of flash color table
			}
		}
	else if (LEDFlashTimer)
		{
		--LEDFlashTimer;
		}
}

// functions to indicate standby and dim the status led according to surrounding brightness

void LEDSetupStandby()					//call once before entering standby, then call LEDStandby repeatedly
{
	LEDSetColor(LEDRed);
	LEDColorBackup = LEDCurrentColor;
	LEDStandbyTimer = LEDmaxStandyTimer;
}

void LEDStandby()
{
	if (LEDStandbyTimer)
		{
		--LEDStandbyTimer;
		}
		else
		{
			if (extBrightness > 0xFFFF0)	// must be equal to 0xFFFF << 4
				{
			    htim_StatusPWM->Instance->CCR1 = 0x0FFFF;
				}
			else
				{
				//dim red LED along with external brightness
			    htim_StatusPWM->Instance->CCR1 = (extBrightness >> 4 & 0x0FFFF);
			    }
		}
}


// two functions to indicate lower and upper limit of brightness setting

void LEDSetupLimit()
{
	if (maxLimit == limit)
		{
		LEDLimitFlashTimer = LEDmaxLimitFlashTimer;
		LEDSetColorTemp(LEDGreen);
		}
	else if (minLimit == limit)
		{
		LEDLimitFlashTimer = LEDmaxLimitFlashTimer;
		LEDSetColorTemp(LEDRed);
		}
}

void LEDLimit()		//blanks LED to indicate brightness limit
{
	if (LEDLimitFlashTimer)
		{
		--LEDLimitFlashTimer;
		if (0==LEDLimitFlashTimer)	//Restore LED status
			{
			LEDSetColorTemp(LEDCurrentColor);
			}
		}
}

// function to set PWM for status led

void LEDSetColor(unsigned char i)
{
	LEDCurrentColor = i;
	LEDSetColorTemp(i);
}

void LEDSetColorTemp(unsigned char i)
{
	unsigned int tempPWM;
	tempPWM = colorTable[i][0];	//blue
	tempPWM *= tempPWM;

    htim_StatusPWM->Instance->CCR2 = tempPWM;

	tempPWM = colorTable[i][1];	//green
	tempPWM *= tempPWM;

    htim_StatusPWM->Instance->CCR3 = tempPWM;

	tempPWM = colorTable[i][2];	//red
	tempPWM *= tempPWM;

    htim_StatusPWM->Instance->CCR1 = tempPWM;
}

