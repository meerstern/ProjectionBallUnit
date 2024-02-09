/*
 *  	ProjectionBall.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/rtc.h"
#include "hardware/watchdog.h"
#include "ProjectionBall.h"
#include "rtc_rv8803.h"
#include "rtc_sd30XX.h"
#include "encoder_ma732.h"
#include "motor_ctrl.h"
#include "path_ctrl.h"
#include "flash_ctrl.h"
#include "console.h"


static semaphore_t 	sem;
static bool 		CtrlEventFlg = false;
static bool 		PathEventFlg = false;
struct repeating_timer control_timer;
struct repeating_timer path_timer;
alarm_pool_t* 		core0Alarm;
alarm_pool_t* 		core1Alarm;

bool control_timer_callback(struct repeating_timer *t)
{    
	if(CtrlEventFlg == false)
		CtrlEventFlg = true;
	else
	{
		gpio_put(PIN_STBY, 0);
		cancel_repeating_timer(&control_timer);
		puts("Control Timer Call Err");
	}
    return true;
}

bool path_timer_callback(struct repeating_timer *t)
{    
	if(PathEventFlg==false)
		PathEventFlg=true;
	// else
	// {
	// 	cancel_repeating_timer(&path_timer);
	// 	puts("Path Timer Call Err");
	// }
    return true;
}

void core1_main()
{

	core1Alarm = alarm_pool_create(1, 4);
	//irq_set_priority(0,0x00);
	alarm_pool_add_repeating_timer_us(core1Alarm, 80, control_timer_callback, NULL, &control_timer);//80us
	watchdog_enable(1, 1);
	//Control Loop
	while (true)
    {
		//Control Event
		if( CtrlEventFlg == true )
		{
			CtrlEventFlg = false;
			MotorCtrlLoop();
			UpdateUserButton();
			if( IsResetEnable()==false)
				watchdog_update();							
		}
	}
}

void core0_main()
{	

	core0Alarm = alarm_pool_create(0, 4);
	alarm_pool_add_repeating_timer_ms(core0Alarm, 320, path_timer_callback, NULL, &path_timer);
	while (true)
	{
		
		if( PathEventFlg == true)
		{
			PathEventFlg = false;
			PathCtrlLoop();				
			ConsoleGetString();
#ifdef ENABLE_DEBUG_OUTPUT
			DebugMotorCtrl();
#endif
			
		}
	
	}	
}

void ioInit()
{
	//Motor Direction Pin
	gpio_init(PIN_STBY);
	gpio_init(PIN_LSR);
	gpio_init(PIN_ERR);
	gpio_init(PIN_A1);
	gpio_init(PIN_A2);
	gpio_init(PIN_B1);
	gpio_init(PIN_B2);
	gpio_init(PIN_MODE);
	gpio_init(PIN_PATTERN);	
    gpio_set_dir(PIN_STBY, GPIO_OUT);
	gpio_set_dir(PIN_LSR, GPIO_OUT);
	gpio_set_dir(PIN_ERR, GPIO_OUT);
	gpio_set_dir(PIN_A1, GPIO_OUT);
    gpio_set_dir(PIN_A2, GPIO_OUT);
    gpio_set_dir(PIN_B1, GPIO_OUT);
    gpio_set_dir(PIN_B2, GPIO_OUT);
	gpio_set_dir(PIN_MODE, GPIO_IN);
    gpio_set_dir(PIN_PATTERN, GPIO_IN);
	gpio_put(PIN_STBY, 0);
	gpio_put(PIN_LSR, 0);
	gpio_put(PIN_ERR, 1);//Check LED 
	gpio_put(PIN_A1, 0);
	gpio_put(PIN_A2, 0);
	gpio_put(PIN_B1, 0);
	gpio_put(PIN_B2, 0);
    gpio_pull_up(PIN_MODE);
    gpio_pull_up(PIN_PATTERN);
	
	// PWM initialisation
	gpio_set_function(PIN_PWM_A, GPIO_FUNC_PWM);
    gpio_set_function(PIN_PWM_B, GPIO_FUNC_PWM);


    // SPI initialisation
    spi_init(SPI_PORT, 8000*1000);//10MHz
#ifdef USE_MA732
	spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
#endif
#ifdef USE_AS5048A
	spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
#endif
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS1,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    

    // Chip select initialise
    gpio_init(PIN_CS1);
	gpio_init(PIN_CS2);
	gpio_set_dir(PIN_CS1, GPIO_OUT);
	gpio_set_dir(PIN_CS2, GPIO_OUT);
    gpio_put(PIN_CS1, 1);
    gpio_put(PIN_CS2, 1);
	sleep_ms(5);
    gpio_put(PIN_ERR, 0);

    // I2C Initialisation at 100Khz
    i2c_init(i2c0, 200*1000);    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

	//UART
    uart_init(UART_ID, 2400); 
    gpio_set_function(PIN_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_RX, GPIO_FUNC_UART);
    uart_set_baudrate(UART_ID, BAUD_RATE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);    
    irq_set_exclusive_handler(UART0_IRQ, OnUartRx);
    irq_set_enabled(UART0_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

	//RTC Init
	rtc_init();
  

}

int main()
{
    	
	stdio_init_all();
	sem_init(&sem, 1, 1);	
	ioInit();

#ifdef ENABLE_FLASH_TEST
	TestFlashReadWrite();
#endif

	if(watchdog_caused_reboot())
	{
		sleep_ms(1000);			
	}

	MotorCtrlInit();

#ifdef ENABLE_ENCODER_CHECK_MODE
	uint16_t encVal0, encVal1;
	while(1)
	{
		sleep_ms(300);
		MA732ReadAngleBit(PIN_CS1, &encVal0);
		MA732ReadAngleBit(PIN_CS2, &encVal1);
		printf("Val0: %d, Val1:%d\n",encVal0>>2,encVal1>>2);		
	}
#endif

	sleep_ms(500);
    printf("\r\n\r\n ProjectionBall Unit"VER_STR" \r\n");
	printf("[Build: "__DATE__"]\r\n");

#ifdef USE_RTC_RV8803
	if( IsRtcBOR() == true )
		printf("RTC was Cleared. Check Battery & Set Datetime!\r\n");
#endif
	GetDateTime();

	//Update HW RTC
	UpdateHwRtc();
	RestoreUserData();
	ClearBuffer();
	sleep_ms(500);	

	multicore_launch_core1(core1_main);		
	core0_main();

    return 0;
}
