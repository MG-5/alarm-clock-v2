#define set_bit(port, bit)		port |= (1 << (bit))
#define clear_bit(port, bit)	port &= ~(1 << (bit))
#define toggle_bit(port, bit)	port ^= (1 << (bit))

#define check_bit(pin,bit)		((pin>>(bit)) & 1)