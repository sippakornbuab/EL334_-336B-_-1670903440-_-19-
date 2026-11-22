#include "buzzer_pwm.h"

// High-Frequency PWM Carrier Frequencies (Hz) & Duty Cycles (%)
static const uint16_t note_pwm_freq[] = {0, 2000, 2500, 3200, 4000, 5500, 4500};
static const uint8_t  note_pwm_duty[] = {0,   35,   45,   60,   80,  100,  100};

// Jingle Bells Melody Notes (High Octave)
const uint8_t melody_notes[SONG_LENGTH] = {
    // Phrase 1: Jin-gle bells, jin-gle bells (E5 - E5 - E5, E5 - E5 - E5)
    NOTE_E5, REST, NOTE_E5, REST, NOTE_E5, REST,
    NOTE_E5, REST, NOTE_E5, REST, NOTE_E5, REST,
    // Phrase 2: Jin-gle all the way (E5 - G5 - C5 - D5 - E5)
    NOTE_E5, REST, NOTE_G5, REST, NOTE_C5, REST, NOTE_D5, REST, NOTE_E5, REST,
    // Phrase 3: Oh what fun it is to ride (F5 - F5 - F5 - F5 - F5 - E5 - E5)
    NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_E5, REST, NOTE_E5, REST,
    // Phrase 4: In a one-horse o-pen sleigh (E5 - D5 - D5 - E5 - D5 - G5)
    NOTE_E5, REST, NOTE_D5, REST, NOTE_D5, REST, NOTE_E5, REST, NOTE_D5, REST, NOTE_G5, REST
};

// Song Tempo Durations (in MELODY_SPEED units = 0.12s per unit)
const uint8_t melody_durations[SONG_LENGTH] = {
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 3, 1, 1, 1, 6, 3,
    2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1,
    2, 1, 2, 1, 2, 1, 2, 1, 4, 1, 6, 4
};

void init_PWM_Buzzer(void) {
    // Enable clock gating for Port A and FlexTimer 0 (FTM0)
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;

    // Keep PTA1 as GPIO Input (Hi-Z) initially so Active-Low buzzer stays silent
    PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
    PTA->PDDR &= ~MASK(BUZZER_PIN);

    // Disable write protection and stop timer before configuration
    FTM0->MODE |= FTM_MODE_WPDIS_MASK;
    FTM0->SC = 0x00;

    // Configure Channel 6 (PTA1) for Edge-Aligned Low-True PWM (MSB=1, ELSA=1)
    FTM0->CONTROLS[6].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;
    FTM0->CONTROLS[6].CnV = 0;

    // Select Bus Clock (20.97152 MHz) with Prescaler = 1 (PS = 0)
    FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}

void set_High_Tone(uint8_t note_id) {
    uint16_t freq;
    uint8_t duty;
    uint32_t mod_val;
    uint32_t cnv_val;

    if (note_id == REST || note_id > BEEP_TONE) {
        // Switch PTA1 back to GPIO Input (Hi-Z) to prevent 5V leakage hum
        PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
        PTA->PDDR &= ~MASK(BUZZER_PIN);
        FTM0->CONTROLS[6].CnV = 0;
        return;
    }

    freq = note_pwm_freq[note_id];
    duty = note_pwm_duty[note_id];

    // Calculate MOD = (BUS_CLOCK_HZ / freq) - 1
    mod_val = (BUS_CLOCK_HZ / freq) - 1;

    if (duty >= 100) {
        cnv_val = mod_val + 1;
    } else {
        cnv_val = (mod_val * duty) / 100;
    }

    FTM0->CNT = 0;
    FTM0->MOD = mod_val;
    FTM0->CONTROLS[6].CnV = cnv_val;

    // Route FTM0_CH6 PWM output to PTA1 (MUX = 3)
    PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(3);
}
