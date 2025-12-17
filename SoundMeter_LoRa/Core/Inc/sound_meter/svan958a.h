/*
 * svan958a.h
 *
 *  Created on: Sep 22, 2025
 *      Author: devesh.sehgal
 */

#ifndef INC_SOUND_METER_SVAN958A_H_
#define INC_SOUND_METER_SVAN958A_H_

#include <main.h>
#include <stdint.h>

/******** USER SETTINGS ************/

// UART mapping
#define SVAN_UART_HANDLE		huart2
extern UART_HandleTypeDef SVAN_UART_HANDLE;
extern uint8_t rx_byte;

// Polling rate in ms
#define SVAN_POLL_PERIOD_MS 	1000

// Which channel to poll
// Sound (SLM): channel 4, profile 1
#define SVAN_SLM_CH				4
#define SVAN_SLM_PROFILE		1

// Vibration (VLM): channel 1-3 (set 0 to disable)
#define SVAN_VLM_CH1			1
#define SVAN_VLM_CH2			2
#define SVAN_VLM_CH3			3

// Result data structure
typedef struct
{
	int   p;			// result set id = ch + 4*(profile-1)
	float T_sec;		// measurement time [s]
	int   overload;		// V (0/1)
	float SPL_dB;		// S - sound pressure level
	float LEQ_dB;		// R - time averaged sound level
	float SEL_dB;		// U - sound exposure level
	float PEAK_dB;		// P - peak sound level
	float MAX_dB;		// M - maximum SPL
	float MIN_dB;		// N - minimum SPL
	int   has_T, has_SPL, has_LEQ, has_SEL, has_PEAK, has_MAX, has_MIN;
}svan_slm_res_t;

typedef struct
{
	int   p;			// = channel (profile=1)
	float T_sec;		// measurement time [s]
	int   overload;		// V (0/1)
	float RMS_dB;		// R - root mean square value
	float PEAK_dB;		// Q - maximum absolute value
	float PP_dB;		// P - peak to peak
	float MTVV_dB;		// M - maximum transient vibration
	float VDV_dB;		// H - vibration dose value
	float VEC_dB;		// v - vector sum of vibration
	int   has_T, has_RMS, has_PEAK, has_PP, has_MTVV, has_VDV, has_VEC;
}svan_vlm_res_t;

// Initialize Driver
void svan_init(void);

// Process incoming bytes, parse complete frames
void svan_process(void);

// Basic session: STOP -> set modes -> start
void svan_config_and_start(void);

// Poll requests (send #2 frames)
void svan_poll_slm(uint8_t ch, uint8_t profile);
void svan_poll_vlm(uint8_t ch);

// Weak callbacks you can override to consume parsed data
__attribute__((weak)) void svan_on_slm(const svan_slm_res_t* r);
__attribute__((weak)) void svan_on_vlm(const svan_vlm_res_t* r);

// Internals: to be called from HAL callback
void svan_on_rx_byte(uint8_t b);

// Tick (call from main every loop to schedule polls)
void svan_tick_1ms(void);

#endif /* INC_SOUND_METER_SVAN958A_H_ */
