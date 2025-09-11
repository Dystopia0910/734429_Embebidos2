/**
 * @file    ADC.c
 *
 * @brief Implementation of ADC module for potentiometer input.
 *
 * This file initializes the ADC1 peripheral and provides functionality to perform
 * interrupt-based conversions using software triggering. The results are stored
 * in a circular buffer and averaged over the last 5 samples, providing stable
 * readings for temperature simulation.
 *
 * Used in the Real-Time System to read potentiometer values representing
 * simulated temperature, with data processing done in dedicated threads
 * rather than interrupt context to maintain system responsiveness.
 *
 * @par
 * Rodriguez Padilla, Daniel Jiram
 * IE703331
 * Martin del Campo, Mauricio
 * IE734429
 */

#include "ADC.h"

/* ================= Module state ================= */

/* Active channel (configurable) */
static volatile uint32_t s_activeChannel = ADC16_DEFAULT_CHANNEL;

/* 20 ms scheduler reference (driven by caller's tick_ms) */
static uint32_t s_lastKickMs = 0;

/* Rolling buffer (5 samples) for integer °C */
static uint8_t  s_tempBuf[ADC_AVG_WINDOW] = {0};
static uint8_t  s_tempCount = 0;   /* Valid samples (<=5) */
static uint8_t  s_tempIndex = 0;   /* Circular idx [0..4] */

/* Last converted temperature (integer °C) */
static uint8_t  s_lastTempC = 0;

/* ASCII output buffer "dd.d" */
uint8_t gAdcTempAscii[4] = { '0','0','.', '0' };

/* ================= Internal helpers ================= */

/* Integer mapping: raw counts (0..4095) -> °C (0..40). */
static inline uint8_t ADC_CountsToTempC(uint16_t counts){
    uint32_t t = ((uint32_t)counts * (uint32_t)TEMP_MAX_C) / (uint32_t)ADC_FULL_SCALE_COUNTS;
    if (t > TEMP_MAX_C) t = TEMP_MAX_C;
    return (uint8_t)t;
}

/* Push one temperature sample into rolling buffer (size 5). */
static inline void ADC_PushTemp(uint8_t temp_c){
    s_tempBuf[s_tempIndex] = temp_c;
    s_tempIndex = (uint8_t)((s_tempIndex + 1U) % ADC_AVG_WINDOW);
    if (s_tempCount < ADC_AVG_WINDOW){
        s_tempCount++;
    }
}

/* Do ONE blocking conversion on s_activeChannel (POLLING). */
static uint16_t ADC_ConvertOnce_Polling(void){
    adc16_channel_config_t ch = {0};
    ch.channelNumber = s_activeChannel;
    ch.enableInterruptOnConversionCompleted = false;  /* polling, no IRQ */
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    ch.enableDifferentialConversion = false;
#endif

    /* Writing the channel config in software-trigger mode starts the conversion */
    ADC16_SetChannelConfig(ADC16_BASE, ADC16_CHANNEL_GROUP, &ch);

    /* Busy-wait until conversion completes (poll status flag) */
    while (!(kADC16_ChannelConversionDoneFlag &
             ADC16_GetChannelStatusFlags(ADC16_BASE, ADC16_CHANNEL_GROUP)))
    {
        /* INTENTIONALLY EMPTY: this is the polling loop by spec */
    }

    /* Read result and return */
    return ADC16_GetChannelConversionValue(ADC16_BASE, ADC16_CHANNEL_GROUP);
}

/* ================= Public API ================= */

void ADC_InitModule(void){
    adc16_config_t cfg;
    ADC16_GetDefaultConfig(&cfg);

    /* Initialize and ensure software trigger mode (no hardware trigger) */
    ADC16_Init(ADC16_BASE, &cfg);
    ADC16_EnableHardwareTrigger(ADC16_BASE, false);

    /* Optional accuracy features (enable if desired)
     * ADC16_SetHardwareAverage(ADC16_BASE, kADC16_HardwareAverageCount4);
     */

    /* Reset module state */
    s_activeChannel = ADC16_DEFAULT_CHANNEL;
    s_lastKickMs    = 0U;
    s_tempIndex     = 0U;
    s_tempCount     = 0U;
    s_lastTempC     = 0U;

    /* No NVIC setup: we DO NOT use ADC interrupts in polling mode */
}

void ADC_SetChannel(uint32_t channel){
    s_activeChannel = channel;
}

/* Periodic service:
 * - Caller supplies current tick in ms (monotonic).
 * - If 20 ms elapsed, perform ONE blocking conversion (polling).
 * - Convert to °C and push into rolling buffer.
 */
void ADC_Service(uint32_t tick_ms){
    if ((uint32_t)(tick_ms - s_lastKickMs) >= (uint32_t)ADC_SAMPLE_PERIOD_MS){
        s_lastKickMs = tick_ms;

        /* Do a single blocking conversion by POLLING */
        uint16_t counts = ADC_ConvertOnce_Polling();

        /* Post-process entirely outside any ISR (sequential execution) */
        s_lastTempC = ADC_CountsToTempC(counts);
        ADC_PushTemp(s_lastTempC);
    }
}

uint8_t ADC_GetLastTempC(void){
    return s_lastTempC;
}

uint8_t ADC_GetAvgTempC(void){
    if (s_tempCount == 0U) return 0U;

    uint16_t acc = 0U;
    for (uint8_t i = 0U; i < s_tempCount; i++){
        acc += s_tempBuf[i];
    }
    return (uint8_t)(acc / (uint16_t)s_tempCount);
}

/* Format integer temperature to "dd.d" ASCII ("fixed .0"). */
void ADC_FormatTempToAscii(uint8_t temp_c){
    uint8_t tens  = (uint8_t)(temp_c / 10U);
    uint8_t units = (uint8_t)(temp_c % 10U);

    gAdcTempAscii[0] = (uint8_t)(ASCII_ZERO + tens);
    gAdcTempAscii[1] = (uint8_t)(ASCII_ZERO + units);
    gAdcTempAscii[2] = (uint8_t)ASCII_DOT_CHAR;
    gAdcTempAscii[3] = (uint8_t)(ASCII_ZERO + 0U); /* ".0" fixed */
}

void ADC_FormatAvgTempToAscii(void){
    ADC_FormatTempToAscii(ADC_GetAvgTempC());
}
