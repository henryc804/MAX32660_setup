#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "crc.h"

#define FILE_FORMAT_VERSION	0

#ifndef EIO
#define EIO		5
#endif
#ifndef EINVAL
#define EINVAL	22
#endif
#ifndef EFBIG
#define EFBIG	27
#endif

#define FILEPATH_ARG	1
#define TARGETNAME_ARG	2
#define PAGESIZE_ARG	3
#define KEYNAME_ARG		4

#define OUTPUT_EXT			".msbl"
#define MAGIC_VAL			"msbl"
#define ENCRYPT_TYPE_AES	"AES-192"

#define ALIGN4	4

#define AES_NONCE_SIZE	11
#define AES_AUTH_SIZE	16


#define AES_FILE_KEY_START	"aes_key_start"
#define AES_FILE_AAD_START	"aes_aad_start"
#define AES_FILE_KEY_END	"aes_key_end"
#define KEYFILE_EXTENSION	"_key.txt"

#define MAXFILESIZE			0xFFFFFFFFu
#define MAX_KEYNAME_LENGTH	256
#define MAX_AES_AAD_SIZE	32

#define PAD_CHECKBYTE_TO_16	(16 - CHECK_BYTESIZE)
#define MARK_VALID_MAGIC_VAL    0x4D41524B //'MARK'

static EVP_CIPHER_CTX *ctx;

#pragma pack(push, 1)
typedef struct {
	char magic[4];
	uint32_t formatVersion;
	char target[16];
	char enc_type[16];
	uint8_t nonce[AES_NONCE_SIZE];
	uint8_t resv0;
	uint8_t auth[AES_AUTH_SIZE];
	uint16_t numPages;
	uint16_t pageSize;
	uint8_t crcSize;
	uint8_t resv1[3];
} msbl_header;

typedef struct {
	uint32_t CRC32;
	uint32_t length;
	uint32_t validMark;
	uint32_t boot_mode;
} app_header_t;

#pragma pack(pop)


//Initialize aes encryption
void aes_init(uint32_t totalSize, const uint8_t *nonce, unsigned char *aes_key, unsigned char *aes_aad, int aad_size)
{
	int outlen;
	ctx = EVP_CIPHER_CTX_new();

	/* Set cipher type and mode */
	EVP_EncryptInit_ex(ctx, EVP_aes_192_ccm(), NULL, NULL, NULL);

	/* Set nonce length if default 96 bits is not appropriate */
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, AES_NONCE_SIZE, NULL);

	/* Set tag length */
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, AES_AUTH_SIZE, NULL);

	/* Initialise key and IV */
	EVP_EncryptInit_ex(ctx, NULL, NULL, aes_key, nonce);

	/* Set plaintext length: only needed if AAD is used */
	EVP_EncryptUpdate(ctx, NULL, &outlen, NULL, totalSize);

	/* Zero or one call to specify any AAD */
	EVP_EncryptUpdate(ctx, NULL, &outlen, aes_aad, aad_size);
}

//Encrypts plain test into cypher
void aes_encrypt(const uint8_t *plain_text, uint32_t len, uint8_t *cypher)
{
	int outlen = 0;
	int ret;

	/* Encrypt plaintext: can only be called once */
	ret = EVP_EncryptUpdate(ctx, cypher, &outlen, plain_text, len);

	if (outlen != len || !ret) {
		printf("WARNING: EVP_EncryptUpdate failed. outlen=%d len=%d\n", outlen, len);
	}
}

//Called after last encryption is complete to get auth bytes
void aes_finalize(uint8_t *auth)
{
	int outlen;
	uint8_t outbuf[1024];

	/* Finalise: note get no output for CCM */
	EVP_EncryptFinal_ex(ctx, outbuf, &outlen);

	/* Get tag */
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, AES_AUTH_SIZE, auth);

	EVP_CIPHER_CTX_free(ctx);
}

//Parses AES KEY and AAD from given file
int parse_key_from_file(FILE *file, unsigned char **dest, const char *start_tok,
		const char *end_tok, int *keySize)
{
	int i;
	char line[256];

	bool found_key_start = false;
	bool found_key_end = false;
	memset(line, 0, sizeof(line));

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, start_tok)) {
			found_key_start = true;
			break;
		}
		memset(line, 0, sizeof(line));
	}

	if (!found_key_start) {
		printf("Couldn't find the key in given file path\r\n");
		return -EINVAL;
	}

	int key_bytes = 0;
	int int32_kb[128];
	while (key_bytes <= MAX_AES_AAD_SIZE) {
		memset(line, 0, sizeof(line));
		if (!fgets(line, sizeof(line), file))
			break;
		/*Check if end token is reached*/
		if (strstr(line, end_tok)) {
			found_key_end = true;
			break;
		}

		int *out_idx = int32_kb + key_bytes;
		key_bytes += sscanf(line,
							"%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, "
							"%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, ",
							out_idx + 0, out_idx + 1, out_idx + 2, out_idx + 3,
							out_idx + 4, out_idx + 5, out_idx + 6, out_idx + 7,
							out_idx + 8, out_idx + 9, out_idx + 10, out_idx + 11,
							out_idx + 12, out_idx + 13, out_idx + 14, out_idx + 15,
							out_idx + 16, out_idx + 17, out_idx + 18, out_idx + 19,
							out_idx + 20, out_idx + 21, out_idx + 22, out_idx + 23,
							out_idx + 24, out_idx + 25, out_idx + 26, out_idx + 27,
							out_idx + 28, out_idx + 29, out_idx + 30, out_idx + 31);
	}

	/* Check if end token is found and key size is appropriate */
	if (!found_key_end || key_bytes > MAX_AES_AAD_SIZE) {
		printf("Couldn't parse the key in given file path\r\n");
		return -EINVAL;
	}

	/* Allocate the key container depending on the parsed size */
	*dest = (unsigned char*)malloc(key_bytes * sizeof(unsigned char));

	/* Copy the parsed key */
	for (i = 0; i < key_bytes; i++)
		(*dest)[i] = (unsigned char)int32_kb[i];

	/* Indicate parsed key size to the caller */
	*keySize = key_bytes;

	return 0;
}

int main(int argc, char** argv)
{
	int err_code = 0;
	int aad_size;
	int aes_size;
	unsigned char *aes_key;
	unsigned char *aes_aad;
	FILE* fp_in;
	FILE* fp_out;
	FILE* fp_out_bootmem;

//=======================Parse Inputs========================

	if (argc != 4 && argc != 5) {
		printf("Syntax:\n");
		printf("\tmsblGen <file path> <target name> <page size> [<key name>]\n");
		printf("Example target name: 'MAX32630'\n");
		printf("Example key name: ME11A\n");
		return -EINVAL;
	}

	printf("File Format Version: %d\n", FILE_FORMAT_VERSION);

	//Get file name and extention
	char* inputNameWithExt = argv[FILEPATH_ARG];
	char* dotPtr = inputNameWithExt + strlen(inputNameWithExt) - 1;
	char* slashPtr = inputNameWithExt + strlen(inputNameWithExt) - 1;
	while (*dotPtr != '.' && dotPtr != inputNameWithExt)
		dotPtr--;
	while (*slashPtr != '\\' && *slashPtr != '/' && slashPtr != inputNameWithExt)
		slashPtr--;

	if (dotPtr == inputNameWithExt || (slashPtr > dotPtr)) {
		printf("Bad file name!\n");
		err_code = -EINVAL;
		goto error;
	}

	if (slashPtr != inputNameWithExt)
		slashPtr++;

	int inputNameLen = dotPtr - slashPtr + 1;

	char* inputExt = dotPtr;
	char* inputNameNoExt = (char*)malloc(inputNameLen);
	memset(inputNameNoExt, 0, inputNameLen);
	strncpy(inputNameNoExt, slashPtr, inputNameLen - 1);

	if (strcmp(inputExt, ".bin")) {
		printf("WARNING:\n");
		printf("\tFile Extension:\t%s\n", inputExt);
		printf("\tExpected:\t.bin\n");
	}

	//Parse target name and page size
	char* targetName = argv[TARGETNAME_ARG];
	if (strlen(targetName) > 16) {
		printf("Target name is too long!\n");
		err_code = -EINVAL;
		goto error;
	}

	char* pageSizeStr = argv[PAGESIZE_ARG];
	int pageSize;
	int numFound = sscanf(pageSizeStr, "%d", &pageSize);
	if (numFound != 1) {
		printf("Couldn't parse page size!\n");
		err_code = -EINVAL;
		goto error;
	}

	bool encrypt = false;

	char *keyFilePath = NULL;
	if (argc > KEYNAME_ARG) {
		encrypt = true;
		keyFilePath = argv[KEYNAME_ARG];
		/* Check if key name is proper */
		if (strlen(keyFilePath) >= MAX_KEYNAME_LENGTH) {
			printf("Key name is too long!\n");
			goto error;
		}


	}

//===========================Parse keys==========================
	if (encrypt) {
		FILE *keyFile = fopen(keyFilePath, "r");
		if (!keyFile) {
			printf("Couldn't open %s\n", keyFilePath);
			err_code = -EINVAL;
			goto error;
		}
		/* Parse AES KEY */
		err_code = parse_key_from_file(keyFile, &aes_key, AES_FILE_KEY_START, AES_FILE_KEY_END, &aes_size);
		if (err_code) {
			fclose(keyFile);
			goto error;
		}
		printf("Parsed %d byte aes key from key file %s\n", aes_size, keyFilePath);

		/* PARSE AAD Part */
		err_code = parse_key_from_file(keyFile, &aes_aad, AES_FILE_AAD_START, AES_FILE_KEY_END, &aad_size);
		if (err_code) {
			fclose(keyFile);
			goto error;
		}
		printf("Parsed %d byte aad from key file %s\n", aad_size, keyFilePath);

		fclose(keyFile);
	}

//=======================Open input and output binary files========================
	//Open input file
	fp_in = fopen(inputNameWithExt, "rb");
	if (!fp_in) {
		printf("Couldn't open %s\n", inputNameWithExt);
		err_code = -EINVAL;
		goto error;
	}
	ferror(fp_in);

	//Open main output file
	int outputNameLength = strlen(inputNameNoExt) + strlen(OUTPUT_EXT) + 1;
	char* outputNameWithExt = (char*)malloc(outputNameLength);
	strncpy(outputNameWithExt, inputNameNoExt, outputNameLength);
	strncpy((outputNameWithExt + strlen(inputNameNoExt)),
			OUTPUT_EXT,
			(outputNameLength - strlen(inputNameNoExt)));

	printf("Output Files:\n");
	printf("\t%s\n", outputNameWithExt);


	fp_out = fopen(outputNameWithExt, "wb");
	if (!fp_out) {
		printf("Couldn't open %s\n", outputNameWithExt);
		err_code = -EINVAL;
		goto error;
	}
	ferror(fp_out);


	//Open the boot memory output file
	char* bootMemSuffix = "_bootmem";
	char* bootMemExt = ".bin";
	int bootMemFileNameLength = strlen(inputNameNoExt) + strlen(bootMemSuffix) + strlen(bootMemExt);
	char* bootMemFileName = (char*)malloc(bootMemFileNameLength + 1);
	strncpy(bootMemFileName, inputNameNoExt, bootMemFileNameLength);
	strncat(bootMemFileName, bootMemSuffix, bootMemFileNameLength - strlen(inputNameNoExt));
	strncat(bootMemFileName, bootMemExt, bootMemFileNameLength - strlen(inputNameNoExt) - strlen(bootMemSuffix));
	printf("\t%s\n", bootMemFileName);

	fp_out_bootmem = fopen(bootMemFileName, "wb");
	if (!fp_out_bootmem) {
		printf("Couldn't open %s\n", bootMemFileName);
		err_code = -EINVAL;
		goto error;
	}
	ferror(fp_out_bootmem);

//=======================MSBL File Header========================
	//Fill in fileHeader with what we have so far
	msbl_header fileHeader = {};
	strncpy(fileHeader.magic, MAGIC_VAL, sizeof(fileHeader.magic));
	fileHeader.formatVersion = FILE_FORMAT_VERSION;
	strncpy(fileHeader.target, targetName, sizeof(fileHeader.target));
	if (encrypt) {
		printf("Encryption type: %s\n", ENCRYPT_TYPE_AES);
		strncpy(fileHeader.enc_type, ENCRYPT_TYPE_AES, sizeof(fileHeader.enc_type));
	} else {
		printf("Encryption type: plaintext\n");
		memset(fileHeader.enc_type, 0, sizeof(fileHeader.enc_type));
	}
	fileHeader.pageSize = pageSize;
	fileHeader.crcSize = CHECK_BYTESIZE;

	//calculate a random IV/Nonce
	int nidx;

	if (encrypt) {
#if defined(WIN32)
		int err;
		unsigned int randomVal;
		for (nidx = 0; nidx < AES_NONCE_SIZE; nidx++) {
			err = rand_s(&randomVal);
			fileHeader.nonce[nidx] = (uint8_t)(randomVal % 256);
			if (err)
				printf("WARNING: rand_s returned %d\n", err);
		}
#elif defined(UNIX)
		FILE* fp_rand = fopen("/dev/urandom", "rb");
		if (!fp_rand) {
			printf("Couldn't open /dev/urandom\n");
			err_code = -EINVAL;
			goto error;
		}

		for (nidx = 0; nidx < AES_NONCE_SIZE; nidx++) {
			fread(&fileHeader.nonce[nidx], 1, 1, fp_rand);
		}

		fclose(fp_rand);
#else
		srand(time(NULL));
		for (nidx = 0; nidx < AES_NONCE_SIZE; nidx++) {
			fileHeader.nonce[nidx] = (uint8_t)(rand() % 256);
		}
#endif
	} else {
		memset(fileHeader.nonce, 0, sizeof(fileHeader.nonce));
	}

	//Read size of input file in pages, parse number of pages
	fseek(fp_in, 0, SEEK_END);
	long int lInputFileSize = ftell(fp_in);
	fseek(fp_in, 0, SEEK_SET);

	if (lInputFileSize + pageSize > MAXFILESIZE) {
		printf("Input file is too large!\n");
		err_code = -EFBIG;
		goto error;
	}

	uint32_t inputFileSize = (uint32_t)lInputFileSize;
	printf("Input File Size: %d\n", inputFileSize);
	uint32_t inputSizeWithHeader = inputFileSize + pageSize;
	fileHeader.numPages = (uint16_t)((inputSizeWithHeader + (pageSize - 1)) / pageSize);

	printf("Num pages: %d\n", fileHeader.numPages);


//=======================Boot memory (contains app header)========================
	//Malloc buffer needed for application
	uint8_t* inputFileData = (uint8_t*)malloc(fileHeader.numPages * pageSize);
	memset(inputFileData, 0, fileHeader.numPages * pageSize);

	//Read in the .bin input file to memory
	fread(inputFileData, inputFileSize, 1, fp_in);
	int ferr = ferror(fp_in);
	if (ferr) {
		printf("Got file error %d reading from input file!\n", ferr);
		err_code = -EIO;
		goto error;
	}

	//Calculate CRC-32 of entire input
	uint32_t ahLength = inputFileSize;
	uint32_t ahCRC32 = crcCalc(inputFileData, inputFileSize);

	//Set boot_mem values (force to little endian)
	app_header_t bootMem = {0, };

	uint8_t* ptr = (uint8_t*)(&bootMem.length);
	*ptr++ = (ahLength >> 0) & 0xFF;
	*ptr++ = (ahLength >> 8) & 0xFF;
	*ptr++ = (ahLength >> 16) & 0xFF;
	*ptr++ = (ahLength >> 24) & 0xFF;

	ptr = (uint8_t*)(&bootMem.CRC32);
	*ptr++ = (ahCRC32 >> 0) & 0xFF;
	*ptr++ = (ahCRC32 >> 8) & 0xFF;
	*ptr++ = (ahCRC32 >> 16) & 0xFF;
	*ptr++ = (ahCRC32 >> 24) & 0xFF;

	//validMark must be zero in our encrypted file
	bootMem.validMark = 0;

	//Copy app_header_t into the end of inputFileData
	//printf("appheader copied to offset %X\n", pageSize * (fileHeader.numPages - 1));
	memcpy(inputFileData + pageSize * (fileHeader.numPages - 1),
		&bootMem, sizeof(app_header_t));


	//validMark must actually be valid for our plain text bootmem file
	bootMem.validMark = MARK_VALID_MAGIC_VAL;
	ptr = (uint8_t*)(&bootMem.validMark);
	*ptr++ = (MARK_VALID_MAGIC_VAL >> 0) & 0xFF;
	*ptr++ = (MARK_VALID_MAGIC_VAL >> 8) & 0xFF;
	*ptr++ = (MARK_VALID_MAGIC_VAL >> 16) & 0xFF;
	*ptr++ = (MARK_VALID_MAGIC_VAL >> 24) & 0xFF;

	//Write app_header_t into fp_out_bootmem. Fill rest of bootmem page with 0s
	uint8_t zero = 0;
	//int bytesLeftOver = pageSize - sizeof(app_header_t);
	fwrite(&bootMem, sizeof(app_header_t), 1, fp_out_bootmem);
#if 0
	while (bytesLeftOver-- > 0)
		fwrite(&zero, sizeof(zero), 1, fp_out_bootmem);
#endif

	//Close bootmem output file
	ferr = ferror(fp_out_bootmem);
	if (ferr) {
		printf("Got file error %d writing to output!\n", ferr);
		err_code = -EIO;
		goto error;
	}
	fclose(fp_out_bootmem);


//=======================Application data========================
	//Create buffer for our page data
	uint8_t* pageAndCRCData =
		(uint8_t*)malloc(pageSize + CHECK_BYTESIZE + PAD_CHECKBYTE_TO_16 + (ALIGN4 - 1));

	//Calculate size of the entire encrypted application in file (page data plus page crcs)
	uint32_t entireAppSize = fileHeader.numPages * (pageSize + CHECK_BYTESIZE + PAD_CHECKBYTE_TO_16);
	uint8_t* allAppData = (uint8_t*)malloc(entireAppSize);
	uint8_t* encAllAppData = (uint8_t*)malloc(entireAppSize);


	int pageIdx;
	int crcIdx;
	uint32_t pageCRC;
	uint32_t inputFileIdx;
	for (pageIdx = 0; pageIdx < fileHeader.numPages; pageIdx++) {
		//Clear buffer
		memset(pageAndCRCData, 0, pageSize + CHECK_BYTESIZE + PAD_CHECKBYTE_TO_16);

		//Copy input file data into page buffer
		memcpy(pageAndCRCData,
				&inputFileData[pageIdx*pageSize],
				pageSize);

		//Calculate crc of page data
		pageCRC = crcCalc(pageAndCRCData, pageSize);
		//printf("page %d CRC: 0x%X\n", pageIdx, pageCRC);

		//Copy CRC into our page buffer (Little Endian)
		for (crcIdx = 0; crcIdx < CHECK_BYTESIZE; crcIdx++) {
			pageAndCRCData[pageSize + crcIdx] =
				(pageCRC >> (8 * crcIdx) & 0xFF);
		}

		//Copy into allAppData buffer
		memcpy(allAppData + (pageIdx * (pageSize + CHECK_BYTESIZE + PAD_CHECKBYTE_TO_16)),
				pageAndCRCData,
				(pageSize + CHECK_BYTESIZE + PAD_CHECKBYTE_TO_16));
	}

	if (encrypt) {
		//Initialize AES encryption
		aes_init(entireAppSize, fileHeader.nonce, aes_key, aes_aad, aad_size);
		//Encrypt everything at once
		aes_encrypt(allAppData, entireAppSize, encAllAppData);
		//Finialize to get authentication value
		aes_finalize(fileHeader.auth);
	} else {
		memset(fileHeader.auth, 0, sizeof(fileHeader.auth));
	}

//=======================Write MSBL header and application data========================
	fwrite(&fileHeader, sizeof(msbl_header), 1, fp_out);

	if (encrypt) {
		fwrite(encAllAppData, entireAppSize, 1, fp_out);
	} else {
		fwrite(allAppData, entireAppSize, 1, fp_out);
	}

	//Check there were no writing errors
	ferr = ferror(fp_out);
	if (ferr) {
		printf("Got file error %d writing to output!\n", ferr);
		err_code = -EIO;
		goto error;
	}

	fclose(fp_out);

//=======================Append MSBL CRC32========================
	//Calculate CRC32 of outputfile
	fp_out = fopen(outputNameWithExt, "rb");
	if (!fp_out) {
		printf("Couldn't open %s\n", outputNameWithExt);
		err_code = -EINVAL;
		goto error;
	}
	ferror(fp_out);

	fseek(fp_out, 0, SEEK_END);
	long int lEncFileSize = ftell(fp_out);
	fseek(fp_out, 0, SEEK_SET);

	if (lEncFileSize > MAXFILESIZE) {
		printf("Ouput file is too large!\n");
		err_code = -EFBIG;
		goto error;
	}
	int encFileSize = (uint32_t)lEncFileSize;
	uint8_t* encFileData = (uint8_t*)malloc(encFileSize);
	fread(encFileData, encFileSize, 1, fp_out);
	//printf("lencFileSize=%ld\n", lEncFileSize);

	uint32_t encFileCRC32 = crcCalc(encFileData, encFileSize);
	printf("Output file CRC: 0x%X\n", encFileCRC32);


	//Close encrypted file and open for appending
	fclose(fp_out);
	fp_out = fopen(outputNameWithExt, "a");
	if (!fp_out) {
		printf("Couldn't open %s\n", outputNameWithExt);
		err_code = -EINVAL;
		goto error;
	}
	ferror(fp_out);

	//Append CRC (little endian)
	for (crcIdx = 0; crcIdx < CHECK_BYTESIZE; crcIdx++)
		fputc((encFileCRC32 >> (8 * crcIdx) & 0xFF), fp_out);

	ferr = ferror(fp_out);
	if (ferr) {
		printf("Got file error %d writing to output!\n", ferr);
		err_code = -EIO;
		goto error;
	}

//=======================Cleanup========================
	if (fp_out)
		fclose(fp_out);
	if (fp_in)
		fclose(fp_in);
	return 0;

error:

	return err_code;
}
