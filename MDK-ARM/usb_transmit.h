#ifndef BSP_USB_H_
#define BSP_USB_H_

#include "struct_typedef.h"

// 8_bit_length
#define USB_MESSAGE_LENGTH 44
#define USB_MESSAGE_TX_LENGTH 44

#define USB_MESSAGE_TX_CHECKLENGTH 10
#define USB_MESSAGE_CONVERT_32_2_8 11
// 40
typedef struct USB_TX_COMMAND_T_
{
	float q_abad;
  float q_hip;
  float q_knee;
  float qd_abad;
  float qd_hip;
  float qd_knee;
  float c_abad;
	float c_hip;
	float c_knee;
	int32_t stop_number;
  int32_t checksum;
} USB_TX_COMMAND_T;


uint32_t data_checksum(uint32_t* data_to_check, uint32_t check_length);

uint8_t USB_DATA_2_COMMAND(USB_TX_COMMAND_T* uSB_TX_COMMAND);

void uint32touint8(uint32_t* vector_32, uint8_t* vector_8, uint16_t lengthof32);

void USB_DATA_INIT(void);
void USB_REFRESH_DATA(void);
void USB_RUN(void);
void USB_INIT_RUN(void);

uint8_t Enabel_Check(void);

extern USB_TX_COMMAND_T USB_TX_COMMAND;
#endif
