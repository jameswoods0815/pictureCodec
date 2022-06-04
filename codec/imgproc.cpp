#include <iostream>
#include <setjmp.h>
#include <jpeglib.h>

#include "imgproc.h"
#include "lodepng.h"
namespace ljj
{

static Mat_8UC3 read_PNG_file(const std::string &filename)
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	//decode
	unsigned error = lodepng::decode(image, width, height, filename);

	//if there's an error, display it
	if (error) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
	Mat_8UC3 img(width, height);

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			for (size_t c = 0; c < 3; c++) {
				img(x, y)(c) = image[(y * width + x) * 4 + c];
			}
		}
	}

	return img.transpose();
}

struct my_error_mgr {
        struct jpeg_error_mgr pub;                              /* "public" fields */
        jmp_buf setjmp_buffer;                                  /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
        /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
        my_error_ptr myerr = (my_error_ptr)cinfo->err;

        /* Always display the message. */
        /* We could postpone this until after returning, if we chose. */
        (*cinfo->err->output_message)(cinfo);

        /* Return control to the setjmp point */
        longjmp(myerr->setjmp_buffer, 1);
}

static Eigen::Matrix<Eigen::Matrix<uint8_t, 3, 1>, Eigen::Dynamic, Eigen::Dynamic> read_JPEG_file(const std::string &filename)
{
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct my_error_mgr jerr;
	/* More stuff */
	Mat_8UC3 ret;

	/* In this example we want to open the input file before doing anything else,
	 * so that the setjmp() error recovery below can assume the file is open.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to read binary files.
	 */

	FILE * infile = fopen(filename.c_str(), "rb");					/* source file */
	if (!infile) {
		std::cerr << "Could not open " << filename << "." << std::endl;
		return ret;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		std::cerr << "An error occurred while reading " << filename << "." << std::endl;
		return ret;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */

	/* Step 5: Start decompressor */

	jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* We may need to do some setup of our own at this point before reading
	 * the data.  After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */
	/* JSAMPLEs per row in output buffer */
	const int row_stride = cinfo.output_width * cinfo.output_components;	/* physical row width in output buffer */
	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);	/* Output row buffer */

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	if (cinfo.out_color_components != 3) {
		std::cerr << "Only three-channel images are supported. " << filename << " has " << cinfo.out_color_components << "." << std::endl;
		return ret;
	}

	const size_t w = cinfo.output_width;
	const size_t h = cinfo.output_height;

	ret.resize(w, h);

	for (size_t yi = 0; yi < h; yi++) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		JSAMPLE * pixelrow = buffer[0];

		assert((int)cinfo.output_scanline == yi);

		jpeg_read_scanlines(&cinfo, buffer, 1);

		for (size_t xi = 0; xi < w; xi++) {
			for (size_t ci = 0; ci < 3; ci++) {
				ret(xi, yi)(ci) = pixelrow[xi * 3 + ci];
			}
		}
	}

	/* Step 7: Finish decompression */

	jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	 * Here we postpone it until after no more JPEG errors are possible,
	 * so as to simplify the setjmp error logic above.  (Actually, I don't
	 * think that jpeg_destroy can do an error exit, but why assume anything...)
	 */
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */

	return ret.transpose();
}

std::string tolower_str(const std::string &s)
{
	std::string ret = s;

	for (size_t i = 0; i < s.size(); i++) {
		ret[i] = tolower(s[i]);
	}

	return ret;
}

Mat_8UC3 imread(const std::string &filename)
{
	const std::string suffix = filename.substr(filename.size() - 4);
	const std::string lower_suffix = tolower_str(suffix);
	const std::function<Mat_8UC3(const std::string &)> decoder = lower_suffix == ".jpg" ? read_JPEG_file :
	                                                             lower_suffix == ".png" ? read_PNG_file :
	                                                             nullptr;
	Mat_8UC3 img;

	if (decoder != nullptr) {
		img = decoder(filename);
	}

	return img;
}

Mat_8UC1 rgb2gray(const Mat_8UC3 &img)
{
	Mat_8UC1 ret(img.rows(), img.cols());

	for (size_t x = 0; x < img.cols(); x++) {
		for (size_t y = 0; y < img.rows(); y++) {
			ret(y, x) = ((uint16_t)img(y, x)(0) + (uint16_t)img(y, x)(1) + (uint16_t)img(y, x)(2)) / 3;
		}
	}

	return ret;
}

Mat_8UC3 gray2rgb(const Mat_8UC1 &img)
{
	Mat_8UC3 ret(img.rows(), img.cols());

	for (size_t x = 0; x < img.cols(); x++) {
		for (size_t y = 0; y < img.rows(); y++) {
			ret(y, x).fill(img(y, x));
		}
	}

	return ret;
}

Mat_64FC3 rgb2ycbcr(const Mat_8UC3 &img)
{
	Mat_64FC3 ret(img.rows(), img.cols());

	for (size_t x = 0; x < img.cols(); x++) {
		for (size_t y = 0; y < img.rows(); y++) {
			ret(y, x)(0) =   0.0 + ( 0.299 * img(y, x)(0) + 0.587 * img(y, x)(1) + 0.114 * img(y, x)(2));
			ret(y, x)(1) = 128.0 + (-0.169 * img(y, x)(0) - 0.331 * img(y, x)(1) + 0.500 * img(y, x)(2));
			ret(y, x)(2) = 128.0 + ( 0.500 * img(y, x)(0) - 0.419 * img(y, x)(1) - 0.081 * img(y, x)(2));
		}
	}

	return ret;
}

int mycell(double in)
{
    int ret=std::round(in);
    if(ret>255)
        return 255;
    if(ret<0)
        return 0;
    return ret;
}
    
    
Mat_8UC3 ycbcr2rgb(const Mat_64FC3 &img)
{
	Mat_8UC3 ret(img.rows(), img.cols());

	for (size_t x = 0; x < img.cols(); x++) {
		for (size_t y = 0; y < img.rows(); y++) {
            ret(y, x)(0) = mycell(img(y, x)(0) - 0.00093 * (img(y, x)(1) - 128.0) + 1.401687 * (img(y, x)(2) - 128.0));
			ret(y, x)(1) = mycell(img(y, x)(0) - 0.3437  * (img(y, x)(1) - 128.0) - 0.71417  * (img(y, x)(2) - 128.0));
			ret(y, x)(2) = mycell(img(y, x)(0) + 1.77216 * (img(y, x)(1) - 128.0) + 0.00099  * (img(y, x)(2) - 128.0));
		}
	}

	return ret;
}
}
