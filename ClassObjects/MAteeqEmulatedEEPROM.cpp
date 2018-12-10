/*
 * MAteeqEmulatedEEPROM.cpp
 *
 *  Created on: Dec 4, 2018
 *      Author: Msc. Eng. Ateeq
 */

#include "MAteeqEmulatedEEPROM.h"

MAteeqEmulatedEEPROM::MAteeqEmulatedEEPROM(unsigned int startAdressOfEEPROMinFlash, unsigned char firstSectorToUse, unsigned char secondSectorToUse, unsigned short eachSectorSizeInKByte, MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE desiredEEPROMsize)
{
	this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE = eachSectorSizeInKByte * 1024;

	this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS = startAdressOfEEPROMinFlash;

	this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + 0x0000;
	this->EMULATED_EEPROM_DEFINITION_PAGE0_END_ADDRESS  = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + (this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE - 1);
	this->EMULATED_EEPROM_DEFINITION_PAGE0_ID = firstSectorToUse;

	this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE;
	this->EMULATED_EEPROM_DEFINITION_PAGE1_END_ADDRESS  = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + (2 * this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE - 1);
	this->EMULATED_EEPROM_DEFINITION_PAGE1_ID = secondSectorToUse;

	switch (desiredEEPROMsize)
	{
		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_4KB:
			VirtAddVarTab[0] = 0x0000;
			break;

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_3KB:
			VirtAddVarTab[0] = 0x4000;
			break;

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_2KB:
			VirtAddVarTab[0] = 0x8000;
			break;

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_1KB:
			VirtAddVarTab[0] = 0xC000;
			break;

		default:
			break;
	}

	VarDataTab[0] = 0;

	VarValue = 0;
	DataVar = 0;
}

MAteeqEmulatedEEPROM::~MAteeqEmulatedEEPROM()
{
	HAL_FLASH_Lock(); 	// Lock the Flash Program Erase controller
}

bool MAteeqEmulatedEEPROM::Init()
{
	HAL_FLASH_Unlock(); // Unlock the Flash Program Erase controller

	if (this->EE_Init() != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return false;
	}

	return true;
}

unsigned char MAteeqEmulatedEEPROM::GetChar(unsigned int addr)
{
	this->EE_ReadVariable(this->VirtAddVarTab[0]+addr,  &VarDataTab[0]);
	this->DataVar = VarDataTab[0];

	return this->DataVar;
}

bool MAteeqEmulatedEEPROM::PutChar(unsigned int addr, unsigned char new_value)
{
	this->VarValue = new_value;
	if (this->EE_WriteVariable(this->VirtAddVarTab[0]+addr, this->VarValue) != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return false;
	}

	return true;
}

void MAteeqEmulatedEEPROM::MemcpyToEEPROMwithChecksum(unsigned int destination, char *source, unsigned int size)
{
	unsigned char checksum = 0;

	for( ; size > 0; size--)
	{
		checksum = (checksum << 1) || (checksum >> 7);
		checksum += *source;

		this->PutChar(destination++, *(source++));
	}

	this->PutChar(destination, checksum);
}

bool MAteeqEmulatedEEPROM::MemcpyFromEEPROMwithChecksum(char *destination, unsigned int source, unsigned int size)
{
	unsigned char data;
	unsigned char checksum = 0;

	for( ; size > 0; size--)
	{
		data = this->GetChar(source++);
		checksum = (checksum << 1) || (checksum >> 7);
		checksum += data;
		*(destination++) = data;
	}

	if (checksum == this->GetChar(source))
	{
		return true;
	}
	return false;
}

MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS MAteeqEmulatedEEPROM::EE_Format()
{
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef pEraseInit;

	pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE0_ID;
	pEraseInit.NbSectors = 1;
	pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;
	/* Erase Page0 */
	if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS))
	{
		FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
		/* If erase operation was failed, a Flash error code is returned */
		if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
		{
			return (MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus;
		}
	}
	/* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
	FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);

	/* If program operation was failed, a Flash error code is returned */
	if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return (MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus;
	}

	pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE1_ID;
	/* Erase Page1 */
	if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS))
	{
		FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
		/* If erase operation was failed, a Flash error code is returned */
		if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
		{
			return (MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus;
		}
	}

	return MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK;
}
unsigned short MAteeqEmulatedEEPROM::EE_FindValidPage(unsigned short Operation)
{
	unsigned short PageStatus0 = 6, PageStatus1 = 6;

	/* Get Page0 actual status */
	PageStatus0 = (*(__IO unsigned short*) this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS);

	/* Get Page1 actual status */
	PageStatus1 = (*(__IO unsigned short*) this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS);

	/* Write or read operation */
	switch (Operation)
	{
		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_WRITE_IN_VALID_PAGE: /* ---- Write operation ---- */
			if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE)
			{
				/* Page0 receiving data */
				if (PageStatus0 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_RECEIVE_DATA)
				{
					return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0; /* Page0 valid */
				}
				else
				{
					return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE1; /* Page1 valid */
				}
			}
			else if (PageStatus0 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE)
			{
				/* Page1 receiving data */
				if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_RECEIVE_DATA)
				{
					return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE1; /* Page1 valid */
				}
				else
				{
					return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0; /* Page0 valid */
				}
			}
			else
			{
				return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE; /* No valid Page */
			}

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_READ_FROM_VALID_PAGE: /* ---- Read operation ---- */
			if (PageStatus0 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE)
			{
				return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0; /* Page0 valid */
			}
			else if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE)
			{
				return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE1; /* Page1 valid */
			}
			else
			{
				return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE; /* No valid Page */
			}

		default:
			return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0; /* Page0 valid */
	}
}
unsigned short MAteeqEmulatedEEPROM::EE_VerifyPageFullWriteVariable(unsigned short VirtAddress, unsigned short Data)
{
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	unsigned short ValidPage = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0;
	unsigned int Address = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS;
	unsigned int PageEndAddress = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE;

	/* Get valid Page for write operation */
	ValidPage = this->EE_FindValidPage(MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_WRITE_IN_VALID_PAGE);

	/* Check if there is no valid page */
	if (ValidPage == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE)
	{
		return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE;
	}

	/* Get the valid Page start Address */
	Address = (unsigned int) (this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + (unsigned int) (ValidPage * this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE));

	/* Get the valid Page end Address */
	PageEndAddress = (unsigned int) ((this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS - 1) + (unsigned int) ((ValidPage + 1) * this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE));

	/* Check each active page address starting from begining */
	while (Address < PageEndAddress)
	{
		/* Verify if Address and Address+2 contents are 0xFFFFFFFF */
		if ((*(__IO unsigned int*) Address) == 0xFFFFFFFF)
		{
			/* Set variable data */
			FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Address, Data);
			/* If program operation was failed, a Flash error code is returned */
			if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
			{
				return (MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus;
			}
			/* Set variable virtual address */
			FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Address + 2, VirtAddress);
			/* Return program operation status */
			return (MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus;
		}
		else
		{
			/* Next address location */
			Address = Address + 4;
		}
	}

	/* Return PAGE_FULL in case the valid page is full */
	return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE_FULL;
}
unsigned short MAteeqEmulatedEEPROM::EE_PageTransfer(unsigned short VirtAddress, unsigned short Data)
{
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	unsigned int NewPageAddress = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS;
	unsigned short OldPageId = 0;
	unsigned short ValidPage = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0, VarIdx = 0;
	unsigned short EepromStatus = 0, ReadStatus = 0;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef pEraseInit;

	/* Get active Page for read operation */
	ValidPage = this->EE_FindValidPage(MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_READ_FROM_VALID_PAGE);

	if (ValidPage == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE1) /* Page1 valid */
	{
		/* New page address where variable will be moved to */
		NewPageAddress = this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS;

		/* Old page ID where variable will be taken from */
		OldPageId = this->EMULATED_EEPROM_DEFINITION_PAGE1_ID;
	}
	else if (ValidPage == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0) /* Page0 valid */
	{
		/* New page address  where variable will be moved to */
		NewPageAddress = this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS;

		/* Old page ID where variable will be taken from */
		OldPageId = this->EMULATED_EEPROM_DEFINITION_PAGE0_ID;
	}
	else
	{
		return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE; /* No valid Page */
	}

	/* Set the new Page status to RECEIVE_DATA status */
	FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, NewPageAddress, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_RECEIVE_DATA);
	/* If program operation was failed, a Flash error code is returned */
	if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return FlashStatus;
	}

	/* Write the variable passed as parameter in the new active page */
	EepromStatus = this->EE_VerifyPageFullWriteVariable(VirtAddress, Data);
	/* If program operation was failed, a Flash error code is returned */
	if (EepromStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return EepromStatus;
	}

	/* Transfer process: transfer variables from old to the new active page */
	for (VarIdx = 0; VarIdx < 1; VarIdx++)
	{
		if (VirtAddVarTab[VarIdx] != VirtAddress) /* Check each variable except the one passed as parameter */
		{
			/* Read the other last variable updates */
			ReadStatus = this->EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
			/* In case variable corresponding to the virtual address was found */
			if (ReadStatus != 0x1)
			{
				/* Transfer the variable to the new active page */
				EepromStatus = this->EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
				/* If program operation was failed, a Flash error code is returned */
				if (EepromStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return EepromStatus;
				}
			}
		}
	}

	pEraseInit.TypeErase = TYPEERASE_SECTORS;
	pEraseInit.Sector = OldPageId;
	pEraseInit.NbSectors = 1;
	pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;

	/* Erase the old Page: Set old Page status to ERASED status */
	FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
	/* If erase operation was failed, a Flash error code is returned */
	if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return FlashStatus;
	}

	/* Set new Page status to VALID_PAGE status */
	FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, NewPageAddress, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);
	/* If program operation was failed, a Flash error code is returned */
	if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
	{
		return FlashStatus;
	}

	/* Return last operation flash status */
	return FlashStatus;
}
unsigned short MAteeqEmulatedEEPROM::EE_VerifyPageFullyErased(unsigned int Address)
{
	unsigned int ReadStatus = 1;
	unsigned short AddressValue = 0x5555;

	/* Check each active page address starting from end */
	while (Address <= this->EMULATED_EEPROM_DEFINITION_PAGE0_END_ADDRESS)
	{
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(__IO unsigned short*) Address);

		/* Compare the read address with the virtual address */
		if (AddressValue != MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_ERASED)
		{

			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;

			break;
		}
		/* Next address location */
		Address = Address + 4;
	}

	/* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
	return ReadStatus;
}

unsigned short MAteeqEmulatedEEPROM::EE_Init()
{
	unsigned short PageStatus0 = 6, PageStatus1 = 6;
	unsigned short VarIdx = 0;
	unsigned short EepromStatus = 0, ReadStatus = 0;
	signed short x = -1;
	HAL_StatusTypeDef FlashStatus;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef pEraseInit;

	/* Get Page0 status */
	PageStatus0 = (*(__IO unsigned short*) this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS);
	/* Get Page1 status */
	PageStatus1 = (*(__IO unsigned short*) this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS);

	pEraseInit.TypeErase = TYPEERASE_SECTORS;
	pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE0_ID;
	pEraseInit.NbSectors = 1;
	pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;

	/* Check for invalid header states and repair if necessary */
	switch (PageStatus0)
	{
		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_ERASED:
			if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE) /* Page0 erased, Page1 valid */
			{
				/* Erase Page0 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
			}
			else if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_RECEIVE_DATA) /* Page0 erased, Page1 receive */
			{
				/* Erase Page0 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
				/* Mark Page1 as valid */
				FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);
				/* If program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
			}
			else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
			{
				/* Erase both Page0 and Page1 and set Page0 as valid page */
				FlashStatus = (HAL_StatusTypeDef)this->EE_Format();
				/* If erase/program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
			}
			break;

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_RECEIVE_DATA:
			if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE) /* Page0 receive, Page1 valid */
			{
				/* Transfer data from Page1 to Page0 */
				for (VarIdx = 0; VarIdx < 1; VarIdx++)
				{
					if ((*(__IO unsigned short*) (this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
					{
						x = VarIdx;
					}
					if (VarIdx != x)
					{
						/* Read the last variables' updates */
						ReadStatus = this->EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
						/* In case variable corresponding to the virtual address was found */
						if (ReadStatus != 0x1)
						{
							/* Transfer the variable to the Page0 */
							EepromStatus = this->EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
							/* If program operation was failed, a Flash error code is returned */
							if (EepromStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
							{
								return EepromStatus;
							}
						}
					}
				}
				/* Mark Page0 as valid */
				FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);
				/* If program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
				pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE1_ID;
				pEraseInit.NbSectors = 1;
				pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;
				/* Erase Page1 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
			}
			else if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_ERASED) /* Page0 receive, Page1 erased */
			{
				pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE1_ID;
				pEraseInit.NbSectors = 1;
				pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;
				/* Erase Page1 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
				/* Mark Page0 as valid */
				FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);
				/* If program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
			}
			else /* Invalid state -> format eeprom */
			{
				/* Erase both Page0 and Page1 and set Page0 as valid page */
				FlashStatus = (HAL_StatusTypeDef)this->EE_Format();
				/* If erase/program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
			}
			break;

		case MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE:
			if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE) /* Invalid state -> format eeprom */
			{
				/* Erase both Page0 and Page1 and set Page0 as valid page */
				FlashStatus = (HAL_StatusTypeDef)this->EE_Format();
				/* If erase/program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
			}
			else if (PageStatus1 == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_ERASED) /* Page0 valid, Page1 erased */
			{
				pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE1_ID;
				pEraseInit.NbSectors = 1;
				pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;
				/* Erase Page1 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
			}
			else /* Page0 valid, Page1 receive */
			{
				/* Transfer data from Page0 to Page1 */
				for (VarIdx = 0; VarIdx < 1; VarIdx++)
				{
					if ((*(__IO unsigned short*) (this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
					{
						x = VarIdx;
					}
					if (VarIdx != x)
					{
						/* Read the last variables' updates */
						ReadStatus = this->EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
						/* In case variable corresponding to the virtual address was found */
						if (ReadStatus != 0x1)
						{
							/* Transfer the variable to the Page1 */
							EepromStatus = this->EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
							/* If program operation was failed, a Flash error code is returned */
							if (EepromStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
							{
								return EepromStatus;
							}
						}
					}
				}
				/* Mark Page1 as valid */
				FlashStatus = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, this->EMULATED_EEPROM_DEFINITION_PAGE1_BASE_ADDRESS, MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VALID_PAGE);
				/* If program operation was failed, a Flash error code is returned */
				if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
				{
					return FlashStatus;
				}
				pEraseInit.Sector = this->EMULATED_EEPROM_DEFINITION_PAGE0_ID;
				pEraseInit.NbSectors = 1;
				pEraseInit.VoltageRange = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_VOLTAGE_RANGE;
				/* Erase Page0 */
				if (!this->EE_VerifyPageFullyErased(this->EMULATED_EEPROM_DEFINITION_PAGE0_BASE_ADDRESS))
				{
					FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
					/* If erase operation was failed, a Flash error code is returned */
					if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
					{
						return FlashStatus;
					}
				}
			}
			break;

		default: /* Any other state -> format eeprom */
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			FlashStatus = (HAL_StatusTypeDef)this->EE_Format();
			/* If erase/program operation was failed, a Flash error code is returned */
			if ((MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS)FlashStatus != MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK)
			{
				return FlashStatus;
			}
			break;
	}

	return MAteeqEmulatedEEPROM::EMULATED_EEPROM_STATUS_OK;
}
unsigned short MAteeqEmulatedEEPROM::EE_ReadVariable(unsigned short VirtAddress, unsigned short* Data)
{
	unsigned short ValidPage = MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE0;
	unsigned short AddressValue = 0x5555, ReadStatus = 1;
	unsigned int Address = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS;
	unsigned int PageStartAddress = this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS;

	/* Get active Page for read operation */
	ValidPage = this->EE_FindValidPage(MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_READ_FROM_VALID_PAGE);

	/* Check if there is no valid page */
	if (ValidPage == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE)
	{
		return MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_NO_VALID_PAGE;
	}

	/* Get the valid Page start Address */
	PageStartAddress = (unsigned int) (this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS + (unsigned int) (ValidPage * this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE));

	/* Get the valid Page end Address */
	Address = (unsigned int) ((this->EMULATED_EEPROM_DEFINITION_EEPROM_START_ADDRESS - 2) + (unsigned int) ((1 + ValidPage) * this->EMULATED_EEPROM_DEFINITION_PAGE_SIZE));

	/* Check each active page address starting from end */
	while (Address > (PageStartAddress + 2))
	{
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(__IO unsigned short*) Address);

		/* Compare the read address with the virtual address */
		if (AddressValue == VirtAddress)
		{
			/* Get content of Address-2 which is variable value */
			*Data = (*(__IO unsigned short*) (Address - 2));

			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;

			break;
		}
		else
		{
			/* Next address location */
			Address = Address - 4;
		}
	}

	/* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
	return ReadStatus;
}
unsigned short MAteeqEmulatedEEPROM::EE_WriteVariable(unsigned short VirtAddress, unsigned short Data)
{
	unsigned short Status = 0;

	/* Write the variable virtual address and value in the EEPROM */
	Status = this->EE_VerifyPageFullWriteVariable(VirtAddress, Data);

	/* In case the EEPROM active page is full */
	if (Status == MAteeqEmulatedEEPROM::EMULATED_EEPROM_DEFINITION_PAGE_FULL)
	{
		/* Perform Page transfer */
		Status = this->EE_PageTransfer(VirtAddress, Data);
	}

	/* Return last operation status */
	return Status;
}

