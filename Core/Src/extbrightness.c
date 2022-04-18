/*
 * extbrightness.c
 *
 *  Created on: 18.04.2022
 *      Author: Martin
 * Function to obtain brightness
 * requires two volatile variables to store gain setting and value for moving average
 * call frequently
 * divide result by number of data points of the moving average
 * maximum filter size = 2^32 / 2^8 / 1000 = 167777 datapoints
 * result is found in the unsigned long ExtBrightness
*/


#include "extbrightness.h"

ADC_HandleTypeDef *hadc_extbrightness;

unsigned long extBrightness = 0;	//will be filled up to 24 bit with 64 datapoint moving average

PhotoGain_t PhotoGain;

/**amplification factors of photoamp */
const unsigned int photoampfactor[] = {1, 30, 1000};

void Init_ExtBrightness(ADC_HandleTypeDef *handle_adc)
{
	hadc_extbrightness = handle_adc;
}

void Sample_ExtBrightness()
{
    HAL_ADC_Start_IT(hadc_extbrightness);
}

void AddValue_ExtBrightness(ADC_HandleTypeDef *handle_adc)
{
	if (handle_adc->Instance == hadc_extbrightness->Instance && handle_adc->NbrOfConversionRank == extbrightness_ADC_RANK)
	{
		int ADC_Result = HAL_ADC_GetValue(hadc_extbrightness);
		extBrightness -= (extBrightness >> 6);		//in the meantime: remove a 1/64 so we have a moving average over 64 datapoints
		extBrightness += ADC_Result * photoampfactor[PhotoGain.ALL];
		if ((maxphotoamp < ADC_Result) && (maxphotogain > PhotoGain.ALL))
			{
			++PhotoGain.ALL;
			}
		else if ((minphotoamp > ADC_Result) && (minphotogain < PhotoGain.ALL))
			{
			--PhotoGain.ALL;
			}

		//P0_6 = PhotoGain.LSB;		//set gain
		//P0_7 = PhotoGain.MSB;
	}
}

unsigned long Get_ExtBrightness()
{
	return extBrightness;
}
