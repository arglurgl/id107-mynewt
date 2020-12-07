#define ACC_ADDR 0x1F
#define I2C_NUM 1

int kx022_readRegs(uint8_t addr, uint8_t * data, int len);
int kx022_writeRegs(uint8_t * data, int len);
int16_t kx022_getAccAxis(uint8_t addr);