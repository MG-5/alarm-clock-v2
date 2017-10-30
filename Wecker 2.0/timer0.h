/*
 * timer0.h
 *
 * Created: 15.04.2017 17:49:18
 *  Author: Maxi
 */ 
 #include <stdint.h>

#ifndef TIMER0_H_
#define TIMER0_H_

//initialize timer0
void timer0_init();

uint32_t millis();
uint32_t micros();

#endif /* TIMER0_H_ */