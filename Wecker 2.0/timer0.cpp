/*
 * timer0.cpp
 *
 * Created: 15.04.2017 17:47:27
 *  Author: Maxi
 */

#include "timer0.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define clockCyclesPerMicrosecond() (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) (((a)*1000L) / (F_CPU / 1000L))
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile uint32_t timer0_millis = 0;
volatile uint32_t timer0_fract = 0;
volatile uint16_t timer0_overflows = 0;

void timer0_init()
{
  TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler 64 8Mhz/64 -> 8µs
  TIMSK0 |= (1 << TOIE0);              // enable overflow interrupt
}

uint32_t millis() { return timer0_millis; }

uint32_t micros()
{
  return ((uint32_t)(timer0_overflows * (uint32_t)256) + TCNT0) * 8; // 8MHz
}

ISR(TIMER0_OVF_vect)
{
  uint32_t m = timer0_millis;
  uint32_t f = timer0_fract;

  m += MILLIS_INC;
  f += FRACT_INC;
  if (f >= FRACT_MAX)
  {
    f -= FRACT_MAX;
    m += 1;
  }

  timer0_fract = f;
  timer0_millis = m;

  timer0_overflows++;
}
