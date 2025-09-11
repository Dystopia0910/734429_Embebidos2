/**
 * @file    ADC.h
 *
 * @brief ADC module for reading potentiometer input in Real-Time System.
 *
 * This module configures and manages the ADC1 peripheral to sample analog values
 * from a potentiometer connected to the K64. It provides functions to initialize
 * the ADC, start conversions, and retrieve the average of the last 5 samples.
 *
 * The module uses interrupt-based conversion with a circular buffer to store
 * readings, allowing non-blocking operation suitable for real-time systems.
 * Typical usage includes reading simulated temperature values for monitoring.
 *
 * @par
 * Rodriguez Padilla, Daniel Jiram
 * IE703331
 * Martin del Campo, Mauricio
 * IE734429
 */

#ifndef ADC_H_
#define ADC_H_

#include "fsl_adc16.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------- Hardware selection (adjust to your board) -------- */
#define ADC16_BASE              ADC1
#define ADC16_CHANNEL_GROUP     0U
#define ADC16_DEFAULT_CHANNEL   0U   /* Set your potentiometer channel */

/* -------- Scaling constants -------- */
#define ADC_FULL_SCALE_COUNTS   4096U   /* 12-bit ADC */
#define TEMP_MAX_C              40U     /* Map 0..4095 -> 0..40 °C (demo scaling) */

/* -------- Sampling policy -------- */
#define ADC_SAMPLE_PERIOD_MS    20U     /* One conversion every 20 ms */
#define ADC_AVG_WINDOW          5U      /* Rolling average of last 5 samples */

/* -------- ASCII helpers (moved from CONSOLE) -------- */
#define ASCII_ZERO              48      /* '0' */
#define ASCII_DOT_CHAR          '.'     /* '.' */

/* Public ASCII buffer for temperature string "dd.d" (length 4). */
extern uint8_t gAdcTempAscii[4];

/* -------- Public API -------- */

/* Initialize ADC in software-trigger mode (polling, no interrupts). */
void ADC_InitModule(void);

/* Optional: change ADC channel at runtime. */
void ADC_SetChannel(uint32_t channel);

/* Call this periodically from your thread (e.g., every 10 ms).
 * - Triggers a conversion strictly every 20 ms.
 * - Performs POLLING until conversion is done (no IRQ).
 * - Pushes the sample into a 5-point rolling average buffer.
 */
void ADC_Service(uint32_t tick_ms);

/* Last temperature in °C (integer 0..40). */
uint8_t ADC_GetLastTempC(void);

/* Rolling average of last up-to-5 samples (integer °C). */
uint8_t ADC_GetAvgTempC(void);

/* Format integer °C into gAdcTempAscii as "dd.d" (e.g., 23 -> '2','3','.','0'). */
void ADC_FormatTempToAscii(uint8_t temp_c);

/* Convenience: format the rolling average into gAdcTempAscii. */
void ADC_FormatAvgTempToAscii(void);

#endif /* ADC_H_ */
