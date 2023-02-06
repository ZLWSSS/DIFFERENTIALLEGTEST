#include "processing_file.h"
#include "can_bldc_motor.h"
#include "remote_control.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "struct_typedef.h"
#include "bsp_can.h"
#include "bsp_rc.h"
#include "usb_transmit.h"
#include "usbd_custom_hid_if.h"

int tick_counter = 0;
uint8_t TX_BUFFER[USB_MESSAGE_TX_LENGTH];
uint8_t usb_send_counter = 0;

void init_applications(void)
{
	delay_init();
	can_filter_init();
	BLDC_MOTORS_DATA_INIT();
	remote_control_init();
	USB_DATA_INIT();
}

void applications_processing(void)
{
	Control_motors(run_direction);
	USB_DATA_2_COMMAND(&USB_TX_COMMAND);
	uint32touint8((uint32_t*)&USB_TX_COMMAND, TX_BUFFER, USB_MESSAGE_CONVERT_32_2_8);
	usb_send_counter++;
	if(usb_send_counter == 2)
	{
		USBD_CUSTOM_HID_SendReport_FS(TX_BUFFER, USB_MESSAGE_TX_LENGTH);
		usb_send_counter = 0;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		tick_counter++;
		applications_processing();
	}
}
