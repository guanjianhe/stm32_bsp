#ifndef SD_H
#define SD_H

#include "stdint.h"



/* This function must be called from timer interrupt routine in period
 *  of 1 ms to generate card control timing.
 */
extern void disk_timerproc(void);

/* 0:OK, 1:Failed */
extern int sd_init(void);

/* 0:OK, other:Failed */
extern int sd_write(const uint8_t* buff, uint32_t sector, uint16_t count);

/* 0:OK, other:Failed */
extern int sd_read(uint8_t* buff, uint32_t sector, uint16_t count);

/* Get drive capacity in unit of sector (uint32_t) */
extern uint32_t sd_get_sector_count(void);

/* Get erase block size in unit of sector (uint32_t) */
extern uint32_t sd_get_block_size(void);

/* 0:OK, 1:Failed */
extern int sd_ctrl_sync(void);


#endif
