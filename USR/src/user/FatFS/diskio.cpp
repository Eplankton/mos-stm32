/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* FatFs lower layer API */
#include "ff.h"
#include "../../drivers/device/sd.hpp"

using Driver::Device::SD_t;
namespace MOS::User::Global
{
	extern SD_t sd;
}
using MOS::User::Global::sd;

/* 为每个设备定义一个物理编号 */
#define ATA       0 // 预留SD卡使用
#define SPI_FLASH 1 // 外部SPI Flash

/*-----------------------------------------------------------------------*/
/* 获取设备状态                                                          */
/*-----------------------------------------------------------------------*/
//存储设备状态获取
DSTATUS disk_status(
    BYTE pdrv /* 物理编号 */
)
{
	DSTATUS status = STA_NOINIT;
	switch (pdrv) {
		case ATA: /* SD CARD */
			status &= ~STA_NOINIT;
			break;

		case SPI_FLASH: /* SPI Flash */
			break;

		default:
			status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 设备初始化                                                            */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(
    BYTE pdrv /* 物理编号 */
)
{
	DSTATUS status = STA_NOINIT;
	switch (pdrv) {
		case ATA: /* SD CARD */
			if (sd.init() == SD_t::RESPONSE_NO_ERROR) {
				status &= ~STA_NOINIT;
			}
			else {
				status = STA_NOINIT;
			}

			break;

		case SPI_FLASH: /* SPI Flash */
			break;

		default:
			status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 读扇区：读取扇区内容到指定存储区                                              */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(
    BYTE pdrv,    /* 设备物理编号(0..) */
    BYTE* buff,   /* 数据缓存区 */
    DWORD sector, /* 扇区首地址 */
    UINT count    /* 扇区个数(1..128) */
)
{
	DRESULT status       = RES_PARERR;
	SD_t::Error SD_state = SD_t::RESPONSE_NO_ERROR;

	switch (pdrv) {
		case ATA: /* SD CARD */

			SD_state = sd.read_multi_block(
			    buff,
			    sector * SD_t::BLOCK_SIZE,
			    SD_t::BLOCK_SIZE,
			    count
			);

			if (SD_state != SD_t::RESPONSE_NO_ERROR)
				status = RES_PARERR;
			else
				status = RES_OK;
			break;

		case SPI_FLASH:
			break;

		default:
			status = RES_PARERR;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 写扇区：将数据写入指定扇区空间上                                      */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE

DRESULT disk_write(
    BYTE pdrv,        /* 设备物理编号(0..) */
    const BYTE* buff, /* 欲写入数据的缓存区 */
    DWORD sector,     /* 扇区首地址 */
    UINT count        /* 扇区个数(1..128) */
)
{
	DRESULT status       = RES_PARERR;
	SD_t::Error SD_state = SD_t::RESPONSE_NO_ERROR;

	if (!count) {
		return RES_PARERR; /* Check parameter */
	}

	switch (pdrv) {
		case ATA: /* SD CARD */
			SD_state = sd.write_multi_block(
			    (uint8_t*) buff,
			    sector * SD_t::BLOCK_SIZE,
			    SD_t::BLOCK_SIZE,
			    count
			);
			if (SD_state != SD_t::RESPONSE_NO_ERROR)
				status = RES_PARERR;
			else
				status = RES_OK;
			break;

		case SPI_FLASH:
			break;

		default:
			status = RES_PARERR;
	}
	return status;
}

#endif

/*-----------------------------------------------------------------------*/
/* 其他控制                                                              */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl(
    BYTE pdrv, /* 物理编号 */
    BYTE cmd,  /* 控制指令 */
    void* buff /* 写入或者读取数据地址指针 */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv) {
		case ATA: /* SD CARD */
			switch (cmd) {
				// Get R/W sector size (WORD)
				case GET_SECTOR_SIZE:
					*(WORD*) buff = SD_t::BLOCK_SIZE;
					break;
				// Get erase block size in unit of sector (DWORD)
				case GET_BLOCK_SIZE:
					*(DWORD*) buff = 1;
					break;

				case GET_SECTOR_COUNT:
					*(DWORD*) buff = sd.info.capacity / sd.info.block_size;
					break;
				case CTRL_SYNC:
					break;
			}
			status = RES_OK;
			break;

		case SPI_FLASH:
			break;

		default:
			status = RES_PARERR;
	}
	return status;
}
#endif

__WEAK DWORD get_fattime(void)
{
	/* 返回当前时间戳 */
	return ((DWORD) (2024 - 1980) << 25) /* Year 2024 */
	       | ((DWORD) 1 << 21)           /* Month 1 */
	       | ((DWORD) 1 << 16)           /* Mday 1 */
	       | ((DWORD) 0 << 11)           /* Hour 0 */
	       | ((DWORD) 0 << 5)            /* Min 0 */
	       | ((DWORD) 0 >> 1);           /* Sec 0 */
}
