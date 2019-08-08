/* 
 * Copyright (C) 2012-2014 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <makestuff.h>
#include <libfpgalink.h>
#include <libbuffer.h>
#include <liberror.h>
#include <libdump.h>
#include <argtable2.h>
#include <readline/readline.h>
#include <readline/history.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include<unistd.h>
#endif

bool sigIsRaised(void);
void sigRegisterHandler(void);
void wait_timer(int t){
	printf("%s%d%s\n", "Will wait for ",t," seconds...");
	for(int i=0;i<t;++i){
		printf("%d%c\r", t-i,' ');
		sleep(1);
	}
	printf("\n");
}
bool checkAck1(bool* b){
	bool ack1[32]={0,1,0,0,0,1,0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,1,1,1};
	bool result=true;
	for(int i=0;i<32;++i){
		if(ack1[i]!=b[i]){
			result=false;
		}
	}
	return result;
}
void StrToBool(bool* b, char* s, int length){
	for(int i=0;i<length;++i){
		if(s[i]=='0'){
			b[i]=0;
		}
		else{
			b[i]=1;
		}
	}
}
uint8 ToByte(bool* b)
{
    uint8 c = 0;
    int x=1;
    for (int i=0; i < 8; ++i){
    	if(b[7-i]){
    		c=c+x;
    	}
	    	x=x*2;
    }
    return c;
}
void getBool3(uint8 x, bool* b){
	for (int i = 0; i < 3; i++)
	{
		b[2 - i] = x%2;
		x = x/2;
	}
}
void getBool8(uint8 x, bool* b){
	for (int i = 0; i < 8; i++)
	{
		b[7 - i] = x%2;
		x = x/2;
	}
}
void getHex(char*c,bool*b){
    if(!b[0]&&!b[1]&&!b[2]&&!b[3]){
        c[0]='0';
    }
    else if(!b[0]&&!b[1]&&!b[2]&&b[3]){
        c[0]='1';
    }
    else if(!b[0]&&!b[1]&&b[2]&&!b[3]){
        c[0]='2';
    }
    else if(!b[0]&&!b[1]&&b[2]&&b[3]){
        c[0]='3';
    }
    else if(!b[0]&&b[1]&&!b[2]&&!b[3]){
        c[0]='4';
    }
    else if(!b[0]&&b[1]&&!b[2]&&b[3]){
        c[0]='5';
    }
    else if(!b[0]&&b[1]&&b[2]&&!b[3]){
        c[0]='6';
    }
    else if(!b[0]&&b[1]&&b[2]&&b[3]){
        c[0]='7';
    }
    else if(b[0]&&!b[1]&&!b[2]&&!b[3]){
        c[0]='8';
    }
    else if(b[0]&&!b[1]&&!b[2]&&b[3]){
        c[0]='9';
    }
    else if(b[0]&&!b[1]&&b[2]&&!b[3]){
        c[0]='A';
    }
    else if(b[0]&&!b[1]&&b[2]&&b[3]){
        c[0]='B';
    }
    else if(b[0]&&b[1]&&!b[2]&&!b[3]){
        c[0]='C';
    }
    else if(b[0]&&b[1]&&!b[2]&&b[3]){
        c[0]='D';
    }
    else if(b[0]&&b[1]&&b[2]&&!b[3]){
        c[0]='E';
    }
    else {
        c[0]='F';
    }

    if(!b[4]&&!b[5]&&!b[6]&&!b[7]){
        c[1]='0';
    }
    else if(!b[4]&&!b[5]&&!b[6]&&b[7]){
        c[1]='1';
    }
    else if(!b[4]&&!b[5]&&b[6]&&!b[7]){
        c[1]='2';
    }
    else if(!b[4]&&!b[5]&&b[6]&&b[7]){
        c[1]='3';
    }
    else if(!b[4]&&b[5]&&!b[6]&&!b[7]){
        c[1]='4';
    }
    else if(!b[4]&&b[5]&&!b[6]&&b[7]){
        c[1]='5';
    }
    else if(!b[4]&&b[5]&&b[6]&&!b[7]){
        c[1]='6';
    }
    else if(!b[4]&&b[5]&&b[6]&&b[7]){
        c[1]='7';
    }
    else if(b[4]&&!b[5]&&!b[6]&&!b[7]){
        c[1]='8';
    }
    else if(b[4]&&!b[5]&&!b[6]&&b[7]){
        c[1]='9';
    }
    else if(b[4]&&!b[5]&&b[6]&&!b[7]){
        c[1]='A';
    }
    else if(b[4]&&!b[5]&&b[6]&&b[7]){
        c[1]='B';
    }
    else if(b[4]&&b[5]&&!b[6]&&!b[7]){
        c[1]='C';
    }
    else if(b[4]&&b[5]&&!b[6]&&b[7]){
        c[1]='D';
    }
    else if(b[4]&&b[5]&&b[6]&&!b[7]){
        c[1]='E';
    }
    else {
        c[1]='F';
    }
}

void getHexString(char *s,bool* b,uint8 chan){
	s[0]='w';
	bool b2[8];
	if(chan<16){
		getBool8(chan,b2);
		getHex(s,b2);
		s[0]='w';
		s[2]=' ';
		for(int i=0;i<4;++i){
			getHex(&s[2*i+3],&b[8*i]);
		}
		s[11]='\0';
	}
	else{
		getBool8(chan,b2);
		getHex(&s[1],b2);		
		s[3]=' ';
		for(int i=0;i<4;++i){
			getHex(&s[2*i+4],&b[8*i]);
		}
		s[12]='\0';
	}
}
void decrypt(bool *C){
	bool K[32]={1,0,0,1,0,1,0,1,0,1,1,0,0,1,0,0,1,0,0,0,1,0,1,1,1,0,0,0,0,0,1,0};
	int N=0;
	for(int i=0;i<32;++i){
		if(K[i]==0){
			N=N+1;
		}
	}
	bool T[4];
	T[0]=K[0]^K[4]^K[8]^K[12]^K[16]^K[20]^K[24]^K[28];
	T[1]=K[1]^K[5]^K[9]^K[13]^K[17]^K[21]^K[25]^K[29];
	T[2]=K[2]^K[6]^K[10]^K[14]^K[18]^K[22]^K[26]^K[30];
	T[3]=K[3]^K[7]^K[11]^K[15]^K[19]^K[23]^K[27]^K[31];
	
	bool t[4];
	for(int i=0;i<4;++i){
		t[i]=T[i];
	}
	T[3]=!t[3];
	T[2]=!t[2]^t[3];
	T[1]=!t[1]^(t[2]||(t[3]&&!t[2]));
	T[0]=!t[0]^(t[1]||(!t[1]&&(t[2]||(t[3]&&!t[2]))));
	while (N>0){
		for(int i=0;i<32;++i){
			C[i]=C[i]^T[i%4];
		}
		bool t[4];
		for(int i=0;i<4;++i){
			t[i]=T[i];
		}
		T[3]=!t[3];
		T[2]=!t[2]^t[3];
		T[1]=!t[1]^(t[2]||(t[3]&&!t[2]));
		T[0]=!t[0]^(t[1]||(!t[1]&&(t[2]||(t[3]&&!t[2]))));
	    N=N-1;
	}
}
void encrypt(bool *C){
	bool K[32]={1,0,0,1,0,1,0,1,0,1,1,0,0,1,0,0,1,0,0,0,1,0,1,1,1,0,0,0,0,0,1,0};
	int N=0;
	for(int i=0;i<32;++i){
		if(K[i]==1){
			N=N+1;
		}
	}
	bool T[4];
	T[0]=K[0]^K[4]^K[8]^K[12]^K[16]^K[20]^K[24]^K[28];
	T[1]=K[1]^K[5]^K[9]^K[13]^K[17]^K[21]^K[25]^K[29];
	T[2]=K[2]^K[6]^K[10]^K[14]^K[18]^K[22]^K[26]^K[30];
	T[3]=K[3]^K[7]^K[11]^K[15]^K[19]^K[23]^K[27]^K[31];
	while (N>0){
		for(int i=0;i<32;++i){
			C[i]=C[i]^T[i%4];
		}
		int rightzero=3;
		for(;T[rightzero]!=0;rightzero--){
			;
		}
		for(int i=rightzero;i<4;++i){
			T[i]=!T[i];
		}
	    N=N-1;
	}
}

static const char *ptr;
static bool enableBenchmarking = false;

static bool isHexDigit(char ch) {
	return
		(ch >= '0' && ch <= '9') ||
		(ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F');
}

static uint16 calcChecksum(const uint8 *data, size_t length) {
	uint16 cksum = 0x0000;
	while ( length-- ) {
		cksum = (uint16)(cksum + *data++);
	}
	return cksum;
}

static bool getHexNibble(char hexDigit, uint8 *nibble) {
	if ( hexDigit >= '0' && hexDigit <= '9' ) {
		*nibble = (uint8)(hexDigit - '0');
		return false;
	} else if ( hexDigit >= 'a' && hexDigit <= 'f' ) {
		*nibble = (uint8)(hexDigit - 'a' + 10);
		return false;
	} else if ( hexDigit >= 'A' && hexDigit <= 'F' ) {
		*nibble = (uint8)(hexDigit - 'A' + 10);
		return false;
	} else {
		return true;
	}
}

static int getHexByte(uint8 *byte) {
	uint8 upperNibble;
	uint8 lowerNibble;
	if ( !getHexNibble(ptr[0], &upperNibble) && !getHexNibble(ptr[1], &lowerNibble) ) {
		*byte = (uint8)((upperNibble << 4) | lowerNibble);
		byte += 2;
		return 0;
	} else {
		return 1;
	}
}

static const char *const errMessages[] = {
	NULL,
	NULL,
	"Unparseable hex number",
	"Channel out of range",
	"Conduit out of range",
	"Illegal character",
	"Unterminated string",
	"No memory",
	"Empty string",
	"Odd number of digits",
	"Cannot load file",
	"Cannot save file",
	"Bad arguments"
};

typedef enum {
	FLP_SUCCESS,
	FLP_LIBERR,
	FLP_BAD_HEX,
	FLP_CHAN_RANGE,
	FLP_CONDUIT_RANGE,
	FLP_ILL_CHAR,
	FLP_UNTERM_STRING,
	FLP_NO_MEMORY,
	FLP_EMPTY_STRING,
	FLP_ODD_DIGITS,
	FLP_CANNOT_LOAD,
	FLP_CANNOT_SAVE,
	FLP_ARGS
} ReturnCode;

static ReturnCode doRead(
	struct FLContext *handle, uint8 chan, uint32 length, FILE *destFile, uint16 *checksum,
	const char **error)
{
	ReturnCode retVal = FLP_SUCCESS;
	uint32 bytesWritten;
	FLStatus fStatus;
	uint32 chunkSize;
	const uint8 *recvData;
	uint32 actualLength;
	const uint8 *ptr;
	uint16 csVal = 0x0000;
	#define READ_MAX 65536

	// Read first chunk
	chunkSize = length >= READ_MAX ? READ_MAX : length;
	fStatus = flReadChannelAsyncSubmit(handle, chan, chunkSize, NULL, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
	length = length - chunkSize;

	while ( length ) {
		// Read chunk N
		chunkSize = length >= READ_MAX ? READ_MAX : length;
		fStatus = flReadChannelAsyncSubmit(handle, chan, chunkSize, NULL, error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
		length = length - chunkSize;
		
		// Await chunk N-1
		fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");

		// Write chunk N-1 to file
		bytesWritten = (uint32)fwrite(recvData, 1, actualLength, destFile);
		CHECK_STATUS(bytesWritten != actualLength, FLP_CANNOT_SAVE, cleanup, "doRead()");

		// Checksum chunk N-1
		chunkSize = actualLength;
		ptr = recvData;
		while ( chunkSize-- ) {
			csVal = (uint16)(csVal + *ptr++);
		}
	}

	// Await last chunk
	fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
	
	// Write last chunk to file
	bytesWritten = (uint32)fwrite(recvData, 1, actualLength, destFile);
	CHECK_STATUS(bytesWritten != actualLength, FLP_CANNOT_SAVE, cleanup, "doRead()");

	// Checksum last chunk
	chunkSize = actualLength;
	ptr = recvData;
	while ( chunkSize-- ) {
		csVal = (uint16)(csVal + *ptr++);
	}
	
	// Return checksum to caller
	*checksum = csVal;
cleanup:
	return retVal;
}

static ReturnCode doWrite(
	struct FLContext *handle, uint8 chan, FILE *srcFile, size_t *length, uint16 *checksum,
	const char **error)
{
	ReturnCode retVal = FLP_SUCCESS;
	size_t bytesRead, i;
	FLStatus fStatus;
	const uint8 *ptr;
	uint16 csVal = 0x0000;
	size_t lenVal = 0;
	#define WRITE_MAX (65536 - 5)
	uint8 buffer[WRITE_MAX];

	do {
		// Read Nth chunk
		bytesRead = fread(buffer, 1, WRITE_MAX, srcFile);
		if ( bytesRead ) {
			// Update running total
			lenVal = lenVal + bytesRead;

			// Submit Nth chunk
			fStatus = flWriteChannelAsync(handle, chan, bytesRead, buffer, error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doWrite()");

			// Checksum Nth chunk
			i = bytesRead;
			ptr = buffer;
			while ( i-- ) {
				csVal = (uint16)(csVal + *ptr++);
			}
		}
	} while ( bytesRead == WRITE_MAX );

	// Wait for writes to be received. This is optional, but it's only fair if we're benchmarking to
	// actually wait for the work to be completed.
	fStatus = flAwaitAsyncWrites(handle, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doWrite()");

	// Return checksum & length to caller
	*checksum = csVal;
	*length = lenVal;
cleanup:
	return retVal;
}

static int parseLine(struct FLContext *handle, const char *line, const char **error) {
	ReturnCode retVal = FLP_SUCCESS, status;
	FLStatus fStatus;
	struct Buffer dataFromFPGA = {0,};
	BufferStatus bStatus;
	uint8 *data = NULL;
	char *fileName = NULL;
	FILE *file = NULL;
	double totalTime, speed;
	#ifdef WIN32
		LARGE_INTEGER tvStart, tvEnd, freq;
		DWORD_PTR mask = 1;
		SetThreadAffinityMask(GetCurrentThread(), mask);
		QueryPerformanceFrequency(&freq);
	#else
		struct timeval tvStart, tvEnd;
		long long startTime, endTime;
	#endif
	bStatus = bufInitialise(&dataFromFPGA, 1024, 0x00, error);
	CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
	ptr = line;
	do {
		while ( *ptr == ';' ) {
			ptr++;
		}
		switch ( *ptr ) {
		case 'r':{
			uint32 chan;
			uint32 length = 1;
			char *end;
			ptr++;
			
			// Get the channel to be read:
			errno = 0;
			chan = (uint32)strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(chan > 127, FLP_CHAN_RANGE, cleanup);
			ptr = end;

			// Only three valid chars at this point:
			CHECK_STATUS(*ptr != '\0' && *ptr != ';' && *ptr != ' ', FLP_ILL_CHAR, cleanup);

			if ( *ptr == ' ' ) {
				ptr++;

				// Get the read count:
				errno = 0;
				length = (uint32)strtoul(ptr, &end, 16);
				CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);
				ptr = end;
				
				// Only three valid chars at this point:
				CHECK_STATUS(*ptr != '\0' && *ptr != ';' && *ptr != ' ', FLP_ILL_CHAR, cleanup);
				if ( *ptr == ' ' ) {
					const char *p;
					const char quoteChar = *++ptr;
					CHECK_STATUS(
						(quoteChar != '"' && quoteChar != '\''),
						FLP_ILL_CHAR, cleanup);
					
					// Get the file to write bytes to:
					ptr++;
					p = ptr;
					while ( *p != quoteChar && *p != '\0' ) {
						p++;
					}
					CHECK_STATUS(*p == '\0', FLP_UNTERM_STRING, cleanup);
					fileName = malloc((size_t)(p - ptr + 1));
					CHECK_STATUS(!fileName, FLP_NO_MEMORY, cleanup);
					CHECK_STATUS(p - ptr == 0, FLP_EMPTY_STRING, cleanup);
					strncpy(fileName, ptr, (size_t)(p - ptr));
					fileName[p - ptr] = '\0';
					ptr = p + 1;
				}
			}
			if ( fileName ) {
				uint16 checksum = 0x0000;

				// Open file for writing
				file = fopen(fileName, "wb");
				CHECK_STATUS(!file, FLP_CANNOT_SAVE, cleanup);
				free(fileName);
				fileName = NULL;

				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					status = doRead(handle, (uint8)chan, length, file, &checksum, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					status = doRead(handle, (uint8)chan, length, file, &checksum, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Read %d bytes (checksum 0x%04X) from channel %d at %f MiB/s\n",
						length, checksum, chan, speed);
				}
				CHECK_STATUS(status, status, cleanup);

				// Close the file
				fclose(file);
				file = NULL;
			} else {
				size_t oldLength = dataFromFPGA.length;
				bStatus = bufAppendConst(&dataFromFPGA, 0x00, length, error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					fStatus = flReadChannel(handle, (uint8)chan, length, dataFromFPGA.data + oldLength, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					fStatus = flReadChannel(handle, (uint8)chan, length, dataFromFPGA.data + oldLength, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Read %d bytes (checksum 0x%04X) from channel %d at %f MiB/s\n",
						length, calcChecksum(dataFromFPGA.data + oldLength, length), chan, speed);
				}
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			}
			break;
		}
		case 'w':{
			unsigned long int chan;
			size_t length = 1, i;
			char *end, ch;
			const char *p;
			ptr++;
			
			// Get the channel to be written:
			errno = 0;
			chan = strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(chan > 127, FLP_CHAN_RANGE, cleanup);
			ptr = end;

			// There must be a space now:
			CHECK_STATUS(*ptr != ' ', FLP_ILL_CHAR, cleanup);

			// Now either a quote or a hex digit
		   ch = *++ptr;
			if ( ch == '"' || ch == '\'' ) {
				uint16 checksum = 0x0000;

				// Get the file to read bytes from:
				ptr++;
				p = ptr;
				while ( *p != ch && *p != '\0' ) {
					p++;
				}
				CHECK_STATUS(*p == '\0', FLP_UNTERM_STRING, cleanup);
				fileName = malloc((size_t)(p - ptr + 1));
				CHECK_STATUS(!fileName, FLP_NO_MEMORY, cleanup);
				CHECK_STATUS(p - ptr == 0, FLP_EMPTY_STRING, cleanup);
				strncpy(fileName, ptr, (size_t)(p - ptr));
				fileName[p - ptr] = '\0';
				ptr = p + 1;  // skip over closing quote

				// Open file for reading
				file = fopen(fileName, "rb");
				CHECK_STATUS(!file, FLP_CANNOT_LOAD, cleanup);
				free(fileName);
				fileName = NULL;
				
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					status = doWrite(handle, (uint8)chan, file, &length, &checksum, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					status = doWrite(handle, (uint8)chan, file, &length, &checksum, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Wrote "PFSZD" bytes (checksum 0x%04X) to channel %lu at %f MiB/s\n",
						length, checksum, chan, speed);
				}
				CHECK_STATUS(status, status, cleanup);

				// Close the file
				fclose(file);
				file = NULL;
			} else if ( isHexDigit(ch) ) {
				// Read a sequence of hex bytes to write
				uint8 *dataPtr;
				p = ptr + 1;
				while ( isHexDigit(*p) ) {
					p++;
				}
				CHECK_STATUS((p - ptr) & 1, FLP_ODD_DIGITS, cleanup);
				length = (size_t)(p - ptr) / 2;
				data = malloc(length);
				dataPtr = data;
				for ( i = 0; i < length; i++ ) {
					getHexByte(dataPtr++);
					ptr += 2;
				}
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					fStatus = flWriteChannel(handle, (uint8)chan, length, data, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					fStatus = flWriteChannel(handle, (uint8)chan, length, data, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Wrote "PFSZD" bytes (checksum 0x%04X) to channel %lu at %f MiB/s\n",
						length, calcChecksum(data, length), chan, speed);
				}
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				free(data);
				data = NULL;
			} else {
				FAIL(FLP_ILL_CHAR, cleanup);
			}
			break;
		}
		case '+':{
			uint32 conduit;
			char *end;
			ptr++;

			// Get the conduit
			errno = 0;
			conduit = (uint32)strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(conduit > 255, FLP_CONDUIT_RANGE, cleanup);
			ptr = end;

			// Only two valid chars at this point:
			CHECK_STATUS(*ptr != '\0' && *ptr != ';', FLP_ILL_CHAR, cleanup);

			fStatus = flSelectConduit(handle, (uint8)conduit, error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			break;
		}
		default:
			FAIL(FLP_ILL_CHAR, cleanup);
		}
	} while ( *ptr == ';' );
	CHECK_STATUS(*ptr != '\0', FLP_ILL_CHAR, cleanup);

	dump(0x00000000, dataFromFPGA.data, dataFromFPGA.length);

cleanup:
	bufDestroy(&dataFromFPGA);
	if ( file ) {
		fclose(file);
	}
	free(fileName);
	free(data);
	if ( retVal > FLP_LIBERR ) {
		const int column = (int)(ptr - line);
		int i;
		fprintf(stderr, "%s at column %d\n  %s\n  ", errMessages[retVal], column, line);
		for ( i = 0; i < column; i++ ) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "^\n");
	}
	return retVal;
}


static const char *nibbles[] = {
	"0000",  // '0'
	"0001",  // '1'
	"0010",  // '2'
	"0011",  // '3'
	"0100",  // '4'
	"0101",  // '5'
	"0110",  // '6'
	"0111",  // '7'
	"1000",  // '8'
	"1001",  // '9'

	"XXXX",  // ':'
	"XXXX",  // ';'
	"XXXX",  // '<'
	"XXXX",  // '='
	"XXXX",  // '>'
	"XXXX",  // '?'
	"XXXX",  // '@'

	"1010",  // 'A'
	"1011",  // 'B'
	"1100",  // 'C'
	"1101",  // 'D'
	"1110",  // 'E'
	"1111"   // 'F'
};

int main(int argc, char *argv[]) {
	ReturnCode retVal = FLP_SUCCESS, pStatus;
	struct arg_str *ivpOpt = arg_str0("i", "ivp", "<VID:PID>", "            vendor ID and product ID (e.g 04B4:8613)");
	struct arg_str *vpOpt = arg_str1("v", "vp", "<VID:PID[:DID]>", "       VID, PID and opt. dev ID (e.g 1D50:602B:0001)");
	struct arg_str *fwOpt = arg_str0("f", "fw", "<firmware.hex>", "        firmware to RAM-load (or use std fw)");
	struct arg_str *portOpt = arg_str0("d", "ports", "<bitCfg[,bitCfg]*>", " read/write digital ports (e.g B13+,C1-,B2?)");
	struct arg_str *queryOpt = arg_str0("q", "query", "<jtagBits>", "         query the JTAG chain");
	struct arg_str *progOpt = arg_str0("p", "program", "<config>", "         program a device");
	struct arg_uint *conOpt = arg_uint0("c", "conduit", "<conduit>", "        which comm conduit to choose (default 0x01)");
	struct arg_str *actOpt = arg_str0("a", "action", "<actionString>", "    a series of CommFPGA actions");
	struct arg_lit *shellOpt  = arg_lit0("s", "shell", "                    start up an interactive CommFPGA session");
	struct arg_lit *benOpt  = arg_lit0("b", "benchmark", "                enable benchmarking & checksumming");
	struct arg_lit *rstOpt  = arg_lit0("r", "reset", "                    reset the bulk endpoints");
	struct arg_str *dumpOpt = arg_str0("l", "dumploop", "<ch:file.bin>", "   write data from channel ch to file");
	struct arg_lit *helpOpt  = arg_lit0("h", "help", "                     print this help and exit");
	struct arg_str *eepromOpt  = arg_str0(NULL, "eeprom", "<std|fw.hex|fw.iic>", "   write firmware to FX2's EEPROM (!!)");
	struct arg_str *backupOpt  = arg_str0(NULL, "backup", "<kbitSize:fw.iic>", "     backup FX2's EEPROM (e.g 128:fw.iic)\n");
	struct arg_end *endOpt   = arg_end(20);
	struct arg_lit *yash = arg_lit0("y",NULL,"yash's custom option");
	void *argTable[] = {
		ivpOpt, vpOpt, fwOpt, portOpt, queryOpt, progOpt, conOpt, actOpt,
		shellOpt, benOpt, rstOpt, dumpOpt, helpOpt, eepromOpt, backupOpt, yash,endOpt
	};
	const char *progName = "flcli";
	int numErrors;
	struct FLContext *handle = NULL;
	FLStatus fStatus;
	const char *error = NULL;
	const char *ivp = NULL;
	const char *vp = NULL;
	bool isNeroCapable, isCommCapable;
	uint32 numDevices, scanChain[16], i;
	const char *line = NULL;
	uint8 conduit = 0x01;

	if ( arg_nullcheck(argTable) != 0 ) {
		fprintf(stderr, "%s: insufficient memory\n", progName);
		FAIL(1, cleanup);
	}

	numErrors = arg_parse(argc, argv, argTable);

	if ( helpOpt->count > 0 ) {
		printf("FPGALink Command-Line Interface Copyright (C) 2012-2014 Chris McClelland\n\nUsage: %s", progName);
		arg_print_syntax(stdout, argTable, "\n");
		printf("\nInteract with an FPGALink device.\n\n");
		arg_print_glossary(stdout, argTable,"  %-10s %s\n");
		FAIL(FLP_SUCCESS, cleanup);
	}

	if ( numErrors > 0 ) {
		arg_print_errors(stdout, endOpt, progName);
		fprintf(stderr, "Try '%s --help' for more information.\n", progName);
		FAIL(FLP_ARGS, cleanup);
	}

	fStatus = flInitialise(0, &error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

	vp = vpOpt->sval[0];

	printf("Attempting to open connection to FPGALink device %s...\n", vp);
	fStatus = flOpen(vp, &handle, NULL);
	if ( fStatus ) {
		if ( ivpOpt->count ) {
			int count = 60;
			uint8 flag;
			ivp = ivpOpt->sval[0];
			printf("Loading firmware into %s...\n", ivp);
			if ( fwOpt->count ) {
				fStatus = flLoadCustomFirmware(ivp, fwOpt->sval[0], &error);
			} else {
				fStatus = flLoadStandardFirmware(ivp, vp, &error);
			}
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			
			printf("Awaiting renumeration");
			flSleep(1000);
			do {
				printf(".");
				fflush(stdout);
				fStatus = flIsDeviceAvailable(vp, &flag, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				flSleep(250);
				count--;
			} while ( !flag && count );
			printf("\n");
			if ( !flag ) {
				fprintf(stderr, "FPGALink device did not renumerate properly as %s\n", vp);
				FAIL(FLP_LIBERR, cleanup);
			}

			printf("Attempting to open connection to FPGLink device %s again...\n", vp);
			fStatus = flOpen(vp, &handle, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		} else {
			fprintf(stderr, "Could not open FPGALink device at %s and no initial VID:PID was supplied\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	printf(
		"Connected to FPGALink device %s (firmwareID: 0x%04X, firmwareVersion: 0x%08X)\n",
		vp, flGetFirmwareID(handle), flGetFirmwareVersion(handle)
	);

	if ( eepromOpt->count ) {
		if ( !strcmp("std", eepromOpt->sval[0]) ) {
			printf("Writing the standard FPGALink firmware to the FX2's EEPROM...\n");
			fStatus = flFlashStandardFirmware(handle, vp, &error);
		} else {
			printf("Writing custom FPGALink firmware from %s to the FX2's EEPROM...\n", eepromOpt->sval[0]);
			fStatus = flFlashCustomFirmware(handle, eepromOpt->sval[0], &error);
		}
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( backupOpt->count ) {
		const char *fileName;
		const uint32 kbitSize = strtoul(backupOpt->sval[0], (char**)&fileName, 0);
		if ( *fileName != ':' ) {
			fprintf(stderr, "%s: invalid argument to option --backup=<kbitSize:fw.iic>\n", progName);
			FAIL(FLP_ARGS, cleanup);
		}
		fileName++;
		printf("Saving a backup of %d kbit from the FX2's EEPROM to %s...\n", kbitSize, fileName);
		fStatus = flSaveFirmware(handle, kbitSize, fileName, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( rstOpt->count ) {
		// Reset the bulk endpoints (only needed in some virtualised environments)
		fStatus = flResetToggle(handle, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( conOpt->count ) {
		conduit = (uint8)conOpt->ival[0];
	}

	isNeroCapable = flIsNeroCapable(handle);
	isCommCapable = flIsCommCapable(handle, conduit);

	if ( portOpt->count ) {
		uint32 readState;
		char hex[9];
		const uint8 *p = (const uint8 *)hex;
		printf("Configuring ports...\n");
		fStatus = flMultiBitPortAccess(handle, portOpt->sval[0], &readState, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		sprintf(hex, "%08X", readState);
		printf("Readback:   28   24   20   16    12    8    4    0\n          %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf("  %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s\n", nibbles[*p++ - '0']);
		flSleep(100);
	}

	if ( queryOpt->count ) {
		if ( isNeroCapable ) {
			fStatus = flSelectConduit(handle, 0x00, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = jtagScanChain(handle, queryOpt->sval[0], &numDevices, scanChain, 16, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( numDevices ) {
				printf("The FPGALink device at %s scanned its JTAG chain, yielding:\n", vp);
				for ( i = 0; i < numDevices; i++ ) {
					printf("  0x%08X\n", scanChain[i]);
				}
			} else {
				printf("The FPGALink device at %s scanned its JTAG chain but did not find any attached devices\n", vp);
			}
		} else {
			fprintf(stderr, "JTAG chain scan requested but FPGALink device at %s does not support NeroProg\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( progOpt->count ) {
		printf("Programming device...\n");
		if ( isNeroCapable ) {
			fStatus = flSelectConduit(handle, 0x00, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flProgram(handle, progOpt->sval[0], NULL, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		} else {
			fprintf(stderr, "Program operation requested but device at %s does not support NeroProg\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( benOpt->count ) {
		enableBenchmarking = true;
	}
	
	if ( actOpt->count ) {
		printf("Executing CommFPGA actions on FPGALink device %s...\n", vp);
		if ( isCommCapable ) {
			uint8 isRunning;
			fStatus = flSelectConduit(handle, conduit, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, actOpt->sval[0], &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
			} else {
				fprintf(stderr, "The FPGALink device at %s is not ready to talk - did you forget --program?\n", vp);
				FAIL(FLP_ARGS, cleanup);
			}
		} else {
			fprintf(stderr, "Action requested but device at %s does not support CommFPGA\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( dumpOpt->count ) {
		const char *fileName;
		unsigned long chan = strtoul(dumpOpt->sval[0], (char**)&fileName, 10);
		FILE *file = NULL;
		const uint8 *recvData;
		uint32 actualLength;
		if ( *fileName != ':' ) {
			fprintf(stderr, "%s: invalid argument to option -l|--dumploop=<ch:file.bin>\n", progName);
			FAIL(FLP_ARGS, cleanup);
		}
		fileName++;
		printf("Copying from channel %lu to %s", chan, fileName);
		file = fopen(fileName, "wb");
		CHECK_STATUS(!file, FLP_CANNOT_SAVE, cleanup);
		sigRegisterHandler();
		fStatus = flSelectConduit(handle, conduit, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		fStatus = flReadChannelAsyncSubmit(handle, (uint8)chan, 22528, NULL, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		do {
			fStatus = flReadChannelAsyncSubmit(handle, (uint8)chan, 22528, NULL, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fwrite(recvData, 1, actualLength, file);
			printf(".");
		} while ( !sigIsRaised() );
		printf("\nCaught SIGINT, quitting...\n");
		fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		fwrite(recvData, 1, actualLength, file);
		fclose(file);
	}

	if ( shellOpt->count ) {
		printf("\nEntering CommFPGA command-line mode:\n");
		if ( isCommCapable ) {
		   uint8 isRunning;
			fStatus = flSelectConduit(handle, conduit, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				do {
					do {
						line = readline("> ");
					} while ( line && !line[0] );
					if ( line && line[0] && line[0] != 'q' ) {
						add_history(line);
						pStatus = parseLine(handle, line, &error);
						CHECK_STATUS(pStatus, pStatus, cleanup);
						free((void*)line);
					}
				} while ( line && line[0] != 'q' );
			} else {
				fprintf(stderr, "The FPGALink device at %s is not ready to talk - did you forget --xsvf?\n", vp);
				FAIL(FLP_ARGS, cleanup);
			}
		} else {
			fprintf(stderr, "Shell requested but device at %s does not support CommFPGA\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}
	if(yash->count){
		uint8 isRunning;	
		setbuf(stdout, NULL);	//tells OS to not buffer standard output so that printf works immediately
		// system("sudo gtkterm -p /dev/ttyXRUSB0 -s 2400 &"); // opens gtkterm shell
		printf("\nEntering custom mode for CS254 LAB:\n");
		bool read_file = true;
		uint8 ichan=0;	//channel number intially set to 0
		int xco,yco;	//coordinates to be read from board
	    bool arr[64];	//data table
	    int wait_time;
		//overall loop with 32 second timeout at the end
		while(true){
			//loop that iterates over channel numbers
			while(true){
				//read encrypted coordinates from channel 2*i
				struct Buffer dataFromFPGA = {0,};
				BufferStatus bStatus;
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

				bStatus = bufInitialise(&dataFromFPGA, 1024, 0x00, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				size_t oldLength = dataFromFPGA.length;
				bStatus = bufAppendConst(&dataFromFPGA, 0x00, 1, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				fStatus = flReadChannel(handle, 2*ichan, 5, dataFromFPGA.data + oldLength, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				uint8 data1=*(dataFromFPGA.data+(dataFromFPGA.length));
				uint8 data2=*(dataFromFPGA.data+(dataFromFPGA.length+1));
				uint8 data3=*(dataFromFPGA.data+(dataFromFPGA.length+2));
				uint8 data4=*(dataFromFPGA.data+(dataFromFPGA.length+3));
				bool coords[32];
				getBool8(data1,&coords[0]);
				getBool8(data2,&coords[8]);
				getBool8(data3,&coords[16]);
				getBool8(data4,&coords[24]);
				//decrypt coordinates
				decrypt(coords);

				data4=ToByte(&coords[24]);
				int data4c=ToByte(&coords[24]);
				xco = data4>>4;
				yco = data4c & ((uint8)15);
			   printf("\n%s%d%s%d%s%d%s\n","Read encrypted coordinates from channel ",2*ichan," as (",xco,",",yco,")");
			    //put coordinates back on channel 2*i+1
			   encrypt(coords);
			   char strco[13];
			   getHexString(strco,coords,2*ichan+1);
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				if ( isRunning ) {
					pStatus = parseLine(handle, strco, &error);
					CHECK_STATUS(pStatus, pStatus, cleanup);
					fStatus = flSelectConduit(handle, 0x01, &error);
					CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
					fStatus = flIsFPGARunning(handle, &isRunning, &error);
					CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				} 
				else {
					FAIL(FLP_ARGS, cleanup);
				}
				printf("%s%d\n","Put encrpyted coordinates back on channel ",2*ichan+1);

				//try to read ack1 from channel 2*i
				struct Buffer ddataFromFPGA = {0,};
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

				bStatus = bufInitialise(&ddataFromFPGA, 1024, 0x00, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				oldLength = ddataFromFPGA.length;
				bStatus = bufAppendConst(&ddataFromFPGA, 0x00, 1, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				fStatus = flReadChannel(handle, 2*ichan, 5, ddataFromFPGA.data + oldLength, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				data1=*(ddataFromFPGA.data+(ddataFromFPGA.length));
				data2=*(ddataFromFPGA.data+(ddataFromFPGA.length+1));
				data3=*(ddataFromFPGA.data+(ddataFromFPGA.length+2));
				data4=*(ddataFromFPGA.data+(ddataFromFPGA.length+3));
				bool ack1[32];
				getBool8(data1,&ack1[0]);	
				getBool8(data2,&ack1[8]);	
				getBool8(data3,&ack1[16]);	
				getBool8(data4,&ack1[24]);	
				decrypt(ack1);
				bool ack1rec=checkAck1(ack1);
				if(ack1rec){
					printf("%s%d\n", "received ack1 on channel ",2*ichan);
					break;
				}
				printf("%s%d%s\n", "Did not detect ack1 on channel ",2*ichan,", will retry...");
				wait_timer(5);
				//again try to read ack1 from channel 2*i
				struct Buffer dddataFromFPGA = {0,};
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

				bStatus = bufInitialise(&dddataFromFPGA, 1024, 0x00, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				oldLength = dddataFromFPGA.length;
				bStatus = bufAppendConst(&dddataFromFPGA, 0x00, 1, &error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				fStatus = flReadChannel(handle, 2*ichan, 5, dddataFromFPGA.data + oldLength, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				data1=*(dddataFromFPGA.data+(dddataFromFPGA.length));
				data2=*(dddataFromFPGA.data+(dddataFromFPGA.length+1));
				data3=*(dddataFromFPGA.data+(dddataFromFPGA.length+2));
				data4=*(dddataFromFPGA.data+(dddataFromFPGA.length+3));
				getBool8(data1,&ack1[0]);	
				getBool8(data2,&ack1[8]);	
				getBool8(data3,&ack1[16]);	
				getBool8(data4,&ack1[24]);	
				decrypt(ack1);
				ack1rec=checkAck1(ack1);
				if(ack1rec){
					printf("%s%d\n", "received ack1 on channel ",2*ichan);
					break;
				}
				else{
					printf("%s%d%s\n", "Did not detect ack1 on channel ",2*ichan,", moving on to next channel...");
					if(ichan==63){
						ichan=0;
					}
					else{
						ichan=ichan+1;
					}
				}
			}
			//at this point we have ichan as the channel number that board is using
			//sending reverse-ack on channel 2*i+1
			bool ack2[32]={0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,0,1,0,0};
			encrypt(ack2);
			char strack2[13];
		   	getHexString(strack2,ack2,2*ichan+1);
			fStatus = flSelectConduit(handle, 0x01, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, strack2, &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			} 
			else {
				FAIL(FLP_ARGS, cleanup);
			}
			printf("%s%d\n","put ack2 on channel ",2*ichan+1 );
			/// now read data from the file 
		  	if(read_file){
			  	FILE *fptr;
			    fptr = fopen("/home/guptaji/network.txt", "r");
			    if(fptr==NULL){
			    	printf("%s\n","Could not open the file" );
			    }
			    int n=0;
			    int x,y,dir,ok,ns;
			    for(int i=0;i<8;++i){
			        arr[8*i]=0;
			    }
			    while (fscanf(fptr, "%d%*c%d%*c%d%*c%d%*c%d",&x,&y,&dir,&ok,&ns) != EOF && n<8) {
			        if(x==xco&&y==yco){
			            arr[dir*8]=1;
			            arr[dir*8+1]=ok;
			            getBool3(dir,&arr[8*dir+2]);
			            getBool3(ns,&arr[8*dir+5]);
			            n++;
			        }
			    }
			    fclose(fptr);     
			    for(int i=0;i<8;++i){
			        if(arr[8*i]==0){
			        	arr[8*i+1]=0;
			        	getBool3(i,&arr[8*i+2]);
			        	getBool3(0,&arr[8*i+5]);
			        }
			    }
			    encrypt(arr);
			    encrypt(&arr[32]);
			    read_file = false;
			}
		    char strdata1[13];
		    char strdata2[13];
		    getHexString(strdata1,arr,2*ichan+1);
		    getHexString(strdata2,&arr[32],2*ichan+1);
		    //write first 4 bytes
		    fStatus = flSelectConduit(handle, 0x01, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, strdata1, &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			} 
			else {
				FAIL(FLP_ARGS, cleanup);
			}
			printf("%s%d\n", "put first 4 bytes of data on channel ",2*ichan+1);
			//loop for checking ack1 and timeout after 256 seconds
			bool goBack=false;
			for(int i=1;i<=256;++i){
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

				fStatus=flReadChannelAsyncSubmit(handle,2*ichan,5,NULL,&error);	
				CHECK_STATUS(fStatus, fStatus, cleanup);

				const uint8 *recvData;
				uint32 reqLength;
				uint32 actLength;

				fStatus=flReadChannelAsyncAwait(handle,&recvData,&reqLength,&actLength,&error);
				CHECK_STATUS(fStatus, fStatus, cleanup);
				uint8 data1=recvData[1];
				uint8 data2=recvData[2];
				uint8 data3=recvData[3];
				uint8 data4=recvData[4];
				bool ack[32];
				getBool8(data1,ack);
				getBool8(data2,&ack[8]);
				getBool8(data3,&ack[16]);
				getBool8(data4,&ack[24]);
				decrypt(ack);
				bool ack1rec=checkAck1(ack);
				if(ack1rec){
					goBack=false;
					break;
				}
				else{
					goBack=true;
					printf("%s%d%s\n","Did not detect ack1 on attempt ",i,", retrying..." );
					sleep(1);
				}
			}
			if(goBack){
				printf("%s\n", "TIMEOUT: Did not recieve ack1 after 256 attempts, going back...");
				ichan=0;
				continue;
			}
			printf("%s%d\n", "received ack1 on channel ",2*ichan);
			 //write last 4 bytes
		    fStatus = flSelectConduit(handle, 0x01, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, strdata2, &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			} 
			else {
				FAIL(FLP_ARGS, cleanup);
			}
			printf("%s%d\n", "put last 4 bytes of data on channel ",2*ichan+1);
			//loop for checking ack1 and timeout after 256 seconds
			goBack=false;
			for(int i=1;i<=256;++i){
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

				fStatus=flReadChannelAsyncSubmit(handle,2*ichan,5,NULL,&error);	
				CHECK_STATUS(fStatus, fStatus, cleanup);

				const uint8 *recvData;
				uint32 reqLength;
				uint32 actLength;

				fStatus=flReadChannelAsyncAwait(handle,&recvData,&reqLength,&actLength,&error);
				CHECK_STATUS(fStatus, fStatus, cleanup);
				uint8 data1=recvData[1];
				uint8 data2=recvData[2];
				uint8 data3=recvData[3];
				uint8 data4=recvData[4];
				bool ack[32];
				getBool8(data1,ack);
				getBool8(data2,&ack[8]);
				getBool8(data3,&ack[16]);
				getBool8(data4,&ack[24]);
				decrypt(ack);
				bool ack1rec=checkAck1(ack);
				if(ack1rec){
					goBack=false;
					break;
				}
				else{
					goBack=true;
					printf("%s%d%s\n","Did not detect ack1 on attempt ",i,", retrying..." );
					sleep(1);
				}
			}
			if(goBack){
				printf("%s\n", "TIMEOUT: Did not recieve ack1 after 256 attempts, going back...");
				ichan=0;
				continue;
			}			
			printf("%s%d\n", "received ack1 on channel ",2*ichan);
			//sending reverse-ack on channel 2*i+1
			fStatus = flSelectConduit(handle, 0x01, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, strack2, &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
				fStatus = flSelectConduit(handle, 0x01, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				fStatus = flIsFPGARunning(handle, &isRunning, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			} 
			else {
				FAIL(FLP_ARGS, cleanup);
			}
			printf("%s%d\n","put ack2 on channel ",2*ichan+1 );
			wait_timer(24);

			fStatus = flSelectConduit(handle, 0x01, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			for(int i=1;i<31;++i){
				fStatus=flReadChannelAsyncSubmit(handle,2*ichan,5,NULL,&error);	
				if(fStatus==FLP_SUCCESS){
					const uint8 *recvData;
					uint32 reqLength;
					uint32 actLength;

					fStatus=flReadChannelAsyncAwait(handle,&recvData,&reqLength,&actLength,&error);
					if(fStatus==FLP_SUCCESS){
						uint8 data1=recvData[1];
						uint8 data2=recvData[2];
						uint8 data3=recvData[3];
						uint8 data4=recvData[4];

						bool track[32];
						getBool8(data1,track);
						getBool8(data2,&track[8]);
						getBool8(data3,&track[16]);
						getBool8(data4,&track[24]);
						decrypt(track);
						data1=ToByte(track);
						data2=ToByte(&track[8]);
						data3=ToByte(&track[16]);
						data4=ToByte(&track[24]);
						if(data1==0 && data2==0 && data3==0){
							printf("%d\t%d\t%d\t%d\t\n", data1,data2,data3,data4);
							printf("%s\n", "Received data about track from controller!");
							int direction_no=4*track[26]+2*track[27]+1*track[28];
							decrypt(arr);
							decrypt(&arr[32]);
							if(track[24] && arr[8*direction_no]){
								printf("%s%d\n", "Updating stored track information about direction ",direction_no);
								for(int i=1;i<8;++i){
									arr[8*direction_no+i]=track[24+i];
								}
							}
							encrypt(arr);
							encrypt(&arr[32]);
							wait_time=30+i;
							break;
						}
						else{
							wait_time=30;
							sleep(1);
						}
					}
				}
			}
		wait_timer(10+wait_time);
		ichan =0;
		}	
	}

cleanup:
	free((void*)line);
	flClose(handle);
	if ( error ) {
		fprintf(stderr, "%s\n", error);
		flFreeError(error);
	}
	return retVal;
}

