/*
 * SelfWakeup.h
 *
 *  Created on: May 27, 2014
 *      Author: bundgus
 */

#ifndef SELFWAKEUP_H_
#define SELFWAKEUP_H_

#include <stdint.h>

class SelfWakeup {
public:
	SelfWakeup();
	virtual ~SelfWakeup();
	void deepSleep(uint32_t sleep_ms);
private:
	void Init_Timer32(uint32_t SystemFreq, uint32_t matchOutputInterval);
	void DeInit_Timer32();
	void Enable_Timer32();
	void Disable_Timer32();
	void Clear_MAT032();

};

#endif /* SELFWAKEUP_H_ */
