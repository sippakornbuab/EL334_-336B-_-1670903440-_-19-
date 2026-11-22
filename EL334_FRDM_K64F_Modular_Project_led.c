#include "led.h"

void init_LEDs(void) {
    // Enable clock gating for Port D
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

    // Configure PTD0, PTD1, PTD2, PTD3 as GPIO (MUX = 1)
    PORTD->PCR[LED_D1_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D2_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D3_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_D4_PIN] = PORT_PCR_MUX(1);

    // Set LED pins as Outputs
    PTD->PDDR |= (MASK(LED_D1_PIN) | MASK(LED_D2_PIN) | MASK(LED_D3_PIN) | MASK(LED_D4_PIN));

    // Initial Idle State (Active-Low LEDs: PSOR = OFF, PCOR = ON)
    PTD->PSOR = (MASK(LED_D2_PIN) | MASK(LED_D4_PIN));
    PTD->PCOR = (MASK(LED_D1_PIN) | MASK(LED_D3_PIN));
}

void set_LEDs_Running(void) {
    PTD->PSOR = MASK(LED_D1_PIN) | MASK(LED_D3_PIN); // Turn OFF D1 and D3
    PTD->PCOR = MASK(LED_D2_PIN);                    // Turn ON D2 (Running indicator)
}

void set_LEDs_Stopped(void) {
    PTD->PCOR = MASK(LED_D1_PIN);                    // Turn ON D1 (Stopped indicator)
    PTD->PSOR = MASK(LED_D2_PIN) | MASK(LED_D4_PIN); // Turn OFF D2 and D4
}

void toggle_LED_Heartbeat(void) {
    PTD->PTOR = MASK(LED_D4_PIN);                    // Toggle D4 every 1.00s
}
