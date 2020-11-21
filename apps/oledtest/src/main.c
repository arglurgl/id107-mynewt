/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <assert.h>
#include <time.h>
#include <compile_time.h>//macro for build time as unix time, for setting clock,optional

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "os/os_time.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#include "hal/hal_spi.h"
#include "ssd1306/ssd1306.h"

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

struct os_timeval os_time;
struct tm  time_struct;
static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin,g_front_button_pin,g_side_button_pin;

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

	//init pins
    g_led_pin = LED_BLINK_PIN;
    g_front_button_pin = FRONT_BUTTON_PIN;
    g_side_button_pin = SIDE_BUTTON_PIN;
    
    hal_gpio_init_out(g_led_pin, 0);
    hal_gpio_init_in(g_front_button_pin,HAL_GPIO_PULL_NONE);
    hal_gpio_init_in(g_side_button_pin,HAL_GPIO_PULL_UP);

    //set time
    struct os_timeval time_now = { __TIME_UNIX__, 0 }; 
    struct os_timezone my_timezone = { 60, 0 };
    os_settimeofday(&time_now,&my_timezone);

    //Init Oled display
    ssd1306_Init();

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0,0);
    ssd1306_WriteString("Setup",Font_7x10,White);
    ssd1306_SetCursor(0,11);
    ssd1306_WriteString("Done",Font_7x10,White);
    ssd1306_UpdateScreen();
    os_time_delay(OS_TICKS_PER_SEC/2);




     
    while (1) {
        ++g_task1_loops;

        ssd1306_Fill(Black);
        char str[12];
        sprintf(str, "%d", g_task1_loops);
        ssd1306_SetCursor(0,0);
        ssd1306_WriteString(str,Font_7x10,White);

        os_gettimeofday(&os_time,NULL);        
        time_t epoch=os_time.tv_sec;
        struct tm ts = *localtime(&epoch);
        //strftime(str, sizeof(str), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
        strftime(str, sizeof(str), "%H:%M:%S", &ts);
        ssd1306_SetCursor(0,11);
        ssd1306_WriteString(str,Font_7x10,White);   
        strftime(str, sizeof(str), "%d.%m.%y", &ts);   
        ssd1306_SetCursor(0,21);
        ssd1306_WriteString(str,Font_7x10,White);

        ssd1306_UpdateScreen();
        os_time_delay(OS_TICKS_PER_SEC/10);
 
  
        
        if (hal_gpio_read(g_front_button_pin) == 1)
        {
			/* Pulse the motor */
			hal_gpio_write(g_led_pin,1);
			os_time_delay(OS_TICKS_PER_SEC/20);
			hal_gpio_write(g_led_pin,0);
		}
		
		if (hal_gpio_read(g_side_button_pin) == 0)
        {
			/* Pulse the motor */
			hal_gpio_write(g_led_pin,1);
			os_time_delay(OS_TICKS_PER_SEC/2);
			hal_gpio_write(g_led_pin,0);
		}
    }
    assert(0);

    return rc;
}

