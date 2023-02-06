#include "gpio.h"
#include "bsp_led.h"

void turn_on_all_leds(void)
{
	  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_5
                          |GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, GPIO_PIN_RESET);
}