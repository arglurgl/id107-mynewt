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
 
#include "kx022.h"
#include "kx022_priv.h"

#include "hal/hal_i2c.h"
  
void kx022_initialize()
{
    unsigned char buf=0;
    unsigned char reg[2];
 
    DEBUG_PRINT("KX022 init started\n\r");
    kx022_readRegs(KX022_WHO_AM_I, &buf, sizeof(buf));
    if (buf != KX022_WAI_VAL) {
        DEBUG_PRINT("KX022 initialization error. (WAI %d, not %d)\n\r", buf, KX022_WAI_VAL);
        DEBUG_PRINT("Trying to config anyway, in case there is some compatible sensor connected.\n\r");
    }
 
    reg[0] = KX022_CNTL1;
    reg[1] = 0x41;
    kx022_writeRegs(reg, 2);
 
    reg[0] = KX022_ODCNTL;
    reg[1] = 0x02;
    kx022_writeRegs(reg, 2);
 
    reg[0] = KX022_CNTL3;
    reg[1] = 0xD8;
    kx022_writeRegs(reg, 2);
 
    reg[0] = KX022_TILT_TIMER;
    reg[1] = 0x01;
    kx022_writeRegs(reg, 2);
 
    reg[0] = KX022_CNTL1;
    reg[1] = 0xC1;
    kx022_writeRegs(reg, 2);

    DEBUG_PRINT("X: %d\t",(int)(1000*kx022_getAccX()));
    DEBUG_PRINT("Y: %d\t",(int)(1000*kx022_getAccY()));
    DEBUG_PRINT("Z: %d\n",(int)(1000*kx022_getAccZ()));
}
    
float kx022_getAccX()
{
    float ret = kx022_getAccAxis(KX022_XOUT_L)/16384.f;
    return ((float)ret);
}
 
float kx022_getAccY()
{
    float ret = kx022_getAccAxis(KX022_YOUT_L)/16384.f;
    return ((float)ret);
}
 
float kx022_getAccZ()
{
    float ret = kx022_getAccAxis(KX022_ZOUT_L)/16384.f;
    return ((float)ret);
}
 
void kx022_getAccAllAxis(float * res)
{
    res[0] = kx022_getAccX();
    res[1] = kx022_getAccY();
    res[2] = kx022_getAccZ();
}
 
int16_t kx022_getAccAxis(uint8_t addr)
{
    int16_t acc;
    uint8_t res[2]={0,0};
 
    kx022_readRegs(addr, res, 2);
    acc = ((res[1] << 8) | res[0]);
    return acc;
}
 
int kx022_readRegs(uint8_t reg_addr, uint8_t * data, int len)
{
    // int read_nok;
    // char t[1] = {addr};
    
    // m_i2c.write(m_addr, t, 1, true);
    // read_nok = m_i2c.read(m_addr, (char *)data, len);
    // if (read_nok){
    //     DEBUG_PRINT("Read fail\n\r");        
    //     }
    //--------------^Original from mbed^----------------
    
    int rc;

	struct hal_i2c_master_data data_struct = {
		.address = ACC_ADDR,
		.len = 1,
		.buffer = &reg_addr
	};

	/* Register adress write */
	rc = hal_i2c_master_write(I2C_NUM, &data_struct,
			OS_TICKS_PER_SEC / 10, 1);
	if (rc) {
		DEBUG_PRINT("KX022 read8 I2C access failed at address 0x%02X\n", reg_addr);
		goto err;
	}

	/* Read bytes back */
    data_struct.len = len;
    data_struct.buffer = data;
	rc = hal_i2c_master_read(I2C_NUM, &data_struct,
			OS_TICKS_PER_SEC / 10, 1);

	if (rc) {
		DEBUG_PRINT("Failed to read from 0x%02X:0x%02X\n", ACC_ADDR, reg_addr);
	}

err:
	return rc;
}
 
int kx022_writeRegs(uint8_t * data, int len)
{
    // m_i2c.write(m_addr, (char *)data, len);
    //-----^original from mbed^-----------
    int rc;

	struct hal_i2c_master_data data_struct = {
		.address = ACC_ADDR,
		.len = len,
		.buffer = data
	};

	/* Register adress write */
	rc = hal_i2c_master_write(I2C_NUM, &data_struct,
			OS_TICKS_PER_SEC / 10, 1);
	if (rc) {
		DEBUG_PRINT("KX022 I2C write regs failed");
		return rc;
	}
    return 0;
}