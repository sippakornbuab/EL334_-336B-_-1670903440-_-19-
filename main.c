#include "config.h"
#include "led.h"
#include "buzzer_pwm.h"
#include "seven_segment.h"
#include "switches_irq.h"
#include "timer_pit.h"

volatile uint16_t time_count = 0;
volatile uint8_t  is_running = 0;
volatile uint8_t  stopwatch_tick = 0;
volatile uint32_t sys_ticks = 0;
volatile uint32_t last_press_tick = 0;

volatile uint8_t  is_playing_song = 0;
volatile uint8_t  note_index = 0;
volatile uint8_t  note_tick = 0;
volatile uint8_t  tempo_tick = 0;

int main(void) {
    init_LEDs();
    init_PWM_Buzzer();
    init_Switches();
    init_Timer();
    init_7Segment();

    while (1) {
        if (!is_playing_song && ((PTB->PDIR & MASK(SW1_PIN)) != 0)) {
            set_High_Tone(REST);
        }

        display_7Segment(time_count);
    }

    return 0;
}
