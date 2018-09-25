#ifndef RGB_vector_h
#define RGB_vector_h

#include <stdint.h>

class RGB_vector
{
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    void setVector(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0);
};

#endif
