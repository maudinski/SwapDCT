struct jpeg_decompress_struct *initialize_jpeg(FILE *image)
{
	struct jpeg_decompress_struct *jpeg_info = malloc(sizeof(jpeg_decompress_struct));
	struct jpeg_error_mgr jerr;

	jpeg_info.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(jpeg_info);
	jpeg_stdio_src(jpeg_info, image);
	(void) jpeg_read_header(jpeg_info, TRUE); // why (void)? wtf
	return jpeg_info;
}

int *next_block(int *cc, int *x, int *y, jpeg_decompress_struct *jpeg_info, jvirt_barray_ptr *coefficients_array)
{
	jpeg_component_info* component_ptr = jpeg_info.comp_info + *cc; // color channel 0
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
		component_ptr = jpeg_info.comp_info + *cc;
		width = component_ptr->width_in_blocks; // not sure in necessary
		height = component_ptr->height_in_blocks; // ''
		*x = 0;
		*y = 0;
	}
	buffer = (jpeg_info.mem->access_virt_barray)((j_common_ptr)&jpeg_info,
		coefficients_array[*cc], *y, (JDIMENSION)1, FALSE);
	block = buffer[0][*x];

	return (int*) block;
}



/*
void handle_hide(char *image_name, char *message_name)
{
	/* file reading stuff
	FILE *image, *output_image;
	int message_fd;

	if ((image = fopen(image_name, "rb")) == NULL)
		errExit("Couldn't open %s\n", image_name);

	if ((message_fd = open(message_name, O_RDONLY)) == 0)
		errExit("Couldn't open %s\n", message_name);

	/* read file in as one single string, innefficient but easier
	int l = lseek(message_fd, 0, SEEK_END);
	char *msg = (char *) mmap(0, l, PROT_READ, MAP_PRIVATE, message_fd, 0);
	int *bit_stream = make_bitstream(msg); // really inneficient but fuck it

	/* setting up jpeg stuff

	struct jpeg_decompress_struct jpeg_info;
	struct jpeg_error_mgr jerr;

	jpeg_info.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jpeg_info);
	jpeg_stdio_src(&jpeg_info, image);
	(void) jpeg_read_header(&jpeg_info, TRUE);

	jvirt_barray_ptr *coefficients_array = jpeg_read_coefficients(&jpeg_info);

	JBLOCKARRAY buffer; // no idea
	JCOEFPTR block;
	jpeg_component_info* component_ptr = jpeg_info.comp_info; // color channel 0

	int cc = 0; // color component/channel, so 0-2
	int x = 0;  // block x position
	int y = 0; // block y position
	int ret = 0; // return value of hide
	int width = component_ptr->width_in_blocks; // width in blocks
	int height = component_ptr->height_in_blocks; // height in blocks;
	do
	{
		if (x < width) // next column
		{
			x++;
		}
		else if (y < height) // start of next row
		{
			x = 0;
			y++;
		}
		else // switch to the next component
		{
			cc++;
			component_ptr = jpeg_info.comp_info + cc;
			width = component_ptr->width_in_blocks; // not sure in necessary
			height = component_ptr->height_in_blocks; // ''
			x = 0;
			y = 0;
		}
		buffer = (jpeg_info.mem->access_virt_barray)((j_common_ptr)&jpeg_info,
			coefficients_array[cc], y, (JDIMENSION)1, FALSE);
		block = buffer[0][x];

		ret = hide((int *)block, bit_stream);
		bit_stream += ret;

	} while(ret != -1);

	close(message_fd);
	fclose(image);
	//free(bit_stream);

}
*/
