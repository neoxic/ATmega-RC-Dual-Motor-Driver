/*
** Copyright (C) 2021 Arseny Vakhrushev <arseny.vakhrushev@me.com>
**
** This firmware is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This firmware is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this firmware. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "config.h"

#define duty(x) ((450U * (x)) / (PWM_MAX - PWM_MIN))

static int16_t input(uint16_t t, uint16_t *u) {
	if (t < 800 || t > 2200) { // Invalid signal
		*u = 0;
		return 0;
	}
	if (!*u && (t < 1450 || t > 1550)) return 0; // Reset required
	*u = t;
	return t - 1500;
}

static uint16_t output(int16_t t, uint8_t *f, uint8_t *r) {
	*f = t > 50;
	*r = t < -50 ? t = -t, 1 : 0;
	if (t < 50) return 0;
	if (t > 500) return duty(PWM_MAX);
	return duty(PWM_MIN) + t - 50;
}

static uint16_t t1, t2, u1, u2, p1, p2;
static int16_t i1, i2;
static uint8_t f1, f2, r1, r2, tick;

static void update(void) {
#ifdef INDEP
	p1 = output(i1, &f1, &r1);
	p2 = output(i2, &f2, &r2);
#else
	p1 = output(i1 + i2, &f1, &r1);
	p2 = output(i1 - i2, &f2, &r2);
#endif
	TC4H = p1 >> 8;
	OCR4D = p1;
	TC4H = p2 >> 8;
	OCR4B = p2;
#ifdef FWD1_INV
	if (f1) FWD1_ODR &= ~FWD1_PIN;
	else FWD1_ODR |= FWD1_PIN;
#else
	if (f1) FWD1_ODR |= FWD1_PIN;
	else FWD1_ODR &= ~FWD1_PIN;
#endif
#ifdef FWD2_INV
	if (f2) FWD2_ODR &= ~FWD2_PIN;
	else FWD2_ODR |= FWD2_PIN;
#else
	if (f2) FWD2_ODR |= FWD2_PIN;
	else FWD2_ODR &= ~FWD2_PIN;
#endif
#ifdef REV1_INV
	if (r1) REV1_ODR &= ~REV1_PIN;
	else REV1_ODR |= REV1_PIN;
#else
	if (r1) REV1_ODR |= REV1_PIN;
	else REV1_ODR &= ~REV1_PIN;
#endif
#ifdef REV2_INV
	if (r2) REV2_ODR &= ~REV2_PIN;
	else REV2_ODR |= REV2_PIN;
#else
	if (r2) REV2_ODR |= REV2_PIN;
	else REV2_ODR &= ~REV2_PIN;
#endif
	wdt_reset(); // Reset watchdog
}

static void channel(uint8_t i, uint16_t t) {
	switch (i) {
#ifdef IBUS_CH1
		case IBUS_CH1:
			i1 = input(t, &u1);
			break;
#endif
#ifdef IBUS_CH2
		case IBUS_CH2:
			i2 = input(t, &u2);
			break;
#endif
	}
}

#ifdef CLK_16MHZ
#define adjust(x) ((x) >> 1)
#else
#define adjust(x) (x)
#endif

ISR(TIMER1_CAPT_vect) {
	uint16_t t = ICR1;
	uint8_t b = TCCR1B;
	TCCR1B = b ^ 0x40; // ICES=!ICES
	if (b & 0x40) { // Rising edge
		t1 = t;
		return;
	}
	i1 = input(adjust(t - t1), &u1);
	update();
}

ISR(TIMER3_CAPT_vect) {
	uint16_t t = ICR3;
	uint8_t b = TCCR3B;
	TCCR3B = b ^ 0x40; // ICES=!ICES
	if (b & 0x40) { // Rising edge
		t2 = t;
		return;
	}
	i2 = input(adjust(t - t2), &u2);
	update();
}

ISR(USART1_RX_vect) {
	static uint8_t a, b, n = 30;
	static uint16_t u;
	a = b;
	b = UDR1;
	if (a == 0x20 && b == 0x40) { // Sync
		n = 0;
		u = 0xff9f;
		return;
	}
	if (n == 30 || ++n & 1) return;
	uint16_t t = a | (b << 8);
	if (n == 30) { // End of chunk
		if (u != t) return; // Sync lost
		update();
		// Disable RC channels to avoid conflict
		TIMSK1 = 0x01;
		TIMSK3 = 0x00;
		return;
	}
	channel(n >> 1, t);
	u -= a + b;
}

ISR(TIMER1_OVF_vect) {
	static uint8_t cnt, led;
#ifdef CLK_16MHZ
	if (cnt++ < 6) return;
#else
	if (cnt++ < 3) return;
#endif
	cnt = 0;
	tick = 1;
	if (!led) led = (!!u1 + !!u2) * 2 + 1;
	else if (--led) LED_ODR ^= LED_PIN;
}

static int uart_putchar(char c, FILE *f) {
  while (!(UCSR1A & 0x20)); // UDRE=0 (TX in progress)
  UDR1 = c;
  return 0;
}

void main(void) {
	CLKPR = 0x80;
#ifdef CLK_16MHZ
	CLKPR = 0x00; // 16Mhz clock
#else
	CLKPR = 0x01; // 8Mhz clock
#endif
#ifdef CLK_TRIM
	OSCCAL += CLK_TRIM;
#endif

	PWM1_DDR |= PWM1_PIN;
	PWM2_DDR |= PWM2_PIN;
	FWD1_DDR |= FWD1_PIN;
	FWD2_DDR |= FWD2_PIN;
	REV1_DDR |= REV1_PIN;
	REV2_DDR |= REV2_PIN;
	LED_DDR |= LED_PIN;
	PORTB = 0xff;
	PORTC = 0xff;
	PORTD = 0xff;
	PORTE = 0xff;
	PORTF = 0xff;
#ifndef FWD1_INV
	FWD1_ODR &= ~FWD1_PIN;
#endif
#ifndef FWD2_INV
	FWD2_ODR &= ~FWD2_PIN;
#endif
#ifndef REV1_INV
	REV1_ODR &= ~REV1_PIN;
#endif
#ifndef REV2_INV
	REV2_ODR &= ~REV2_PIN;
#endif
#ifndef LED_INV
	LED_ODR &= ~LED_PIN;
#endif

	// RC channel 1
	TCCR1B = 0xc2; // CS=010, ICES=1, ICNC=1 (enable IC1, CLK/8, noise filter)
	TIMSK1 = 0x21; // TOIE=1, ICIE=1 (enable interrupts)

	// RC channel 2
	TCCR3B = 0xc2; // CS=010, ICES=1, ICNC=1 (enable IC3, CLK/8, noise filter)
	TIMSK3 = 0x20; // ICIE=1 (enable interrupts)

	// PWM channels 1,2
	TC4H = duty(100) >> 8;
	OCR4C = duty(100) & 0xff;
#ifdef PWM_INV
	TCCR4A = 0x31; // PWM4B=1, COM4B=11 (enable OC4B active low)
	TCCR4C = 0x3d; // PWM4D=1, COM4D=11 (enable OC4D active low)
#else
	TCCR4A = 0x21; // PWM4B=1, COM4B=10 (enable OC4B active high)
	TCCR4C = 0x29; // PWM4D=1, COM4D=10 (enable OC4D active high)
#endif
	TCCR4D = 0x00; // WGM=00 (fast PWM mode)
	TCCR4B = (0x01 + PWM_DIV) & 0x0f; // CS=0001 (enable counter)

#ifdef CLK_16MHZ
	UBRR1 = 0x10; // 115200 baud @ 16Mhz clock
#else
	UBRR1 = 0x08; // 115200 baud @ 8Mhz clock
#endif
	UCSR1A = 0x02; // U2X=1 (double speed)
	UCSR1B = 0x98; // TXEN=1, RXEN=1, RXCIE=1 (enable RX/TX/interrupts)
	FILE uart = FDEV_SETUP_STREAM(uart_putchar, 0, _FDEV_SETUP_WRITE);
	stdout = &uart;

#ifdef DEBUG
	printf("\n");
	printf(" CH1  CH2     IN1  IN2    PWM1 PWM2    FWD  REV\n");
#else
	wdt_enable(WDTO_250MS); // Enable watchdog
#endif
	sei();
	for (;;) {
		sleep_mode();
#ifdef DEBUG
		if (!tick) continue;
		tick = 0;
		cli();
		uint16_t _u1 = u1, _u2 = u2, _p1 = p1, _p2 = p2;
		int16_t _i1 = i1, _i2 = i2;
		sei();
		printf("%4u %4u    %4d %4d    %4d %4d    %d %d  %d %d\n",
			_u1, _u2, _i1, _i2, _p1, _p2, f1, f2, r1, r2);
#endif
	}
}
