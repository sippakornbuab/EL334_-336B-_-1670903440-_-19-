#ifndef LED_H
#define LED_H

#include "config.h"

void init_LEDs(void);
void set_LEDs_Running(void);
void set_LEDs_Stopped(void);
void toggle_LED_Heartbeat(void);

#endif // LED_H
