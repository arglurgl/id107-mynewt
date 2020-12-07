/* Copyright (c) 2015 ARM Ltd., MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*
*  KX022 Accelerometer library
*
*  @author  Toyomasa Watarai
*  @version 1.0
*  @date    30-December-2015
*
*  Library for "KX022 Accelerometer library" from Kionix a Rohm group
*    http://www.kionix.com/product/KX022-1020
*
*/
//Based on above version, ported to C/mynewt
 
#ifndef KX022_H
#define KX022_H

#define KX022_WAI_VAL         (0x14)
 
#define KX022_XOUT_L          (0x06)
#define KX022_XOUT_H          (0x07)
#define KX022_YOUT_L          (0x08)
#define KX022_YOUT_H          (0x09)
#define KX022_ZOUT_L          (0x0A)
#define KX022_ZOUT_H          (0x0B)
#define KX022_WHO_AM_I        (0x0F)
#define KX022_CNTL1           (0x18)
#define KX022_CNTL3           (0x1A)
#define KX022_ODCNTL          (0x1B)
#define KX022_TILT_TIMER      (0x22)
 
#define _DEBUG
#ifdef _DEBUG
#include <console/console.h>
#define DEBUG_PRINT(...) console_printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif
 
/**
* KX022 accelerometer example
*
* @code
* #include "mbed.h"
* #include "KX022.h"
*
* int main(void) {
*
* KX022 acc(I2C_SDA, I2C_SCL);
* PwmOut rled(LED_RED);
* PwmOut gled(LED_GREEN);
* PwmOut bled(LED_BLUE);
*
*     while (true) {
*         rled = 1.0 - abs(acc.getAccX());
*         gled = 1.0 - abs(acc.getAccY());
*         bled = 1.0 - abs(acc.getAccZ());
*         wait(0.1);
*     }
* }
* @endcode
*/

 
/** Initializa KX022 sensor
 *
 *  Configure sensor setting
 *
 */
void kx022_initialize(void);

/**
 * Get X axis acceleration
 *
 * @returns X axis acceleration
 */
float kx022_getAccX();

/**
 * Get Y axis acceleration
 *
 * @returns Y axis acceleration
 */
float kx022_getAccY();

/**
 * Get Z axis acceleration
 *
 * @returns Z axis acceleration
 */
float kx022_getAccZ();

/**
 * Get XYZ axis acceleration
 *
 * @param res array where acceleration data will be stored
 */
void kx022_getAccAllAxis(float * res);


#endif