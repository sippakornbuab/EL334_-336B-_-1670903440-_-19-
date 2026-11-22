#include "seven_segment.h"

// Common-Anode 7-Segment Encoding (0 - 9)
static const uint8_t seg_code[10] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90
};

// Digit selection ordered from Rightmost (Units: 0x08) to Leftmost (Thousands: 0x01)
static const uint8_t dig_select[4] = {
    0x08, 0x04, 0x02, 0x01
};

void init_7Segment(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK;

    // Shift Clock (PTC3)
    PORTC->PCR[SEG_CLK_PIN] = PORT_PCR_MUX(1);
    PTC->PDDR |= MASK(SEG_CLK_PIN);

    // Storage Latch (PTC6 & PTB23)
    PORTC->PCR[SEG_LATCH_C_PIN] = PORT_PCR_MUX(1);
    PORTB->PCR[SEG_LATCH_B_PIN] = PORT_PCR_MUX(1);
    PTC->PDDR |= MASK(SEG_LATCH_C_PIN);
    PTB->PDDR |= MASK(SEG_LATCH_B_PIN);

    // Serial Data (PTC12 & PTA0)
    PORTC->PCR[SEG_DATA_C_PIN] = PORT_PCR_MUX(1);
    PORTA->PCR[SEG_DATA_A_PIN] = PORT_PCR_MUX(1);
    PTC->PDDR |= MASK(SEG_DATA_C_PIN);
    PTA->PDDR |= MASK(SEG_DATA_A_PIN);
}

void shift_out(uint8_t data) {
    int i;
    for (i = 7; i >= 0; i--) {
        PTC->PCOR = MASK(SEG_CLK_PIN);
        __asm("nop");
        if (data & (1 << i)) {
            PTC->PSOR = MASK(SEG_DATA_C_PIN);
            PTA->PSOR = MASK(SEG_DATA_A_PIN);
        } else {
            PTC->PCOR = MASK(SEG_DATA_C_PIN);
            PTA->PCOR = MASK(SEG_DATA_A_PIN);
        }
        __asm("nop");
        PTC->PSOR = MASK(SEG_CLK_PIN);
        __asm("nop");
    }
}

void display_7Segment(uint16_t number) {
    uint8_t digits[4];
    int i;
    volatile int d;

    digits[0] = number % 10;           // Units (Rightmost)
    digits[1] = (number / 10) % 10;    // Tens
    digits[2] = (number / 100) % 10;   // Hundreds
    digits[3] = (number / 1000) % 10;  // Thousands (Leftmost)

    for (i = 0; i < 4; i++) {
        // Pull Latch LOW before shifting 16 bits
        PTC->PCOR = MASK(SEG_LATCH_C_PIN);
        PTB->PCOR = MASK(SEG_LATCH_B_PIN);

        shift_out(seg_code[digits[i]]);
        shift_out(dig_select[i]);

        // Pull Latch HIGH to update 7-Segment output
        PTC->PSOR = MASK(SEG_LATCH_C_PIN);
        PTB->PSOR = MASK(SEG_LATCH_B_PIN);

        // Short persistence delay for clean brightness
        for (d = 0; d < 2500; d++);
    }
}
