#include "spi.h"

#if 1


/* Set SCLK = PCLK / 256 */
void spi_fclk_slow(void)
{
    SPI1->CR1 = (SPI1->CR1 & ~0x38) | 0x38;
}

/* Set SCLK = PCLK / 2 */
void spi_fclk_fast(void)
{
    SPI1->CR1 = (SPI1->CR1 & ~0x38) | 0x00;
}


void spi_init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

    //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    //选择了串行时钟的稳态:时钟悬空高
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    //数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    //定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    //CRC值计算的多项式
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
    SPI_Init(SPI1, &SPI_InitStructure);
    //使能SPI外设
    SPI_Cmd(SPI1, ENABLE);
}

/* 交换一个字节 */
uint8_t spi_xchg(uint8_t dat)
{
    SPI1->DR = dat;

    while ((SPI1->SR & 0x83) != 0x03) ;

    return (uint8_t)SPI1->DR;
}

/* 读取多字节（必须是偶数） */
void spi_read_multi(uint8_t* buff, uint16_t btr)
{
    uint16_t d;

    SPI1->CR1 &= ~(1 << 6);
    SPI1->CR1 |= ((1 << 6) | (1 << 11));

    SPI1->DR = 0xFFFF;
    btr -= 2;

    do
    {
        while ((SPI1->SR & 0x83) != 0x03);

        d = SPI1->DR;
        SPI1->DR = 0xFFFF;
        buff[1] = d;
        buff[0] = d >> 8;
        buff += 2;
    } while (btr -= 2);

    while ((SPI1->SR & 0x83) != 0x03);

    d = SPI1->DR;
    buff[1] = d;
    buff[0] = d >> 8;

    SPI1->CR1 &= ~((1 << 6) | (1 << 11));
    SPI1->CR1 |= 1 << 6;
}


/* 写多字节（必须是偶数） */
void spi_write_multi(const uint8_t* buff, uint16_t btx)
{
    uint16_t d;

    SPI1->CR1 &= ~(1 << 6);
    SPI1->CR1 |= ((1 << 6) | (1 << 11));

    d = buff[0] << 8 | buff[1];
    buff += 2;
    SPI1->DR = d;
    btx -= 2;

    do
    {
        d = buff[0] << 8 | buff[1];
        buff += 2;

        while ((SPI1->SR & 0x83) != 0x03) ;

        SPI1->DR;
        SPI1->DR = d;
    } while (btx -= 2);

    while ((SPI1->SR & 0x83) != 0x03) ;

    SPI1->DR;

    SPI1->CR1 &= ~((1 << 6) | (1 << 11));
    SPI1->CR1 |= (1 << 6);
}

#else

SPI_InitTypeDef  SPI_InitStructure;

void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

    SPI_InitStructure.SPI_Direction =
        SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode =
        SPI_Mode_Master;       //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize =
        SPI_DataSize_8b;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL =
        SPI_CPOL_High;     //选择了串行时钟的稳态:时钟悬空高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS =
        SPI_NSS_Soft;       //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_256;        //定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_FirstBit =
        SPI_FirstBit_MSB;  //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
    SPI_Init(SPI1,
             &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI1, ENABLE); //使能SPI外设

    SPI1_ReadWriteByte(0xff);//启动传输
}
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16分频  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256分频 (SPI 281.25K@sys 72M)

void SPI1_SetSpeed(u8 SpeedSet)
{
    SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 retry = 0;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
        retry++;

        if (retry > 200)
        {
            return 0;
        }
    }

    SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
    retry = 0;

    while (SPI_I2S_GetFlagStatus(SPI1,
                                 SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;

        if (retry > 200)
        {
            return 0;
        }
    }

    return SPI_I2S_ReceiveData(
               SPI1); //返回通过SPIx最近接收的数据
}

#endif
