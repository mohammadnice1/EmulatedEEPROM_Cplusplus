/*
 * MAteeqEmulatedEEPROM.cpp
 *
 *  Created on: Dec 4, 2018
 *      Author: Msc. Eng. Ateeq
 */

#include "main.h"

#include "stm32f4xx_hal.h"

#include "../ClassObjects/MAteeqEmulatedEEPROM.h"

void Error_Handler(void);
void SystemClock_Config(void);

int main(void)
{

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();


	// ***************** Define the object as a static *************************
	// Here we define the start address of Emulated EEPROM depends on how much flash memory we used for our total code = 15.15 KB used,
	// then we will use the 2nd and 3rd Flash Sectors, as 0 and 1st is occupied
	// sector 0 = 16 KB --> 0x4000
	// sector 1 = 16 KB --> 0x4000
	//
	// 0x4000 + 0x4000 = 0x8000
	// Start Address of Emulated EEPROM in Flash Memory =>    0x08000000   --> is the Start Address of Flash Memory
	//													    +     0x8000
	//                                                        ----------
	//													      0x08008000
	// First Flash Sector = 2
	// Second Flash Sector = 3
	// Size of Each Sector in KByte = 16 KB
	// Desired EEPROM Size = 1 KB
	MAteeqEmulatedEEPROM mateeqEmulatedEEPROMstatic(0x08008000, 2, 3, 16, MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_1KB);
	bool isStaticInit = mateeqEmulatedEEPROMstatic.Init();

	char charStatic[4];
	if (isStaticInit == true)
	{
		mateeqEmulatedEEPROMstatic.PutChar(0, 'M');           		// Put char in address 0
		charStatic[0] = mateeqEmulatedEEPROMstatic.GetChar(0);   	// Get char From address 0

		mateeqEmulatedEEPROMstatic.PutChar(1, 'O');           		// Put char in address 1
		charStatic[1] = mateeqEmulatedEEPROMstatic.GetChar(1);   	// Get char From address 1

		mateeqEmulatedEEPROMstatic.PutChar(2, 'H');           		// Put char in address 2
		charStatic[2] = mateeqEmulatedEEPROMstatic.GetChar(2);   	// Get char From address 2

		mateeqEmulatedEEPROMstatic.PutChar(3, 'A');           		// Put char in address 3
		charStatic[3] = mateeqEmulatedEEPROMstatic.GetChar(3);   	// Get char From address 3
	}


	// ***************** Define the object as a pointer ************************
	// Here we define the start address of Emulated EEPROM like we want, as the remains of Flash memory is not occupied by the total code
	// we will use for example the 5th and 6th Flash Sectors
	// sector 0 = 16 KB --> 0x4000
	// sector 1 = 16 KB --> 0x4000
	// sector 2 = 16 KB --> 0x4000
	// sector 3 = 16 KB --> 0x4000
	// sector 4 = 64 KB --> 0x10000
	//
	// 0x4000 + 0x4000 + 0x4000 + 0x4000 + 0x10000 = 0x20000
	// Start Address of Emulated EEPROM in Flash Memory =>    0x08000000   --> is the Start Address of Flash Memory
	//													    +    0x20000
	//                                                        ----------
	//													      0x08020000
	// First Flash Sector = 5
	// Second Flash Sector = 6
	// Size of Each Sector in KByte = 128 KB
	// Desired EEPROM Size = 4 KB
	MAteeqEmulatedEEPROM* mateeqEmulatedEEPROMpointer = new MAteeqEmulatedEEPROM(0x08020000, 5, 6, 128, MAteeqEmulatedEEPROM::EMULATED_EEPROM_SIZE_4KB);
	bool isPointerInit = mateeqEmulatedEEPROMpointer->Init();

	char charPointer[5];
	if (isPointerInit == true)
	{
		mateeqEmulatedEEPROMpointer->PutChar(0, 'A');            	// Put char in address 0
		charPointer[0] = mateeqEmulatedEEPROMpointer->GetChar(0);   // Get char From address 0

		mateeqEmulatedEEPROMpointer->PutChar(1, 'T');            	// Put char in address 1
		charPointer[1] = mateeqEmulatedEEPROMpointer->GetChar(1);   // Get char From address 1

		mateeqEmulatedEEPROMpointer->PutChar(2, 'E');            	// Put char in address 2
		charPointer[2] = mateeqEmulatedEEPROMpointer->GetChar(2);   // Get char From address 2

		mateeqEmulatedEEPROMpointer->PutChar(3, 'E');            	// Put char in address 3
		charPointer[3] = mateeqEmulatedEEPROMpointer->GetChar(3);   // Get char From address 3

		mateeqEmulatedEEPROMpointer->PutChar(4, 'Q');            	// Put char in address 4
		charPointer[4] = mateeqEmulatedEEPROMpointer->GetChar(4);   // Get char From address 4
	}


	// ******************* Define String and Store it, then restore it ***************************
	char ateeqString[16] = "Msc. Eng. Ateeq";
	mateeqEmulatedEEPROMpointer->MemcpyToEEPROMwithChecksum(10, ateeqString, sizeof(ateeqString));           // Store at Address 10

	char* ateeqStringRestore = new char();
	mateeqEmulatedEEPROMpointer->MemcpyFromEEPROMwithChecksum(ateeqStringRestore, 10, sizeof(ateeqString)); // Restore From Address 10



	while (1)
	{

	}
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/**Configure the main internal regulator output voltage*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/**Initializes the CPU, AHB and APB busses clocks*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/**Initializes the CPU, AHB and APB busses clocks*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

void Error_Handler(void)
{

}
