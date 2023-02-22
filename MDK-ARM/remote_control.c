#include "remote_control.h"
#include "bsp_rc.h"
#include "main.h"
#include "tim.h"
#include "usb_transmit.h"
#include "usbd_custom_hid_if.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
uint8_t STOP_TX_BUFFER[USB_MESSAGE_TX_LENGTH];

static void sbus_to_rc(volatile const uint8_t* sbus_buf, RC_ctrl_t* rc_crtl);

RC_ctrl_t rc_ctrl;
static uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];
int8_t motors_enabled = 0;
int8_t motors_disabled = 1;
int8_t ready_to_go = 0;
int8_t tim_triggered = 0;
RF run_direction;
int8_t permit_forward = 1;
int8_t permit_reverse = 0;
int8_t stop_experiment = 0;
int8_t stopped = 0;

void remote_control_init(void)
{
	RC_init(sbus_rx_buf[0],sbus_rx_buf[1],SBUS_RX_BUF_NUM);
}

const RC_ctrl_t *get_remote_control_point(void)
{
    return &rc_ctrl;
}


void USART1_IRQHandler(void)
{
    if(huart1.Instance->SR & UART_FLAG_RXNE)//接收到数据
    {
        __HAL_UART_CLEAR_PEFLAG(&huart1);
    }
    else if(USART1->SR & UART_FLAG_IDLE)
    {
        static uint16_t this_time_rx_len = 0;

        __HAL_UART_CLEAR_PEFLAG(&huart1);
        if ((hdma_usart1_rx.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            __HAL_DMA_DISABLE(&hdma_usart1_rx);
            this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart1_rx.Instance->NDTR;
            hdma_usart1_rx.Instance->NDTR = SBUS_RX_BUF_NUM;
            hdma_usart1_rx.Instance->CR |= DMA_SxCR_CT;
            __HAL_DMA_ENABLE(&hdma_usart1_rx);

            if(this_time_rx_len == RC_FRAME_LENGTH)
            {
                sbus_to_rc(sbus_rx_buf[0], &rc_ctrl);
							  manage_motors();
							  check_ready();
            }
        }
        else
        {
            __HAL_DMA_DISABLE(&hdma_usart1_rx);
            this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart1_rx.Instance->NDTR;
            hdma_usart1_rx.Instance->NDTR = SBUS_RX_BUF_NUM;
            DMA2_Stream2->CR &= ~(DMA_SxCR_CT);
            __HAL_DMA_ENABLE(&hdma_usart1_rx);

            if(this_time_rx_len == RC_FRAME_LENGTH)
            {
                sbus_to_rc(sbus_rx_buf[1], &rc_ctrl);
								manage_motors();
								check_ready();
            }
        }
    }
}

static void sbus_to_rc(volatile const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl)
{
    if (sbus_buf == NULL || rc_ctrl == NULL)
    {
        return;
    }

    rc_ctrl->rc.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;        //!< Channel 0
    rc_ctrl->rc.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff; //!< Channel 1
    rc_ctrl->rc.ch[2] = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) |          //!< Channel 2
                         (sbus_buf[4] << 10)) &0x07ff;
    rc_ctrl->rc.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff; //!< Channel 3
    rc_ctrl->rc.s[0] = ((sbus_buf[5] >> 4) & 0x0003);                  //!< Switch left
    rc_ctrl->rc.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2;                       //!< Switch right
    rc_ctrl->mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);                    //!< Mouse X axis
    rc_ctrl->mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);                    //!< Mouse Y axis
    rc_ctrl->mouse.z = sbus_buf[10] | (sbus_buf[11] << 8);                  //!< Mouse Z axis
    rc_ctrl->mouse.press_l = sbus_buf[12];                                  //!< Mouse Left Is Press ?
    rc_ctrl->mouse.press_r = sbus_buf[13];                                  //!< Mouse Right Is Press ?
    rc_ctrl->key.v = sbus_buf[14] | (sbus_buf[15] << 8);                    //!< KeyBoard value
    rc_ctrl->rc.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);                 //NULL

    rc_ctrl->rc.ch[0] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[4] -= RC_CH_VALUE_OFFSET;
}

void manage_motors(void)
{
	switch(rc_ctrl.rc.s[1])
	{
		case 1:
			if(!motors_disabled)
			{
				DISABLE_LEGS();
				motors_disabled = 1;
				motors_enabled = 0;
				ready_to_go = 0;
			}
			break;
		case 3:
			if(!motors_enabled)
			{
				if(rc_ctrl.rc.s[0] == 3)
				{
					ENABLE_LEGS();
					motors_enabled = 1;
					ready_to_go = 1;
					motors_disabled = 0;
				}
				else{
					Error_Handler();
				}
			}
			break;
		case 2:
			Error_Handler();
			break;
		default:
			break;
	}
}

void check_ready(void)
{
	if(ready_to_go)
	{
		if(rc_ctrl.rc.ch[4] == -660)
		{
			stop_experiment = 1;
			
			if(!stopped)
			{
				USB_DATA_2_COMMAND(&USB_TX_COMMAND);
				uint32touint8((uint32_t*)&USB_TX_COMMAND, STOP_TX_BUFFER, USB_MESSAGE_CONVERT_32_2_8);
				USBD_CUSTOM_HID_SendReport_FS(STOP_TX_BUFFER, USB_MESSAGE_TX_LENGTH);
				stopped = 1;
			}
		}
		else if(rc_ctrl.rc.ch[4] == 660)
		{
			stop_experiment = 0;
			stopped = 0;
		}
		
		switch(rc_ctrl.rc.s[0])
		{
			case 1:
				if((!tim_triggered)&permit_forward)
				{
					HAL_TIM_Base_Start_IT(&htim2);
					tim_triggered = 1;
					run_direction = forward;
					permit_forward = 0;
				}
				break;
			case 2:
				if((!tim_triggered) & permit_reverse)
				{
					HAL_TIM_Base_Start_IT(&htim2);
					tim_triggered = 1;
					run_direction = reverse;
					permit_reverse = 0;
					permit_forward = 1;
				}
				break;
			default:
				tim_triggered = 0;
				permit_reverse = 1;
				break;
		}
		
		if(rc_ctrl.rc.ch[1] == 660)
		{
			HAL_TIM_Base_Start_IT(&htim3);
		}
		else if(rc_ctrl.rc.ch[1] == -660)
		{
			HAL_TIM_Base_Stop_IT(&htim3);
		}
	}
}
