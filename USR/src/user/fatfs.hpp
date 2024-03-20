#ifndef _MOS_FATFS_
#define _MOS_FATFS_

#include "src/mos/kernel/task.hpp"
#include "FatFS/ff.h"

namespace MOS::User::FatFs
{
	static FATFS fs; /* FatFs文件系统对象 */
	static FIL file; /* 文件对象 */

	void launch()
	{
		static BYTE write_buf[]    = "2024 MOS File System Test\n"; /* 写缓冲区*/
		static BYTE read_buf[1024] = {0};                           /* 读缓冲区 */

		static FRESULT res; /* 文件操作结果 */
		static UINT fnum;   /* 文件成功读写数量 */

		// 挂载文件系统，文件系统挂载时会对SPI设备初始化
		res = f_mount(&fs, "0:", 1);

		/*----------------------- 格式化测试 ---------------------------*/
		/* 如果没有文件系统就格式化创建创建文件系统 */
		if (res == FR_NO_FILESYSTEM) {
			kprintf("init\n");
			/* 格式化 */
			res = f_mkfs("0:", 0, 0);

			if (res == FR_OK) {
				kprintf("good\n");
				/* 格式化后，先取消挂载 */
				res = f_mount(NULL, "0:", 1);
				/* 重新挂载 */
				res = f_mount(&fs, "0:", 1);
			}
			else {
				kprintf("bad\n");
				while (true) {
					MOS_NOP();
				}
			}
		}
		else if (res != FR_OK) {
			kprintf("bad(%d)\r\n", res);
			while (true) {
				MOS_NOP();
			}
		}
		else {
			kprintf("good\n");
		}

		/*--------------------- 文件系统测试：写测试 -----------------------*/
		/* 打开文件，如果文件不存在则创建它 */
		res = f_open(&file, "0:rw_test.txt", FA_CREATE_ALWAYS | FA_WRITE);
		if (res == FR_OK) {
			kprintf("good\n");
			/* 将指定存储区内容写入到文件内 */
			res = f_write(&file, write_buf, sizeof(write_buf), &fnum);
			/* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如需写入大量数据使用f_write_co替代上面f_write即可 */
			//res_sd=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
			if (res == FR_OK) {
				kprintf("w: %d\n", fnum);
				kprintf("w: %s\n", write_buf);
			}
			else {
				kprintf("bad: (%d)\n", res);
			}
			/* 不再读写，关闭文件 */
			f_close(&file);
		}
		else {
			kprintf("failed\n");
		}

		/*------------------ 文件系统测试：读测试 --------------------------*/
		res = f_open(&file, "0:rw_test.txt", FA_OPEN_EXISTING | FA_READ);
		if (res == FR_OK) {
			kprintf("good\n");
			res = f_read(&file, read_buf, sizeof(read_buf), &fnum);
			/* 实测SPI_SD驱动下读取大于512字节的数据在SD卡里打开会显示乱码，如需读取大量数据使用f_read_co替代上面f_read即可 */
			//res_sd = f_read_co(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
			if (res == FR_OK) {
				kprintf("r: %d\n", fnum);
				kprintf("r: %s\n", read_buf);
			}
			else {
				kprintf("bad: (%d)\n", res);
			}
		}
		else {
			kprintf("bad\n");
		}
		/* 不再读写，关闭文件 */
		f_close(&file);

		/* 不再使用文件系统，取消挂载文件系统 */
		f_mount(NULL, "0:", 1);

		/* 操作完成，停机 */
		Kernel::Task::block();
	}
}

#endif