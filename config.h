#ifndef CONFIG_H
#define CONFIG_H

#include <MK64F12.h>

// ============================================================================
// 1. General Bit-Mask Helper Macro
// ============================================================================
#define MASK(x) (1UL << (x))

// ============================================================================
// 2. System Clock & Timing Configuration (Base Tick = 0.01s / 10ms)
// ============================================================================
#define BUS_CLOCK_HZ        20971520UL
#define PIT0_10MS_LDVAL     209714UL

// Independent Speed Dividers (in 0.01s base ticks)
#define STOPWATCH_SPEED     100     // 100 * 0.01s = 1.00 second per count
#define MELODY_SPEED        12      // 12 * 0.01s  = 0.12 second per beat unit
#define DEBOUNCE_TICKS      20      // 20 * 0.01s  = 200 ms non-blocking debounce

// ============================================================================
// 3. Multifunctional Shield Hardware Pin Definitions
// ============================================================================
#define LED_D1_PIN          1       // PTD1
#define LED_D2_PIN          3       // PTD3
#define LED_D3_PIN          2       // PTD2
#define LED_D4_PIN          0       // PTD0

#define SW1_PIN             3       // PTB3  (Start / Pause Stopwatch)
#define SW2_PIN             10      // PTB10 (Reset Stopwatch)
#define SW3_PIN             11      // PTB11 (Toggle Jingle Bells Melody)

#define BUZZER_PIN          1       // PTA1 (MUX 3 = FTM0_CH6, MUX 1 = GPIO Hi-Z)

#define SEG_CLK_PIN         3       // PTC3  (Shift Clock)
#define SEG_LATCH_C_PIN     6       // PTC6  (Storage Latch)
#define SEG_LATCH_B_PIN     23      // PTB23 (Storage Latch)
#define SEG_DATA_C_PIN      12      // PTC12 (Serial Data)
#define SEG_DATA_A_PIN      0       // PTA0  (Serial Data)

// ============================================================================
// 4. Shared Volatile Global State Variables
// ============================================================================
extern volatile uint16_t time_count;
extern volatile uint8_t  is_running;
extern volatile uint8_t  stopwatch_tick;
extern volatile uint32_t sys_ticks;
extern volatile uint32_t last_press_tick;

extern volatile uint8_t  is_playing_song;
extern volatile uint8_t  note_index;
extern volatile uint8_t  note_tick;
extern volatile uint8_t  tempo_tick;

#endif // CONFIG_H
