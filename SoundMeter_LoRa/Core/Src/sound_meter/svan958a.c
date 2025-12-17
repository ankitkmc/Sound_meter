/*
 * svan958a.c
 *
 *  Created on: Sep 22, 2025
 *      Author: devesh.sehgal
 */

#include "sound_meter/svan958a.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define RX_BUFF_SIZE	1024
volatile uint8_t RxBuff[RX_BUFF_SIZE] = {0};
volatile uint16_t RxBuff_Head = 0, RxBuff_Tail = 0;

uint8_t rx_byte = 0;
uint32_t last_poll_ms = 0;

extern uint8_t transmitBuff[8];

/*////////////////////////////////////////////////////////////////////////////

	******************** UART Interface Functions ************************

////////////////////////////////////////////////////////////////////////////*/

static void uart_send(const uint8_t* d, uint16_t len)
{
    HAL_UART_Transmit(&SVAN_UART_HANDLE, (uint8_t*)d, len, 500);
}

static void uart_send_str(const char* s)
{
    uart_send((const uint8_t*)s, (uint16_t)strlen(s));
}


static void uart_sendf(const char* fmt, ...)
{
    char buf[256];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    uart_send_str(buf);
}


/*////////////////////////////////////////////////////////////////////////////

	****************** Receive Buffer Utility Functions ******************

////////////////////////////////////////////////////////////////////////////*/

static inline uint16_t rxBuff_count(void)
{
    uint16_t h = RxBuff_Head, t = RxBuff_Tail;
    return (h >= t) ? (h - t) : (RX_BUFF_SIZE - t + h);
}

static inline int rxBuff_get(uint8_t* out)
{
    if (rxBuff_count() == 0)
	{
    	return 0;
	}
    *out = RxBuff[RxBuff_Tail];
    RxBuff_Tail = (RxBuff_Tail + 1) % RX_BUFF_SIZE;
    return 1;
}

static inline void rxBuff_put(uint8_t b)
{
    uint16_t nxt = (RxBuff_Head + 1) % RX_BUFF_SIZE;
    if (nxt != RxBuff_Tail)
    {
    	RxBuff[RxBuff_Head] = b;
    	RxBuff_Head = nxt;
    } // drop if full
}


/*////////////////////////////////////////////////////////////////////////////

	************************ Public API Functions ***********************

////////////////////////////////////////////////////////////////////////////*/

void svan_on_rx_byte(uint8_t b)
{
	rxBuff_put(b);
}

void svan_init(void)
{
    // Prime 1-byte interrupt receive
    HAL_UART_Receive_IT(&SVAN_UART_HANDLE, &rx_byte, 1);
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart == &SVAN_UART_HANDLE)
//    {
//        svan_on_rx_byte(rx_byte);
//        HAL_UART_Receive_IT(&SVAN_UART_HANDLE, &rx_byte, 1); // re-arm
//    }
//}

static uint32_t ms_now(void)
{
	return HAL_GetTick();
}

void svan_tick_1ms(void)
{
    // call this from your main loop to keep a 1s scheduler
    uint32_t now = ms_now();
    if (now - last_poll_ms >= SVAN_POLL_PERIOD_MS)
    {
        last_poll_ms = now;
        // Poll Sound (SLM)
        if (SVAN_SLM_CH)
		{
        	svan_poll_slm(SVAN_SLM_CH, SVAN_SLM_PROFILE);
		}
        // Poll Vibration (VLM)
        if (SVAN_VLM_CH1)
        {
        	svan_poll_vlm(SVAN_VLM_CH1);
        }
        if (SVAN_VLM_CH2)
        {
        	svan_poll_vlm(SVAN_VLM_CH2);
        }
        if (SVAN_VLM_CH3)
        {
        	svan_poll_vlm(SVAN_VLM_CH3);
        }
    }
}

/*////////////////////////////////////////////////////////////////////////////

	*********************** Data Parsing Functions **********************

////////////////////////////////////////////////////////////////////////////*/

static int parse_float(const char* s, float* out)
{
    char* end = NULL;
    float v = strtof(s, &end);
    if (end==s)
    {
    	return 0;
    }
    *out = v;
    return 1;
}

static int parse_int(const char* s, int* out)
{
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (end==s)
    {
    	return 0;
    }
    *out = (int)v;
    return 1;
}

// Token iterator: returns pointer to next token end or NULL
static const char* next_token(const char* s, char* tok, size_t tokcap)
{
	// skip commas
	while(*s == ',')
	{
		s++;
	}
	if(*s == 0 || *s == ';')
	{
		return NULL;
	}
	size_t i = 0;
	while(*s && *s != ',' && *s != ';' && i < tokcap - 1)
	{
		tok[i++] = *s++;
	}
	tok[i] = 0;
	return s;
}

// #2 - reply parsers
static int parse_hash2_slm(const char* line, svan_slm_res_t* r)
{
	// expecting "#2,<p>,X...,;"
	memset(r, 0, sizeof(*r));
	if (strncmp(line, "#2,", 3) != 0)
	{
		return 0;
	}
	const char* s = line + 3;
	//p
	char tok[64];
	s = next_token(s, tok, sizeof(tok));
	if(!s)
	{
		return 0;
	}
	if (!parse_int(tok, &r->p))
	{
		return 0;
	}

	// tokens Xvalue, or X(k)value with parentheses we ignore
	while ((s = next_token(s, tok, sizeof(tok))))
	{
		char key = tok[0];
		const char* valp = tok+1;

		// handle parenthesized variants e.g. B(2)66.7 , L(50)54.9 (we ignore the () part)
		if (*valp == '(')
		{
			while (*valp && *valp!=')')
			{
				valp++;
			}
			if (*valp==')')
			{
				valp++;
			}
		}

		float fv; int iv;

		switch (key)
		{
            case 'T': if (parse_float(valp,&fv)){ r->T_sec=fv; r->has_T=1; } break;
            case 'V': if (parse_int  (valp,&iv)){ r->overload=iv; } break;
            case 'S': if (parse_float(valp,&fv)){ r->SPL_dB=fv; r->has_SPL=1; } break;
            case 'R': if (parse_float(valp,&fv)){ r->LEQ_dB=fv; r->has_LEQ=1; } break;
            case 'U': if (parse_float(valp,&fv)){ r->SEL_dB=fv; r->has_SEL=1; } break;
            case 'P': if (parse_float(valp,&fv)){ r->PEAK_dB=fv; r->has_PEAK=1; } break;
            case 'M': if (parse_float(valp,&fv)){ r->MAX_dB=fv; r->has_MAX=1; } break;
            case 'N': if (parse_float(valp,&fv)){ r->MIN_dB=fv; r->has_MIN=1; } break;
            case 'B': /* Lden variants available as B(k) */ break;
            case 'L': /* percentiles */ break;
            case 'r': /* underrange flag if needed */ break;
            default: break;
        }
    }
	return 1;
}

static int parse_hash2_vlm(const char* line, svan_vlm_res_t* r)
{
    memset(r, 0, sizeof(*r));
    if (strncmp(line, "#2,", 3) != 0)
    {
    	return 0;
    }
    const char* s = line + 3;
    char tok[64];
    s = next_token(s, tok, sizeof(tok));
    if (!s)
    {
    	return 0;
    }
    if (!parse_int(tok, &r->p))
    {
    	return 0;
    }

    while ((s = next_token(s, tok, sizeof(tok))))
    {
        char key = tok[0];
        const char* valp = tok+1;
        if (*valp=='(')
        {
            while (*valp && *valp!=')')
            {
            	valp++;
            }
            if (*valp==')')
            {
            	valp++;
            }
        }
        float fv; int iv;
        switch (key)
        {
            case 'T': if (parse_float(valp,&fv)){ r->T_sec=fv; r->has_T=1; } break;
            case 'V': if (parse_int  (valp,&iv)){ r->overload=iv; } break;
            case 'R': if (parse_float(valp,&fv)){ r->RMS_dB=fv; r->has_RMS=1; } break;
            case 'Q': if (parse_float(valp,&fv)){ r->PEAK_dB=fv; r->has_PEAK=1; } break;
            case 'P': if (parse_float(valp,&fv)){ r->PP_dB=fv;   r->has_PP=1; } break; // P–P
            case 'M': if (parse_float(valp,&fv)){ r->MTVV_dB=fv; r->has_MTVV=1; } break;
            case 'H': if (parse_float(valp,&fv)){ r->VDV_dB=fv;  r->has_VDV=1; } break;
            case 'v': if (parse_float(valp,&fv)){ r->VEC_dB=fv;  r->has_VEC=1; } break;
            default: break;
        }
    }
    return 1;
}


// Decide if a #2 reply is SLM or VLM by presence of S/R/U vs Q/H/v etc.
// You can also track what you last requested.
static void dispatch_hash2_line(const char* line)
{
	// quick sniff: if it contains ",S" or ",U" it's SLM; if it contains ",Q" or ",H" it's VLM
	if (strstr(line, ",S") || strstr(line, ",U") || strstr(line, ",L"))
	{
		svan_slm_res_t r;
		if (parse_hash2_slm(line, &r))
		{
			svan_on_slm(&r);
		}
	}
	else
	{
		svan_vlm_res_t r;
		if (parse_hash2_vlm(line, &r))
		{
			svan_on_vlm(&r);
		}
	}
}

// Line Assembler and Processor
void svan_process(void)
{
	static char line[320];
	static int pos = 0;
	uint8_t b;

	while (rxBuff_get(&b))
	{
		if (pos < (int)sizeof(line)-1)
		{
			line[pos++] = (char)b;
			if (b == ';')
			{
				// complete frame
				line[pos]=0;
				if (strncmp(line,"#2,", 3) == 0)
				{
					dispatch_hash2_line(line);
				}
				// handle other replies (#1 acknowledgements etc.) as needed
				pos=0;
			}
		}
		else
		{
			pos=0; // overflow -> reset
		}
	}
}


// Helpers to compute p index
static inline int p_from_ch_profile(int ch, int profile)
{
	return ch + 4*(profile - 1);
}

// Poll requests
void svan_poll_slm(uint8_t ch, uint8_t profile)
{
    int p = p_from_ch_profile(ch, profile);
    // T,V,P,M,N,S (SPL), R (LEQ), U (SEL)
    uart_sendf("#2,c,%d,T?,V?,P?,M?,N?,S?,R?,U?;", p);
}

void svan_poll_vlm(uint8_t ch)
{
    int p = p_from_ch_profile(ch, 1);
    // T,V, Q(PEAK), P(P-P), M(MTVV), R(RMS), H(VDV), v(Vector)
    uart_sendf("#2,c,%d,T?,V?,Q?,P?,M?,R?,H?,v?;", p);
}

// --------- Basic session config ----------
void svan_config_and_start(void)
{
    // Ensure STOP state (settings require S0)
    uart_send_str("#1,S0;");

    // Level Meter function (M1) – it runs SLM/VLM together as meter; spectra are separate functions
    uart_send_str("#1,M1;");

    // Example: force Ch1..3 to Vibration, Ch4 to Sound
    // Z0:n = Vibration, Z1:n = Sound  (Appendix A.10 – control codes)
    uart_send_str("#1,Z0:1,Z0:2,Z0:3,Z1:4;");

    // Integration period 10 s (Dnns), infinite cycles (K0), Linear RMS/LEQ integration (L0)
    uart_send_str("#1,D10s,K0,L0;");

    // Start measurement
    uart_send_str("#1,S1;");
}

// --------- Weak callbacks ----------
__attribute__((weak)) void svan_on_slm(const svan_slm_res_t* r)
{
    // Default: print via ITM / semihosting / another UART (implement your own printf)
    // Example minimal: (replace with your logging)
    // printf("[SLM p=%d] T=%.0fs LEQ=%.2f dB SPL=%.2f dB PEAK=%.1f MAX=%.1f MIN=%.1f\r\n",
    //        r->p, r->T_sec, r->LEQ_dB, r->SPL_dB, r->PEAK_dB, r->MAX_dB, r->MIN_dB);
	sprintf((char *)&transmitBuff[0], (const char *)"%.1f", r->PEAK_dB);
	sprintf((char *)&transmitBuff[4], (const char *)"%.1f", r->MAX_dB);
    (void)r;
}

__attribute__((weak)) void svan_on_vlm(const svan_vlm_res_t* r)
{
    // printf("[VLM p=%d] T=%.0fs RMS=%.2f dB PEAK=%.1f P-P=%.1f MTVV=%.1f VDV=%.1f VEC=%.1f\r\n",
    //        r->p, r->T_sec, r->RMS_dB, r->PEAK_dB, r->PP_dB, r->MTVV_dB, r->VDV_dB, r->VEC_dB);
    (void)r;
}







