/*
 * fadelight.c
 *
 *  Created on: May 20, 2023
 *      Author: Martin
 */

#define colorCount 5

// array with meaning full color combinations

static unsigned int colors[colorCount][maxChannel]=
{
		{255, 000, 000, 000},

		{ 95, 255, 000, 000},
		{000, 255, 000, 000},
		{000, 255, 000, 180},
		{000, 255,  52, 000},

		{ 40, 000,  52, 000},
		{000, 000,  52, 000},
		{000, 000,  52, 110},
		{000, 123,  52, 000},

		{000, 000, 000, 255},
		{120, 000, 000, 180}
};
