#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"



int pairs[32][2]; // = {hard coded pairs}



void useage();
void errExit(const char szFmt[], ...);
void handle_hide(char *, char *);
void handle_extract(char *, int);



void main(int argc, char **argv)
{

	int size;

	if (argc != 4 || argv[1][0] != '-' || strlen(argv[1]) != 2)
		useage();


	switch(argv[1][1])
	{
		case 'h':
			// check if correct data types
			handle_hide(argv[2], argv[3]);
			break;

		case 'e':
			size = atoi(argv[3]);

			if (/*didn't work*/)
			{
				printf("Size must be integer\n");
				useage();
			}

			handle_extract(argv[2], size);
			break;

		default:
			useage();
			break;
	}
}




void handle_hide(char *image_name, char *message_name)
{

	FILE *image, *message, *output_image;

	if ((image = fopen(image_name, "rb")) == NULL)
		errExit("Couldn't open %s\n", image_name);

	if ((message = fopen(message_name, "r")) == NULL)
		errExit("Couldn't open %s\n", message_name);

	// maybe some more set up stuff, setting things up as a stream

	hide(/*some parameters*/);

}




void handle_extract(char *image_name, int size)
{

	FILE *image, *output;

	if ((image = fopen(image_name, "rb")) == NULL)
		errExit("Couldn't open %s\n", image_name);

	if ((output = fopen("extracted_message.txt", "w")) == NULL)
		errExit("Couldn't create/open extracted_message.txt");

	// maybe some set up stuff, stream stuff

	extract(/*paramaters*/);

}


// this is assuming that data is an array of 8x8 blocks, each in an array
// also assumes that this function returns the message
void extract(int data[][], int len) {

    int i, j, *pair, msgCount = 0, *block, c, shift;

    char * msg = calloc(len);

    len = len * 8; // len should be character length, and this algo is by bit

    for (j = 0; msgCount < len; j++) {

        block = data[j]

        for (i = 0; i < 32 && msgCount < len; i++) {

            pair = pairs[i]

            if (block[pair[0]] == block[pair[1]])
                continue;

            msgCount++;

            if (block[pair[0]] > block[pair[1]]) {
                c = msgCount / 8;
                shift = msgCount % 8;
                msg[c] = msg[c] & (1 >> (shift - 1));
            }
            // otherwise the message bit is 0, keep it the same
        }

    }

    return msg;

}


void hide()
{


}




void useage()
{
	printf("Useage:\n\n");
	printf("Hiding: \n\t'swapdct -h image.jpg message.txt'\n\n");
	printf("Extracting: \n\t'swapdct -e image.jpg message_size(integer)'\n\n");
	exit(1);
}



void errExit(const char szFmt[], ... )
{
    va_list args;               // This is the standard C variable argument list type
    va_start(args, szFmt);      // This tells the compiler where the variable arguments
                                // begins.  They begin after szFmt.
    printf("ERROR: ");
    vprintf(szFmt, args);       // vprintf receives a printf format string and  a
                                // va_list argument
    va_end(args);               // let the C environment know we are finished with the
                                // va_list argument
    printf("\n");
    exit(1);
}
