#ifndef __ENCODE__
#define __ENCODE__

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <fftw3.h>

//#include "Arithmetic.hpp"
#include "imgproc.h"
#include "seg.h"
#include "s_dct.h"
#include "s_idct.h"

namespace ljj
{
class Point {
public:
	size_t x, y;
};

class Rect {
public:
	size_t x, y, width, height;

	Point tl() const
	{
		return Point { x, y };
	}
};

typedef struct Shape {
	struct Point leftTop;
	Mat_bool mask;
	std::vector<Point> border;
} Shape;

template <class T>
T square(T x)
{
	return x * x;
}

    std::vector<uint16_t> calcNecessaryShape(const Mat_16UC1 &label);

    Mat_16UC1 mergeLabel(const Mat_16UC1 &label, const Mat_64FC3 &ycbcr, const size_t block_num, const double var);

    Rect calcConvexRect(const Mat_bool &src);

    bool isMaskBorder(const Mat_8UC1 &mask, const size_t x, const size_t y);

    Shape makeShapeWithMask(const Mat_bool &mask);

    std::vector<Shape> splitShapeArray(const Mat_16UC1 &src);

    void dct(std::vector<double> &v);
    void idct(std::vector<double> &v);
    Mat_64FC3 makeShapeTexture(const Mat_8UC3 &rgb_img, const Shape &shape);

    Mat_8UC3 solveTexture(const Shape &shape, const Mat_64FC3 &YCrCbImg);

    std::vector<Vec3d> zigzagSerialize(const Shape &shape, const Mat_64FC3 &img);

    Mat_64FC3 zigzagDeserialize(const Shape &shape, const std::vector<Vec3d> &zVec);
    
    Mat_64FC3 solveTextureYcbcr(const Shape &shape, const Mat_64FC3 &YCrCbImg,IDCTcontext *icontext);
    Mat_64FC3 makeShapeTextureYcbcr(const Mat_64FC3 &src, const Shape &shape, DCTcontext *context);
    Mat_bool decode_mask(const size_t rows, const size_t cols, const std::vector<uint8_t> &encoded_data);
    std::vector<uint8_t> encode_mask(const Mat_bool &mask);



}

#endif
