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

#pragma once

// #define INDEP // Independent mode
// #define DEBUG // Debug mode (UART_TX on pin D3 at 115200 baud)
// #define CLK_16MHZ // Use F_CPU=16Mhz instead of 8Mhz
// #define CLK_TRIM 0 // Internal clock trimming
#define PWM_MIN 10 // Minimum non-zero PWM duty cycle (%)
#define PWM_MAX 100 // Maximum non-zero PWM duty cycle (%)
#define PWM_DIV 0 // PWM frequency divider (0..14), F_PWM=F_CPU*(PWM_MAX-PWM_MIN)/(45000*2^PWM_DIV)
// #define PWM_INV // PWM active low
// #define FWD1_INV // Forward 1 active low
// #define FWD2_INV // Forward 2 active low
// #define REV1_INV // Reverse 1 active low
// #define REV2_INV // Reverse 2 active low
// #define LED_INV // LED active low

// RC-to-RX channel mapping (1..14)
#define IBUS_CH1 3
#define IBUS_CH2 4

// Output pins
#define PWM1_DDR DDRD
#define PWM1_PIN 0x80 // D7
#define PWM2_DDR DDRB
#define PWM2_PIN 0x40 // B6
#define FWD1_ODR PORTF
#define FWD1_DDR DDRF
#define FWD1_PIN 0x80 // F7
#define FWD2_ODR PORTF
#define FWD2_DDR DDRF
#define FWD2_PIN 0x40 // F6
#define REV1_ODR PORTB
#define REV1_DDR DDRB
#define REV1_PIN 0x10 // B4
#define REV2_ODR PORTB
#define REV2_DDR DDRB
#define REV2_PIN 0x20 // B5
#define LED_ODR PORTD
#define LED_DDR DDRD
#define LED_PIN 0x40 // D6
