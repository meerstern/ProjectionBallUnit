/*
 *  	ProjectionBall.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef PROJECTIONBALL_H
#define PROJECTIONBALL_H

#define VER_STR     " v1.2"

/* IO Pin Definitions */
#define PIN_TX      0
#define PIN_RX      1
#define PIN_PWM_A	2
#define PIN_PWM_B	3
#define PIN_MODE	6
#define PIN_PATTERN	7
#define PIN_LSR		8
#define PIN_ERR		9
#define PIN_STBY	21
#define PIN_A1		22
#define PIN_A2		23
#define PIN_B1		24
#define PIN_B2		25

#define SPI_PORT spi0

#define PIN_MISO 16
#define PIN_CS1  17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_CS2  20

/* Uart Setting */
#define UART_ID     uart0
#define BAUD_RATE   9600
#define DATA_BITS   8
#define STOP_BITS   1
#define PARITY      UART_PARITY_NONE

/* I2C Setting */
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

/* For Control */
#define ENCODER_BUFFER_SIZE	8
#define ENABLE_RUNAWAY_DETECTION

/* Encoder Type */
#define USE_MA732
//#define USE_AS5048A

/* For Gain Tuning */
#define ENABLE_IN_POS
//#define ENABLE_STEP_CMD


/* For Debug */
//#define ENABLE_ENCODER_CHECK_MODE
#define ENABLE_DEBUG_OUTPUT
//#define ENABLE_DEBUG_INFO
//#define ENABLE_SIN_CMD
#define SIN_CMD_AMP		160
//#define ENABLE_FLASH_TEST

//#define DISABLE_MOTOR
#define ENABLE_PROJECTION_ANGLE

//#define ENABLE_CALIBRATION_MODE

/* RTC Type */
//#define USE_RTC_RV8803
#define USE_RTC_SD30XX




#endif