#include "crc.h"
#include <stdio.h>
#include <stdint.h>

static uint8_t lookupInitialized = 0;
static uint32_t crcLookup[256];

uint8_t reflect8(uint8_t in)
{
	uint8_t out = 0x00;
	int8_t bit = 7;

	while (bit >= 0) {
		if (in & (1 << bit))
			out |= (1 << (7 - bit));
		bit--;
	}

	return out;
}

uint32_t reflect32(uint32_t in)
{
	uint32_t out = 0x00000000;
	int8_t bit = 31;

	while (bit >= 0) {
		if (in & (1 << bit))
			out |= (1 << (31 - bit));
		bit--;
	}

	return out;
}

//Produce values for lookup table
//These are the results of every 8-bit value run through a CRC calculation with our polynomial
void crcInit()
{
	uint32_t MSB = 1 << 31;
	uint32_t bit;
	uint32_t remainder;
	uint32_t topByte = 0;

	if (lookupInitialized)
		return;

	while (topByte < 256) {
		remainder = topByte << 24;

		//If top bit is set, XOR with POLY and shift
		//Otherwise just shift
		for (bit = 0; bit < 8; bit++) {
			if (remainder & MSB)
				remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
			else
				remainder <<= 1;

		}

		//printf("lookup[%d]=0x%X\n", topByte, remainder);
		crcLookup[topByte++] = remainder;
	}

	lookupInitialized = 1;
}

//Processes multiple blocks of data sequentially
//Set block = FIRST_CRC_BLOCK for the first block of data
//Set block = LAST_CRC_BLOCK for the last block of data
//Set block = 0 for all other blocks of data
//The same pointer to result is passed each time, and the value is the result
void crcMultiblockCalc(uint8_t *msg, uint32_t size,  uint32_t *result, uint32_t block)
{
	uint32_t remInit = 0xFFFFFFFF;
	uint32_t msgIdx = 0;
	uint8_t topByte;

	//Sanity check
	if (!result)
		return;

	if (block & FIRST_CRC_BLOCK)
		*result = remInit;

	while (msgIdx < size) {
		//Get top byte of remainder
		topByte = (uint8_t)(*result >> 24);

		//XOR top byte w/ next msg byte reflected
		topByte ^= reflect8(msg[msgIdx++]);

		//Shift remainder up one byte and XOR with our crc'd lookup
		*result = (*result << 8) ^ crcLookup[topByte];
	}

	if (block & LAST_CRC_BLOCK)
		*result = (reflect32(*result) ^ 0xFFFFFFFF);
}

//Calculate the crc remainder for the given message
uint32_t crcCalc(uint8_t *msg, uint32_t size)
{
	uint32_t remainder = 0xFFFFFFFF;
	uint32_t msgIdx = 0;
	uint8_t topByte;

	crcInit();

	while (msgIdx < size) {
		//Get top byte of remainder
		topByte = (uint8_t)(remainder >> 24);

		//XOR top byte w/ next msg byte reflected
		topByte ^= reflect8(msg[msgIdx++]);

		//Shift remainder up one byte and XOR with our crc'd lookup
		remainder = (remainder << 8) ^ crcLookup[topByte];
	}

	return (reflect32(remainder) ^ 0xFFFFFFFF);
}

//Return 0 if msg checksum is verified,
//Return -1 if mismatch
uint8_t crcVerifyMsg(uint8_t *msg, uint32_t size)
{
	crcInit();

	//Grab the checksum from the end of the msg (Big Endian)
	uint32_t checksumLoc = size - (CHECK_BITSIZE / 8);
	int byte;
	uint32_t checksum = 0;

	for (byte = 0; byte < (CHECK_BITSIZE / 8); byte++)
		checksum |= (msg[checksumLoc + byte] << (CHECK_BITSIZE - (8 * (byte + 1))));



	//Calculate checksum of msg (excluding checksum at end of message)
	uint32_t crcVal = crcCalc(&msg[0], size - (CHECK_BITSIZE / 8));

	//Compare
	if (crcVal != checksum) {
		printf("Message corrupted!\n");
		printf("Received checksum=0x%X\n", checksum);
		printf("Calculated checksum=0x%X\n", crcVal);
		return -1;
	}

	return 0;
}

#if 0
uint8_t testMsg[] = {'1','2','3','4','5','6','7','8','9',0xCB,0xF4,0x39,0x26};
uint32_t testChecksum = 0xCBF43926;

int main(int argc, char **argv)
{
	crcInit();

	uint32_t multiBlockResult;
	int i;
	for (i = 0; i < 9; i++) {
		if (i == 0)
			crcMultiblockCalc(&testMsg[i], 1, &multiBlockResult, FIRST_CRC_BLOCK);
		else if (i == 8)
			crcMultiblockCalc(&testMsg[i], 1, &multiBlockResult, LAST_CRC_BLOCK);
		else
			crcMultiblockCalc(&testMsg[i], 1, &multiBlockResult, 0);

	}
		 
	printf("multiBlockResult = 0x%X\n", multiBlockResult);

	uint32_t singleBlockResult = crcCalc(&testMsg[0], 9);

	printf("singleBlockResult = 0x%X\n", singleBlockResult);


/*
	uint8_t match = crcVerifyMsg(&testMsg[0], sizeof(testMsg));
	if (match)
		printf("Message and checksum match\n");
	else
		printf("Message and checksum do not match\n");


	uint32_t crcVal = crcCalc(&testMsg[0], sizeof(testMsg));
	printf("Got: 0x%X\n", crcVal);
	printf("Exp: 0xCBF43926\n");
*/
}
#endif

