/*
 * SelfWakeup.cpp
 *
 *  Created on: May 27, 2014
 *      Author: bundgus
 */

#include "SelfWakeup.h"
#include "LPC11xx.h"

extern "C" void FLEX_INT1_IRQHandler(void) {

	// Clear pending start logic interrupt
	LPC_SYSCON->STARTRSRP0CLR |= (1 << (1));
	return;
}

SelfWakeup::SelfWakeup() {
	// TODO Auto-generated constructor stub

}

SelfWakeup::~SelfWakeup() {
	// TODO Auto-generated destructor stub
}

void SelfWakeup::Init_Timer32(uint32_t SystemFreq, uint32_t matchOutputInterval) {

	// 	SystemFreq is the freq output of the AHBCLKDIV
	volatile uint32_t prescaler = (SystemFreq / 100) - 1;
	volatile uint32_t match = (matchOutputInterval) - 1;
	// Turn on 32-bit Timer 0 and IOCON
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9) | (1 << 16);
	// Disable and Reset 32-bit Timer 0
	LPC_TMR32B0->TCR = (1 << 1);
	// Setup 32-bit Timer 0 prescaler
	LPC_TMR32B0->PR = prescaler;
	// 32-bit Timer 0 Match Reg 0: Interrupt Disabled, Reset Enabled, Do not Stop
	LPC_TMR32B0->MCR = (1 << 7);
	// 32-bit Timer 0 Match Reg 2 value
	LPC_TMR32B0->MR2 = match;
	// 32-bit Timer 0 External Match; Clear output and set on match
	LPC_TMR32B0->EMR = (0x2 << 8);
	// 32-bit Timer 0 Enable MAT0 onto IOCON: CT32BO_MAT0: PIO0_1
	LPC_IOCON->PIO0_1 = (2 << 0);
	return;
}

void SelfWakeup::deepSleep(uint32_t sleep_ms){

	// Configure Wakeup I/O
	// Specify the start logic to allow the chip to be waken up
	LPC_SYSCON->STARTAPRP0 |= (1 << (1));		// Rising edge
	LPC_SYSCON->STARTRSRP0CLR |= (1 << (1));		// Clear pending bit
	LPC_SYSCON->STARTERP0 |= (1 << (1));		// Enable Start Logic

	// Initialize 32-bit Timer 0
	Init_Timer32(1000, sleep_ms);

	// Turn off all other peripheral dividers
	//LPC_SYSCON->SSP0CLKDIV = 0;
	//LPC_SYSCON->SSP1CLKDIV = 0;
	//LPC_SYSCON->WDTCLKDIV = 0;

	// Turn on the watchdog oscillator
	LPC_SYSCON->PDRUNCFG &= ~(1 << 6);

	// Run watchdog oscillator
	//SLOWEST WDT FREQ 0.6 MHz, 11111: 2 *(1 + DIVSEL) = 64
	//600KHz/64 = 9375 Hz
	LPC_SYSCON->WDTOSCCTRL = (0x1 << 5) | 0x1F;
	// Switch MAINCLKSEL to Watchdog Oscillator
	LPC_SYSCON->MAINCLKSEL = 2;
	LPC_SYSCON->MAINCLKUEN = 0;
	LPC_SYSCON->MAINCLKUEN = 1;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));

	// Make sure that only the watchdog oscillator & Flash are running now
	// LPC_SYSCON->PDRUNCFG = ~((1<<6) | (1<<2) | (1<<9) | (1<<12));
	// Initialize 32-bit Timer 0
	// Remove Reset and Enable Timer
	LPC_TMR32B0->TCR = (1 << 0);
	//Enable_Timer32();

	// Clear the Deep Sleep Flag
	LPC_PMU->PCON |= (1 << 8);

	// Specify peripherals to be powered up again when returning from deep sleep mode
	LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;

	// Configure PDSLEEPCFG
	LPC_SYSCON->PDSLEEPCFG = 0x000018BF;

	// Specify Deep Sleep mode before entering mode
	SCB->SCR |= (1 << 2);		//Set SLEEPDEEP bit

	LPC_SYSCON->STARTRSRP0CLR |= (1 << (1));
	NVIC_ClearPendingIRQ(WAKEUP1_IRQn);
	NVIC_EnableIRQ(WAKEUP1_IRQn);

	// Go to Sleep
	__WFI();

	// Clear the match output register value
	LPC_TMR32B0->EMR &= ~(0x2 << 8);
	//DeInit_Timer32();
	// Disable and Reset
	LPC_TMR32B0->TCR = (1 << 1);
	// Turn of 32-bit Timer 0
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 9);

	// switch back to pll output clock
	LPC_SYSCON->MAINCLKSEL = 3;
	LPC_SYSCON->MAINCLKUEN = 0;
	LPC_SYSCON->MAINCLKUEN = 1;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));

	LPC_TMR16B0->TCR = 0;
	// Clear the match output register value
	LPC_TMR32B0->EMR &= ~(0x2 << 8);
	//Clear_MAT032();                // Clear match bit on timer
	LPC_SYSCON->STARTRSRP0CLR = (1 << 1); // Clear pending bit on start logic
	//LPC_SYSCON->SYSAHBCLKCTRL = 0x183DF;        // SYSAHBCLKCTRL ACTIVE
}
