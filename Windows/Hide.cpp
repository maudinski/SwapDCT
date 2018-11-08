// Hide.cpp
// 
// This file contains all the routines to Embed/Extract Message Data
//
#include "Main.h"


char tmpMessage[] = "Hello World! This is my message. Want it to be long to test the program.";

char gOutputFileName[260];
unsigned char gBitMask1[8]    = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
unsigned short gBitMask1_2[8] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
unsigned short gBitMask2_2[8] = { 0xFFFE, 0xFFFC, 0xFFF8, 0xFFF0, 0xFFE0, 0xFFC0, 0xFF80, 0xFF00 };
//unsigned char gBitMask3[8] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
unsigned int gBitCapacity = 0;

char *gMsgBuffer = NULL;
unsigned int gMsgSize = 0;
double gAlpha = 1.0;			// jpenquan.h
double gUniformityFactor = 1.2;
unsigned int gImageQuality = 50;
bool gExtractMsg = false;
bool gDestroyMsg = false;
bool gWipeMsg = false;
bool gHideMsg = false;
double lg2; // lg2 = natural log 2
double lgQ; // lg2 of quality factor

// encoding/decoding objects
JpegEncoder gJpgEncoder;
JpegDecoder gJpgDecoder;
BmpEncoder gBitmapEncoder;
BmpDecoder gBitmapDecoder;
BitmapImage gImage;

typedef union BITFIELD
{
	unsigned char cByte;	// Used for printing byte value.
	struct {				// Used for storing bits.
		unsigned char bit1 : 1;
		unsigned char bit2 : 1;
		unsigned char bit3 : 1;
		unsigned char bit4 : 1;
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
		unsigned char bit8 : 1;
	} BYTE;
} BITFIELD;

// This function gets numBits from the message starting from the last position
// up until the end of the message (numBits range from 1 to 8)
unsigned char getBitsFromBuffer(unsigned int numBits, unsigned char *inBuffer, unsigned int inBufferLength)
{

	return(0);
} // getBitsFromBuffer

// this function gets the Uniformity Factor
double getUniformity_E(JpegEncoderCoefficientBlock data)
{
	int row, col;
	short tmpData;

	for(row = 0; row < JpegSampleWidth; row++)
		for(col = 0; col < JpegSampleWidth; col++)
			tmpData = data[row][col];

	return(0);
} // getUniformity_E

// hide the data in a block of coefficients
void hideInBlock(JpegEncoderCoefficientBlock *data, JpegEncoderQuantizationTable &qt)
{
	// check for simple conversions - no hiding/extracting
    if(gHideMsg == false) return;
	if (gHidingLevel == NULL)	// Assert hiding level
		gHidingLevel = 20;
    

	int i, store;
	unsigned int temp;
	static char startBoolean = 1;
    static char bitCount = 1;
    static unsigned int byteCount = 0; 

	// This array is used for indexing which pairs will be swapped.
	// The first 32 values are paired with the second 32 values accordingly.
    static unsigned char indices[64] = {55,47,46,45,39,38,37,31,30,29,28,27,23,22,21,20,19,15,14,13,12,11,10, 9, 7, 6, 5, 4, 3, 2,1, 0 
                                       ,62,61,53,54,60,52,44,59,51,43,35,36,58,50,42,34,26,57,49,41,33,25,17,18,56,48,40,32,24,16,8,63};                        
	static BITFIELD byteData;

    // Return if message is completely stored already.
    if (byteCount >= gMsgSize) return;
	
    // Grab byte. 
	if (startBoolean == 1)	
	{	// First read.
		memset(&byteData, 0, 1);	// Initialize just in case (probably not needed).
		memcpy(&byteData, &gMsgBuffer[byteCount + 3 + sizeof(gOutputFileName)], sizeof(byteData));
		startBoolean = 0;
		gHidingLevel--;	// Adjust flag number for array indexing
	}
	else
	{	// Resumes at static byteCount.
		memcpy(&byteData, &gMsgBuffer[byteCount], sizeof(byteData));
	}	
		
    // Process byte.
	for (i = 0; i < gHidingLevel; i++)
	{
		// Ensure we have a valid pair of coefficients to hide in.
		if ((*data)[indices[i]%8][indices[i]/8] == (*data)[indices[i + 32]%8][indices[i+32]/8])
			continue;
		
		// Grab current bit's value.
		switch (bitCount)
		{
		case 1:
			store = (byteData.BYTE.bit1 > 0) ? 1 : 0;
			break;
		case 2:
			store = (byteData.BYTE.bit2 > 0) ? 1 : 0;
			break;
		case 3:
			store = (byteData.BYTE.bit3 > 0) ? 1 : 0;
			break;
		case 4:
			store = (byteData.BYTE.bit4 > 0) ? 1 : 0;
			break;
		case 5:
			store = (byteData.BYTE.bit5 > 0) ? 1 : 0;
			break;
		case 6:
			store = (byteData.BYTE.bit6 > 0) ? 1 : 0;
			break;
		case 7:
			store = (byteData.BYTE.bit7 > 0) ? 1 : 0;
			break;
		case 8: // Last bit in byte.
			store = (byteData.BYTE.bit8 > 0) ? 1 : 0;
			byteCount++;    // Update byte count because we finished this byte.
			//fprintf(stderr, "Finished storing: %c		Total stored: %d Bytes\n", byteData.cByte, byteCount);
			memcpy(&byteData, &gMsgBuffer[byteCount], sizeof(byteData));   // Read new byte.
			gBitCapacity = byteCount * 8;
			break;
		}   // End switch

		// Hide bit by ordering pairs.
		switch (store)
		{		// Note: The ugly %8's and /8's are accessing row and columns appropriately.
		case 1: // First Value > Second Value means 1.
			if ((*data)[indices[i]%8][indices[i]/8] < (*data)[indices[i+32]%8][indices[i+32]/8])
			{   // Swap.
				temp = (*data)[indices[i]%8][indices[i]/8];
				(*data)[indices[i]%8][indices[i]/8] = (*data)[indices[i+32]%8][indices[i+32]/8];
				(*data)[indices[i+32]%8][indices[i+32]/8] = temp;
				
			}
			break;
		case 0: // First Value < Second Value means 0.
			if ((*data)[indices[i]%8][indices[i]/8] > (*data)[indices[i+32]%8][indices[i+32]/8])
			{   // Swap.
				temp = (*data)[indices[i]%8][indices[i]/8];
				(*data)[indices[i]%8][indices[i]/8] = (*data)[indices[i+32]%8][indices[i+32]/8];
				(*data)[indices[i+32]%8][indices[i+32]/8] = temp;
			}
			break;
		}   // End switch
		
		// Update bit count.
		bitCount++;
		if (bitCount > 8)
			bitCount = 1;
	}   // End loop
    
    return;
} // hideInBlock

// takes the buffer, extracts a filename, and writes the rest of the data to disk
void writeMsg()
{
	return;
} // writeMsg

// uses the first 4 bytes in the message buffer to set the actual size of the message
void setMsgSize()
{
	return;
} // setMsgSize

// takes some number of bits and places them in a inBufferfer
int putBitsInBuffer(unsigned int numBits, unsigned char bits, unsigned char *outBuffer, unsigned int outBufferLength)
{
	return(SUCCESS);
} // putBitsInBuffer

// this function gets the Uniformity Factor
double getUniformity_D(JpegDecoderCoefficientBlock data)
{
	return(0);
} // getUniformity_D

// this function removes the bits from a block
void extractFromBlock(JpegDecoderCoefficientBlock data, const JpegDecoderQuantizationTable &qt)
{

    // check for simple conversions - no hiding/extracting
    if(gExtractMsg == false) return;
	if (gHidingLevel == NULL)	// Assert hiding level
		gHidingLevel = 20;
    
	int i, store;
    static char bitCount = 1;
	static unsigned int byteCount = 0;
	static FILE *gfptrOutputFile;
	static unsigned char startBoolean = 1;

	/* This array is used to index the coefficients.
	   The first 32 numbers represent the index which are paired with 
	   the 2nd 32 numbers. Thus, indices[i] + indices[i+32] are a pair. */
    static unsigned char indices[64] = {55,47,46,45,39,38,37,31,30,29,28,27,23,22,21,20,19,15,14,13,12,11,10, 9, 7, 6, 5, 4, 3, 2,1, 0 
                                       ,62,61,53,54,60,52,44,59,51,43,35,36,58,50,42,34,26,57,49,41,33,25,17,18,56,48,40,32,24,16,8,63};                        
    static BITFIELD byteData;	// Static in order to save fields between tables.
	
	if (startBoolean == 1)
	{
		gHidingLevel--;	// Adjust flag number for array indexing
		startBoolean = 0;
	}

    // Open file for write if needed.
	char out[] = "output.txt";
	if (gfptrOutputFile == NULL)
		gfptrOutputFile = fopen(out, "a+");	// Currently in append mode.
	if (gfptrOutputFile == NULL)
	{
		fprintf(stderr, "Could not open output file: %s\n\n", out);
		exit(1);
	}

    // Process data.
    for (i = 0; i< gHidingLevel; i++)
    {
        // Grab bit value.
		if (data[indices[i]%8][indices[i]/8] == data[indices[i+32]%8][indices[i+32]/8])
            continue;   // Skip those that are equal.
        else 
			store = (data[indices[i]%8][indices[i]/8] > data[indices[i+32]%8][indices[i+32]/8]) ? 1 : 0;
	
        // Place bit's value in bitfield.
        switch (bitCount)
        {  
            case 1:
                byteData.BYTE.bit1 = (store == 0) ? 0 : 1;
                break;
            case 2:
                byteData.BYTE.bit2 = (store == 0) ? 0 : 1;
                break;
            case 3:
                byteData.BYTE.bit3 = (store == 0) ? 0 : 1;
                break;
            case 4:
                byteData.BYTE.bit4 = (store == 0) ? 0 : 1;
                break;
            case 5:
                byteData.BYTE.bit5 = (store == 0) ? 0 : 1;
                break;
            case 6:
                byteData.BYTE.bit6 = (store == 0) ? 0 : 1;
                break;
            case 7:
                byteData.BYTE.bit7 = (store == 0) ? 0 : 1;
                break;
            case 8: // Last bit.
                byteData.BYTE.bit8 = (store == 0) ? 0 : 1;
                // Write byte.
				/*
				fprintf(stderr, "%d%d%d%d%d%d%d%d: %c\n"
					, byteData.BYTE.bit8
					, byteData.BYTE.bit7
					, byteData.BYTE.bit6
					, byteData.BYTE.bit5
					, byteData.BYTE.bit4
					, byteData.BYTE.bit3
					, byteData.BYTE.bit2
					, byteData.BYTE.bit1
					, byteData.cByte);
				//*/
				fwrite(&byteData, sizeof(byteData), 1, gfptrOutputFile);
				byteCount++;
				gBitCapacity = byteCount*8;
                break;
        }   // End switch
        
        // Update bit count.
        bitCount++;
        if (bitCount > 8) bitCount = 1;
        
    }   // End loop
        
    return;
} // extractFromBlock

/*
typedef struct _MESSAGE
{
	union
	{
		unsigned char bit1:1;
		unsigned char bit2:2;
		unsigned char bit3:3;
		unsigned char bit4:4;
		unsigned char bit5:5;
		unsigned char bit6:6;
		unsigned char bit7:7;
		unsigned char bit8:8;
	} BYTE;
} MESSAGE;

 bitfield read message
void readMsg()
{
	MESSAGE tMsg;

	tMsg.BYTE.bit8 = 0xE7;

	printf("1=%x, 2=%x, 3=%x, 4=%x, 5=%x, 6=%x, 7=%x, 8=%x \n", 
		tMsg.BYTE.bit1, tMsg.BYTE.bit2, tMsg.BYTE.bit3, tMsg.BYTE.bit4, 
		tMsg.BYTE.bit5, tMsg.BYTE.bit6, tMsg.BYTE.bit7, tMsg.BYTE.bit8);
	return;
}
//*/

