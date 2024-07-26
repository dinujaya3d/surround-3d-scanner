#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "VL53L0X.h"

#define STEP_PIN_1 24
#define DIR_PIN_1 23
#define ENA_PIN_1 22

#define STEP_PIN_2 27
#define DIR_PIN_2 26
#define ENA_PIN_2 25

#define XY_REV 200
#define XZ_REV 50

#define ANGLE_INCREMENT 1.8

#define F_CPU 16000000UL // Assuming a 16 MHz clock
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16) / 2)

float measurements[XZ_REV][3];

void uart_init(unsigned int ubrr) {
  // Set baud rate
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  // Enable receiver and transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  // Set frame format: 8 data bits, 1 stop bit
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(unsigned char data) {
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0)));
  // Put data into buffer, sends the data
  UDR0 = data;
}

void uart_print(const char *str) {
  while (*str) {
    uart_transmit(*str++);
  }
}

void i2c_init(void) {
  TWSR = 0x00;
  TWBR = (uint8_t)TWBR_val;
}

void i2c_start(void) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void) {
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  while (TWCR & (1 << TWSTO));
}

void i2c_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack(void) {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}

uint8_t i2c_read_nack(void) {
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}

void tof(uint8_t bus) {
  i2c_start();
  i2c_write(0x70 << 1); // TCA9548A address is 0x70
  i2c_write(1 << bus);  // send byte to select bus
  i2c_stop();
}

void setup() {
  uart_init(MYUBRR);  // Initialize the UART
  i2c_init();         // Initialize I2C

  tof(1);

  // Set pin modes using direct port manipulation
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2); // STEP_PIN_1, DIR_PIN_1, ENA_PIN_1
  DDRB |= (1 << DDB3) | (1 << DDB4) | (1 << DDB5); // STEP_PIN_2, DIR_PIN_2, ENA_PIN_2

  // Enable pins
  PORTB &= ~(1 << PORTB2); // ENA_PIN_1 LOW
  PORTB &= ~(1 << PORTB5); // ENA_PIN_2 LOW

  // Move to the starting position
  PORTB &= ~(1 << PORTB4); // DIR_PIN_2 LOW
  for (int i = 0; i < XZ_REV / 2; i++) {
    PORTB |= (1 << PORTB3); // STEP_PIN_2 HIGH
    PORTB &= ~(1 << PORTB3); // STEP_PIN_2 LOW
    delayMicroseconds(50);
  }

  uart_print("VL53L0X test with Stepper Motor\n");
}

void loop() {
  for (int i = 0; i < XY_REV; i++) {
    if (i % 2 == 0) {
      PORTB |= (1 << PORTB4); // DIR_PIN_2 HIGH
    } else {
      PORTB &= ~(1 << PORTB4); // DIR_PIN_2 LOW
    }
    for (int j = 0; j < XZ_REV; j++) {
      i2c_start();
      i2c_write(0x29 << 1); // VL53L0X address
      i2c_write(0x00); // Register to read
      i2c_start();
      i2c_write((0x29 << 1) | 1); // Read mode
      uint8_t range = i2c_read_nack();
      i2c_stop();

      if (i % 2 == 0) {
        if (range != 255) {  // Check if range is valid
          measurements[j][0] = j * ANGLE_INCREMENT - 45;
          measurements[j][1] = i * ANGLE_INCREMENT;
          measurements[j][2] = range;
        } else {
          measurements[j][0] = j * ANGLE_INCREMENT - 45;
          measurements[j][1] = i * ANGLE_INCREMENT;
          measurements[j][2] = 10000;
        }
      } else {
        if (range != 255) {  // Check if range is valid
          measurements[XZ_REV - j - 1][0] = j * ANGLE_INCREMENT - 45;
          measurements[XZ_REV - j - 1][1] = i * ANGLE_INCREMENT;
          measurements[XZ_REV - j - 1][2] = range;
        } else {
          measurements[XZ_REV - j - 1][0] = j * ANGLE_INCREMENT - 45;
          measurements[XZ_REV - j - 1][1] = i * ANGLE_INCREMENT;
          measurements[XZ_REV - j - 1][2] = 10000;
        }
      }
      PORTB |= (1 << PORTB3); // STEP_PIN_2 HIGH
      PORTB &= ~(1 << PORTB3); // STEP_PIN_2 LOW
    }
    uart_print("aaa[");
    for (int k = 0; k < XZ_REV; k++) {
      uart_transmit('[');
      uart_print(measurements[k][0]);
      uart_transmit(',');
      uart_print(measurements[k][1]);
      uart_transmit(',');
      uart_print(measurements[k][2]);
      uart_transmit(']');
      if (k < XZ_REV - 1) uart_transmit(',');
    }
    uart_print("]\n");
    PORTB |= (1 << PORTB0); // STEP_PIN_1 HIGH
    PORTB &= ~(1 << PORTB0); // STEP_PIN_1 LOW
  }
}
