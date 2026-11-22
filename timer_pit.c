#include "timer_pit.h"
#include "led.h"
#include "buzzer_pwm.h"

void init_Timer(void) {
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR = 0x00;

    PIT->CHANNEL[0].LDVAL = PIT0_10MS_LDVAL;
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

    NVIC_EnableIRQ(PIT0_IRQn);
}

void PIT0_IRQHandler(void) {
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    sys_ticks++;

    if (is_running) {
        stopwatch_tick++;
        if (stopwatch_tick >= STOPWATCH_SPEED) {
            stopwatch_tick = 0;
            time_count++;
            if (time_count > 9999) {
                time_count = 0;
            }
            toggle_LED_Heartbeat();
        }
    }

    if (is_playing_song) {
        tempo_tick++;
        if (tempo_tick >= MELODY_SPEED) {
            tempo_tick = 0;
            note_tick++;
            if (note_tick >= melody_durations[note_index]) {
                note_tick = 0;
                note_index++;
                if (note_index >= SONG_LENGTH) {
                    note_index = 0;
                }
                set_High_Tone(melody_notes[note_index]);
            }
        }
    }
}
