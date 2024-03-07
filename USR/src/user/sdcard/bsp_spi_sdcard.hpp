/**
  ******************************************************************************
  * @file    stm32_eval_spi_sd.c
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file provides a set of functions needed to manage the SPI SD 
  *          Card memory mounted on STM32xx-EVAL board (refer to stm32_eval.h
  *          to know about the boards supporting this memory). 
  *          It implements a high level communication layer for read and write 
  *          from/to this memory. The needed STM32 hardware resources (SPI and 
  *          GPIO) are defined in stm32xx_eval.h file, and the initialization is 
  *          performed in SD_LowLevel_Init() function declared in stm32xx_eval.c 
  *          file.
  *          You can easily tailor this driver to any other development board, 
  *          by just adapting the defines for hardware resources and 
  *          SD_LowLevel_Init() function.
  *            
  *          +-------------------------------------------------------+
  *          |                     Pin assignment                    |
  *          +-------------------------+---------------+-------------+
  *          |  STM32 SPI Pins         |     SD        |    Pin      |
  *          +-------------------------+---------------+-------------+
  *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
  *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
  *          |                         |   GND         |    3 (0 V)  |
  *          |                         |   VDD         |    4 (3.3 V)|
  *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
  *          |                         |   GND         |    6 (0 V)  |
  *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
  *          +-------------------------+---------------+-------------+
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_spi_sdcard.h"

//��¼��������
uint8_t SD_Type = SD_TYPE_NOT_SD; //�洢��������
SD_CardInfo SDCardInfo;           //���ڴ洢������Ϣ

/**
  * @brief  DeInitializes the SD/SD communication.
  * @param  None
  * @retval None
  */
void SD_DeInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	SPI_Cmd(SD_SPI, DISABLE); /*!< SD_SPI disable */
	SPI_I2S_DeInit(SD_SPI);   /*!< DeInitializes the SD_SPI */

	/*!< SD_SPI Periph clock disable */
	RCC_APB1PeriphClockCmd(SD_SPI_CLK, DISABLE);
	/*!< DeRemap SPI3 Pins */
	//  GPIO_PinRemapConfig(GPIO_Remap_SPI3, DISABLE);

	/*!< Configure SD_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin  = SD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MISO_PIN;
	GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
	GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI_CS_PIN pin: SD Card CS pin */
	GPIO_InitStructure.GPIO_Pin = SD_CS_PIN;
	GPIO_Init(SD_CS_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI_DETECT_PIN pin: SD Card detect pin */
	//  GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN;
	//  GPIO_Init(SD_DETECT_GPIO_PORT, &GPIO_InitStructure);
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	/*!< SD_SPI_CS_GPIO, SD_SPI_MOSI_GPIO, SD_SPI_MISO_GPIO, SD_SPI_DETECT_GPIO 
       and SD_SPI_SCK_GPIO Periph clock enable */

	/*!< SD_SPI Periph clock enable */
	/*!< AFIO Periph clock enable */
	//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//  /*!< Remap SPI3 Pins */
	//  GPIO_PinRemapConfig(GPIO_Remap_SPI3,ENABLE);

	/*!< Configure SD_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin   = SD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
	GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin  = SD_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI_CS_PIN pin: SD Card CS pin */
	GPIO_InitStructure.GPIO_Pin   = SD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(SD_CS_GPIO_PORT, &GPIO_InitStructure);

	/*!< SD_SPI Config */
	SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial     = 7;
	SPI_Init(SD_SPI, &SPI_InitStructure);

	SPI_Cmd(SD_SPI, ENABLE); /*!< SD_SPI enable */
}
/**
  * @brief  Initializes the SD/SD communication.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI4, ENABLE);
	RCC_AHB1PeriphClockCmd(
	    RCC_AHB1Periph_GPIOG |
	        RCC_AHB1Periph_GPIOG,
	    ENABLE
	);

	/*!< Initialize SD_SPI */
	GPIO_Configuration();

	/*!< SD chip select high */
	SD_CS_HIGH();

	volatile uint32_t i = 0;
	/*!< Send dummy byte 0xFF, 10 times with CS high */
	/*!< Rise CS and MOSI for 80 clocks cycles */
	for (i = 0; i < 100; i++)
	{
		/*!< Send dummy byte 0xFF */
		SD_WriteByte(SD_DUMMY_BYTE);
	}

	//��ȡ��������,��ೢ��10��
	i = 0;
	do
	{
		/*------------Put SD in SPI mode--------------*/
		/*!< SD initialized and set to SPI mode properly */
		SD_GoIdleState();

		/*Get card type*/
		SD_GetCardType();

	} while (SD_Type == SD_TYPE_NOT_SD && i++ > 1000);

	//��֧�ֵĿ�
	if (SD_Type == SD_TYPE_NOT_SD)
		return SD_RESPONSE_FAILURE;

	return SD_GetCardInfo(&SDCardInfo);
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
uint8_t SD_Detect(void)
{
	__IO uint8_t status = SD_PRESENT;

	/*!< Check GPIO to detect SD */
	//  if (GPIO_ReadInputData(SD_DETECT_GPIO_PORT) & SD_DETECT_PIN)
	//  {
	//    status = SD_NOT_PRESENT;
	//  }
	return status;
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD 
  *         card information.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCardInfo(SD_CardInfo* cardinfo)
{
	SD_Error status = SD_RESPONSE_FAILURE;

	status = SD_GetCSDRegister(&(cardinfo->SD_csd));
	status = SD_GetCIDRegister(&(cardinfo->SD_cid));

	if ((SD_Type == SD_TYPE_V1) || (SD_Type == SD_TYPE_V2))
	{

		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1);
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;
	}
	else if (SD_Type == SD_TYPE_V2HC)
	{
		cardinfo->CardCapacity  = (uint64_t) (cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
		cardinfo->CardBlockSize = 512;
	}

	/*!< Returns the reponse */
	return status;
}

/**
  * @brief  Reads a block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the 
  *                  SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint64_t ReadAddr, uint16_t BlockSize)
{
	uint32_t i      = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;

	//SDHC�����С�̶�Ϊ512���Ҷ������еĵ�ַ�ĵ�λ��sector
	if (SD_Type == SD_TYPE_V2HC)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
	SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

	/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
	if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	{
		/*!< Now look for the data token to signify the start of the data */
		if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
		{
			/*!< Read the SD block data : read NumByteToRead data */
			for (i = 0; i < BlockSize; i++)
			{
				/*!< Save the received data */
				*pBuffer = SD_ReadByte();

				/*!< Point to the next location where the byte read will be saved */
				pBuffer++;
			}
			/*!< Get CRC bytes (not really needed by us, but required by SD) */
			SD_ReadByte();
			SD_ReadByte();
			/*!< Set response value to success */
			rvalue = SD_RESPONSE_NO_ERROR;
		}
	}
	/*!< SD chip select high */
	SD_CS_HIGH();

	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);

	/*!< Returns the reponse */
	return rvalue;
}

/**
  * @brief  Reads multiple block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the 
  *                  SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadMultiBlocks(uint8_t* pBuffer, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	uint32_t i = 0, Offset = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;

	//SDHC�����С�̶�Ϊ512���Ҷ������еĵ�ַ�ĵ�λ��sector
	if (SD_Type == SD_TYPE_V2HC)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	/*!< SD chip select low */
	SD_CS_LOW();
	/*!< Data transfer */
	while (NumberOfBlocks--)
	{
		/*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
		SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
		/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
		if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
		{
			return SD_RESPONSE_FAILURE;
		}
		/*!< Now look for the data token to signify the start of the data */
		if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
		{
			/*!< Read the SD block data : read NumByteToRead data */
			for (i = 0; i < BlockSize; i++)
			{
				/*!< Read the pointed data */
				*pBuffer = SD_ReadByte();
				/*!< Point to the next location where the byte read will be saved */
				pBuffer++;
			}
			/*!< Set next read address*/
			Offset += 512;
			/*!< get CRC bytes (not really needed by us, but required by SD) */
			SD_ReadByte();
			SD_ReadByte();
			/*!< Set response value to success */
			rvalue = SD_RESPONSE_NO_ERROR;
		}
		else
		{
			/*!< Set response value to failure */
			rvalue = SD_RESPONSE_FAILURE;
		}

		/* ��� Send dummy byte ��ֹ������ʧ�� */
		SD_WriteByte(SD_DUMMY_BYTE);
	}
	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);
	/*!< Returns the reponse */
	return rvalue;
}

/**
  * @brief  Writes a block on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on 
  *                  the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint64_t WriteAddr, uint16_t BlockSize)
{
	uint32_t i      = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;

	//SDHC�����С�̶�Ϊ512����д�����еĵ�ַ�ĵ�λ��sector
	if (SD_Type == SD_TYPE_V2HC)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write multiple block */
	SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);

	/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
	if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	{
		/*!< Send a dummy byte */
		SD_WriteByte(SD_DUMMY_BYTE);

		/*!< Send the data token to signify the start of the data */
		SD_WriteByte(0xFE);

		/*!< Write the block data to SD : write count data by block */
		for (i = 0; i < BlockSize; i++)
		{
			/*!< Send the pointed byte */
			SD_WriteByte(*pBuffer);
			/*!< Point to the next location where the byte read will be saved */
			pBuffer++;
		}
		/*!< Put CRC bytes (not really needed by us, but required by SD) */
		SD_ReadByte();
		SD_ReadByte();

		/*!< Read data response */
		if (SD_GetDataResponse() == SD_DATA_OK)
		{
			rvalue = SD_RESPONSE_NO_ERROR;
		}
	}
	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);

	/*!< Returns the reponse */
	return rvalue;
}

/**
  * @brief  Writes many blocks on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on 
  *                  the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteMultiBlocks(uint8_t* pBuffer, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	uint32_t i = 0, Offset = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;

	//SDHC�����С�̶�Ϊ512����д�����еĵ�ַ�ĵ�λ��sector
	if (SD_Type == SD_TYPE_V2HC)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/*!< SD chip select low */
	SD_CS_LOW();
	/*!< Data transfer */
	while (NumberOfBlocks--)
	{
		/*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks */
		SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr + Offset, 0xFF);
		/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
		if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
		{
			return SD_RESPONSE_FAILURE;
		}
		/*!< Send dummy byte */
		SD_WriteByte(SD_DUMMY_BYTE);
		/*!< Send the data token to signify the start of the data */
		SD_WriteByte(SD_START_DATA_SINGLE_BLOCK_WRITE);
		/*!< Write the block data to SD : write count data by block */
		for (i = 0; i < BlockSize; i++)
		{
			/*!< Send the pointed byte */
			SD_WriteByte(*pBuffer);
			/*!< Point to the next location where the byte read will be saved */
			pBuffer++;
		}
		/*!< Set next write address */
		Offset += 512;
		/*!< Put CRC bytes (not really needed by us, but required by SD) */
		SD_ReadByte();
		SD_ReadByte();
		/*!< Read data response */
		if (SD_GetDataResponse() == SD_DATA_OK)
		{
			/*!< Set response value to success */
			rvalue = SD_RESPONSE_NO_ERROR;
		}
		else
		{
			/*!< Set response value to failure */
			rvalue = SD_RESPONSE_FAILURE;
		}
	}
	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);
	/*!< Returns the reponse */
	return rvalue;
}

/**
  * @brief  Read the CSD card register.
  *         Reading the contents of the CSD register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_csd: pointer on an SCD register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCSDRegister(SD_CSD* SD_csd)
{
	uint32_t i      = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;
	uint8_t CSD_Tab[16];

	/*!< SD chip select low */
	SD_CS_LOW();
	/*!< Send CMD9 (CSD register) or CMD10(CSD register) */
	SD_SendCmd(SD_CMD_SEND_CSD, 0, 0xFF);
	/*!< Wait for response in the R1 format (0x00 is no errors) */
	if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	{
		if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
		{
			for (i = 0; i < 16; i++)
			{
				/*!< Store CSD register value on CSD_Tab */
				CSD_Tab[i] = SD_ReadByte();
			}
		}
		/*!< Get CRC bytes (not really needed by us, but required by SD) */
		SD_WriteByte(SD_DUMMY_BYTE);
		SD_WriteByte(SD_DUMMY_BYTE);
		/*!< Set response value to success */
		rvalue = SD_RESPONSE_NO_ERROR;
	}
	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);

	/*!< Byte 0 */
	SD_csd->CSDStruct      = (CSD_Tab[0] & 0xC0) >> 6;
	SD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
	SD_csd->Reserved1      = CSD_Tab[0] & 0x03;

	/*!< Byte 1 */
	SD_csd->TAAC = CSD_Tab[1];

	/*!< Byte 2 */
	SD_csd->NSAC = CSD_Tab[2];

	/*!< Byte 3 */
	SD_csd->MaxBusClkFrec = CSD_Tab[3];

	/*!< Byte 4 */
	SD_csd->CardComdClasses = CSD_Tab[4] << 4;

	/*!< Byte 5 */
	SD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
	SD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;

	/*!< Byte 6 */
	SD_csd->PartBlockRead   = (CSD_Tab[6] & 0x80) >> 7;
	SD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
	SD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
	SD_csd->DSRImpl         = (CSD_Tab[6] & 0x10) >> 4;
	SD_csd->Reserved2       = 0; /*!< Reserved */

	SD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;

	//V1����SDSC������Ϣ
	if ((SD_Type == SD_TYPE_V1) || (SD_Type == SD_TYPE_V2))
	{
		/*!< Byte 7 */
		SD_csd->DeviceSize |= (CSD_Tab[7]) << 2;

		/*!< Byte 8 */
		SD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

		SD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
		SD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

		/*!< Byte 9 */
		SD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
		SD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
		SD_csd->DeviceSizeMul      = (CSD_Tab[9] & 0x03) << 1;
		/*!< Byte 10 */
		SD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
	}
	//SDHC������Ϣ
	else if (SD_Type == SD_TYPE_V2HC)
	{
		SD_csd->DeviceSize = (CSD_Tab[7] & 0x3F) << 16;

		SD_csd->DeviceSize |= (CSD_Tab[8] << 8);

		SD_csd->DeviceSize |= (CSD_Tab[9]);
	}

	SD_csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
	SD_csd->EraseGrMul  = (CSD_Tab[10] & 0x3F) << 1;

	/*!< Byte 11 */
	SD_csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
	SD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

	/*!< Byte 12 */
	SD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
	SD_csd->ManDeflECC        = (CSD_Tab[12] & 0x60) >> 5;
	SD_csd->WrSpeedFact       = (CSD_Tab[12] & 0x1C) >> 2;
	SD_csd->MaxWrBlockLen     = (CSD_Tab[12] & 0x03) << 2;

	/*!< Byte 13 */
	SD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
	SD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
	SD_csd->Reserved3           = 0;
	SD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

	/*!< Byte 14 */
	SD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
	SD_csd->CopyFlag         = (CSD_Tab[14] & 0x40) >> 6;
	SD_csd->PermWrProtect    = (CSD_Tab[14] & 0x20) >> 5;
	SD_csd->TempWrProtect    = (CSD_Tab[14] & 0x10) >> 4;
	SD_csd->FileFormat       = (CSD_Tab[14] & 0x0C) >> 2;
	SD_csd->ECC              = (CSD_Tab[14] & 0x03);

	/*!< Byte 15 */
	SD_csd->CSD_CRC   = (CSD_Tab[15] & 0xFE) >> 1;
	SD_csd->Reserved4 = 1;

	/*!< Return the reponse */
	return rvalue;
}

/**
  * @brief  Read the CID card register.
  *         Reading the contents of the CID register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_cid: pointer on an CID register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCIDRegister(SD_CID* SD_cid)
{
	uint32_t i      = 0;
	SD_Error rvalue = SD_RESPONSE_FAILURE;
	uint8_t CID_Tab[16];

	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD10 (CID register) */
	SD_SendCmd(SD_CMD_SEND_CID, 0, 0xFF);

	/*!< Wait for response in the R1 format (0x00 is no errors) */
	if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	{
		if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
		{
			/*!< Store CID register value on CID_Tab */
			for (i = 0; i < 16; i++)
			{
				CID_Tab[i] = SD_ReadByte();
			}
		}
		/*!< Get CRC bytes (not really needed by us, but required by SD) */
		SD_WriteByte(SD_DUMMY_BYTE);
		SD_WriteByte(SD_DUMMY_BYTE);
		/*!< Set response value to success */
		rvalue = SD_RESPONSE_NO_ERROR;
	}
	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);

	/*!< Byte 0 */
	SD_cid->ManufacturerID = CID_Tab[0];

	/*!< Byte 1 */
	SD_cid->OEM_AppliID = CID_Tab[1] << 8;

	/*!< Byte 2 */
	SD_cid->OEM_AppliID |= CID_Tab[2];

	/*!< Byte 3 */
	SD_cid->ProdName1 = CID_Tab[3] << 24;

	/*!< Byte 4 */
	SD_cid->ProdName1 |= CID_Tab[4] << 16;

	/*!< Byte 5 */
	SD_cid->ProdName1 |= CID_Tab[5] << 8;

	/*!< Byte 6 */
	SD_cid->ProdName1 |= CID_Tab[6];

	/*!< Byte 7 */
	SD_cid->ProdName2 = CID_Tab[7];

	/*!< Byte 8 */
	SD_cid->ProdRev = CID_Tab[8];

	/*!< Byte 9 */
	SD_cid->ProdSN = CID_Tab[9] << 24;

	/*!< Byte 10 */
	SD_cid->ProdSN |= CID_Tab[10] << 16;

	/*!< Byte 11 */
	SD_cid->ProdSN |= CID_Tab[11] << 8;

	/*!< Byte 12 */
	SD_cid->ProdSN |= CID_Tab[12];

	/*!< Byte 13 */
	SD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
	SD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

	/*!< Byte 14 */
	SD_cid->ManufactDate |= CID_Tab[14];

	/*!< Byte 15 */
	SD_cid->CID_CRC   = (CID_Tab[15] & 0xFE) >> 1;
	SD_cid->Reserved2 = 1;

	/*!< Return the reponse */
	return rvalue;
}

/**
  * @brief  Send 5 bytes command to the SD card.
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @retval None
  */
void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
	uint32_t i = 0x00;

	uint8_t Frame[6];

	Frame[0] = (Cmd | 0x40); /*!< Construct byte 1 */

	Frame[1] = (uint8_t) (Arg >> 24); /*!< Construct byte 2 */

	Frame[2] = (uint8_t) (Arg >> 16); /*!< Construct byte 3 */

	Frame[3] = (uint8_t) (Arg >> 8); /*!< Construct byte 4 */

	Frame[4] = (uint8_t) (Arg); /*!< Construct byte 5 */

	Frame[5] = (Crc); /*!< Construct CRC: byte 6 */

	for (i = 0; i < 6; i++)
	{
		SD_WriteByte(Frame[i]); /*!< Send the Cmd bytes */
	}
}

/**
  * @brief  Get SD card data response.
  * @param  None
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
uint8_t SD_GetDataResponse(void)
{
	uint32_t i = 0;
	uint8_t response, rvalue;

	while (i <= 64)
	{
		/*!< Read resonse */
		response = SD_ReadByte();
		/*!< Mask unused bits */
		response &= 0x1F;
		switch (response)
		{
			case SD_DATA_OK: {
				rvalue = SD_DATA_OK;
				break;
			}
			case SD_DATA_CRC_ERROR:
				return SD_DATA_CRC_ERROR;
			case SD_DATA_WRITE_ERROR:
				return SD_DATA_WRITE_ERROR;
			default: {
				rvalue = SD_DATA_OTHER_ERROR;
				break;
			}
		}
		/*!< Exit loop in case of data ok */
		if (rvalue == SD_DATA_OK)
			break;
		/*!< Increment loop counter */
		i++;
	}

	/*!< Wait null data */
	while (SD_ReadByte() == 0)
		;

	/*!< Return response */
	return response;
}

/**
  * @brief  Returns the SD response.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetResponse(uint8_t Response)
{
	uint32_t Count = 0xFFF;

	/*!< Check if response is got or a timeout is happen */
	while ((SD_ReadByte() != Response) && Count)
	{
		Count--;
	}
	if (Count == 0)
	{
		/*!< After time out */
		return SD_RESPONSE_FAILURE;
	}
	else
	{
		/*!< Right response got */
		return SD_RESPONSE_NO_ERROR;
	}
}

/**
  * @brief  Returns the SD status.
  * @param  None
  * @retval The SD status.
  */
uint16_t SD_GetStatus(void)
{
	uint16_t Status = 0;

	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD13 (SD_SEND_STATUS) to get SD status */
	SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF);

	Status = SD_ReadByte();
	Status |= (uint16_t) (SD_ReadByte() << 8);

	/*!< SD chip select high */
	SD_CS_HIGH();

	/*!< Send dummy byte 0xFF */
	SD_WriteByte(SD_DUMMY_BYTE);

	return Status;
}

/**
  * @brief  ��ȡSD���İ汾���ͣ�������SDSC��SDHC
  * @param  ��
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCardType(void)
{
	uint32_t i     = 0;
	uint32_t Count = 0xFFF;

	uint8_t R7R3_Resp[4];
	uint8_t R1_Resp;

	SD_CS_HIGH();

	/*!< Send Dummy byte 0xFF */
	SD_WriteByte(SD_DUMMY_BYTE);

	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD8 */
	SD_SendCmd(SD_CMD_SEND_IF_COND, 0x1AA, 0x87);

	/*!< Check if response is got or a timeout is happen */
	while (((R1_Resp = SD_ReadByte()) == 0xFF) && Count)
	{
		Count--;
	}
	if (Count == 0)
	{
		/*!< After time out */
		return SD_RESPONSE_FAILURE;
	}

	//��Ӧ = 0x05   ��V2.0�Ŀ�
	if (R1_Resp == (SD_IN_IDLE_STATE | SD_ILLEGAL_COMMAND))
	{
		/*----------Activates the card initialization process-----------*/
		do
		{
			/*!< SD chip select high */
			SD_CS_HIGH();

			/*!< Send Dummy byte 0xFF */
			SD_WriteByte(SD_DUMMY_BYTE);

			/*!< SD chip select low */
			SD_CS_LOW();

			/*!< ����CMD1���V1 �汾���ĳ�ʼ�� */
			SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF);
			/*!< Wait for no error Response (R1 Format) equal to 0x00 */
		} while (SD_GetResponse(SD_RESPONSE_NO_ERROR));
		//V1�汾�Ŀ���ɳ�ʼ��

		SD_Type = SD_TYPE_V1;

		//������MMC��

		//��ʼ������
	}
	//��Ӧ 0x01   V2.0�Ŀ�
	else if (R1_Resp == SD_IN_IDLE_STATE)
	{
		/*!< ��ȡCMD8 ��R7��Ӧ */
		for (i = 0; i < 4; i++)
		{
			R7R3_Resp[i] = SD_ReadByte();
		}

		/*!< SD chip select high */
		SD_CS_HIGH();

		/*!< Send Dummy byte 0xFF */
		SD_WriteByte(SD_DUMMY_BYTE);

		/*!< SD chip select low */
		SD_CS_LOW();

		//�жϸÿ��Ƿ�֧��2.7-3.6V��ѹ
		if (R7R3_Resp[2] == 0x01 && R7R3_Resp[3] == 0xAA)
		{
			//֧�ֵ�ѹ��Χ�����Բ���
			Count = 200;
			//������ʼ��ָ��CMD55+ACMD41
			do
			{
				//CMD55����ǿ���������ACMD����
				SD_SendCmd(SD_CMD_APP_CMD, 0, 0xFF);
				if (!SD_GetResponse(SD_RESPONSE_NO_ERROR)) // SD_IN_IDLE_STATE
					return SD_RESPONSE_FAILURE;            //��ʱ����

				//ACMD41�����HCS���λ
				SD_SendCmd(SD_ACMD_SD_SEND_OP_COND, 0x40000000, 0xFF);

				if (Count-- == 0)
					return SD_RESPONSE_FAILURE; //���Դ�����ʱ
			} while (SD_GetResponse(SD_RESPONSE_NO_ERROR));

			//��ʼ��ָ����ɣ���ȡOCR��Ϣ��CMD58

			//-----------����SDSC SDHC�����Ϳ�ʼ-----------

			Count = 200;
			do
			{
				/*!< SD chip select high */
				SD_CS_HIGH();

				/*!< Send Dummy byte 0xFF */
				SD_WriteByte(SD_DUMMY_BYTE);

				/*!< SD chip select low */
				SD_CS_LOW();

				/*!< ����CMD58 ��ȡOCR�Ĵ��� */
				SD_SendCmd(SD_CMD_READ_OCR, 0, 0xFF);
			} while (SD_GetResponse(SD_RESPONSE_NO_ERROR) || Count-- == 0);

			if (Count == 0)
				return SD_RESPONSE_FAILURE; //���Դ�����ʱ

			//��Ӧ��������ȡR3��Ӧ

			/*!< ��ȡCMD58��R3��Ӧ */
			for (i = 0; i < 4; i++)
			{
				R7R3_Resp[i] = SD_ReadByte();
			}

			//�����յ�OCR�е�bit30(CCS)
			//CCS = 0:SDSC			 CCS = 1:SDHC
			if (R7R3_Resp[0] & 0x40) //���CCS��־
			{
				SD_Type = SD_TYPE_V2HC;
			}
			else
			{
				SD_Type = SD_TYPE_V2;
			}
			//-----------����SDSC SDHC�汾�������̽���-----------
		}
	}

	/*!< SD chip select high */
	SD_CS_HIGH();
	/*!< Send dummy byte: 8 Clock pulses of delay */
	SD_WriteByte(SD_DUMMY_BYTE);

	//��ʼ����������
	return SD_RESPONSE_NO_ERROR;
}

/**
  * @brief  Put SD in Idle state.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GoIdleState(void)
{
	/*!< SD chip select low */
	SD_CS_LOW();

	/*!< Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
	SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);

	/*!< Wait for In Idle State Response (R1 Format) equal to 0x01 */
	if (SD_GetResponse(SD_IN_IDLE_STATE))
	{
		/*!< No Idle State Response: return response failue */
		return SD_RESPONSE_FAILURE;
	}

	SD_CS_HIGH();

	/*!< Send Dummy byte 0xFF */
	SD_WriteByte(SD_DUMMY_BYTE);

	//��������
	return SD_RESPONSE_NO_ERROR;
}

/**
  * @brief  Write a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
uint8_t SD_WriteByte(uint8_t Data)
{
	/*!< Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{
		asm volatile("");
	}

	/*!< Send the byte */
	SPI_I2S_SendData(SD_SPI, Data);

	/*!< Wait to receive a byte*/
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	{
		asm volatile("");
	}

	/*!< Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SD_SPI);
}

/**
  * @brief  Read a byte from the SD.
  * @param  None
  * @retval The received byte.
  */
uint8_t SD_ReadByte(void)
{
	uint8_t Data = 0;

	/*!< Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{
	}
	/*!< Send the byte */
	SPI_I2S_SendData(SD_SPI, SD_DUMMY_BYTE);

	/*!< Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	{
	}
	/*!< Get the received data */
	Data = SPI_I2S_ReceiveData(SD_SPI);

	/*!< Return the shifted data */
	return Data;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
