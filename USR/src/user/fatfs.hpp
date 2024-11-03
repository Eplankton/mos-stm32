#ifndef _MOS_FATFS_
#define _MOS_FATFS_

#include "src/core/kernel/utils.hpp"
#include "fatfs/ff.h"

namespace MOS::FileSys
{
	struct FatFs : public FATFS
	{
		using Raw_t  = FATFS;
		using Path_t = const TCHAR*;
		using Opt_t  = BYTE;

		MOS_INLINE auto // 挂载文件系统
		mount(Path_t path = "0:", Opt_t opt = 1)
		{
			return f_mount(this, path, opt);
		}

		MOS_INLINE void // 卸载文件系统
		umount(Path_t path = "0:", Opt_t opt = 1)
		{
			f_mount(NULL, path, opt);
		}

		MOS_INLINE auto // 文件系统格式化
		mkfs(Path_t path = "0:")
		{
			return f_mkfs(path, 0, 0);
		}

		struct File_t
		{
			using Raw_t  = FIL;
			using Path_t = const TCHAR*;
			using Buf_t  = void*;
			using Res_t  = FRESULT;
			using Len_t  = UINT;

			enum class OpenMode : BYTE
			{
				Read  = FA_OPEN_EXISTING | FA_READ,
				Write = FA_CREATE_ALWAYS | FA_WRITE,
			};

			Raw_t& raw;

			MOS_INLINE auto // 打开文件，如果文件不存在则创建它
			open(Path_t path, OpenMode mode)
			{
				return f_open(&raw, path, (BYTE) mode);
			}

			MOS_INLINE auto
			close() { return f_close(&raw); }

			MOS_INLINE ~File_t() { close(); }

			MOS_INLINE auto
			read(const Buf_t buf, Len_t len)
			{
				struct ReadRes_t
				{
					Res_t fres;
					Len_t fnum;
				} res;

				res.fres = f_read(
				    &raw,
				    buf,
				    len,
				    &res.fnum
				);

				return res;
			}

			MOS_INLINE auto
			write(Buf_t src, Len_t len)
			{
				struct WriteRes_t
				{
					Res_t fres;
					Len_t fnum;
				} res;

				res.fres = f_write(
				    &raw,
				    src,
				    len,
				    &res.fnum
				);

				return res;
			}

			MOS_INLINE auto
			append(Buf_t src, Len_t len)
			{
				struct WriteRes_t
				{
					Res_t fres;
					Len_t fnum;
				} res;

				res.fres = f_lseek(&raw, f_size(&raw));

				if (res.fres != FR_OK) {
					return res;
				}

				res.fres = f_write(&raw, src, len, &res.fnum);
				return res;
			}
		};

		using File_t    = FatFs::File_t;
		using RawFile_t = File_t::Raw_t;
	};

	using File_t    = FatFs::File_t;
	using RawFile_t = FatFs::RawFile_t;

	void init(FatFs& fs)
	{
		static FIL test_file_raw; /* 文件裸对象 */

		auto mnt_or_fmt = [&] {
			// 挂载文件系统，文件系统挂载时会对 SPI-SD 设备初始化 -> 调用 sd.init()
			auto res = fs.mount("0:", 1); /* 文件操作结果 */

			/*----------------------- 格式化测试 ---------------------------*/
			/* 如果没有文件系统就格式化创建创建文件系统 */
			if (res == FR_NO_FILESYSTEM) {
				MOS_MSG("No FatFs, fmt...");
				res = fs.mkfs("0:"); /* 格式化 */

				if (res == FR_OK) {
					MOS_MSG("FatFs fmt success!");
					res = fs.mount("0:", 1); /* 格式化后，先取消挂载 */
					res = fs.mount("0:", 1); /* 重新挂载 */
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
				return;
			}
			else {
				MOS_MSG("FatFs mnt!");
			}
		};

		auto w_test = [&] {
			static BYTE w_buf[] = "hello, world!"; /* 写缓冲区*/

			/*--------------------- 文件系统测试：写测试 -----------------------*/
			File_t file {test_file_raw}; /* 创建文件对象 */
			auto res = file.open(        // 打开文件，如果文件不存在则创建它
			    "0:rw_test.txt",
			    File_t::OpenMode::Write
			);

			if (res == FR_OK) {
				MOS_MSG("File open success!");
				auto [res, num] = file.write(w_buf, sizeof(w_buf)); /* 将指定存储区内容写入到文件内 */

				/* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如需写入大量数据使用f_write_co替代上面f_write即可 */
				// res_sd=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
				if (res == FR_OK)
					MOS_MSG("W(%d) <- \"%s\"", num, w_buf);
				else
					MOS_MSG("Bad:(%d)", res);
			}
			else {
				MOS_MSG("File open failed!");
			}
		};

		auto r_test = [&] {
			static BYTE r_buf[64] = {0}; /* 读缓冲区 */

			/*------------------ 文件系统测试：读测试 --------------------------*/
			File_t file {test_file_raw}; /* 创建文件对象 */
			auto res = file.open(        // 打开文件，如果文件不存在则创建它
			    "0:rw_test.txt",
			    File_t::OpenMode::Read
			);

			if (res == FR_OK) {
				MOS_MSG("File open success!");

				auto [res, num] = file.read(r_buf, sizeof(r_buf));

				// 实测 SPI_SD 驱动下读取大于 512 字节的数据在 SD 卡里打开会显示乱码
				// 如需读取大量数据使用 f_read_co 替代上面 f_read 即可
				// res_sd = f_read_co(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
				if (res == FR_OK) {
					MOS_MSG("R(%d) -> \"%s\"", num, r_buf);
				}
				else {
					MOS_MSG("Bad:(%d)", res);
				}
			}
			else {
				MOS_MSG("File open failed!");
			}
		};

		kprintf("\n----------< FatFs R/W Test >---------\n");
		mnt_or_fmt();
		w_test();
		r_test();
		kprintf("-------------------------------------\n\n");

		// fs.umount();
	}
}

#endif