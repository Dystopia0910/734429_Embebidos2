/*
 * RMS.h
 *
 *  Created on: 5 sep. 2025
 *      Author: mauri
 */

#ifndef RMS_H_
#define RMS_H_

typedef enum{
	STANDBY = 0,
	READY,
	EXECUTE
}ThdState;

typedef struct{
	void(*ThreadHandler)(void);
	uint8_t ThreadState;
	uint8_t ThreadRate;
	uint8_t SystemTime;
}ThdObj;

#endif /* RMS_H_ */
