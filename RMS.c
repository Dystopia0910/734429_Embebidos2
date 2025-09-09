/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pit.h"
#include "RMS.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PIT_BASEADDR PIT
#define DEMO_PIT_CHANNEL  kPIT_Chnl_0
#define PIT_LED_HANDLER   PIT0_IRQHandler
#define PIT_IRQ_ID        PIT0_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define LED_INIT()       LED_RED_INIT(LOGIC_LED_ON)
#define LED_TOGGLE()     LED_RED_TOGGLE()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void Thd_2ms(void);
void Thd_5ms(void);
void Thd_10ms(void);
void Thd_idle(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool pitIsrFlag = false;

ThdObj ThreadTable[] = {
		{Thd_2ms, STANDBY, 2, 0},
		{Thd_5ms, STANDBY, 5, 0},
		{Thd_10ms, STANDBY, 10, 0},
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void PIT_LED_HANDLER(void)
{
    static uint8_t tableCounter = 0;

	/* Clear interrupt flag.*/
    PIT_ClearStatusFlags(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, kPIT_TimerFlag);
    pitIsrFlag = true;
    /* Added for, and affects, all PIT handlers. For CPU clock which is much larger than the IP bus clock,
     * CPU can run out of the interrupt handler before the interrupt flag being cleared, resulting in the
     * CPU's entering the handler again and again. Adding DSB can prevent the issue from happening.
     */

    for(tableCounter=0;tableCounter<3;tableCounter++){
    	ThreadTable[tableCounter].SystemTime++;
    	if(ThreadTable[tableCounter].SystemTime>=ThreadTable[tableCounter].ThreadRate){
    		ThreadTable[tableCounter].ThreadState = READY;
    		ThreadTable[tableCounter].SystemTime = 0;
    	}
    }

    __DSB();
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t tableCounter;

	/* Structure of initialize PIT */
    pit_config_t pitConfig;

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Initialize and enable LED */
    LED_INIT();

    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(DEMO_PIT_BASEADDR, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, USEC_TO_COUNT(1000U, PIT_SOURCE_CLOCK));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

    /* Start channel 0 */
    PRINTF("\r\nStarting channel No.0 ...");
    PIT_StartTimer(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL);

    while (true)
    {
        for(tableCounter=0;tableCounter<3;tableCounter++){
        	if(ThreadTable[tableCounter].ThreadState == READY){
        		ThreadTable[tableCounter].ThreadState = EXECUTE;
        		ThreadTable[tableCounter].ThreadHandler();
        		ThreadTable[tableCounter].ThreadState = STANDBY;
        	}
        	else{
        		Thd_idle();
        	}
        }
    }
}

void Thd_2ms(void){
	volatile static uint8_t counter = 0;
	counter++;
}

void Thd_5ms(void){
	volatile static uint8_t counter = 0;
	counter++;
}

void Thd_10ms(void){
	volatile static uint8_t counter = 0;
	counter++;
}

void Thd_idle(void){}
