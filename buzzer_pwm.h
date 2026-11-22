#ifndef BUZZER_PWM_H
#define BUZZER_PWM_H

#include "config.h"

#define NOTE_C5     1   // High Do  (2000 Hz)
#define NOTE_D5     2   // High Re  (2500 Hz)
#define NOTE_E5     3   // High Mi  (3200 Hz)
#define NOTE_F5     4   // High Fa  (4000 Hz)
#define NOTE_G5     5   // High Sol (5500 Hz)
#define BEEP_TONE   6   // High-pitch tactile feedback for S1 press (4500 Hz)
#define REST        0   // Clean Mute (Hi-Z Input)

#define SONG_LENGTH 48

extern const uint8_t melody_notes[SONG_LENGTH];
extern const uint8_t melody_durations[SONG_LENGTH];

void init_PWM_Buzzer(void);
void set_High_Tone(uint8_t note_id);

#endif // BUZZER_PWM_H
