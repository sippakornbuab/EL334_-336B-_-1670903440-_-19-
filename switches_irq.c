#include "switches_irq.h"
#include "led.h"
#include "buzzer_pwm.h"

void init_Switches(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

    PORTB->PCR[SW1_PIN] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[SW2_PIN] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[SW3_PIN] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    PTB->PDDR &= ~(MASK(SW1_PIN) | MASK(SW2_PIN) | MASK(SW3_PIN));

    NVIC_EnableIRQ(PORTB_IRQn);
}

void PORTB_IRQHandler(void) {
    uint32_t isfr = PORTB->ISFR;
    PORTB->ISFR = 0xFFFFFFFF;

    if ((sys_ticks - last_press_tick) < DEBOUNCE_TICKS) {
        return;
    }

    if (isfr & MASK(SW1_PIN)) {
        last_press_tick = sys_ticks;
        if (!is_playing_song) {
            set_High_Tone(BEEP_TONE);
        }

        is_running = !is_running;
        if (is_running) {
            set_LEDs_Running();
        } else {
            set_LEDs_Stopped();
        }
    }
    else if (isfr & MASK(SW3_PIN)) {
        last_press_tick = sys_ticks;
        is_playing_song = !is_playing_song;

        if (is_playing_song) {
            note_index = 0;
            note_tick = 0;
            tempo_tick = 0;
            set_High_Tone(melody_notes[0]);
        } else {
            set_High_Tone(REST);
        }
    }
    else if (isfr & MASK(SW2_PIN)) {
        last_press_tick = sys_ticks;
        time_count = 0;
        stopwatch_tick = 0;
        is_running = 0;
        set_LEDs_Stopped();
        PTD->PSOR = MASK(LED_D2_PIN) | MASK(LED_D4_PIN);
        PTD->PCOR = MASK(LED_D1_PIN) | MASK(LED_D3_PIN);
    }
}
