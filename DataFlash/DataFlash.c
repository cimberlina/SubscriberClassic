/*
 * DataFlash.c
 *
 *  Created on: Mar 1, 2012
 *      Author: ironman
 */
#include "includes.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif


#define	DEV_DFLASH0			1
#define	DEV_DFLASH1			2

// Chip Select para DataFlash0
//#define FLASH_CS0_OFF() 	GPIO_SetValue(0, 1<<16)
//#define FLASH_CS0_ON()  	GPIO_ClearValue(0, 1<<16 )
#define FLASH_CS1_OFF() 	GPIO_SetValue(0, 1<<16)
#define FLASH_CS1_ON()  	GPIO_ClearValue(0, 1<<16 )
// Chip Select para DataFlash1
#define FLASH_CS0_OFF() 	GPIO_SetValue(1, 1<<24)
#define FLASH_CS0_ON()  	GPIO_ClearValue(1, 1<<24 )

#define FLASH_CMD_RDID      0x9F        /* read device ID */
#define FLASH_CMD_RDSR      0xD7        /* read status register */
#define FLASH_CMD_FAST_READ 0x0B        /* read data (extra command setup time allows higher SPI clock) */

#define FLASH_CMD_SE        0x7C        /* sector erase */
#define FLASH_CMD_BE        0x50        /* block erase */
#define FLASH_CMD_PE        0x81        /* page erase */
#define FLASH_CMD_PP_BUF1    0x82        /* page program through buffer 1 */
#define FLASH_CMD_PP_BUF2    0x85        /* page program through buffer 2 */

#define FLASH_CMD_DP        0xB9        /* deep power down */
#define FLASH_CMD_RES       0xAB        /* release from deep power down */


#define ATMEL_MANU_ID   0x1F
#define ATMEL_DEVICE_ID1 ((1 << 5) | (0x05) ) /* DataFlash, 8 MBit */

/* status register defines */
#define STATUS_RDY      	(1 << 7)
#define STATUS_COMP     	(1 << 6)
#define STATUS_PROTECT  	(1 << 1)
#define STATUS_POW2     	(1 << 0)



#define FLAG_IS_POW2 0x01

struct _flash_info
{
    char* name;
    uint32_t jedec_id;
    uint32_t nr_pages;
    uint16_t pagesize;
    uint16_t pageoffset;
    uint8_t flags;
};

/******************************************************************************
 * External global variables
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/

static uint16_t pageSize = 0;
static uint16_t pageOffset = 0;
static uint8_t  pageIsPow2 = 0;
static uint8_t  pageSizeChanged = FALSE;
static uint32_t flashTotalSize = 0;

static struct _flash_info flash_devices[] = {
        {"AT45DB081D", 0x1F2500, 4096, 264, 9, 0},
        {"AT45DB081D", 0x1F2500, 4096, 256, 8, FLAG_IS_POW2},
        {"AT45DB161D", 0x1F2600, 4096, 528, 10, 0},
        {"AT45DB161D", 0x1F2600, 4096, 512, 9, FLAG_IS_POW2},
};

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static void SSPSend(uint8_t *buf, uint32_t Length)
{
    SSP_DATA_SETUP_Type xferConfig;

	xferConfig.tx_data = buf;
	xferConfig.rx_data = NULL;
	xferConfig.length  = Length;

    SSP_ReadWrite(LPC_SSP0, &xferConfig, SSP_TRANSFER_POLLING);
}

static void SSPReceive( uint8_t *buf, uint32_t Length )
{
    SSP_DATA_SETUP_Type xferConfig;

	xferConfig.tx_data = NULL;
	xferConfig.rx_data = buf;
	xferConfig.length  = Length;

    SSP_ReadWrite(LPC_SSP0, &xferConfig, SSP_TRANSFER_POLLING);
}

static void exitDeepPowerDown(void)
{
    uint8_t cmd = FLASH_CMD_RES;
    FLASH_CS0_ON();

    SSPSend( (uint8_t *)&cmd, 1 );

    FLASH_CS0_OFF();
}

static void readDeviceId(uint8_t *deviceId)
{
    uint8_t cmd = FLASH_CMD_RDID;

    FLASH_CS0_ON();

    SSPSend( (uint8_t *)&cmd, 1 );
    SSPReceive( deviceId, 4 );

    FLASH_CS0_OFF();
}

static uint8_t readStatus(void)
{
    uint8_t cmd = FLASH_CMD_RDSR;
    uint8_t status = 0;

    FLASH_CS0_ON();

    SSPSend( (uint8_t *)&cmd, 1 );
    SSPReceive( &status, 1 );

    FLASH_CS0_OFF();

    return status;
}

static void pollIsBusy(void)
{
  uint8_t status = 0;
  int i = 0;

  do
  {
    for (i = 0; i < 0x2000; i++);

    status = readStatus();
  }
  while ((status & STATUS_RDY) == 0);
}


static void setAddressBytes(uint8_t* addr, uint32_t offset)
{
	uint16_t page;
	uint16_t off; 
	
	page = offset / pageSize;
	off  = offset % pageSize;

	addr[2] = (uint8_t)(off & 0x00FF);
	addr[1] = ((uint8_t)(off >> 8)) & 0x03;

	addr[1] |= ((uint8_t)(page << 2)) & 0xFC;
	addr[0] = (uint8_t)(page >> 6) & 0x3F;

//    if (!pageIsPow2) {
//        page = offset / pageSize;
//        off  = offset % pageSize;
//
//        /* buffer address bits */
//        addr[2] = (off & 0xff);
//        addr[1] = (off / pageSize);
//
//        /* page address bits */
//        addr[1] |= ((page & ((1 << (16-pageOffset))-1)) << (pageOffset-8));
//        addr[0] = ((page >> (16-pageOffset)) & ((1 << (16-pageOffset))-1) );
//    }
//    else {
//
//        addr[0] = ((offset >> 16) & 0xFF);
//        addr[1] = ((offset >>  8) & 0xFF);
//        addr[2] = ((offset >>  0) & 0xFF);
//
//    }
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Initialize the SPI Flash Driver
 *
 * Returns:
 *   TRUE if initialization successful, otherwise FALSE
 *
 *****************************************************************************/
uint32_t flash0_init (void)
{
    uint8_t deviceId[4];
    uint8_t status = 0;
    uint8_t flag   = 0;
    uint32_t id = 0;
    int i = 0;


    FLASH_CS0_OFF();

    exitDeepPowerDown();
    readDeviceId(deviceId);

    do {
        status = readStatus();
    } while ((status & STATUS_RDY) == 0);

    if ((status & STATUS_POW2) != 0)
    {
        flag = FLAG_IS_POW2;
    }

    id = (deviceId[0] << 16 | deviceId[1] << 8 | deviceId[2]);

    for (i = 0; i < sizeof(flash_devices) / sizeof(struct _flash_info); i++) {
        if (flash_devices[i].jedec_id == id
                && flash_devices[i].flags == flag)
        {
            pageSize   = flash_devices[i].pagesize;
            pageOffset = flash_devices[i].pageoffset;
            pageIsPow2 = ((flash_devices[i].flags & FLAG_IS_POW2) != 0) ;
            flashTotalSize = flash_devices[i].pagesize * flash_devices[i].nr_pages;

            return TRUE;
        }
    }


    return FALSE;
}

/******************************************************************************
 *
 * Description:
 *    Write data to flash
 *
 * Params:
 *   [in] buf - data to write to flash
 *   [in] offset - offset into the flash
 *   [in] len - number of bytes to write
 *
 * Returns:
 *   number of written bytes
 *
 *****************************************************************************/
uint32_t flash0_write(int dfbuff, uint8_t* buf, uint32_t offset, uint32_t len)
{
    uint16_t wLen;
    uint32_t written = 0;
    uint8_t addr[4];

    if (len > flashTotalSize || len+offset > flashTotalSize) {
        return 0;
    }

    if (pageSizeChanged) {
        return 0;
    }

    //cargo la pagina en el buffer
	if(dfbuff == 1)
		addr[0] = 0x53;
	else
		addr[0] = 0x55;
    setAddressBytes(&addr[1], offset);
    FLASH_CS0_ON();
    SSPSend(addr, 4);
    FLASH_CS0_OFF();
    pollIsBusy();

    /* write up to first page boundry */
    wLen = (((( (offset%pageSize) / pageSize) + 1) * pageSize)
            - (offset%pageSize));
    wLen = MIN(wLen, len);

    while (len) {

        /* write address */
    	if(dfbuff == 1)
    		addr[0] = FLASH_CMD_PP_BUF1;
    	else
    		addr[0] = FLASH_CMD_PP_BUF2;
        setAddressBytes(&addr[1], offset);

        FLASH_CS0_ON();

        SSPSend(addr, 4);

        /* write data */
        SSPSend(&buf[written], wLen);

        FLASH_CS0_OFF();

        /* delay to wait for a write cycle */
        //eepromDelay();

        len     -= wLen;
        written += wLen;
        offset  += wLen;
        wLen = MIN(pageSize, len);

        pollIsBusy();
    }

    return written;
}



/******************************************************************************
 *
 * Description:
 *    Read data from flash
 *
 * Params:
 *   [in] buf - data buffer
 *   [in] offset - offset into the flash
 *   [in] len - number of bytes to read
 *
 * Returns:
 *   number of read bytes
 *
 *****************************************************************************/
uint32_t flash0_read(uint8_t* buf, uint32_t offset, uint32_t len)
{
    uint8_t addr[5];

    if (len > flashTotalSize || len+offset > flashTotalSize) {
        return 0;
    }

    if (pageSizeChanged) {
        return 0;
    }

    addr[0] = FLASH_CMD_FAST_READ;

    setAddressBytes(&addr[1], offset);
    addr[4] = (0);

    FLASH_CS0_ON();

    SSPSend(addr, 5);
    SSPReceive(buf, len);

    FLASH_CS0_OFF();

    return len;
}

/******************************************************************************
 *
 * Description:
 *    Get flash page size
 *
 * Returns:
 *   256 or 264
 *
 *****************************************************************************/
uint16_t flash0_getPageSize(void)
{
    return pageSize;
}

/******************************************************************************
 *
 * Description:
 *    Change page size to binary page, i.e. 256 bytes.
 *
 *    NOTE: This operation cannot be undone and requires a power-cycle
 *    before taking effect.
 *
 *****************************************************************************/
void flash0_setToBinaryPageSize(void)
{
    uint8_t data[4];

    data[0] = 0x3D;
    data[1] = 0x2A;
    data[2] = 0x80;
    data[3] = 0xA6;

    FLASH_CS0_ON();

    SSPSend( (uint8_t *)data, 4 );

    FLASH_CS0_OFF();

    pageSizeChanged = (!pageIsPow2);
}


