#ifndef _DEVICE_SD_
#define _DEVICE_SD_

#include "../stm32f4xx/spi.hpp"

namespace Driver::Device
{
	using HAL::STM32F4xx::SPI_t;
	using HAL::STM32F4xx::GPIO_t;

	struct SD_t
	{
		enum Error
		{
			// SD reponses and error flags
			RESPONSE_NO_ERROR    = (0x00),
			IN_IDLE_STATE        = (0x01),
			ERASE_RESET          = (0x02),
			ILLEGAL_COMMAND      = (0x04),
			COM_CRC_ERROR        = (0x08),
			ERASE_SEQUENCE_ERROR = (0x10),
			ADDRESS_ERROR        = (0x20),
			PARAMETER_ERROR      = (0x40),
			RESPONSE_FAILURE     = (0xFF),

			// Data response error
			DATA_OK          = (0x05),
			DATA_CRC_ERROR   = (0x0B),
			DATA_WRITE_ERROR = (0x0D),
			DATA_OTHER_ERROR = (0xFF)
		};

		// Card Specific Data: CSD Register
		struct CSD
		{
			__IO uint8_t CSDStruct;           /* CSD structure */
			__IO uint8_t SysSpecVersion;      /* System specification version */
			__IO uint8_t Reserved1;           /* Reserved */
			__IO uint8_t TAAC;                /* Data read access-time 1 */
			__IO uint8_t NSAC;                /* Data read access-time 2 in CLK cycles */
			__IO uint8_t MaxBusClkFrec;       /* Max. bus clock frequency */
			__IO uint16_t CardComdClasses;    /* Card command classes */
			__IO uint8_t RdBlockLen;          /* Max. read data block length */
			__IO uint8_t PartBlockRead;       /* Partial blocks for read allowed */
			__IO uint8_t WrBlockMisalign;     /* Write block misalignment */
			__IO uint8_t RdBlockMisalign;     /* Read block misalignment */
			__IO uint8_t DSRImpl;             /* DSR implemented */
			__IO uint8_t Reserved2;           /* Reserved */
			__IO uint32_t DeviceSize;         /* Device Size */
			__IO uint8_t MaxRdCurrentVDDMin;  /* Max. read current @ VDD min */
			__IO uint8_t MaxRdCurrentVDDMax;  /* Max. read current @ VDD max */
			__IO uint8_t MaxWrCurrentVDDMin;  /* Max. write current @ VDD min */
			__IO uint8_t MaxWrCurrentVDDMax;  /* Max. write current @ VDD max */
			__IO uint8_t DeviceSizeMul;       /* Device size multiplier */
			__IO uint8_t EraseGrSize;         /* Erase group size */
			__IO uint8_t EraseGrMul;          /* Erase group size multiplier */
			__IO uint8_t WrProtectGrSize;     /* Write protect group size */
			__IO uint8_t WrProtectGrEnable;   /* Write protect group enable */
			__IO uint8_t ManDeflECC;          /* Manufacturer default ECC */
			__IO uint8_t WrSpeedFact;         /* Write speed factor */
			__IO uint8_t MaxWrBlockLen;       /* Max. write data block length */
			__IO uint8_t WriteBlockPaPartial; /* Partial blocks for write allowed */
			__IO uint8_t Reserved3;           /* Reserded */
			__IO uint8_t ContentProtectAppli; /* Content protection application */
			__IO uint8_t FileFormatGrouop;    /* File format group */
			__IO uint8_t CopyFlag;            /* Copy flag (OTP) */
			__IO uint8_t PermWrProtect;       /* Permanent write protection */
			__IO uint8_t TempWrProtect;       /* Temporary write protection */
			__IO uint8_t FileFormat;          /* File Format */
			__IO uint8_t ECC;                 /* ECC code */
			__IO uint8_t CSD_CRC;             /* CSD CRC */
			__IO uint8_t Reserved4;           /* always 1*/
		};

		// Card Identification Data: CID Register
		struct CID
		{
			__IO uint8_t ManufacturerID; /* ManufacturerID */
			__IO uint16_t OEM_AppliID;   /* OEM/Application ID */
			__IO uint32_t ProdName1;     /* Product Name part1 */
			__IO uint8_t ProdName2;      /* Product Name part2*/
			__IO uint8_t ProdRev;        /* Product Revision */
			__IO uint32_t ProdSN;        /* Product Serial Number */
			__IO uint8_t Reserved1;      /* Reserved1 */
			__IO uint16_t ManufactDate;  /* Manufacturing Date */
			__IO uint8_t CID_CRC;        /* CID CRC */
			__IO uint8_t Reserved2;      /* always 1 */
		};

		// SD Card information
		struct Info
		{
			CSD csd;
			CID cid;
			uint64_t capacity;
			uint32_t block_size;
		};

		static constexpr auto BLOCK_SIZE = 0x200;
		static constexpr auto DUMMY_BYTE = 0xFF;

		// Start Data Tokens:
		// Tokens (necessary because at nop/idle (and CS active) only 0xff is
		// on the data/command line)
		enum StartToken : uint8_t
		{
			START_DATA_SINGLE_BLOCK_READ    = 0xFE, /* Data token start byte, Start Single Block Read */
			START_DATA_MULTIPLE_BLOCK_READ  = 0xFE, /* Data token start byte, Start Multiple Block Read */
			START_DATA_SINGLE_BLOCK_WRITE   = 0xFE, /* Data token start byte, Start Single Block Write */
			START_DATA_MULTIPLE_BLOCK_WRITE = 0xFD, /* Data token start byte, Start Multiple Block Write */
			STOP_DATA_MULTIPLE_BLOCK_WRITE  = 0xFD, /* Data toke stop byte, Stop Multiple Block Write */
		};

		enum Cmd : uint8_t
		{
			GO_IDLE_STATE      = 0,  /* CMD0 = 0x40 */
			SEND_OP_COND       = 1,  /* CMD1 = 0x41 */
			SEND_IF_COND       = 8,  /* CMD8 = 0x48 */
			SEND_CSD           = 9,  /* CMD9 = 0x49 */
			SEND_CID           = 10, /* CMD10 = 0x4A */
			STOP_TRANSMISSION  = 12, /* CMD12 = 0x4C */
			SEND_STATUS        = 13, /* CMD13 = 0x4D */
			SET_BLOCKLEN       = 16, /* CMD16 = 0x50 */
			READ_SINGLE_BLOCK  = 17, /* CMD17 = 0x51 */
			READ_MULT_BLOCK    = 18, /* CMD18 = 0x52 */
			SET_BLOCK_COUNT    = 23, /* CMD23 = 0x57 */
			WRITE_SINGLE_BLOCK = 24, /* CMD24 = 0x58 */
			WRITE_MULT_BLOCK   = 25, /* CMD25 = 0x59 */
			PROG_CSD           = 27, /* CMD27 = 0x5B */
			SET_WRITE_PROT     = 28, /* CMD28 = 0x5C */
			CLR_WRITE_PROT     = 29, /* CMD29 = 0x5D */
			SEND_WRITE_PROT    = 30, /* CMD30 = 0x5E */
			SD_ERASE_GRP_START = 32, /* CMD32 = 0x60 */
			SD_ERASE_GRP_END   = 33, /* CMD33 = 0x61 */
			UNTAG_SECTOR       = 34, /* CMD34 = 0x62 */
			ERASE_GRP_START    = 35, /* CMD35 = 0x63 */
			ERASE_GRP_END      = 36, /* CMD36 = 0x64 */
			UNTAG_ERASE_GROUP  = 37, /* CMD37 = 0x65 */
			ERASE              = 38, /* CMD38 = 0x66 */
			READ_OCR           = 58, /* CMD58 */
			APP_CMD            = 55, /* CMD55 返回0x01*/
			SD_SEND_OP_COND    = 41, /* ACMD41  返回0x00*/
		};

		enum class Type
		{
			Err  = 0, // 非SD卡
			V1   = 1, // V1.0的卡
			V2   = 2, // SDSC
			V2HC = 4, // SDHC
		};

		struct PortPin_t
		{
			using Port_t    = GPIO_t&;
			using PortRaw_t = GPIO_t::Raw_t;
			using Pin_t     = GPIO_t::Pin_t;

			Port_t port;
			Pin_t pin;

			PortPin_t(PortRaw_t port, Pin_t pin)
			    : port(GPIO_t::convert(port)), pin(pin) {}

			void as_output() { port.as_output(pin); }
			void set_high() { port.set_bits(pin); }
			void set_low() { port.reset_bits(pin); }
		};

		SPI_t& spi;
		PortPin_t sclk; // SCLK
		PortPin_t mosi; // MOSI
		PortPin_t miso; // MISO
		PortPin_t cs;   // CS

		// Info & Type
		Type type = Type::Err;
		Info info;

		SD_t(
		    SPI_t::Raw_t spi, PortPin_t sclk,
		    PortPin_t miso, PortPin_t mosi, PortPin_t cs
		): spi(SPI_t::convert(spi)),
		   sclk(sclk),
		   miso(miso),
		   mosi(mosi),
		   cs(cs) {}

		uint8_t
		write_byte(uint8_t data)
		{
			while (spi.get_flag_status(SPI_FLAG_TXE) == RESET) {
				// Wait until the transmit buffer is empty
			}

			/* Send the byte */
			spi.send_data(data);

			while (spi.get_flag_status(SPI_FLAG_RXNE) == RESET) {
				// Wait to receive a byte
			}

			/*!< Return the byte read from the SPI bus */
			return spi.recv_data();
		}

		uint8_t read_byte()
		{
			while (spi.get_flag_status(SPI_FLAG_TXE) == RESET) {
				/* Wait until the transmit buffer is empty */
			}

			/* Send the byte */
			spi.send_data(DUMMY_BYTE);

			while (spi.get_flag_status(SPI_FLAG_RXNE) == RESET) {
				/* Wait until a data is received */
			}

			/*!< Return the shifted data */
			return (uint8_t) spi.recv_data();
		}

		void
		send_cmd(Cmd cmd, uint32_t arg, uint8_t crc)
		{
			uint8_t frame[] = {
			    (uint8_t) (cmd | 0x40), /* Byte 1 */
			    (uint8_t) (arg >> 24),  /* Byte 2 */
			    (uint8_t) (arg >> 16),  /* Byte 3 */
			    (uint8_t) (arg >> 8),   /* Byte 4 */
			    (uint8_t) (arg),        /* Byte 5 */
			    (crc),                  /* Byte 6 */
			};

			for (auto byte: frame) {
				write_byte(byte); /* Send the Cmd bytes */
			}
		}

		uint8_t get_data_resp(void)
		{
			uint8_t res;
			for (uint32_t i = 0; i <= 64; i++) {
				/*!< Read resonse and Mask unused bits */
				res      = read_byte() & 0x1F;
				auto err = Error::DATA_OTHER_ERROR;

				switch (res) {
					case Error::DATA_OK: {
						err = Error::DATA_OK;
						break;
					}
					case Error::DATA_CRC_ERROR:
						return Error::DATA_CRC_ERROR;
					case Error::DATA_WRITE_ERROR:
						return Error::DATA_WRITE_ERROR;
					default: {
						err = Error::DATA_OTHER_ERROR;
						break;
					}
				}
				/*!< Exit loop in case of data ok */
				if (err == Error::DATA_OK)
					break;
			}

			while (read_byte() == 0) {
				/*!< Wait null data */
			}

			/*!< Return response */
			return res;
		}

		Error get_resp(uint8_t Response)
		{
			uint32_t Count = 0xFFF;

			/*!< Check if response is got or a timeout is happen */
			while ((read_byte() != Response) && Count) {
				Count--;
			}
			if (Count == 0) {
				/*!< After time out */
				return Error::RESPONSE_FAILURE;
			}
			else {
				/*!< Right response got */
				return Error::RESPONSE_NO_ERROR;
			}
		}

		uint16_t get_status(void)
		{
			uint16_t status = 0;

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD13 (SD_SEND_STATUS) to get SD status */
			send_cmd(Cmd::SEND_STATUS, 0, 0xFF);

			status = read_byte();
			status |= (uint16_t) (read_byte() << 8);

			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte 0xFF */
			write_byte(DUMMY_BYTE);

			return status;
		}

		Error get_type(void)
		{
			uint32_t Count = 0xFFF;

			uint8_t R7R3_Resp[4];
			uint8_t R1_Resp;

			cs.set_high();

			/*!< Send Dummy byte 0xFF */
			write_byte(DUMMY_BYTE);

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD8 */
			send_cmd(Cmd::SEND_IF_COND, 0x1AA, 0x87);

			/*!< Check if response is got or a timeout is happen */
			while (((R1_Resp = read_byte()) == 0xFF) && Count) {
				Count--;
			}
			if (Count == 0) {
				/*!< After time out */
				return Error::RESPONSE_FAILURE;
			}

			//响应 = 0x05   非V2.0的卡
			if (R1_Resp == (Error::IN_IDLE_STATE | Error::ILLEGAL_COMMAND)) {
				/*----------Activates the card initialization process-----------*/
				do {
					/*!< SD chip select high */
					cs.set_high();

					/*!< Send Dummy byte 0xFF */
					write_byte(DUMMY_BYTE);

					/*!< SD chip select low */
					cs.set_low();

					/*!< 发送CMD1完成V1 版本卡的初始化 */
					send_cmd(Cmd::SEND_OP_COND, 0, 0xFF);
					/*!< Wait for no error Response (R1 Format) equal to 0x00 */
				} while (get_resp(Error::RESPONSE_NO_ERROR));

				//V1版本的卡完成初始化

				type = Type::V1;
				//初始化正常
			}
			//响应 0x01 -> V2.0的卡
			else if (R1_Resp == Error::IN_IDLE_STATE) {
				/*!< 读取CMD8 的R7响应 */
				for (uint32_t i = 0; i < 4; i++) {
					R7R3_Resp[i] = read_byte();
				}

				/*!< SD chip select high */
				cs.set_high();

				/*!< Send Dummy byte 0xFF */
				write_byte(DUMMY_BYTE);

				/*!< SD chip select low */
				cs.set_low();

				//判断该卡是否支持2.7-3.6V电压
				if (R7R3_Resp[2] == 0x01 && R7R3_Resp[3] == 0xAA) {
					//支持电压范围，可以操作
					Count = 200;
					//发卡初始化指令CMD55+ACMD41
					do {
						//CMD55，以强调下面的是ACMD命令
						send_cmd(Cmd::APP_CMD, 0, 0xFF);
						if (!get_resp(Error::RESPONSE_NO_ERROR)) // SD_IN_IDLE_STATE
							return Error::RESPONSE_FAILURE;      //超时返回

						//ACMD41命令带HCS检查位
						send_cmd(Cmd::SD_SEND_OP_COND, 0x40000000, 0xFF);

						if (Count-- == 0)
							return Error::RESPONSE_FAILURE; //重试次数超时
					} while (get_resp(Error::RESPONSE_NO_ERROR));

					//初始化指令完成，读取OCR信息，CMD58

					//-----------鉴别SDSC SDHC卡类型开始-----------

					Count = 200;
					do {
						/*!< SD chip select high */
						cs.set_high();

						/*!< Send Dummy byte 0xFF */
						write_byte(DUMMY_BYTE);

						/*!< SD chip select low */
						cs.set_low();

						/*!< 发送CMD58 读取OCR寄存器 */
						send_cmd(Cmd::READ_OCR, 0, 0xFF);
					} while (get_resp(Error::RESPONSE_NO_ERROR) || Count-- == 0);

					if (Count == 0) {
						return Error::RESPONSE_FAILURE; //重试次数超时
					}

					//响应正常，读取R3响应

					/*!< 读取CMD58的R3响应 */
					for (uint32_t i = 0; i < 4; i++) {
						R7R3_Resp[i] = read_byte();
					}

					//检查接收到OCR中的bit30(CCS)
					//CCS = 0:SDSC / CCS = 1:SDHC
					if (R7R3_Resp[0] & 0x40) { //检查CCS标志
						type = Type::V2HC;
					}
					else {
						type = Type::V2;
					}
				}
				//-----------鉴别SDSC SDHC版本卡的流程结束-----------
			}

			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			//初始化正常返回
			return Error::RESPONSE_NO_ERROR;
		}

		Error go_idle(void)
		{
			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
			send_cmd(Cmd::GO_IDLE_STATE, 0, 0x95);

			/*!< Wait for In Idle State Response (R1 Format) equal to 0x01 */
			if (get_resp(Error::IN_IDLE_STATE)) {
				/*!< No Idle State Response: return response failue */
				return Error::RESPONSE_FAILURE;
			}

			cs.set_high();

			/*!< Send Dummy byte 0xFF */
			write_byte(DUMMY_BYTE);

			//正常返回
			return Error::RESPONSE_NO_ERROR;
		}

		Error
		read_block(
		    uint8_t* buf,
		    uint64_t r_addr,
		    uint16_t blc_sz
		)
		{
			Error err = RESPONSE_FAILURE;

			//SDHC卡块大小固定为512，且读命令中的地址的单位是sector
			if (type == Type::V2HC) {
				blc_sz = 512;
				r_addr /= 512;
			}

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
			send_cmd(READ_SINGLE_BLOCK, r_addr, 0xFF);

			/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
			if (!get_resp(RESPONSE_NO_ERROR)) {
				/*!< Now look for the data token to signify the start of the data */
				if (!get_resp(START_DATA_SINGLE_BLOCK_READ)) {
					/*!< Read the SD block data : read NumByteToRead data */
					for (uint32_t i = 0; i < blc_sz; i++) {
						/*!< Save the received data */
						*buf = read_byte();

						/*!< Point to the next location where the byte read will be saved */
						buf++;
					}
					/*!< Get CRC bytes (not really needed by us, but required by SD) */
					read_byte();
					read_byte();

					/*!< Set response value to success */
					err = RESPONSE_NO_ERROR;
				}
			}
			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Returns the reponse */
			return err;
		}

		Error
		read_multi_block(
		    uint8_t* buf,
		    uint64_t r_addr,
		    uint16_t blc_sz,
		    uint32_t num_of_blc
		)
		{
			uint32_t offset = 0;
			Error err       = RESPONSE_FAILURE;

			//SDHC卡块大小固定为512，且读命令中的地址的单位是sector
			if (type == Type::V2HC) {
				blc_sz = 512;
				r_addr /= 512;
			}

			/*!< SD chip select low */
			cs.set_low();

			/*!< Data transfer */
			while (num_of_blc--) {
				/*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
				send_cmd(READ_SINGLE_BLOCK, r_addr + offset, 0xFF);
				/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
				if (get_resp(RESPONSE_NO_ERROR)) {
					return RESPONSE_FAILURE;
				}
				/*!< Now look for the data token to signify the start of the data */
				if (!get_resp(START_DATA_SINGLE_BLOCK_READ)) {
					/*!< Read the SD block data : read NumByteToRead data */
					for (uint32_t i = 0; i < blc_sz; i++) {
						*buf = read_byte(); /*!< Read the pointed data */
						/*!< Point to the next location where the byte read will be saved */
						buf++;
					}

					offset += 512; /*!< Set next read address*/
					/*!< get CRC bytes (not really needed by us, but required by SD) */
					read_byte();
					read_byte();
					/*!< Set response value to success */
					err = RESPONSE_NO_ERROR;
				}
				else {
					/*!< Set response value to failure */
					err = RESPONSE_FAILURE;
				}

				/* 添加 Send dummy byte 防止读操作失败 */
				write_byte(DUMMY_BYTE);
			}
			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Returns the reponse */
			return err;
		}

		Error
		write_block(
		    uint8_t* buf,
		    uint64_t w_addr,
		    uint16_t blc_sz
		)
		{
			Error err = RESPONSE_FAILURE;

			//SDHC卡块大小固定为512，且写命令中的地址的单位是sector
			if (type == Type::V2HC) {
				blc_sz = 512;
				w_addr /= 512;
			}

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write multiple block */
			send_cmd(WRITE_SINGLE_BLOCK, w_addr, 0xFF);

			/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
			if (!get_resp(RESPONSE_NO_ERROR)) {
				/*!< Send a dummy byte */
				write_byte(DUMMY_BYTE);

				/*!< Send the data token to signify the start of the data */
				write_byte(0xFE);

				/*!< Write the block data to SD : write count data by block */
				for (uint32_t i = 0; i < blc_sz; i++) {
					/*!< Send the pointed byte */
					write_byte(*buf);
					/*!< Point to the next location where the byte read will be saved */
					buf++;
				}
				/*!< Put CRC bytes (not really needed by us, but required by SD) */
				read_byte();
				read_byte();

				/*!< Read data response */
				if (get_data_resp() == DATA_OK) {
					err = RESPONSE_NO_ERROR;
				}
			}
			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Returns the reponse */
			return err;
		}

		Error
		write_multi_block(
		    uint8_t* buf,
		    uint64_t w_addr,
		    uint16_t blc_sz,
		    uint32_t num_of_blc
		)
		{
			uint32_t offset = 0;
			Error err       = RESPONSE_FAILURE;

			//SDHC卡块大小固定为512，且写命令中的地址的单位是sector
			if (type == Type::V2HC) {
				blc_sz = 512;
				w_addr /= 512;
			}

			/*!< SD chip select low */
			cs.set_low();

			/*!< Data transfer */
			while (num_of_blc--) {
				/*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks */
				send_cmd(WRITE_SINGLE_BLOCK, w_addr + offset, 0xFF);
				/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
				if (get_resp(RESPONSE_NO_ERROR)) {
					return RESPONSE_FAILURE;
				}

				/*!< Send dummy byte */
				write_byte(DUMMY_BYTE);

				/*!< Send the data token to signify the start of the data */
				write_byte(START_DATA_SINGLE_BLOCK_WRITE);

				/*!< Write the block data to SD : write count data by block */
				for (uint32_t i = 0; i < blc_sz; i++) {
					/*!< Send the pointed byte */
					write_byte(*buf);
					/*!< Point to the next location where the byte read will be saved */
					buf++;
				}

				/*!< Set next write address */
				offset += 512;

				/*!< Put CRC bytes (not really needed by us, but required by SD) */
				read_byte();
				read_byte();

				/*!< Read data response */
				if (get_data_resp() == DATA_OK) {
					/*!< Set response value to success */
					err = RESPONSE_NO_ERROR;
				}
				else {
					/*!< Set response value to failure */
					err = RESPONSE_FAILURE;
				}
			}
			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Returns the reponse */
			return err;
		}

		Error get_csd()
		{
			Error err = Error::RESPONSE_FAILURE;
			uint8_t CSD_Tab[16];

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD9 (CSD register) or CMD10(CSD register) */
			send_cmd(Cmd::SEND_CSD, 0, 0xFF);

			/*!< Wait for response in the R1 format (0x00 is no errors) */
			if (!get_resp(Error::RESPONSE_NO_ERROR)) {
				if (!get_resp(StartToken::START_DATA_SINGLE_BLOCK_READ)) {
					for (uint32_t i = 0; i < 16; i++) {
						/*!< Store CSD register value on CSD_Tab */
						CSD_Tab[i] = read_byte();
					}
				}
				/*!< Get CRC bytes (not really needed by us, but required by SD) */
				write_byte(DUMMY_BYTE);
				write_byte(DUMMY_BYTE);
				/*!< Set response value to success */
				err = Error::RESPONSE_NO_ERROR;
			}
			/*!< SD chip select high */
			cs.set_high();
			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Byte 0 */
			info.csd.CSDStruct      = (CSD_Tab[0] & 0xC0) >> 6;
			info.csd.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
			info.csd.Reserved1      = CSD_Tab[0] & 0x03;

			/*!< Byte 1 */
			info.csd.TAAC = CSD_Tab[1];

			/*!< Byte 2 */
			info.csd.NSAC = CSD_Tab[2];

			/*!< Byte 3 */
			info.csd.MaxBusClkFrec = CSD_Tab[3];

			/*!< Byte 4 */
			info.csd.CardComdClasses = CSD_Tab[4] << 4;

			/*!< Byte 5 */
			info.csd.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
			info.csd.RdBlockLen = CSD_Tab[5] & 0x0F;

			/*!< Byte 6 */
			info.csd.PartBlockRead   = (CSD_Tab[6] & 0x80) >> 7;
			info.csd.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
			info.csd.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
			info.csd.DSRImpl         = (CSD_Tab[6] & 0x10) >> 4;
			info.csd.Reserved2       = 0; /*!< Reserved */

			info.csd.DeviceSize = (CSD_Tab[6] & 0x03) << 10;

			//V1卡与SDSC卡的信息
			if ((type == Type::V1) || (type == Type::V2)) {
				/*!< Byte 7 */
				info.csd.DeviceSize |= (CSD_Tab[7]) << 2;

				/*!< Byte 8 */
				info.csd.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

				info.csd.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
				info.csd.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

				/*!< Byte 9 */
				info.csd.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
				info.csd.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
				info.csd.DeviceSizeMul      = (CSD_Tab[9] & 0x03) << 1;
				/*!< Byte 10 */
				info.csd.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
			}
			//SDHC卡的信息
			else if (type == Type::V2HC) {
				info.csd.DeviceSize = (CSD_Tab[7] & 0x3F) << 16;

				info.csd.DeviceSize |= (CSD_Tab[8] << 8);

				info.csd.DeviceSize |= (CSD_Tab[9]);
			}

			info.csd.EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
			info.csd.EraseGrMul  = (CSD_Tab[10] & 0x3F) << 1;

			/*!< Byte 11 */
			info.csd.EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
			info.csd.WrProtectGrSize = (CSD_Tab[11] & 0x7F);

			/*!< Byte 12 */
			info.csd.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
			info.csd.ManDeflECC        = (CSD_Tab[12] & 0x60) >> 5;
			info.csd.WrSpeedFact       = (CSD_Tab[12] & 0x1C) >> 2;
			info.csd.MaxWrBlockLen     = (CSD_Tab[12] & 0x03) << 2;

			/*!< Byte 13 */
			info.csd.MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
			info.csd.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
			info.csd.Reserved3           = 0;
			info.csd.ContentProtectAppli = (CSD_Tab[13] & 0x01);

			/*!< Byte 14 */
			info.csd.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
			info.csd.CopyFlag         = (CSD_Tab[14] & 0x40) >> 6;
			info.csd.PermWrProtect    = (CSD_Tab[14] & 0x20) >> 5;
			info.csd.TempWrProtect    = (CSD_Tab[14] & 0x10) >> 4;
			info.csd.FileFormat       = (CSD_Tab[14] & 0x0C) >> 2;
			info.csd.ECC              = (CSD_Tab[14] & 0x03);

			/*!< Byte 15 */
			info.csd.CSD_CRC   = (CSD_Tab[15] & 0xFE) >> 1;
			info.csd.Reserved4 = 1;

			/*!< Return the reponse */
			return err;
		}

		Error get_cid()
		{
			Error err = Error::RESPONSE_FAILURE;
			uint8_t CID_Tab[16];

			/*!< SD chip select low */
			cs.set_low();

			/*!< Send CMD10 (CID register) */
			send_cmd(Cmd::SEND_CID, 0, 0xFF);

			/*!< Wait for response in the R1 format (0x00 is no errors) */
			if (!get_resp(Error::RESPONSE_NO_ERROR))
			{
				if (!get_resp(StartToken::START_DATA_SINGLE_BLOCK_READ))
				{
					/*!< Store CID register value on CID_Tab */
					for (uint32_t i = 0; i < 16; i++) {
						CID_Tab[i] = read_byte();
					}
				}
				/*!< Get CRC bytes (not really needed by us, but required by SD) */
				write_byte(DUMMY_BYTE);
				write_byte(DUMMY_BYTE);
				/*!< Set response value to success */
				err = Error::RESPONSE_NO_ERROR;
			}
			/*!< SD chip select high */
			cs.set_high();
			/*!< Send dummy byte: 8 Clock pulses of delay */
			write_byte(DUMMY_BYTE);

			/*!< Byte 0 */
			info.cid.ManufacturerID = CID_Tab[0];

			/*!< Byte 1 */
			info.cid.OEM_AppliID = CID_Tab[1] << 8;

			/*!< Byte 2 */
			info.cid.OEM_AppliID |= CID_Tab[2];

			/*!< Byte 3 */
			info.cid.ProdName1 = CID_Tab[3] << 24;

			/*!< Byte 4 */
			info.cid.ProdName1 |= CID_Tab[4] << 16;

			/*!< Byte 5 */
			info.cid.ProdName1 |= CID_Tab[5] << 8;

			/*!< Byte 6 */
			info.cid.ProdName1 |= CID_Tab[6];

			/*!< Byte 7 */
			info.cid.ProdName2 = CID_Tab[7];

			/*!< Byte 8 */
			info.cid.ProdRev = CID_Tab[8];

			/*!< Byte 9 */
			info.cid.ProdSN = CID_Tab[9] << 24;

			/*!< Byte 10 */
			info.cid.ProdSN |= CID_Tab[10] << 16;

			/*!< Byte 11 */
			info.cid.ProdSN |= CID_Tab[11] << 8;

			/*!< Byte 12 */
			info.cid.ProdSN |= CID_Tab[12];

			/*!< Byte 13 */
			info.cid.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
			info.cid.ManufactDate = (CID_Tab[13] & 0x0F) << 8;

			/*!< Byte 14 */
			info.cid.ManufactDate |= CID_Tab[14];

			/*!< Byte 15 */
			info.cid.CID_CRC   = (CID_Tab[15] & 0xFE) >> 1;
			info.cid.Reserved2 = 1;

			/*!< Return the reponse */
			return err;
		}

		Error get_info()
		{
			Error status = Error::RESPONSE_FAILURE;

			status = get_csd();
			status = get_cid();

			if ((type == Type::V1) || (type == Type::V2)) {
				info.capacity = (info.csd.DeviceSize + 1);
				info.capacity *= (1 << (info.csd.DeviceSizeMul + 2));
				info.block_size = 1 << (info.csd.RdBlockLen);
				info.capacity *= info.block_size;
			}
			else if (type == Type::V2HC) {
				info.capacity   = (uint64_t) (info.csd.DeviceSize + 1) * 512 * 1024;
				info.block_size = 512;
			}

			/*!< Returns the reponse */
			return status;
		}

		Error init()
		{
			spi.init({
			    .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
			    .SPI_Mode              = SPI_Mode_Master,
			    .SPI_DataSize          = SPI_DataSize_8b,
			    .SPI_CPOL              = SPI_CPOL_High,
			    .SPI_CPHA              = SPI_CPHA_2Edge,
			    .SPI_NSS               = SPI_NSS_Soft,
			    .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
			    .SPI_FirstBit          = SPI_FirstBit_MSB,
			    .SPI_CRCPolynomial     = 7,
			});

			spi.enable();
			cs.as_output();

			/*!< SD chip select high */
			cs.set_high();

			/*!< Send dummy byte 0xFF, 10 times with CS high */
			/*!< Rise CS and MOSI for 80 clocks cycles */
			for (uint32_t i = 0; i < 10; i++) {
				/*!< Send dummy byte 0xFF */
				write_byte(DUMMY_BYTE);
			}

			// Retry for 10 times
			for (uint32_t i = 0;
			     type == Type::Err && i < 10;
			     i++) {
				go_idle();
				get_type();
			}

			//不支持的卡
			if (type == Type::Err) {
				return Error::RESPONSE_FAILURE;
			}

			return get_info();
		}
	};
}

#endif
