// *** Hardwarespecific defines ***
#if defined (__AVR__)
#define UTFT_cbi(reg, bitmask) *reg &= ~bitmask
#define UTFT_sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); UTFT_cbi(reg, bitmask);
#define pulse_low(reg, bitmask) UTFT_cbi(reg, bitmask); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) pgm_read_byte(&cfont.font[x])  

#define regtype volatile uint8_t
#define regsize uint8_t
#define bitmapdatatype unsigned int*
#endif
	
#if defined(__arm__)

#define UTFT_cbi(reg, bitmask) *reg &= ~bitmask
#define UTFT_sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) UTFT_sbi(reg, bitmask); UTFT_cbi(reg, bitmask);
#define pulse_low(reg, bitmask) UTFT_cbi(reg, bitmask); UTFT_sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) cfont.font[x]  

#define pgm_read_word(data) *data
#define pgm_read_byte(data) *data
#define bitmapdatatype unsigned short*

#if defined(TEENSYDUINO) && TEENSYDUINO >= 117
  #define regtype volatile uint8_t
  #define regsize uint8_t
#else
  #define regtype volatile uint32_t
  #define regsize uint32_t
#endif

#endif

#if defined(ESP8266)
#define UTFT_cbi(reg, bitmask) digitalWrite(bitmask,LOW)
#define UTFT_sbi(reg, bitmask) digitalWrite(bitmask,HIGH)

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) cfont.font[x]  

#define bitmapdatatype unsigned short*

#define regtype volatile uint32_t
#define regsize uint32_t

#endif	

#if defined (__CPU_ARC__)
#define UTFT_cbi(reg, bitmask) *reg &= ~bitmask
#define UTFT_sbi(reg, bitmask) *reg |= bitmask
//#define pulse_high(reg, bitmask) UTFT_sbi(reg, bitmask); UTFT_cbi(reg, bitmask);
//#define pulse_low(reg, bitmask) UTFT_cbi(reg, bitmask); UTFT_sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) pgm_read_byte(&cfont.font[x])  

#define regtype volatile uint32_t
#define regsize uint32_t
#define bitmapdatatype unsigned int*
#endif



	