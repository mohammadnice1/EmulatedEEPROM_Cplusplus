/*
 * MAteeqEmulatedEEPROM.h
 *
 *  Created on: Dec 4, 2018
 *      Author: Msc. Eng. Ateeq
 */

#ifndef MATEEQEMULATEDEEPROM_H_
#define MATEEQEMULATEDEEPROM_H_

#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "stm32f4xx_hal.h"

class MAteeqEmulatedEEPROM
{
	public:
		enum EMULATED_EEPROM_DEFINITION
		{
			/* Used Flash pages for EEPROM emulation */
			EMULATED_EEPROM_DEFINITION_PAGE0 = (unsigned short)0x0000,
			EMULATED_EEPROM_DEFINITION_PAGE1 = (unsigned short)0x0001,
			EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE = (unsigned short)0x00AB, 	/* No valid page define */

			/* Valid pages in read and write defines */
			EMULATED_EEPROM_DEFINITION_READ_FROM_VALID_PAGE = (unsigned char)0x00,
			EMULATED_EEPROM_DEFINITION_WRITE_IN_VALID_PAGE = (unsigned char)0x01,

			/* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word  */
			EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE = (unsigned char)VOLTAGE_RANGE_3,

			EMULATED_EEPROM_DEFINITION_VALID_PAGE = (unsigned short)0x0000,		/* Page containing valid data */
			EMULATED_EEPROM_DEFINITION_ERASED = (unsigned short)0xFFFF,     	/* Page is empty */
			EMULATED_EEPROM_DEFINITION_RECEIVE_DATA = (unsigned short)0xEEEE,	/* Page is marked to receive data */
			EMULATED_EEPROM_DEFINITION_PAGE_FULL = (unsigned char)0x80, 		/* Page full define */
		};
		enum EMULATED_EEPROM_STATUS
		{
			EMULATED_EEPROM_STATUS_OK      = (unsigned int)HAL_OK,
			EMULATED_EEPROM_STATUS_ERROR   = (unsigned int)HAL_ERROR,
			EMULATED_EEPROM_STATUS_BUSY    = (unsigned int)HAL_BUSY,
			EMULATED_EEPROM_STATUS_TIMEOUT = (unsigned int)HAL_TIMEOUT,
		};
		enum EMULATED_EEPROM_SIZE
		{
			EMULATED_EEPROM_SIZE_1KB = 1,
			EMULATED_EEPROM_SIZE_2KB = 2,
			EMULATED_EEPROM_SIZE_3KB = 3,
			EMULATED_EEPROM_SIZE_4KB = 4,
		};

		MAteeqEmulatedEEPROM(unsigned int startAdressOfEEPROMinFlash,
							 unsigned char firstSectorToUse,
							 unsigned char secondSectorToUse,
							 unsigned short eachSectorSizeInKByte,
							 MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE desiredEEPROMsize);

		virtual ~MAteeqEmulatedEEPROM();

		bool Init();

		unsigned char GetChar(unsigned int addr);
		bool PutChar(unsigned int addr, unsigned char new_value);
		void MemcpyToEEPROMwithChecksum(unsigned int destination, char *source, unsigned int size);
		bool MemcpyFromEEPROMwithChecksum(char *destination, unsigned int source, unsigned int size);

	private:
		unsigned int EMULATED_EEPROM_DEFINITION_PAGE_SIZE; 	//-|--> /* Define the size of the sectors to be used */
															//-|--> /* Page size = flash sector size = 16KByte */

		unsigned int EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS;	//-|--> /* EEPROM start address in Flash */
																		//-|--> /* EEPROM emulation start address: from sector2 : after 16KByte of used Flash memory */

		unsigned int EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS;	//-|
		unsigned int EMULATED_EEPROM_DEFINITION_PAGE0_END_ADDRESS;	//-|
		unsigned char EMULATED_EEPROM_DEFINITION_PAGE0_ID;			//-|--> /* Pages 0 and 1 base and end addresses */
		unsigned int EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS;	//-|
		unsigned int EMULATED_EEPROM_DEFINITION_PAGE1_END_ADDRESS;	//-|
		unsigned char EMULATED_EEPROM_DEFINITION_PAGE1_ID;			//-|

		unsigned short VirtAddVarTab[1]; //-|
		unsigned short VarDataTab[1];    //-|--> /* Virtual address defined by the user: 0xFFFF value is prohibited */
		unsigned short VarValue;         //-|

		unsigned short DataVar; 		 //-|--> /* Global variable used to store variable value in read sequence */

		EMULATED_EEPROM_STATUS EE_Format();
		unsigned short EE_FindValidPage(unsigned short Operation);
		unsigned short EE_VerifyPageFullWriteVariable(unsigned short VirtAddress, unsigned short Data);
		unsigned short EE_PageTransfer(unsigned short VirtAddress, unsigned short Data);
		unsigned short EE_VerifyPageFullyErased(unsigned int Address);

		unsigned short EE_Init();
		unsigned short EE_ReadVariable(unsigned short VirtAddress, unsigned short* Data);
		unsigned short EE_WriteVariable(unsigned short VirtAddress, unsigned short Data);
};

#endif /* MATEEQEMULATEDEEPROM_H_ */
