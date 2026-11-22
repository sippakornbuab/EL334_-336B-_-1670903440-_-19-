#include <MK64F12.h>

#define MASK(x) (1UL << (x))

// --- Speed Configuration (in 0.01s base ticks) ---
#define STOPWATCH_SPEED 100 // S1 counting interval: 100 = exact 1.00 second
#define MELODY_SPEED    12  // S3 song tempo tick:   12 = 0.12s per beat unit (original tempo)

// --- Hardware Pin Definitions ---
#define LED_D1 1      // PTD1
#define LED_D2 3      // PTD3 
#define LED_D3 2      // PTD2 
#define LED_D4 0      // PTD0
#define BUZZER_PIN 1  // PTA1 (Connected to FTM0_CH6 on MK64F12)

// --- High-Pitch Note Definitions ---
#define NOTE_C5   1   // High Do  (2000 Hz)
#define NOTE_D5   2   // High Re  (2500 Hz)
#define NOTE_E5   3   // High Mi  (3200 Hz)
#define NOTE_F5   4   // High Fa  (4000 Hz)
#define NOTE_G5   5   // High Sol (5500 Hz)
#define BEEP_TONE 6   // High-pitch feedback for S1 press (4500 Hz)
#define REST      0   // Clean Mute (Hi-Z)

// --- High-Frequency PWM (Hz) & Duty Cycle (%) Lookup Tables ---
const uint16_t note_pwm_freq[]  = {0, 2000, 2500, 3200, 4000, 5500, 4500};
const uint8_t  note_pwm_duty[]  = {0,   35,   45,   60,   80,  100,  100};

// --- Jingle Bells Melody Notes (High Octave) ---
const uint8_t melody_notes[] = {
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

// --- Song Tempo Durations (in MELODY_SPEED units) ---
const uint8_t melody_durations[] = {
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 4, 2,
    2, 1, 2, 1, 3, 1, 1, 1, 6, 3,
    2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1,
    2, 1, 2, 1, 2, 1, 2, 1, 4, 1, 6, 4
};
#define SONG_LENGTH 48

// --- Global Variables ---
volatile uint16_t time_count = 0;      
volatile uint8_t is_running = 0;       
volatile uint8_t stopwatch_tick = 0;   // Independent tick counter for S1 stopwatch speed
volatile uint32_t sys_ticks = 0;       // Free-running 10ms tick counter for non-blocking debounce
volatile uint32_t last_press_tick = 0; // Timestamp of last valid button press

// --- Melody Control Variables (S3) ---
volatile uint8_t is_playing_song = 0;  // 0 = Off, 1 = Playing
volatile uint8_t note_index = 0;       // Current note index
volatile uint8_t note_tick = 0;        // Note duration counter
volatile uint8_t tempo_tick = 0;       // Independent tick counter for S3 melody speed

const uint8_t seg_code[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
// Digit selection ordered from Rightmost (Units: 0x08) to Leftmost (Thousands: 0x01)
const uint8_t dig_select[] = {0x08, 0x04, 0x02, 0x01}; 

// ==========================================
// 1. Peripheral Initialization Functions
// ==========================================

void init_LEDs(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
    PORTD->PCR[LED_D1] = PORT_PCR_MUX(1); 
    PORTD->PCR[LED_D2] = PORT_PCR_MUX(1); 
    PORTD->PCR[LED_D3] = PORT_PCR_MUX(1); 
    PORTD->PCR[LED_D4] = PORT_PCR_MUX(1); 
    
    PTD->PDDR |= (MASK(LED_D1) | MASK(LED_D2) | MASK(LED_D3) | MASK(LED_D4));
    PTD->PSOR = (MASK(LED_D2) | MASK(LED_D4)); 
    PTD->PCOR = (MASK(LED_D1) | MASK(LED_D3)); 
}

void init_PWM_Buzzer(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;       
    SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;        // Enable clock for FTM0 module
    
    // Keep PTA1 as GPIO Input (Hi-Z) initially so the Active-Low buzzer stays silent
    PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1); 
    PTA->PDDR &= ~MASK(BUZZER_PIN);
    
    // Configure FTM0 for High-Frequency Edge-Aligned PWM
    FTM0->MODE |= FTM_MODE_WPDIS_MASK;        // Disable write protection
    FTM0->SC = 0x00;                          // Stop timer before setup
    
    // Configure Channel 6 (PTA1): Low-true pulses (MSB=1, ELSA=1, ELSB=0)
    FTM0->CONTROLS[6].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;
    FTM0->CONTROLS[6].CnV = 0;
    
    // Select System/Bus Clock with Prescaler = 1 (PS = 0)
    FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}

void init_Switches(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    PORTB->PCR[3]  = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 
    PORTB->PCR[10] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 
    PORTB->PCR[11] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 
    
    PTB->PDDR &= ~(MASK(3) | MASK(10) | MASK(11)); 
    NVIC_EnableIRQ(PORTB_IRQn); 
}

void init_Timer(void) {
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; 
    PIT->MCR = 0x00; 
    
    // PIT Channel 0: 0.01s (10 ms) base tick at default 20.97152 MHz Bus Clock
    // 20,971,520 Hz * 0.01s = 209,715 ticks -> LDVAL = 209714 (Exact 1.00s per 100 ticks)
    PIT->CHANNEL[0].LDVAL = 209714; 
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK; 
    NVIC_EnableIRQ(PIT0_IRQn); 
}

void init_7Segment(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[3] = PORT_PCR_MUX(1); PTC->PDDR |= MASK(3);
    PORTC->PCR[6] = PORT_PCR_MUX(1); PORTB->PCR[23] = PORT_PCR_MUX(1);
    PTC->PDDR |= MASK(6); PTB->PDDR |= MASK(23);
    PORTC->PCR[12] = PORT_PCR_MUX(1); PORTA->PCR[0] = PORT_PCR_MUX(1);
    PTC->PDDR |= MASK(12); PTA->PDDR |= MASK(0);
}

// ==========================================
// 2. 7-Segment & High-Frequency PWM Functions
// ==========================================

void shift_out(uint8_t data) {
    int i;
    for (i = 7; i >= 0; i--) {
        PTC->PCOR = MASK(3); 
        __asm("nop");
        if (data & (1 << i)) {
            PTC->PSOR = MASK(12); PTA->PSOR = MASK(0);
        } else {
            PTC->PCOR = MASK(12); PTA->PCOR = MASK(0);
        }
        __asm("nop");
        PTC->PSOR = MASK(3); 
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
        PTC->PCOR = MASK(6); PTB->PCOR = MASK(23); 
        shift_out(seg_code[digits[i]]); 
        shift_out(dig_select[i]);       
        PTC->PSOR = MASK(6); PTB->PSOR = MASK(23); 
        for (d = 0; d < 2500; d++); 
    }
}

// Configure high-frequency PWM period (MOD) and duty cycle (CnV) per note
void set_High_Tone(uint8_t note_id) {
    uint16_t freq;
    uint8_t duty;
    uint32_t mod_val;
    uint32_t cnv_val;
    
    if (note_id == REST || note_id > BEEP_TONE) {
        // Switch PTA1 back to GPIO Input (Hi-Z) for clean silence between beats
        PORTA->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
        PTA->PDDR &= ~MASK(BUZZER_PIN);
        FTM0->CONTROLS[6].CnV = 0;
        return;
    }
    
    freq = note_pwm_freq[note_id];
    duty = note_pwm_duty[note_id];
    
    // Calculate MOD for 20.97152 MHz Bus Clock: MOD = (20,971,520 / freq) - 1
    mod_val = (20971520UL / freq) - 1;
    
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

// ==========================================
// 3. Interrupt Service Routines (ISR)
// ==========================================

void PORTB_IRQHandler(void) {
    uint32_t isfr = PORTB->ISFR; 
    PORTB->ISFR = 0xFFFFFFFF; // Clear all PORTB interrupt flags immediately
    
    // Non-blocking debounce: ignore bounces within 200ms (20 * 10ms ticks)
    if ((sys_ticks - last_press_tick) < 20) {
        return;
    }
    
    // --- Switch S1: Start / Pause Stopwatch + High-Pitch Beep on Press ---
    if (isfr & MASK(3)) {
        last_press_tick = sys_ticks;
        if (!is_playing_song) {
            set_High_Tone(BEEP_TONE);         // High-pitch beep while S1 is held
        }
        
        is_running = !is_running; 
        if (is_running) {
            PTD->PSOR = MASK(LED_D1) | MASK(LED_D3); 
            PTD->PCOR = MASK(LED_D2); 
        } else {
            PTD->PCOR = MASK(LED_D1); 
            PTD->PSOR = MASK(LED_D2) | MASK(LED_D4); 
        }
    }
    
    // --- Switch S3: Toggle High-Pitch PWM "Jingle Bells" Melody ---
    else if (isfr & MASK(11)) {
        last_press_tick = sys_ticks;
        is_playing_song = !is_playing_song; 
        
        if (is_playing_song) {
            note_index = 0;
            note_tick = 0;
            tempo_tick = 0;
            set_High_Tone(melody_notes[0]);   // Start playing first high note immediately
        } else {
            set_High_Tone(REST);              // Cut off PWM and mute buzzer
        }
    }
    
    // --- Switch S2: Reset Stopwatch ---
    else if (isfr & MASK(10)) {
        last_press_tick = sys_ticks;
        time_count = 0; 
        stopwatch_tick = 0;
        is_running = 0; 
        PTD->PSOR = MASK(LED_D2) | MASK(LED_D4); 
        PTD->PCOR = MASK(LED_D1) | MASK(LED_D3); 
    }
}

// Timer 0 ISR: 0.01s base tick driving independent Stopwatch (1s) & Melody speeds
void PIT0_IRQHandler(void) {
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK; 
    sys_ticks++; // Increment 10ms system tick for debounce
    
    // 1. Independent Stopwatch Counter (100 * 0.01s = exact 1.00 second per tick)
    if (is_running) {
        stopwatch_tick++;
        if (stopwatch_tick >= STOPWATCH_SPEED) {
            stopwatch_tick = 0;
            time_count++; 
            if (time_count > 9999) time_count = 0; 
            PTD->PTOR = MASK(LED_D4); 
        }
    }
    
    // 2. Independent Melody Tempo Counter (12 * 0.01s = 0.12s per beat unit)
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
                set_High_Tone(melody_notes[note_index]); // Update PWM frequency & duty cycle
            }
        }
    }
}

// ==========================================
// 4. Main Function
// ==========================================

int main(void) {
    init_LEDs();
    init_PWM_Buzzer();
    init_Switches();
    init_Timer();
    init_7Segment();
    
    while(1) {
        // Keep buzzer muted (Hi-Z) when song is off and S1 is released
        if (!is_playing_song && ((PTB->PDIR & MASK(3)) != 0)) {
            set_High_Tone(REST);
        }
        
        display_7Segment(time_count);
    }
    return 0;
}