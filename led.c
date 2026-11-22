#include "led.h"

void init_LEDs(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

    PORTD->PCR[LED_D1_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D2_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D3_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D4_PIN] = PORT_PCR_MUX(1);

    PTD->PDDR |= (MASK(LED_D1_PIN) | MASK(LED_D2_PIN) | MASK(LED_D3_PIN) | MASK(LED_D4_PIN));

    PTD->PSOR = (MASK(LED_D2_PIN) | MASK(LED_D4_PIN));
    PTD->PCOR = (MASK(LED_D1_PIN) | MASK(LED_D3_PIN));
}

void set_LEDs_Running(void) {
    PTD->PSOR = MASK(LED_D1_PIN) | MASK(LED_D3_PIN);
    PTD->PCOR = MASK(LED_D2_PIN);
}

void set_LEDs_Stopped(void) {
    PTD->PCOR = MASK(LED_D1_PIN);
    PTD->PSOR = MASK(LED_D2_PIN) | MASK(LED_D4_PIN);
}

void toggle_LED_Heartbeat(void) {
    PTD->PTOR = MASK(LED_D4_PIN);
}
