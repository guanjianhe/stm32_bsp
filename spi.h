#ifndef SPI_H
#define SPI_H
#include "stm32f10x.h"

#if 1

    extern void spi_init(void);
    extern uint8_t spi_xchg(uint8_t dat);
    extern void spi_read_multi(uint8_t* buff, uint16_t btr);
    extern void spi_write_multi(const uint8_t* buff, uint16_t btx);
    extern void spi_fclk_slow(void);
    extern void spi_fclk_fast(void);
#else
    void SPI1_Init(void);            //初始化SPI口
    void SPI1_SetSpeed(u8 SpeedSet); //设置SPI速度
    u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
#endif


#endif
