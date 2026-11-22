#ifndef TIMER_PIT_H
#define TIMER_PIT_H

#include "config.h"

void init_Timer(void);
void PIT0_IRQHandler(void);

#endif // TIMER_PIT_H
