/*
 * setbrightness.c
 *
 *  Created on: 24.12.2019
 *      Author: Martin
 */

#include "setbrightness.h"

TIM_HandleTypeDef *htim_PWM;				//handle to address timer

bool LightOn;
int FocusChannel;
unsigned char Brightness[4];				//current value

unsigned char Brightness_start[] = {0x3F,0x3F,0x3F,0x3F};	//value before lights off
unsigned int PWM_Offset[] = {0,0,0,0};   			//PWM value, where the driver effectively starts to generate an output
unsigned int ExtBrightness_last = 0x01FFF;			//external brightness during lights off divided by 256
unsigned char WriteTimer;					/* time until Brightness is saved in calls to StoreBrightness() */

signed int PWM_set[] = {0,0,0,0};			//current PWM value
signed int PWM_incr[] = {0,0,0,0};			//PWM dimming step size
unsigned int PWM_incr_cnt[] = {0,0,0,0};	//no of steps required to reach target PWM value

void PWM_Init(TIM_HandleTypeDef *handle_tim)
{
	htim_PWM = handle_tim;
	htim_PWM->Instance->CCR1 = 0;
	htim_PWM->Instance->CCR2 = 0;
	htim_PWM->Instance->CCR3 = 0;
	htim_PWM->Instance->CCR4 = 0;
	HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_4);
	Update_PWM_Offset(0);
	Update_PWM_Offset(1);
	Update_PWM_Offset(2);
	Update_PWM_Offset(3);
}

void Update_PWM_Offset(unsigned char i)
{
	PWM_Offset[i]  = GLOBAL_settings_ptr->PWM_Offset[i];
	PWM_Offset[i] *= PWM_Offset[i];
}

void PWM_SetPulseWidth(int channel)
{
	PWM_set[channel] += PWM_incr[channel];
	--PWM_incr_cnt[channel];
	//limit to 16 bit
	if (PWM_set[channel] > maxPWM)
	{
		PWM_set[channel] = maxPWM;
	}
}

void PWM_StepDim()		// perform next dimming step, must frequently called for dimming action
						// contains repeated code to optimize performance
{
	if (PWM_incr_cnt[0])
		{
		PWM_SetPulseWidth(0);
	    htim_PWM->Instance->CCR3 = PWM_set[0];
	    HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_3); //TODO required?
		}

	if (PWM_incr_cnt[1])
		{
		PWM_SetPulseWidth(1);
	    htim_PWM->Instance->CCR1 = PWM_set[1];
	    HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_1); //TODO required?
		}

	if (PWM_incr_cnt[2])
		{
		PWM_SetPulseWidth(2);
	    htim_PWM->Instance->CCR4 = PWM_set[2];
	    HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_4); //TODO required?
		}

	if (PWM_incr_cnt[3])
		{
		PWM_SetPulseWidth(3);
	    htim_PWM->Instance->CCR2 = PWM_set[3];
	    HAL_TIM_PWM_Start(htim_PWM, TIM_CHANNEL_2); //TODO required?
		}
}

void PWM_SetupDim(unsigned char i, signed int PWM_dimsteps, signed int Steps)
{
	signed int temp;
	limit=0;						//reset limit indicator
	temp = Brightness[i] + Steps;
	if (GLOBAL_settings_ptr->maxBrightness[i] < temp)		//avoid overflow
		{
		temp = GLOBAL_settings_ptr->maxBrightness[i];
		limit = maxLimit;
		}
	else if (0 > temp)
		{
		limit = minLimit;
		temp = 0;
		}
	Brightness[i] = temp;

	temp = temp * (temp + 2) - PWM_set[i];
	if ((temp > PWM_dimsteps) || ((temp<0) && (-temp>PWM_dimsteps)))	// if we have more difference then steps to go
		{
		PWM_incr[i] = temp / PWM_dimsteps;
		PWM_set[i] += (temp - PWM_incr[i]*PWM_dimsteps); 		//calculate remainder, brackets to avoid overflow!?
		PWM_incr_cnt[i] = PWM_dimsteps;
		}
	else
		{
		if (0<temp)		// if we would have a stepsize smaller then one, we better reduce the number of steps
			{
			PWM_incr[i] = 1;
			PWM_incr_cnt[i] = temp;
			}
		else if (0>temp)
			{
			PWM_incr[i] = -1;
			PWM_incr_cnt[i] = -temp;				//count must be a positive number!
			}
		}
}

void PWM_SetupNow(unsigned char i, signed char Steps)
{
	PWM_SetupDim(i, 1, Steps);
	PWM_StepDim();
}

unsigned int sqrt32(unsigned long a)
{
	unsigned int rem=0;
	unsigned int root=0;
	unsigned int divisor=0;
	int i;

	// Iterate 16 times, because the maximum number of bits in the result is 16 bits
	for(i=0;i<16;i++)
	{
	   	root<<=1;
   		rem=(rem << 2)+(a>>30);
    		a<<=2;
    		divisor=(root<<1)+1;
    		if (divisor<=rem)
	    		{
         		rem-=divisor;
         		root+=1;
     		}
   	}
   	return root;
}

void SwLightOn(unsigned char i, unsigned int relBrightness)
{
	unsigned long temp;
	unsigned char minBrightness;					//avoid reduction to very low brightness values by external light

	minBrightness = GLOBAL_settings_ptr->minBrightness[i];
	temp=Brightness_start[i];
	temp=(temp*relBrightness)>>4;
	if (GLOBAL_settings_ptr->maxBrightness[i] < temp)						//limit brightness to maximum
		{
		Brightness[i] = GLOBAL_settings_ptr->maxBrightness[i];
		}
	else if ((Brightness_start[i]>temp) && (minBrightness>temp))		//limit brightness ..
		{
		if (minBrightness>Brightness_start[i])
			{
			Brightness[i] = Brightness_start[i];		// .. to last value if it is smaller than minimum brightness
			}
		else
			{
			Brightness[i] = minBrightness;			// .. to minimum brightness if the last value was larger than the minimum brightness
			}
		}
	else
		{
		Brightness[i] = temp;					// or just take the calculated value!
		}
	PWM_SetupDim(i, fadetime, 0);
}

void SwLightOff(unsigned char i)
{
	Brightness_start[i]=Brightness[i];
	Brightness[i]=0;
	PWM_SetupDim(i, fadetime, 0);
}

void SwAllLightOn()
{
	unsigned int relBrightness;
	if (LightOn == false)						//remote signal might try to switch a switched on light on again
		{
		FocusChannel=startupfocus;
		LightOn=true;
		relBrightness=sqrt32(extBrightness/GLOBAL_settings_ptr->ExtBrightness_last);
		SwLightOn(0, relBrightness);
		SwLightOn(1, relBrightness);
		SwLightOn(2, relBrightness);
		SwLightOn(3, relBrightness);
		LEDOn();
		}
}

void SwAllLightOff()
{
	if (LightOn == true)						//remote signal might try to switch a switched on light on again
		{
		LightOn=false;
		alarmState.alarmFlag=0;
		SwLightOff(0);
		SwLightOff(1);
		SwLightOff(2);
		SwLightOff(3);
		HAL_Delay(750);
		SetExtBrightness_last();
		SenderMode=GLOBAL_settings_ptr->SenderMode; 		//reset mode
		LEDSetupStandby();
		}
}

void ToggleFocus()
{
	FocusChannel++;
	if (FocusChannel > MaxFocusChannel)
		FocusChannel = 0;
}

int PreviewToggelFocus()
{
	int temp = FocusChannel;
	temp++;
	if (temp > MaxFocusChannel)
		temp = 0;
	return temp;
}

void SetExtBrightness_last()
{
	unsigned int ExtBrightness_last=(extBrightness>>8) & 0xFFFF;
	if (0==ExtBrightness_last)
		{
		ExtBrightness_last=1;
		}
	GLOBAL_settings_ptr->ExtBrightness_last=ExtBrightness_last;
}

void StoreBrightness()
{
	if (1<WriteTimer)		/* store current brightness after timeout */
		{
		--WriteTimer;
		}
	else if (1 == WriteTimer)
		{
		if (LightOn)
			{
			memcpy(Brightness_start, GLOBAL_settings_ptr->Brightness_start, sizeof(Brightness_start));
			SetExtBrightness_last();
			GLOBAL_settings_ptr->ExtBrightness_last=ExtBrightness_last;
			SettingsWrite();
			}
		WriteTimer=0;
		}
}
