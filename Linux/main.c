#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdarg.h>


int pairs[32][2] = {{55, 62}, {47, 61}, {46, 53}, {45, 54}, {39, 60}, {38, 52}, {37, 44},
	{31, 59}, {30, 51}, {29, 43}, {28, 35}, {27, 36}, {23, 58}, {22, 50}, {21, 42},
	{20, 34}, {19, 26}, {15, 57}, {14, 49}, {13, 41}, {12, 33}, {11, 25}, {10, 17},
	{9, 18}, {7, 56}, {6, 48}, {5, 40}, {4, 32}, {3, 24}, {2, 16}, {1, 8}, {0, 63}};




void useage();
void errExit(const char szFmt[], ...);
void handle_hide(char *, char *);
void handle_extract(char *, int);

jpeg_decompress_struct *initialize_jpeg(FILE *image);

int hide(int *, int *);
int extract(int **, int);

void write_jpeg(FILE *file, jpeg_decompress_struct in_cinfo, jvirt_barray_ptr *coeffs_array);
int *make_bitstream(char *);

int *next_block(int *cc, int *x, int *y, jpeg_decompress_struct *jpeg_info, jvirt_barray_ptr *coefficients_array);


/*******************************************************************
********************************************************************/

int main(int argc, char **argv)
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

			//handle_extract(argv[2], size);
			break;

		default:
			useage();
			break;
	}

	return 0;
}


/*******************************************************************
********************************************************************/

void handle_hide(char *image_name, char *message_name)
{
	/* file reading stuff */
	FILE *image, *output_image;
	int message_fd;

	if ((image = fopen(image_name, "rb")) == NULL)
		errExit("Couldn't open %s\n", image_name);

	if ((message_fd = open(message_name, O_RDONLY)) == 0)
		errExit("Couldn't open %s\n", message_name);

	/* read file in as one single string, innefficient but easier */
	int l = lseek(message_fd, 0, SEEK_END);
	char *msg = (char *) mmap(0, l, PROT_READ, MAP_PRIVATE, message_fd, 0);
	int *bit_stream = make_bitstream(msg); // really inneficient but fuck it

	/* setting up jpeg stuff */
	jpeg_decompress_struct *jpeg_info = initialize_jpeg(image);
	//exit(1);
	jvirt_barray_ptr *coefficients_array = jpeg_read_coefficients(jpeg_info);
	//exit(1);
	int* block;

	int cc = 0; // color component/channel, so 0-2
	int x = 0;  // block x position
	int y = 0; // block y position
	int ret = 0; // return value of hide
	do
	{

		block = next_block(&cc, &x, &y, jpeg_info, coefficients_array);
		ret = hide(block, bit_stream);
		bit_stream += ret;
		//printf("here\n");

	} while(ret != -1);

	close(message_fd);
	fclose(image);

}

/*******************************************************************
Get's passes a single block at a time and a message string, then swap dct hides as much
as it can

returns:
	the amount hidden (0-32)
	-1	the message is ended


pair[0] > pair[1] means that bit is a 1
pair[0] < pair[1] means that bit is a 0
********************************************************************/
// something like that
int hide(int *block, int *bit_stream)
{
	int i, bits_hidden = 0, *pair, temp;

	for (i = 0; i < 32 && bit_stream[bits_hidden] != -1; i++)
	{
		pair = pairs[i];

		if (block[pair[0]] == block[pair[1]])
			continue;

		if (block[pair[0]] > block[pair[1]] && bit_stream[bits_hidden] == 0)
		{
			temp = block[pair[0]];
			block[pair[0]] = block[pair[1]];
			block[pair[1]] = temp;
		}

		bits_hidden += 1;

	}

	if (bit_stream[bits_hidden] == -1)
		return -1;

	return bits_hidden;

}

/*******************************************************************
creates a bit stream, end it with a -1
such a nasty function im sorry
********************************************************************/
int *make_bitstream(char *msg)
{
	int len = strlen(msg);
	int *bits = (int *) malloc(sizeof(int) * len * 8 + 1);
	int i, j;
	for (i = 0; i < len; i++)
	{
		for (j = 0; j < 8; j++)
			bits[(i * 8) + j] = (((int) msg[i]) & (1 << j)) != 0 ? 1 : 0;
	}
	bits[len] = -1;
	return bits;
}


/*******************************************************************
********************************************************************

void handle_extract(char *image_name, int size)
{

	FILE *image, *output;

	if ((image = fopen(image_name, "rb")) == NULL)
		errExit("Couldn't open %s\n", image_name);

	if ((output = fopen("extracted_message.txt", "w")) == NULL)
		errExit("Couldn't create/open extracted_message.txt");

	// maybe some set up stuff, stream stuff

	//dummy
	int **i, j;
	extract(i, j);

	//fclose();

}

/*******************************************************************
this is old, won't work like this anymore.
********************************************************************
// this is assuming that data is an array of 8x8 blocks, each in an array
// also assumes that this function returns the message
int extract(int **data, int len)
{

	int i, j, *pair, msgCount = 0, *block, c, shift;

	char * msg = (char *)calloc(1, len);

	len = len * 8; // len should be character length, and this algo is by bit

	for (j = 0; msgCount < len; j++)
	{

		block = data[j];
		// block = next_block(/*some parameters*);

		for (i = 0; i < 32 && msgCount < len; i++)
		{

			pair = pairs[i];

			if (block[pair[0]] == block[pair[1]])
				continue;

			msgCount++;

			if (block[pair[0]] > block[pair[1]])
			{
				c = msgCount / 8;
				shift = msgCount % 8;
				msg[c] = msg[c] & (1 >> (shift - 1));
			}
			// otherwise the message bit is 0, keep it the same
		}

	}

	//return msg;
	return 1;
}*/

/*******************************************************************
********************************************************************/

jpeg_decompress_struct *initialize_jpeg(FILE *image)
{
	jpeg_decompress_struct *jpeg_info = (jpeg_decompress_struct *)malloc(sizeof(jpeg_decompress_struct));

	jpeg_error_mgr *jerr = (jpeg_error_mgr *)malloc(sizeof(jpeg_error_mgr));

	jpeg_info->err = jpeg_std_error(jerr);
	jpeg_create_decompress(jpeg_info);
	jpeg_stdio_src(jpeg_info, image);
	(void) jpeg_read_header(jpeg_info, TRUE); // why (void)? wtf
	return jpeg_info;
}

/*******************************************************************
********************************************************************/

int *next_block(int *cc, int *x, int *y, jpeg_decompress_struct *jpeg_info, jvirt_barray_ptr *coefficients_array)
{
	jpeg_component_info* component_ptr = jpeg_info->comp_info + *cc; // color channel 0
	JBLOCKARRAY buffer; // no idea

	int width = component_ptr->width_in_blocks; // width in blocks
	int height = component_ptr->height_in_blocks; // height in blocks;

	if (*x < width) // next column
	{
		*x++;
	}
	else if (*y < height) // start of next row
	{
		*x = 0;
		*y++;
	}
	else // switch to the next component
	{
		*cc++;
		component_ptr = jpeg_info->comp_info + *cc;
		width = component_ptr->width_in_blocks; // not sure in necessary
		height = component_ptr->height_in_blocks; // ''
		*x = 0;
		*y = 0;
	}
	buffer = (jpeg_info->mem->access_virt_barray)((j_common_ptr)jpeg_info,
		coefficients_array[*cc], *y, (JDIMENSION)1, FALSE);

	JCOEFPTR block = buffer[0][*x];

	return (int*) block;
}


/*******************************************************************
writing a jpeg file
********************************************************************/

void write_jpeg(FILE *file, jpeg_decompress_struct in_cinfo, jvirt_barray_ptr *coeffs_array)
{
	struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, file);

        j_compress_ptr cinfo_ptr = &cinfo;
        jpeg_copy_critical_parameters((j_decompress_ptr)&in_cinfo,cinfo_ptr);
        jpeg_write_coefficients(cinfo_ptr, coeffs_array);

        jpeg_finish_compress( &cinfo );
        jpeg_destroy_compress( &cinfo );

}
/*******************************************************************
prints useage and exits
********************************************************************/

void useage()
{
	printf("Useage:\n\n");
	printf("Hiding: \n\t'swapdct -h image.jpg message.txt'\n\n");
	printf("Extracting: \n\t'swapdct -e image.jpg message_size(integer)'\n\n");
	exit(1);
}

/********************************************************************
printf with an exit
********************************************************************/

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

/*******************************************************************
********************************************************************/
