/* Function to decode and code RC5 commands for LampenSteuerung
   call decoder state machine at 4499 Hz for four times oversampling of the 889µs demodulated RC5 signal pulses
 */

#include "RC5.h"

volatile unsigned char rCommand;   	//Bitte erhalten! Wenn Befehl fertig: auswerten
volatile unsigned char rAddress;   	//Bitte erhalten! Wenn Befehl fertig: auswerten
volatile unsigned char rCounter;  	//Bitte erhalten!

volatile bool RTbit;				//Togglebit von RC5

unsigned char SenderMode;			//Mode for sending commands to other devices
TIM_OC_InitTypeDef sConfigOC = {0};

void RC5SignalSampling(GPIO_PinState signal)		//int from Timer to read RC5 state
{
	static unsigned char rc5state;
	static bool Rbit;					//Detektiertes Bit von RC5

	rc5state=rc5state<<1;
	rc5state|=!signal;					//Insert inverted input
	rc5state=tblRemote[rc5state];		//get next state
	if (33==rc5state)					//Startsequenz erkannt?
		{
		rCounter=0;						//alles zurücksetzen
		rAddress=0;
		rCommand=0;
		}
	else if ((42==rc5state) || (50==rc5state))	 //Erkanntes Bit einorden
		{
		if (42==rc5state)			//Nutzbit 1 erkannt?
			{
			Rbit=1;
			}
		else if (50==rc5state)		//Nutzbit 0 erkannt?
			{
			Rbit=0;
			}
   			++rCounter;				//Da neues Bit ...
		if (1==rCounter)
			{
			RTbit=Rbit;
			}
		else if (7>rCounter)		//Adressbit
			{
			rAddress=rAddress<<1;
			rAddress|=Rbit;
			}
		else						//Commandbit
			{
  			rCommand=rCommand<<1;
  			rCommand|=Rbit;
  			}
		}
}


//setup brightness values
void SetLightRemote(unsigned char i, signed char steps)
{
	PWM_SetupDim(i, Brightness_steps, steps, 0);
	FocusChannel=i;
	LightOn=1;
	LEDOn();
	LEDSetupLimit();
}

void SetBrightnessRemote(unsigned char i)
{
	if (GLOBAL_settings_ptr->ReceiverMode>=ComModeConditional)
		{
		Brightness[i]=((rCommand<<1) & 0x7E) + RTbit;
		SetLightRemote(i,0);
		}
}

void SetBrightnessLevelRemote()
{
	if (rCommand<=9)
		{
		Brightness[FocusChannel]=(((unsigned int)(rCommand)*(unsigned int)GLOBAL_settings_ptr->maxBrightness[FocusChannel])/(9)) & 0xFF;
		SetLightRemote(FocusChannel,0);
		}
}

void DecodeRemote()
{
	bool static RTbitold;				//Togglebit des letzten Befehls von RC5

	if (12==rCounter)
		{
		char serial_Buffer[64];
		sprintf(serial_Buffer,"RC5 Addr %d Cmd %d\r\n", rAddress, rCommand);
		log_serial(serial_Buffer);

		if (GLOBAL_settings_ptr->RC5Addr==rAddress)
			{
			if (RTbit ^ RTbitold)		//Neue Taste erkannt
				{
				switch (rCommand)
					{
					case 12:			//Standby
						SwAllLightOn();
						break;
					case 13:			//mute
						SwAllLightOff();
						break;
					default:
						SetBrightnessLevelRemote();
						break;
	  				}
				}

	  		switch (rCommand)			//new or same key pressed
	  			{
	  			case 16:				//incr vol
					SetLightRemote(0, RemoteSteps);
					break;
	  			case 17:				//decr vol
					SetLightRemote(0, -RemoteSteps);
					break;
				case 32:				//incr channel
					SetLightRemote(FocusChannel, RemoteSteps);
					break;
				case 33:				//decr channel
					SetLightRemote(FocusChannel, -RemoteSteps);
					break;
				case 55:				//red
					FocusChannel = 1;
					break;
				case 54:				//green
					FocusChannel = 3;
					break;
				case 50:				//yellow => white
					FocusChannel = 0;
					break;
				case 52:				//blue
					FocusChannel = 2;
					break;
	  			}
	  		RTbitold=RTbit;				//Togglebit speichern
	  		}
	  	else if ((RC5Addr_first>=rAddress) & ((RC5Addr_first + maxVirtualChannel -1) <= rAddress) & ( RC5Addr_com > rAddress))
	  		{
	  		SetBrightnessRemote(rAddress - RC5Addr_first);
	  		}
	  	else if (RC5Addr_com==rAddress)
	  		{
	  		switch (rCommand)
	  			{
	  			case RC5Cmd_AlarmStart:
	  				if (ComModeAlarm<=GLOBAL_settings_ptr->ReceiverMode)
	  					{
	  					alarmState.alarmTrigger = 1;
	  					}
	  				break;
	  			case RC5Cmd_AlarmEnd:
	  				if (ComModeAlarm<=GLOBAL_settings_ptr->ReceiverMode)
	  					{
	  					AlarmEnd();
	  					}
	  				break;
	  			case RC5Cmd_Off:
					if (ComModeConditional<=GLOBAL_settings_ptr->ReceiverMode)
						{
						SwAllLightOff();
						}
	  				break;
	  			case RC5Cmd_On:
					if (ComModeConditional<=GLOBAL_settings_ptr->ReceiverMode)
						{
						SwAllLightOn();
						}
	  				break;
	  			}
	  		}
		rCounter=0;					//Nach Erkennung zurücksetzen
		}
}



// RC5 Sender
// due to the resolution of the delay we send a bit longer pulses

void Delay()
{
	irCounter = 0;
	while(irCounter<32);
}

//send zero
void SendBit0()
{
 	//890us Impuls mit 36kHz senden
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);
	Delay();
	//890us Pause
    sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);
	Delay();
}

//Send one
void SendBit1()
{
	//890us Pause
    sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);
	Delay();
	//890us Impuls mit 36kHz senden
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);
	Delay();
}

//Sends RC5 code
void SendCommand(unsigned char address, unsigned char code, unsigned char toggle)
{
	unsigned char mask;
	unsigned char i;

    sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	sConfigOC.Pulse = 111;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);

	//disable RC5 decoder for the moment
	HAL_NVIC_DisableIRQ(TIM6_IRQn);
	HAL_TIM_Base_Start_IT(&htim10);

	SendBit1();	//1st Startbit=1
	SendBit1();	//2nd Startbit=1

	//Togglebit
	if(toggle==0)
		{
		SendBit0();
    		}
   	else
    		{
     		SendBit1();
    		}

	//5 Bit Address
   	mask=0x10;	//Begin with MSB
   	for(i=0; i<5; i++)
    		{
		if(address&mask)
      			{
       			SendBit1();
      			}
     		else
      			{
       			SendBit0();
      			}
		mask>>=1;	//Next bit
    		}

	//6 Bit Code
   	mask=0x20;
	for(i=0; i<6; i++)
		{
		if(code&mask)
			{
       			SendBit1();
      			}
     		else
     	 		{
       			SendBit0();
      			}
     		mask>>=1;
		}

	//switch off IR-LED anyway, just to be sure
    sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_Base_Stop_IT(&htim10);

	//irq on again
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
}


//Sends RC5 code if required, including repeats
void SendRC5(unsigned char address, unsigned char code, unsigned char toggle, unsigned char requiredmode, unsigned repeats)
{
	unsigned char j;
	if (SenderMode>=requiredmode)
		{
		for(j=1; j<=repeats; j++)
			{
			SendCommand(address, code, toggle);
			if (j<repeats)			//skip last pause in sequence of repeated commands
				{
				HAL_Delay(89);		//wait 88.9ms
				}
		   	}
	   	}
}
