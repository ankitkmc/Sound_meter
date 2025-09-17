/*
 * lora.c
 *
 *  Created on: Sep 10, 2025
 *      Author: devesh.sehgal
 */

#include "lora.h"

extern UART_HandleTypeDef huart1;
extern uint8_t buffer[MAX_BYTES];
extern uint8_t init_cplt_flag;
extern uint8_t rx_count;


/*////////////////////////////////////////////////////////////////////////////

	************************** LoRa Datatypes ***************************

////////////////////////////////////////////////////////////////////////////*/


/******************************************************************************
	@LoRa mode : Default mode
	@Description : All the parameter values set to default as mentioned in datasheet.

******************************************************************************/
lora_para_t default_mode =
{
	.address_high = DEFAULT_ADDH_VALUE,
	.address_low = DEFAULT_ADDL_VALUE,
	.net_id = DEFAULT_NETID_VALUE,
	.serial = DEFAULT_SERIAL_VALUE,
	.power = DEFAULT_POWER_VALUE,
	.channel = DEFAULT_CHANNEL_VALUE,
	.transmission_mode = DEFAULT_TRANSIMISSION_VALUE,
	.crypt_high = DEFAULT_CRYPTH_VALUE,
	.crypt_low = DEFAULT_CRYPTL_VALUE
};

/******************************************************************************
	@Transmission mode : Transparent mode
	@Description : The user can input data through the serial port, and the module
				   will start wireless transmission. The wireless receiving function
				   of the module is turned on. After receiving the wireless data, it
				   will be output through the serial port TXD pin.

******************************************************************************/
lora_para_t transparent_mode =
{
	.address_high = BROADCAST_ADDH_VALUE,
	.address_low = BROADCAST_ADDL_VALUE,
	.net_id = BROADCAST_NETID_VALUE,
	.serial = BROADCAST_SERIAL_VALUE,
	.power = BROADCAST_POWER_VALUE,
	.channel = BROADCAST_CHANNEL_VALUE,
	.transmission_mode = BROADCAST_TRANSIMISSION_VALUE,
	.crypt_high = BROADCAST_CRYPTH_VALUE,
	.crypt_low = BROADCAST_CRYPTL_VALUE
};

/******************************************************************************
	@Transmission mode : WOR mode
	@Description : When defined as the transmitter, a wake-up code will be
				   automatically added for a certain period of time before transmitting.

******************************************************************************/
lora_para_t wor_mode =
{
	.address_high = WOR_ADDH_VALUE,
	.address_low = WOR_ADDL_VALUE,
	.net_id = WOR_NETID_VALUE,
	.serial = WOR_NETID_VALUE,
	.power = WOR_POWER_VALUE,
	.channel = WOR_CHANNEL_VALUE,
	.transmission_mode = WOR_TRANSIMISSION_VALUE,
	.crypt_high = WOR_CRYPTH_VALUE,
	.crypt_low = WOR_CRYPTL_VALUE
};

/******************************************************************************
	@Transmission mode : Relay mode
	@Description : After the relay function is enabled, if the target address
	 	 	 	   is not the module itself, the module will start a forwarding.

******************************************************************************/
lora_para_t relay_mode =
{
	.address_high = RELAY_ADDH_VALUE,
	.address_low = RELAY_ADDL_VALUE,
	.net_id = RELAY_NETID_VALUE,
	.serial = RELAY_SERIAL_VALUE,
	.power = RELAY_POWER_VALUE,
	.channel = RELAY_POWER_VALUE,
	.transmission_mode = RELAY_TRANSIMISSION_VALUE,
	.crypt_high = RELAY_CRYPTH_VALUE,
	.crypt_low = RELAY_CRYPTL_VALUE
};


/*////////////////////////////////////////////////////////////////////////////

	******************** LoRa Interface Functions ************************

////////////////////////////////////////////////////////////////////////////*/


/******************************************************************************
	@Function : void lora_cfg_io(uint8_t status)
	@Description : The module has four working modes, which are set by pins M1 and M0.

******************************************************************************/
void lora_cfg_io(uint8_t status)
{
	if(status == CFG_REGISTER)
	{
		M0_RESET();
		M1_SET();
		HAL_Delay(5);
	}
	else if(status == NORMAL_STATUS)
	{
		M0_RESET();
		M1_RESET();
		HAL_Delay(5);
	}
	else if(status == WOR_STATUS)
	{
		M0_SET();
		M1_RESET();
		HAL_Delay(5);
	}
	else if(status == SLEEP_STATUS)
	{
		M0_SET();
		M1_SET();
		HAL_Delay(5);
	}
}

/******************************************************************************
	@Function : uint8_t lora_cfg_mode(uint8_t mode)
	@Description : The module has four transmission modes, which are set by
				   writing the values to its register.

******************************************************************************/
uint8_t lora_cfg_mode(uint8_t mode)
{
	if(mode == TRANSPARENT_MODE)
	{
		lora_cfg_io(CFG_REGISTER);
		if(lora_write_register(transparent_mode) == SUCCESS)
		{
			lora_cfg_io(NORMAL_STATUS);
			return SUCCESS;
		}
		else
		{
			return ERROR;
		}
	}
	else if(mode == RELAY_MODE)
	{
		lora_cfg_io(CFG_REGISTER);
		if(lora_write_register(relay_mode) == SUCCESS)
		{
			lora_cfg_io(NORMAL_STATUS);
			return SUCCESS;
		}
		else
		{
			return ERROR;
		}
	}
	else if(mode == WOR_TRANSMISSION_MODE)
	{
		lora_cfg_io(CFG_REGISTER);
		if(lora_write_register(wor_mode) == SUCCESS)
		{
			lora_cfg_io(WOR_STATUS);
			return SUCCESS;
		}
		else
		{
			return ERROR;
		}
	}
	else
	{
		return ERROR;
	}
}

/******************************************************************************
	@Function : uint8_t lora_write_register(lora_para_t para)
	@Description : This function is responsible for writing the configuration
				   to the registers according to the selected transmission mode.

******************************************************************************/
uint8_t lora_write_register(lora_para_t para)
{
	uint8_t i;

	buffer[0] = CFG_HEADER;
	buffer[1] = REG_START;
	buffer[2] = REG_NUMBER;

	for(i = 3; i < 12; i++)
	{
		buffer[i] = *(&para.address_high + i - 3);
	}
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)buffer, 12);
	HAL_UART_Receive_IT(&huart1, (uint8_t *)buffer, 12);
	if(buffer[0] == CFG_RETURN)
	{
		memset(buffer, 0x00, sizeof(buffer));
		rx_count = 0;
		init_cplt_flag = SUCCESS;
		return SUCCESS;
	}
	return ERROR;
}

/******************************************************************************
	@Function : uint8_t lora_send(int8_t *send_data)
	@Description : This function coded for transmitting the data to LoRa module
				   using UART.

******************************************************************************/
uint8_t lora_send(uint8_t *send_data)
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)send_data, strlen((const char *)send_data));
	return SUCCESS;
}























