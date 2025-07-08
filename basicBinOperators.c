/*
 * This program simulates a microcontroller transmitting sensor data as a 32-bit unsigned integer (uint32_t).
 * The data format is structured as follows:
 * - Bits [0–7]   : Temperature in Celsius (range: -20 to 235).
 * - Bits [8–14]  : Pressure in hPa (range: 1010 to 1137).
 * - Bits [15–18] : Humidity as 4 individual bits. If more than two bits are set, it triggers an alarm.
 * - Bits [19–31] : Fluid level in a tank, measured in liters (range: 0 to 8191).
 *
 * The microcontroller sends this data as a hexadecimal string, which the user inputs.
 * The program parses the string into a number and decodes each sensor value bit by bit.
 */


#include<stdio.h>
#include<stdint.h>
#include<inttypes.h>
#include<stdbool.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>

#define TEMPERATURE_BITS_MASK		0xff
#define PRESSURE_BITS_SHIFT			8
#define PRESSURE_BITS_MASK			0x7f
#define HUMIDITY_BITS_SHIFT			15
#define HUMIDITY_BITS_MASK			0xf
#define FLUID_LEVEL_BITS_SHIFT		19
#define BITS_TO_BYTES				0x8
#define MAX_HEX_DIGITS				8
#define HEX_INPUT_BUFFER_SIZE		9			// 8 bits + 1 for \0, SIZE_OF_DATA + 1
#define HUMIDITY_BITS				4


 /**
	getBuffer
	@brief Reads user input into a character buffer.
	@param input Pointer to the buffer where input will be stored.
	@param table_size Maximum size of the buffer.
	
	flushBuffer
	@brief Flushes the input buffer to discard extra characters.
	
	enterData
	@brief Prompts the user to enter hexadecimal data, validates it, and converts to uppercase.
	Handles "END" command and input errors.
	@param enteredData Buffer to store the input.
	@param max_size Maximum allowed input length.
	@param size_of_whole_table Size of the input buffer.

	dataToUpperCAse
	@brief Converts all lowercase characters in the input to uppercase.
	@param dataToBeTranformed Pointer to the string to be transformed.
	@param max_size Length of the string.

	flushData
	@brief Clears (flushes) the content of the given character buffer by setting all elements to 0.
	@param data Pointer to the buffer to be cleared.
	@param size Size of the buffer.

	checkIfEnd
	@brief Checks whether the user entered the keyword "END" to terminate the program.
	@param data Pointer to the user input string.
	@return true if input equals "END" (case-insensitive), false otherwise.

	checkEnteredDAta
	@brief Verifies if all characters in the input are valid hexadecimal digits.
	@param data Pointer to the input string.
	@param size Size of the string to check.
	@return true if input is valid, false otherwise

	convertToNumber
	@brief Converts a hexadecimal string into a 32-bit unsigned integer.
	@param input Pointer to the hex string to convert.
	@return The converted uint32_t value.

	getTemperature
	@brief Extracts the temperature from the 32-bit data value.
	@param data The full 32-bit input data.
	@param shift Number of bits to shift (usually 0).
	@return Temperature as a signed 16-bit integer.

	getPressure
	@brief Extracts the pressure value from the 32-bit data.
	@param data The full 32-bit input data.
	@param pressureBits Number of bits used for pressure.
	@param shift Number of bits to shift to reach pressure field.
	@return Pressure in hPa as an unsigned 16-bit integer.

	getHumidity
	@brief Extracts the 4-bit humidity field from the 32-bit data.
	@param data The full 32-bit input data.
	@param humidityBits Number of bits used for humidity (should be 4).
	@param shift Number of bits to shift to reach humidity field.
	@return Humidity value as an unsigned 8-bit integer.

	getFluidLevel
	@brief Extracts the fluid level value from the 32-bit data.
	@param data The full 32-bit input data.
	@param shift Number of bits to shift to reach fluid level field.
	@return Fluid level in liters as an unsigned 16-bit integer.

	alarm
	@brief Prints an alarm message if specific thresholds are exceeded (e.g., humidity bits).
	@param temperature Temperature value.
	@param pressure Pressure value.
	@param humidity Humidity raw bit value.
	@param fluidLevel Fluid level in liters.

	countHumidityBits
	@brief Counts the number of bits set to 1 in a humidity field.
	@param humidityValue 4-bit humidity value.
	@param maxBits Number of bits to check (should be 4).
	@return The number of bits that are set to 1.

 */

void getBuffer(char* input, uint8_t table_size);
void flushBuffer(void);
void enterData(char*, uint8_t, uint8_t);
void dataToUpperCase(char*, uint8_t);
//void removeNewLineChar(char*, uint8_t);
void flushData(char*, uint8_t);
bool checkIfEnd(char*);
bool checkEnteredData(char*, uint8_t);
uint32_t convertToNumber(char*);
int16_t getTemperature(uint32_t, uint8_t);
uint16_t getPressure(uint32_t, uint8_t, uint8_t);
uint8_t getHumidity(uint32_t, uint8_t, uint8_t);
uint16_t getFluidLevel(uint32_t, uint8_t);
void alarm(int16_t, uint16_t, uint8_t, uint16_t);
int countHumidityBits(uint8_t, uint8_t);

int main(void) {

	char dataString[HEX_INPUT_BUFFER_SIZE] = { '\0' };
	uint32_t receivedData = 0;

	int16_t temperature;
	uint16_t pressure;
	uint8_t humidity;
	uint16_t fluidLevel;

	while (1) {

		enterData(dataString, MAX_HEX_DIGITS, HEX_INPUT_BUFFER_SIZE);
		printf("Received data = %s\n", dataString);
		receivedData = convertToNumber(dataString);
		printf("Data after convertion = %" PRIx32 " = %" PRIu32 "\n", receivedData, receivedData);
		temperature = getTemperature(receivedData, TEMPERATURE_BITS_MASK);
		printf("Temperature = %" PRIx16 " = %" PRIi16 "\n", temperature, temperature);
		pressure = getPressure(receivedData, PRESSURE_BITS_MASK, PRESSURE_BITS_SHIFT);
		printf("Pressure = %" PRIx16 " = %" PRIu16 "\n", pressure, pressure);
		humidity = getHumidity(receivedData, HUMIDITY_BITS_MASK, HUMIDITY_BITS_SHIFT);
		printf("Humidity = %" PRIx8 " = %" PRIu8 "\n", humidity, humidity);
		fluidLevel = getFluidLevel(receivedData, FLUID_LEVEL_BITS_SHIFT);
		printf("Fluid level = %" PRIx16 " = %" PRIu16 "\n", fluidLevel, fluidLevel);
		alarm(temperature, pressure, humidity, fluidLevel);
	}
	return 0;
}

void getBuffer(char* input, uint8_t table_size) {
	char tempChar;

	//printf("Enter a hexadecimal number that simulates the data received from the microcontroller:\n");
	for (int i = 0; i < table_size; i++)
	{
		tempChar = getchar();
		if (tempChar == '\n')
		{
			input[i] = '\0';
			break;
		}
		else if (i >= (table_size - 1)) {
			input[table_size - 1] = '\0';
			flushBuffer();
		}
		else
		{
			input[i] = tempChar;
		}
	}
}

void flushBuffer(void) {
	while (getchar() != '\n');
}

void enterData(char* enteredData, uint8_t max_size, uint8_t size_of_whole_table) {
	do
	{
		printf("Enter a hexadecimal number that simulates the data received from the microcontroller:\n");
		//fgets(enteredData, size_of_whole_table, stdin);
		getBuffer(enteredData, size_of_whole_table);

		//removenewlinechar(entereddata, max_size);
		if (enteredData[0] != '\0') {
			dataToUpperCase(enteredData, max_size);
		}

		//for (int i = 0; i < max_size; i++)
		//{
		//	putchar(enteredData[i]);
		//}

		if (enteredData[0] == '\0') {
			printf("You have entered empty data! Try again\n");
			flushData(enteredData, size_of_whole_table);
		}

		else if (checkIfEnd(enteredData))
		{
			printf("Closiong the program...\n");
			exit(0);
		}

		else if (!(checkEnteredData(enteredData, max_size)))
		{
			printf("You have entered wrong data! Use only 0-9 and A-F\n");
			flushData(enteredData, size_of_whole_table);
		}

		else
		{
			break;
		}
	} while (true);
}

void dataToUpperCase(char* dataToBeTranformed, uint8_t max_size) {
	for (int i = 0; i < max_size && dataToBeTranformed[i] != '\0'; i++)
	{
		dataToBeTranformed[i] = toupper(dataToBeTranformed[i]);
	}
}

//void removeNewLineChar(char* enteredString, uint8_t max_size) {
//	for (int i = 0; i < max_size; i++)
//	{
//		if (enteredString[i] == '\n') {
//			enteredString[i] = '\0';
//		}
//	}
//}

void flushData(char* dataToBeFlashed, uint8_t size_of_table) {
	for (int i = 0; i < size_of_table && dataToBeFlashed[i] != '\0'; i++)
	{
		dataToBeFlashed[i] = '\0';
	}
}

bool checkIfEnd(char* checkIfDataIsEnd) {
	if (!(strcmp(checkIfDataIsEnd, "END")))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool checkEnteredData(char* dataToBeChecked, uint8_t max_size) {
	for (int i = 0; i < max_size && dataToBeChecked[i] != '\0'; i++)
	{
		if (!(isdigit(dataToBeChecked[i]))) {
			if (dataToBeChecked[i] < 'A' || dataToBeChecked[i] > 'F') {
				return false;
			}
		}
	}
	return true;
}

uint32_t convertToNumber(char* stringToBeConverted) {
	return ((uint32_t)(strtoul(stringToBeConverted, NULL, 16)));
}

int16_t getTemperature(uint32_t tempData, uint8_t maskForTemp) {
	tempData = tempData & maskForTemp;
	return (((short int)tempData) - 20);
}

uint16_t getPressure(uint32_t tempData, uint8_t maskForPressure, uint8_t shiftForPressure) {
	tempData = tempData >> shiftForPressure;
	tempData = tempData & maskForPressure;
	return (((int)tempData) + 1010);
}

uint8_t getHumidity(uint32_t tempData, uint8_t maskForHumidity, uint8_t shiftForHumidity) {
	tempData = tempData >> shiftForHumidity;
	tempData = tempData & maskForHumidity;
	return ((uint8_t)tempData);
}

uint16_t getFluidLevel(uint32_t tempData, uint8_t shiftForFluidLevel) {
	tempData = tempData >> shiftForFluidLevel;
	return ((uint16_t)tempData);
}

void alarm(int16_t temperatureData, uint16_t pressureData, uint8_t humidityData, uint16_t fluidLevelData) {
	if (temperatureData <= 4)
	{
		printf("Alarm! Temperature of fluid  = %" PRIi16 " is lower or equal 4 Celsius!\n", temperatureData);
	}
	else if (temperatureData > 100)
	{
		printf("Alarm! Temperature of fluid = %" PRIi16 " is greater than 100 Celsius!\n", temperatureData);
	}

	if (pressureData < 1013)
	{
		printf("Alarm! Pressure in tank = %" PRIu16 " is lower then a normal pressure (1013 hPa)\n", pressureData);
	}
	else if (pressureData > 1135)
	{
		printf("Alarm! Pressure in tank = %" PRIu16 " is greater than maximal (1135 hpa)\n", pressureData);
	}

	if (countHumidityBits(humidityData, HUMIDITY_BITS)) {
		printf("Alarm! The measured humidity level exceeds the acceptable range\n");
	}

	if (fluidLevelData <= 0)
	{
		printf("Alarm! Tank is empty!\n");
	}
	else if (fluidLevelData > 8000)
	{
		printf("Alarm! Fluid level = %" PRIu16 " l. Maximal fluid level is 8100 l!\n", fluidLevelData);
	}
}

int countHumidityBits(uint8_t bitsToBeCounted, uint8_t checkedBits) {
	uint8_t counter = 0;
	uint8_t temp = 0;

	for (int i = 0; i < checkedBits; i++)
	{
		temp = (bitsToBeCounted >> i) & 0x1;
		if (temp)
		{
			counter++;
		}
	}
	return (counter > 2);
}
