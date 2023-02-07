#include "usb_transmit.h"
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"
#include "can_bldc_motor.h"
#include "remote_control.h"
#include "tim.h"

uint8_t check_flag;
int SPI_LOOP;
const BLDC_Measure_TypeDef* Local_BLDC_Motor;

USB_TX_COMMAND_T USB_TX_COMMAND;

void uint32touint8(uint32_t* vector_32, uint8_t* vector_8, uint16_t lengthof32)
{
	for(int i = 0; i < lengthof32; i++)
	{
		vector_8[4 * i] = vector_32[i];
		vector_8[4 * i + 1] = vector_32[i] >> 8;
		vector_8[4 * i + 2] = vector_32[i] >> 16;
		vector_8[4 * i + 3] = vector_32[i] >> 24;
	}
}

uint32_t data_checksum(uint32_t* data_to_check, uint32_t check_length)
{
	uint32_t t = 0;
	for(int i = 0; i < check_length; i++)
	{
		t = t ^ data_to_check[i];
	}
	
	return t;
}

uint8_t USB_DATA_2_COMMAND(USB_TX_COMMAND_T* uSB_TX_COMMAND)
{
	uSB_TX_COMMAND->q_abad = Local_BLDC_Motor->Position[0];
	uSB_TX_COMMAND->q_hip = Local_BLDC_Motor->Position[1];
	uSB_TX_COMMAND->q_knee = Local_BLDC_Motor->Position[2];
	  
	uSB_TX_COMMAND->qd_abad = Local_BLDC_Motor->Velocity[0];
	uSB_TX_COMMAND->qd_hip = Local_BLDC_Motor->Velocity[1];
	uSB_TX_COMMAND->qd_knee = Local_BLDC_Motor->Velocity[2];

	uSB_TX_COMMAND->c_abad = Local_BLDC_Motor->Current[0];
	uSB_TX_COMMAND->c_hip = Local_BLDC_Motor->Current[1];
	uSB_TX_COMMAND->c_knee = Local_BLDC_Motor->Current[2];
	
//	uSB_TX_COMMAND->c_abad = 1;
//	uSB_TX_COMMAND->c_hip = 2;
//	uSB_TX_COMMAND->c_knee = 1;
	
	if(stop_experiment)
	{
		uSB_TX_COMMAND->stop_number = 1;
	}
	else
	{
		uSB_TX_COMMAND->stop_number = 0;
	}
	uSB_TX_COMMAND->checksum = data_checksum((uint32_t*)uSB_TX_COMMAND,USB_MESSAGE_TX_CHECKLENGTH);
	
	return 0;
}



void USB_DATA_INIT(void)
{
	memset(&USB_TX_COMMAND, 0, sizeof(USB_TX_COMMAND_T));
	Local_BLDC_Motor = get_BLDC_Measure();
}

const USB_TX_COMMAND_T* get_USB_TX_COMMAND()
{
	return &USB_TX_COMMAND;
}


