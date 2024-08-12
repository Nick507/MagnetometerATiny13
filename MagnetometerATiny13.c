// Libraries
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// Pin definitions
#define I2C_SDA         PB3                   // serial data pin
#define I2C_SCL         PB4                   // serial clock pin
#define HALL_SENSOR     1

const uint8_t OLED_FONT[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, //   0 
  0x08, 0x08, 0x08, 0x08, 0x08, // - 13
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0 16
  0x00, 0x42, 0x7F, 0x40, 0x00, // 1 17
  0x42, 0x61, 0x51, 0x49, 0x46, // 2 18
  0x21, 0x41, 0x45, 0x4B, 0x31, // 3 19
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4 20
  0x27, 0x45, 0x45, 0x45, 0x39, // 5 21
  0x3C, 0x4A, 0x49, 0x49, 0x30, // 6 22
  0x01, 0x71, 0x09, 0x05, 0x03, // 7 23
  0x36, 0x49, 0x49, 0x49, 0x36, // 8 24
  0x06, 0x49, 0x49, 0x29, 0x1E, // 9 25
};

// I2C implemented with assumption that there is no pullup resistors (as it is in 0.96'' OLED)
// I2C macros
#define I2C_SDA_HIGH()  PORTB |=  (1<<I2C_SDA)
#define I2C_SDA_LOW()   PORTB &= ~(1<<I2C_SDA)
#define I2C_SCL_HIGH()  PORTB |=  (1<<I2C_SCL)
#define I2C_SCL_LOW()   PORTB &= ~(1<<I2C_SCL)

// static variables
uint8_t curX = 0, curY = 0;

// I2C init function
void i2cInit(void) {
  DDRB  |= ((1<<I2C_SDA)|(1<<I2C_SCL));
  PORTB |= ((1<<I2C_SDA)|(1<<I2C_SCL));
}

// I2C transmit one data byte to the slave, ignore ACK bit, no clock stretching allowed
void i2cWrite(uint8_t data) 
{
  for(uint8_t i = 0; i < 8; i++) 
  {
    if (data & 0x80) I2C_SDA_HIGH(); else I2C_SDA_LOW();
    I2C_SCL_HIGH(); 
    data = data << 1;
    I2C_SCL_LOW();
  }
  I2C_SDA_HIGH();                         // pullup SDA line
  DDRB &= ~(1<<I2C_SDA);				  // input SDA line
  I2C_SCL_HIGH();                         // clock for ACK bit
  // here can read SDA to check ack
  I2C_SCL_LOW();                          // clock LOW again
  DDRB |= (1<<I2C_SDA);
}

// I2C start transmission
void i2cStart(void) 
{
	// start condition: SDA goes low first, then SCL goes low
	I2C_SDA_LOW();
	I2C_SCL_LOW();

	// write slave address
	// 0011 1100 - 3C    - used in arduino libraries, when R/W bit is added implicitly
	// 0111 1000 - 78
	i2cWrite(0x78); 
}

// I2C stop transmission
void i2cStop(void) 
{
	I2C_SDA_LOW();                          // prepare SDA for LOW to HIGH transition
	I2C_SCL_HIGH();                         // stop condition: SCL goes HIGH first
	I2C_SDA_HIGH();                         // stop condition: SDA goes HIGH second
}

void i2cCommand(uint8_t c)
{
  i2cStart(); 
  i2cWrite(0); // command mode
  i2cWrite(c);
  i2cStop();
}

void displayCursor(uint8_t xpos, uint8_t ypos) 
{
	i2cStart();                   // start transmission to OLED
	i2cWrite(0);               // set command mode
	i2cWrite(0xB0 | (ypos & 0x07));        // set start page
	i2cWrite(xpos & 0x0F);                 // set low nibble of start column
	i2cWrite(0x10 | (xpos >> 4));          // set high nibble of start column
	i2cStop();                             // stop transmission
	curX = xpos;
	curY = ypos;
}

void displayClear(void) 
{
	for (uint8_t i = 0; i < 8; i++)
	{
	    displayCursor(0, i);
	    i2cStart();
	    i2cWrite(0x40);
	    for(uint8_t i=0; i < 128; i++) i2cWrite(0x0);
	    i2cStop();
	}
    displayCursor(0, 0);
}

void displayInit()
{
	i2cInit();
	_delay_ms(200);
	i2cCommand(0x8D); // charge pump setting
	i2cCommand(0x14);
	i2cCommand(0xAF); // switch on OLED
	displayClear();
}

/*
void displayPutc(char ch) 
{
	uint16_t offset = ch - ' ';
	offset += offset << 2;
	for(uint8_t i=0; i < 5; i++) i2cWrite(pgm_read_byte(&OLED_FONT[offset++]));
	i2cWrite(0x00);
}
*/

// scale 4x4
void displayPutc(char ch) 
{
	if(ch == ' ') ch = 0;
	else if(ch == '-') ch = 1;
	else ch = ch - 46;
	uint16_t offset = ch;
	offset += offset << 2;

	for(uint8_t y = 0; y < 4; y++)
	{
		i2cStart();
		i2cWrite(0x40);

		for(uint8_t i = 0; i < 5; i++) 
		{
			uint8_t b = pgm_read_byte(&OLED_FONT[offset + i]);
			uint8_t d = b & (1 << (y << 1)) ? 0x0F : 0;
			d = d | (b & (2 << (y << 1)) ? 0xF0 : 0);
			for(uint8_t x = 0; x < 4; x++) i2cWrite(d);
	    }
		i2cStop();

		displayCursor(curX, curY + 1);
	}

	displayCursor(curX + 6 * 4, curY - 4);
}

/*
void displayPrint(const char* p) 
{
	i2cStart();
	i2cWrite(0x40);
	char c;
	while((c = pgm_read_byte(p++))) 
	{
		displayPutc(c);
	}
	i2cStop();
}
*/

void displayPrint(int32_t v) 
{
	if(v < 0)
	{
		displayPutc('-');
		v = -v;
	} else displayPutc(' ');

	uint8_t d = '0';
	while(v >= 100)
	{
		v = v - 100;
		d++;
	};
	displayPutc(d);

	d = '0';
	while(v >= 10)
	{
		v = v - 10;
		d++;
	};
	displayPutc(d);

	displayPutc('0' + v);
}

int16_t analogRead(uint8_t pin)
{
  // MUX1 & MUX0 are 2 lowest bits in ADMUX
  ADMUX = (ADMUX & 0xFC) | pin;
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // Prescaler to 128 -> F_CPU / 128
  while(ADCSRA & _BV(ADSC)); // Wait for conversion
  int16_t result = ADCW;
  ADCSRA = 0; // turn off ADC
  return result;
}

int main() 
{
	displayInit();

	uint32_t avg = 0;
	for(uint16_t i = 0; i < 1024; i++)
	{
		avg += analogRead(HALL_SENSOR);
	}
	avg = avg >> 10;

	while(1)
	{
		uint32_t cur = 0;
		for(uint16_t i = 0; i < 256; i++)
		{
			cur += analogRead(HALL_SENSOR);
		}
		cur = (cur >> 8) - avg;

		displayCursor(16, 2);
		displayPrint(cur);
	}

	return 0;
}

