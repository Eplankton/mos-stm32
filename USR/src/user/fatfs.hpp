#ifndef _MOS_FATFS_
#define _MOS_FATFS_

#include "src/mos/kernel/task.hpp"
#include "fatfs/ff.h"

namespace MOS::FatFs
{
	void mnt_or_fmt(FATFS& fs)
	{
		// 挂载文件系统，文件系统挂载时会对 SPI-SD 设备初始化 -> 调用 sd.init()
		auto res = f_mount(&fs, "0:", 1); /* 文件操作结果 */

		/*----------------------- 格式化测试 ---------------------------*/
		/* 如果没有文件系统就格式化创建创建文件系统 */
		if (res == FR_NO_FILESYSTEM) {
			MOS_MSG("No FatFs, fmt...");
			/* 格式化 */
			res = f_mkfs("0:", 0, 0);

			if (res == FR_OK) {
				MOS_MSG("FatFs fmt success!");
				res = f_mount(NULL, "0:", 1); /* 格式化后，先取消挂载 */
				res = f_mount(&fs, "0:", 1);  /* 重新挂载 */
			}
			else {
				MOS_MSG("FatFs fmt failed!");
				while (true) {
					MOS_NOP();
				}
			}
		}
		else if (res != FR_OK) {
			MOS_MSG("Bad:(%d)", res);
			while (true) {
				MOS_NOP();
			}
		}
		else {
			MOS_MSG("FatFs mnt!");
		}
	}

	// Write Test
	void w_test(FATFS& fs)
	{
		static BYTE w_buf[] = "hello, world!"; /* 写缓冲区*/
		static FIL file;                       /* 文件对象 */
		UINT fnum;                             /* 文件成功读写数量 */

		/*--------------------- 文件系统测试：写测试 -----------------------*/
		/* 打开文件，如果文件不存在则创建它 */
		auto res = f_open(
		    &file,
		    "0:rw_test.txt",
		    FA_CREATE_ALWAYS | FA_WRITE
		);

		if (res == FR_OK) {
			MOS_MSG("File open success!");

			/* 将指定存储区内容写入到文件内 */
			res = f_write(
			    &file,
			    w_buf,
			    sizeof(w_buf), &fnum
			);

			/* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如需写入大量数据使用f_write_co替代上面f_write即可 */
			//res_sd=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
			if (res == FR_OK) {
				MOS_MSG("W= %d B <- \"%s\"", fnum, w_buf);
			}
			else {
				MOS_MSG("Bad:(%d)", res);
			}

			f_close(&file); /* 不再读写，关闭文件 */
		}
		else {
			MOS_MSG("File open failed!");
		}

		f_close(&file); /* 不再读写，关闭文件 */
	}

	// Read Test
	void r_test(FATFS& fs)
	{
		static BYTE r_buf[1024] = {0}; /* 读缓冲区 */
		static FIL file;               /* 文件对象 */
		UINT fnum;                     /* 文件成功读写数量 */

		/*------------------ 文件系统测试：读测试 --------------------------*/
		/* 打开文件，如果文件不存在则创建它 */
		auto res = f_open(
		    &file,
		    "0:rw_test.txt",
		    FA_OPEN_EXISTING | FA_READ
		);

		if (res == FR_OK) {
			MOS_MSG("File open success!");

			res = f_read(
			    &file,
			    r_buf,
			    sizeof(r_buf),
			    &fnum
			);

			// 实测SPI_SD驱动下读取大于512字节的数据在SD卡里打开会显示乱码
			// 如需读取大量数据使用f_read_co替代上面f_read即可
			// res_sd = f_read_co(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
			if (res == FR_OK) {
				MOS_MSG("R= %d B -> \"%s\"", fnum, r_buf);
			}
			else {
				MOS_MSG("Bad:(%d)", res);
			}
		}
		else {
			MOS_MSG("File open failed!");
		}

		f_close(&file); /* 不再读写，关闭文件 */
	}

	void test()
	{
		static FATFS fatfs; /* FatFs文件系统对象 */
		kprintf("--------< FatFs R/W Test >--------\n");
		mnt_or_fmt(fatfs);
		w_test(fatfs);
		r_test(fatfs);
		f_mount(NULL, "0:", 1); /* 取消挂载文件系统 */
		kprintf("----------------------------------\n");
	}
}

#endif