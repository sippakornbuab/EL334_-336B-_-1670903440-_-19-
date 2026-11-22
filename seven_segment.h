#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include "config.h"

void init_7Segment(void);
void shift_out(uint8_t data);
void display_7Segment(uint16_t number);

#endif // SEVEN_SEGMENT_H
