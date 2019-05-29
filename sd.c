#include "sd.h"
#include "spi.h"

#if 0
#define spi_xchg SPI1_ReadWriteByte
#define spi_init SPI1_Init
#define spi_fclk_slow() SPI1_SetSpeed(SPI_BaudRatePrescaler_256)
#define spi_fclk_fast() SPI1_SetSpeed(SPI_BaudRatePrescaler_2)

#define spi_read_multi SD_RecvData
#define spi_write_multi SD_SendData

void SD_RecvData(u8* buf, u16 len)
{
    while (len--) //开始接收数据
    {
        *buf = SPI1_ReadWriteByte(0xFF);
        buf++;
    }
}

void SD_SendData(const u8* buf, u16 len)
{
    uint8_t rec;
    u16 i;

    for (i = 0; i < len; i++)
    {
        rec = SPI1_ReadWriteByte(buf[i]);
    }
}


#endif

#define CS_HIGH() GPIOA->BSRR = GPIO_Pin_3
#define CS_LOW()  GPIOA->BRR  = GPIO_Pin_3

#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)        /* ERASE_ER_BLK_START */
#define CMD33   (33)        /* ERASE_ER_BLK_END */
#define CMD38   (38)        /* ERASE */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC      0x01        /* MMC ver 3 */
#define CT_SD1      0x02        /* SD ver 1 */
#define CT_SD2      0x04        /* SD ver 2 */
#define CT_SDC      (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK    0x08        /* Block addressing */

static volatile uint16_t Timer1, Timer2;

static uint8_t CardType;          /* Card type flags */


static int wait_ready(  /* 1:Ready, 0:Timeout */
    uint16_t wt         /* Timeout [ms] */
)
{
    uint8_t d;


    Timer2 = wt;

    do
    {
        d = spi_xchg(0xFF);
        /* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
    } while (d != 0xFF && Timer2);  /* Wait for card goes ready or timeout */

    return (d == 0xFF) ? 1 : 0;
}

static void deselect(void)
{
    CS_HIGH();      /* Set CS# high */
    spi_xchg(0xFF); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/* 1:OK, 0:Timeout */
static int select(void)
{
    CS_LOW();       /* Set CS# low */
    spi_xchg(0xFF); /* Dummy clock (force DO enabled) */

    if (wait_ready(500))
    {
        return 1;    /* Wait for card ready */
    }

    deselect();
    return 0;   /* Timeout */
}


/* 1:OK, 0:Error */
static int rcvr_datablock(
    uint8_t* buff,         /* Data buffer */
    uint16_t btr            /* Data block length (byte) */
)
{
    uint8_t token;

    Timer1 = 200;

    do                              /* Wait for DataStart token in timeout of 200ms */
    {
        token = spi_xchg(0xFF);
        /* This loop will take a time. Insert rot_rdq() here for multitask envilonment. */
    } while ((token == 0xFF) && Timer1);

    if (token != 0xFE)
    {
        return 0;    /* Function fails if invalid DataStart token or timeout */
    }

    spi_read_multi(buff, btr);      /* Store trailing data to the buffer */
    spi_xchg(0xFF);
    spi_xchg(0xFF);         /* Discard CRC */

    return 1;                       /* Function succeeded */
}

/* 1:OK, 0:Failed */
static int xmit_datablock(
    const uint8_t* buff,   /* Ponter to 512 byte data to be sent */
    uint8_t token          /* Token */
)
{
    uint8_t resp;


    if (!wait_ready(500))
    {
        return 0;    /* Wait for card ready */
    }

    spi_xchg(token);                    /* Send token */

    /* Send data if token is other than StopTran */
    if (token != 0xFD)
    {
        spi_write_multi(buff, 512);      /* Data */
        spi_xchg(0xFF);
        spi_xchg(0xFF); /* Dummy CRC */

        resp = spi_xchg(0xFF);              /* Receive data resp */

        if ((resp & 0x1F) != 0x05)
        {
            return 0;    /* Function fails if the data packet was not accepted */
        }
    }

    return 1;
}

static uint8_t send_cmd(   /* Return value: R1 resp (bit7==1:Failed to send) */
    uint8_t cmd,       /* Command index */
    uint32_t arg       /* Argument */
)
{
    uint8_t n, res;


    if (cmd & 0x80)     /* Send a CMD55 prior to ACMD<n> */
    {
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);

        if (res > 1)
        {
            return res;
        }
    }

    /* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12)
    {
        deselect();

        if (!select())
        {
            return 0xFF;
        }
    }

    /* Send command packet */
    spi_xchg(0x40 | cmd);               /* Start + command index */
    spi_xchg((uint8_t)(arg >> 24));        /* Argument[31..24] */
    spi_xchg((uint8_t)(arg >> 16));        /* Argument[23..16] */
    spi_xchg((uint8_t)(arg >> 8));         /* Argument[15..8] */
    spi_xchg((uint8_t)arg);                /* Argument[7..0] */
    n = 0x01;                           /* Dummy CRC + Stop */

    if (cmd == CMD0)
    {
        n = 0x95;    /* Valid CRC for CMD0(0) */
    }

    if (cmd == CMD8)
    {
        n = 0x87;    /* Valid CRC for CMD8(0x1AA) */
    }

    spi_xchg(n);

    /* Receive command resp */
    if (cmd == CMD12)
    {
        spi_xchg(0xFF);    /* Diacard following one byte when CMD12 */
    }

    n = 10;                             /* Wait for response (10 bytes max) */

    do
    {
        res = spi_xchg(0xFF);
    } while ((res & 0x80) && --n);

    return res;                         /* Return received response */
}

/* 0:OK, other:Failed */
int sd_write(
    const uint8_t* buff,   /* Ponter to the data to write */
    uint32_t sector,       /* Start sector number (LBA) */
    uint16_t count          /* Number of sectors to write (1..128) */
)
{
    if (!count)
    {
        return 4;
    }

    if (!(CardType & CT_BLOCK))
    {
        sector *= 512;    /* LBA ==> BA conversion (byte addressing cards) */
    }

    if (count == 1)     /* Single sector write */
    {
        if ((send_cmd(CMD24, sector) == 0)  /* WRITE_BLOCK */
                && xmit_datablock(buff, 0xFE))
        {
            count = 0;
        }
    }
    else                /* Multiple sector write */
    {
        if (CardType & CT_SDC)
        {
            send_cmd(ACMD23, count);    /* Predefine number of sectors */
        }

        if (send_cmd(CMD25, sector) == 0)   /* WRITE_MULTIPLE_BLOCK */
        {
            do
            {
                if (!xmit_datablock(buff, 0xFC))
                {
                    break;
                }

                buff += 512;
            } while (--count);

            if (!xmit_datablock(0, 0xFD))
            {
                count = 1;    /* STOP_TRAN token */
            }
        }
    }

    deselect();

    return count ? 1 : 0;  /* Return result */
}


/* 0:OK, other:Failed */
int sd_read(
    uint8_t* buff,     /* Pointer to the data buffer to store read data */
    uint32_t sector,   /* Start sector number (LBA) */
    uint16_t count      /* Number of sectors to read (1..128) */
)
{
    if (!count)
    {
        return 4;
    }

    if (!(CardType & CT_BLOCK))
    {
        sector *= 512;    /* LBA ot BA conversion (byte addressing cards) */
    }

    if (count == 1)     /* Single sector read */
    {
        /* READ_SINGLE_BLOCK */
        if ((send_cmd(CMD17, sector) == 0) && rcvr_datablock(buff, 512))
        {
            count = 0;
        }
    }
    else                /* Multiple sector read */
    {
        if (send_cmd(CMD18, sector) == 0)   /* READ_MULTIPLE_BLOCK */
        {
            do
            {
                if (!rcvr_datablock(buff, 512))
                {
                    break;
                }

                buff += 512;
            } while (--count);

            send_cmd(CMD12, 0);             /* STOP_TRANSMISSION */
        }
    }

    deselect();

    return count ? 1 : 0;  /* Return result */
}


/* Get drive capacity in unit of sector (uint32_t) */
uint32_t sd_get_sector_count(void)
{
    uint8_t n, csd[16];
    uint32_t csize;
    uint32_t sector_count = 0;

    if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
    {
        if ((csd[0] >> 6) == 1)     /* SDC ver 2.00 */
        {
            csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
            sector_count = csize << 10;
        }
        else                        /* SDC ver 1.XX or MMC ver 3 */
        {
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(
                        csd[6] & 3) << 10) + 1;
            sector_count = csize << (n - 9);
        }
    }

    return sector_count;
}

/* Get erase block size in unit of sector (DWORD) */
uint32_t sd_get_block_size(void)
{
    uint8_t n, csd[16];
    uint32_t block_size = 0;

    if (CardType & CT_SD2)      /* SDC ver 2.00 */
    {
        if (send_cmd(ACMD13, 0) == 0)   /* Read SD status */
        {
            spi_xchg(0xFF);

            if (rcvr_datablock(csd, 16))                /* Read partial block */
            {
                for (n = 64 - 16; n; n--)
                {
                    spi_xchg(0xFF);    /* Purge trailing data */
                }

                block_size = 16UL << (csd[10] >> 4);
            }
        }
    }
    else                        /* SDC ver 1.XX or MMC */
    {
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))    /* Read CSD */
        {
            if (CardType & CT_SD1)      /* SDC ver 1.XX */
            {
                block_size = (((csd[10] & 63) << 1) + ((uint16_t)(csd[11] & 128) >> 7) + 1)
                             << ((
                                     csd[13] >> 6) - 1);
            }
            else                        /* MMC */
            {
                block_size = ((uint16_t)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((
                                 csd[11] & 224) >> 5) + 1);
            }
        }
    }

    return block_size;
}

/* 0:OK, 1:Failed */
int sd_ctrl_sync(void)
{
    int res = 0;

    if (select())
    {
        res = 0;
    }
    else
    {
        res = 1;
    }

    return res;
}

/* 返回：0成功；1失败 */
int sd_init(void)
{
    int retval = 0;
    uint8_t n, cmd, ty, ocr[4];

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4); //PA2.3.4上拉


    /*****************************************************/

    spi_init();

    spi_fclk_slow();

    for (n = 10; n; n--)
    {
        spi_xchg(0xFF);    /* Send 80 dummy clocks */
    }

    ty = 0;

    if (send_cmd(CMD0, 0) == 1)             /* Put the card SPI/Idle state */
    {
        Timer1 = 1000;                      /* Initialization timeout = 1 sec */

        if (send_cmd(CMD8, 0x1AA) == 1)     /* SDv2? */
        {
            for (n = 0; n < 4; n++)
            {
                ocr[n] = spi_xchg(0xFF);    /* Get 32 bit return value of R7 resp */
            }

            /* Is the card supports vcc of 2.7-3.6V? */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                /* Wait for end of initialization with ACMD41(HCS) */
                while (Timer1 && send_cmd(ACMD41, 1UL << 30));

                if (Timer1 && send_cmd(CMD58, 0) == 0)          /* Check CCS bit in the OCR */
                {
                    for (n = 0; n < 4; n++)
                    {
                        ocr[n] = spi_xchg(0xFF);
                    }

                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* Card id SDv2 */
                }
            }
        }
        else        /* Not SDv2 card */
        {
            if (send_cmd(ACMD41, 0) <= 1)       /* SDv1 or MMC? */
            {
                ty = CT_SD1;
                cmd = ACMD41;  /* SDv1 (ACMD41(0)) */
            }
            else
            {
                ty = CT_MMC;
                cmd = CMD1;    /* MMCv3 (CMD1(0)) */
            }

            while (Timer1 && send_cmd(cmd, 0)) ;        /* Wait for end of initialization */

            if (!Timer1 || send_cmd(CMD16, 512) != 0)   /* Set block length: 512 */
            {
                ty = 0;
            }
        }
    }

    CardType = ty;  /* Card type */
    deselect();

    if (ty)             /* OK */
    {
        spi_fclk_fast();            /* Set fast clock */
        retval = 0;
    }
    else                /* Failed */
    {
        retval = 1;
    }

    return retval;
}


/*-----------------------------------------------------------------------*/
/* Device timer function                                                 */
/*-----------------------------------------------------------------------*/
/* This function must be called from timer interrupt routine in period
/  of 1 ms to generate card control timing.
*/

void disk_timerproc(void)
{
    uint16_t n;

    n = Timer1;                     /* 1kHz decrement timer stopped at 0 */

    if (n)
    {
        Timer1 = --n;
    }

    n = Timer2;

    if (n)
    {
        Timer2 = --n;
    }
}




