#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <iostream>
#include <string>

int write_jpeg_file(std::string outname,jpeg_decompress_struct in_cinfo, jvirt_barray_ptr *coeffs_array ){

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;

    if ((infile = fopen(outname.c_str(), "wb")) == NULL) {
      fprintf(stderr, "can't open %s\n", outname.c_str());
      return 0;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, infile);

    j_compress_ptr cinfo_ptr = &cinfo;
    jpeg_copy_critical_parameters((j_decompress_ptr)&in_cinfo,cinfo_ptr);
    jpeg_write_coefficients(cinfo_ptr, coeffs_array);

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
    fclose( infile );

    return 1;
}

int read_jpeg_file( std::string filename, std::string outname )
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;

    if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
      fprintf(stderr, "can't open %s\n", filename.c_str());
      return 0;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);


    jvirt_barray_ptr *coeffs_array = jpeg_read_coefficients(&cinfo);

    //change one dct:
    int ci = 0; // between 0 and number of image component
    int by = 0; // between 0 and compptr_one->height_in_blocks
    int bx = 0; // between 0 and compptr_one->width_in_blocks
    int bi = 0; // between 0 and 64 (8x8)
    JBLOCKARRAY buffer_one;
    JCOEFPTR blockptr_one;
    jpeg_component_info* compptr_one;
    compptr_one = cinfo.comp_info + ci; // getting the color channel pointer
    buffer_one = (cinfo.mem->access_virt_barray)((j_common_ptr)&cinfo, coeffs_array[ci], by, (JDIMENSION)1, FALSE);
    blockptr_one = buffer_one[0][bx];
    blockptr_one[bi]++;

    write_jpeg_file(outname, cinfo, coeffs_array);

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );

    return 1;


}

int main()
{
    std::string infilename = "you_image.jpg", outfilename = "out_image.jpg";

    /* Try opening a jpeg*/
    if( read_jpeg_file( infilename, outfilename ) > 0 )
    {
        std::cout << "It's Okay..." << std::endl;
    }
    else return -1;
    return 0;
}
