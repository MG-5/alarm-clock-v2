/*
 * IR_Receiver.h
 *
 * Created: 17.04.2017 00:00:22
 *  Author: Maxi
 */ 

#ifndef INFRARED_H_
#define INFRARED_H_

void poll_IR();
void IREvent(uint8_t protocol, uint16_t address, uint16_t command);

#endif /* INFRARED_H_ */