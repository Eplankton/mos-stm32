/**
  ******************************************************************************
  * @file    stm32_eval_spi_sd.h
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file contains all the functions prototypes for the stm32_eval_spi_sd
  *          firmware driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EVAL_SPI_SD_H
#define __STM32_EVAL_SPI_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

	/** @addtogroup Utilities
  * @{
  */

#define SD_SPI               SPI4
#define SD_SPI_CLK           RCC_APB2Periph_SPI4
#define SD_SPI_APBxClock_FUN RCC_APB2PeriphClockCmd

#define SD_SPI_SCK_PIN       GPIO_Pin_2
#define SD_SPI_SCK_GPIO_PORT GPIOE
#define SD_SPI_SCK_GPIO_CLK  RCC_AHB1Periph_GPIOE

#define SD_SPI_MISO_PIN       GPIO_Pin_5
#define SD_SPI_MISO_GPIO_PORT GPIOE
#define SD_SPI_MISO_GPIO_CLK  RCC_AHB1Periph_GPIOE

#define SD_SPI_MOSI_PIN       GPIO_Pin_6
#define SD_SPI_MOSI_GPIO_PORT GPIOE
#define SD_SPI_MOSI_GPIO_CLK  RCC_AHB1Periph_GPIOE

#define SD_CS_PIN       GPIO_Pin_3
#define SD_CS_GPIO_PORT GPIOG
#define SD_CS_GPIO_CLK  RCC_AHB1Periph_GPIOG

	/** @addtogroup STM32_EVAL
  * @{
  */

	/** @addtogroup Common
  * @{
  */

	/** @addtogroup STM32_EVAL_SPI_SD
  * @{
  */

	/** @defgroup STM32_EVAL_SPI_SD_Exported_Types
  * @{
  */

	typedef enum
	{
		/**
  * @brief  SD reponses and error flags
  */
		SD_RESPONSE_NO_ERROR    = (0x00),
		SD_IN_IDLE_STATE        = (0x01),
		SD_ERASE_RESET          = (0x02),
		SD_ILLEGAL_COMMAND      = (0x04),
		SD_COM_CRC_ERROR        = (0x08),
		SD_ERASE_SEQUENCE_ERROR = (0x10),
		SD_ADDRESS_ERROR        = (0x20),
		SD_PARAMETER_ERROR      = (0x40),
		SD_RESPONSE_FAILURE     = (0xFF),

		/**
  * @brief  Data response error
  */
		SD_DATA_OK          = (0x05),
		SD_DATA_CRC_ERROR   = (0x0B),
		SD_DATA_WRITE_ERROR = (0x0D),
		SD_DATA_OTHER_ERROR = (0xFF)
	} SD_Error;

	/** 
  * @brief  Card Specific Data: CSD Register   
  */
	typedef struct
	{
		__IO uint8_t CSDStruct;           /*!< CSD structure */
		__IO uint8_t SysSpecVersion;      /*!< System specification version */
		__IO uint8_t Reserved1;           /*!< Reserved */
		__IO uint8_t TAAC;                /*!< Data read access-time 1 */
		__IO uint8_t NSAC;                /*!< Data read access-time 2 in CLK cycles */
		__IO uint8_t MaxBusClkFrec;       /*!< Max. bus clock frequency */
		__IO uint16_t CardComdClasses;    /*!< Card command classes */
		__IO uint8_t RdBlockLen;          /*!< Max. read data block length */
		__IO uint8_t PartBlockRead;       /*!< Partial blocks for read allowed */
		__IO uint8_t WrBlockMisalign;     /*!< Write block misalignment */
		__IO uint8_t RdBlockMisalign;     /*!< Read block misalignment */
		__IO uint8_t DSRImpl;             /*!< DSR implemented */
		__IO uint8_t Reserved2;           /*!< Reserved */
		__IO uint32_t DeviceSize;         /*!< Device Size */
		__IO uint8_t MaxRdCurrentVDDMin;  /*!< Max. read current @ VDD min */
		__IO uint8_t MaxRdCurrentVDDMax;  /*!< Max. read current @ VDD max */
		__IO uint8_t MaxWrCurrentVDDMin;  /*!< Max. write current @ VDD min */
		__IO uint8_t MaxWrCurrentVDDMax;  /*!< Max. write current @ VDD max */
		__IO uint8_t DeviceSizeMul;       /*!< Device size multiplier */
		__IO uint8_t EraseGrSize;         /*!< Erase group size */
		__IO uint8_t EraseGrMul;          /*!< Erase group size multiplier */
		__IO uint8_t WrProtectGrSize;     /*!< Write protect group size */
		__IO uint8_t WrProtectGrEnable;   /*!< Write protect group enable */
		__IO uint8_t ManDeflECC;          /*!< Manufacturer default ECC */
		__IO uint8_t WrSpeedFact;         /*!< Write speed factor */
		__IO uint8_t MaxWrBlockLen;       /*!< Max. write data block length */
		__IO uint8_t WriteBlockPaPartial; /*!< Partial blocks for write allowed */
		__IO uint8_t Reserved3;           /*!< Reserded */
		__IO uint8_t ContentProtectAppli; /*!< Content protection application */
		__IO uint8_t FileFormatGrouop;    /*!< File format group */
		__IO uint8_t CopyFlag;            /*!< Copy flag (OTP) */
		__IO uint8_t PermWrProtect;       /*!< Permanent write protection */
		__IO uint8_t TempWrProtect;       /*!< Temporary write protection */
		__IO uint8_t FileFormat;          /*!< File Format */
		__IO uint8_t ECC;                 /*!< ECC code */
		__IO uint8_t CSD_CRC;             /*!< CSD CRC */
		__IO uint8_t Reserved4;           /*!< always 1*/
	} SD_CSD;

	/** 
  * @brief  Card Identification Data: CID Register   
  */
	typedef struct
	{
		__IO uint8_t ManufacturerID; /*!< ManufacturerID */
		__IO uint16_t OEM_AppliID;   /*!< OEM/Application ID */
		__IO uint32_t ProdName1;     /*!< Product Name part1 */
		__IO uint8_t ProdName2;      /*!< Product Name part2*/
		__IO uint8_t ProdRev;        /*!< Product Revision */
		__IO uint32_t ProdSN;        /*!< Product Serial Number */
		__IO uint8_t Reserved1;      /*!< Reserved1 */
		__IO uint16_t ManufactDate;  /*!< Manufacturing Date */
		__IO uint8_t CID_CRC;        /*!< CID CRC */
		__IO uint8_t Reserved2;      /*!< always 1 */
	} SD_CID;

	/** 
  * @brief SD Card information 
  */
	typedef struct
	{
		SD_CSD SD_csd;
		SD_CID SD_cid;
		uint64_t CardCapacity;  /*!< Card Capacity */
		uint32_t CardBlockSize; /*!< Card Block Size */
	} SD_CardInfo;

	extern SD_CardInfo SDCardInfo; //���ڴ洢������Ϣ

/**
  * @}
  */

/** @defgroup STM32_EVAL_SPI_SD_Exported_Constants
  * @{
  */

/**
  * @brief  Block Size
  */
#define SD_BLOCK_SIZE 0x200

/**
  * @brief  Dummy byte
  */
#define SD_DUMMY_BYTE 0xFF

/**
  * @brief  Start Data tokens:
  *         Tokens (necessary because at nop/idle (and CS active) only 0xff is 
  *         on the data/command line)  
  */
#define SD_START_DATA_SINGLE_BLOCK_READ    0xFE /*!< Data token start byte, Start Single Block Read */
#define SD_START_DATA_MULTIPLE_BLOCK_READ  0xFE /*!< Data token start byte, Start Multiple Block Read */
#define SD_START_DATA_SINGLE_BLOCK_WRITE   0xFE /*!< Data token start byte, Start Single Block Write */
#define SD_START_DATA_MULTIPLE_BLOCK_WRITE 0xFD /*!< Data token start byte, Start Multiple Block Write */
#define SD_STOP_DATA_MULTIPLE_BLOCK_WRITE  0xFD /*!< Data toke stop byte, Stop Multiple Block Write */

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT     ((uint8_t) 0x01)
#define SD_NOT_PRESENT ((uint8_t) 0x00)

/**
  * @brief  Commands: CMDxx = CMD-number | 0x40
  */
#define SD_CMD_GO_IDLE_STATE      0  /*!< CMD0 = 0x40 */
#define SD_CMD_SEND_OP_COND       1  /*!< CMD1 = 0x41 */
#define SD_CMD_SEND_IF_COND       8  /*!< CMD8 = 0x48 */
#define SD_CMD_SEND_CSD           9  /*!< CMD9 = 0x49 */
#define SD_CMD_SEND_CID           10 /*!< CMD10 = 0x4A */
#define SD_CMD_STOP_TRANSMISSION  12 /*!< CMD12 = 0x4C */
#define SD_CMD_SEND_STATUS        13 /*!< CMD13 = 0x4D */
#define SD_CMD_SET_BLOCKLEN       16 /*!< CMD16 = 0x50 */
#define SD_CMD_READ_SINGLE_BLOCK  17 /*!< CMD17 = 0x51 */
#define SD_CMD_READ_MULT_BLOCK    18 /*!< CMD18 = 0x52 */
#define SD_CMD_SET_BLOCK_COUNT    23 /*!< CMD23 = 0x57 */
#define SD_CMD_WRITE_SINGLE_BLOCK 24 /*!< CMD24 = 0x58 */
#define SD_CMD_WRITE_MULT_BLOCK   25 /*!< CMD25 = 0x59 */
#define SD_CMD_PROG_CSD           27 /*!< CMD27 = 0x5B */
#define SD_CMD_SET_WRITE_PROT     28 /*!< CMD28 = 0x5C */
#define SD_CMD_CLR_WRITE_PROT     29 /*!< CMD29 = 0x5D */
#define SD_CMD_SEND_WRITE_PROT    30 /*!< CMD30 = 0x5E */
#define SD_CMD_SD_ERASE_GRP_START 32 /*!< CMD32 = 0x60 */
#define SD_CMD_SD_ERASE_GRP_END   33 /*!< CMD33 = 0x61 */
#define SD_CMD_UNTAG_SECTOR       34 /*!< CMD34 = 0x62 */
#define SD_CMD_ERASE_GRP_START    35 /*!< CMD35 = 0x63 */
#define SD_CMD_ERASE_GRP_END      36 /*!< CMD36 = 0x64 */
#define SD_CMD_UNTAG_ERASE_GROUP  37 /*!< CMD37 = 0x65 */
#define SD_CMD_ERASE              38 /*!< CMD38 = 0x66 */

#define SD_CMD_READ_OCR         58 /*!< CMD58 */
#define SD_CMD_APP_CMD          55 /*!< CMD55 ����0x01*/
#define SD_ACMD_SD_SEND_OP_COND 41 /*!< ACMD41  ����0x00*/

//SD��������
#define SD_TYPE_NOT_SD 0 //��SD��
#define SD_TYPE_V1     1 //V1.0�Ŀ�
#define SD_TYPE_V2     2 //SDSC
#define SD_TYPE_V2HC   4 //SDHC

/** @defgroup STM32_EVAL_SPI_SD_Exported_Macros
  * @{
  */
/** 
  * @brief  Select SD Card: ChipSelect pin low   
  */
#define SD_CS_LOW() GPIO_ResetBits(SD_CS_GPIO_PORT, SD_CS_PIN)
/** 
  * @brief  Deselect SD Card: ChipSelect pin high   
  */
#define SD_CS_HIGH() GPIO_SetBits(SD_CS_GPIO_PORT, SD_CS_PIN)
	/**
  * @}
  */

	/** @defgroup STM32_EVAL_SPI_SD_Exported_Functions
  * @{
  */
	void SD_DeInit(void);
	SD_Error SD_Init(void);
	uint8_t SD_Detect(void);
	SD_Error SD_GetCardInfo(SD_CardInfo* cardinfo);
	SD_Error SD_GetCardType(void);
	SD_Error SD_ReadBlock(uint8_t* pBuffer, uint64_t ReadAddr, uint16_t BlockSize);
	SD_Error SD_ReadMultiBlocks(uint8_t* pBuffer, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
	SD_Error SD_WriteBlock(uint8_t* pBuffer, uint64_t WriteAddr, uint16_t BlockSize);
	SD_Error SD_WriteMultiBlocks(uint8_t* pBuffer, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
	SD_Error SD_GetCSDRegister(SD_CSD* SD_csd);
	SD_Error SD_GetCIDRegister(SD_CID* SD_cid);

	void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc);
	SD_Error SD_GetResponse(uint8_t Response);
	uint8_t SD_GetDataResponse(void);
	SD_Error SD_GoIdleState(void);
	uint16_t SD_GetStatus(void);

	uint8_t SD_WriteByte(uint8_t byte);
	uint8_t SD_ReadByte(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_EVAL_SPI_SD_H */
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
