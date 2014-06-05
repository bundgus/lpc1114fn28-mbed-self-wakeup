#include "mbed.h"
#include "SelfWakeup.h"

#define sleepmode swu.deepSleep
//#define sleepmode wait_ms

SelfWakeup swu;

int main() {
	sleepmode(2000);
}
