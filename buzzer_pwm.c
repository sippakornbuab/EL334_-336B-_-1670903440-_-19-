#include "buzzer_pwm.h"

static const uint16_t note_pwm_freq[] = {0, 2000, 2500, 3200, 4000, 5500, 4500};
static const uint8_t  note_pwm_duty[] = {0,   35,   45,   60,   80,  100,  100};

const uint8_t melody_notes[SONG_LENGTH] = {
    NOTE_E5, REST, NOTE_E5, REST, NOTE_E5, REST,
    NOTE_E5, REST, NOTE_E5, REST, NOTE_E5, REST,
    NOTE_E5, REST, NOTE_G5, REST, NOTE_C5, REST, NOTE_D5, REST, NOTE_E5, REST,
    NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_F5, REST, NOTE_E5, REST, NOTE_E5, REST,
    NOTE_E5, REST, NOTE_D5, REST, NOTE_D5, REST, NOTE_E5, REST, NOTE_D5, REST, NOTE_G5, REST
};

const uint8_t melody_durations[SONG_LENGTH] = {
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 3, 1, 1, 1, 6, 3,
    2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1,
    2, 1, 2, 1, 2, 1, 2, 1, 4, 1, 6, 4
};

void init_PWM_Buzzer(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;

    PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
    PTA->PDDR &= ~MASK(BUZZER_PIN);

    FTM0->MODE |= FTM_MODE_WPDIS_MASK;
    FTM0->SC = 0x00;

    FTM0->CONTROLS[6].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;
    FTM0->CONTROLS[6].CnV = 0;

    FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}

void set_High_Tone(uint8_t note_id) {
    uint16_t freq;
    uint8_t duty;
    uint32_t mod_val;
    uint32_t cnv_val;

    if (note_id == REST || note_id > BEEP_TONE) {
        PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
        PTA->PDDR &= ~MASK(BUZZER_PIN);
        FTM0->CONTROLS[6].CnV = 0;
        return;
    }

    freq = note_pwm_freq[note_id];
    duty = note_pwm_duty[note_id];

    mod_val = (BUS_CLOCK_HZ / freq) - 1;

    if (duty >= 100) {
        cnv_val = mod_val + 1;
    } else {
        cnv_val = (mod_val * duty) / 100;
    }

    FTM0->CNT = 0;
    FTM0->MOD = mod_val;
    FTM0->CONTROLS[6].CnV = cnv_val;

    PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(3);
}
