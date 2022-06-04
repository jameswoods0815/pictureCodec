#ifndef __IMREAD_H__
#define __IMREAD_H__

#include <vector>
#include <string>

#include "imgtype.h"

namespace ljj {
    

Mat_8UC3 imread(const std::string &filename);
Mat_8UC1 rgb2gray(const Mat_8UC3 &img);
Mat_8UC3 gray2rgb(const Mat_8UC1 &img);
Mat_64FC3 rgb2ycbcr(const Mat_8UC3 &img);
Mat_8UC3 ycbcr2rgb(const Mat_64FC3 &img);

template <class T, int n>
std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> split(const Eigen::Matrix<Eigen::Matrix<T, n, 1>, Eigen::Dynamic, Eigen::Dynamic> &src)
{
	std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> ret(n, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>(src.rows(), src.cols()));

	for (size_t x = 0; x < src.cols(); x++) {
		for (size_t y = 0; y < src.rows(); y++) {
			for (size_t c = 0; c < n; c++) {
				ret[c](y, x) = src(y, x)(c);
			}
		}
	}

	return ret;
}

template <int n, class T>
Eigen::Matrix<Eigen::Matrix<T, n, 1>, Eigen::Dynamic, Eigen::Dynamic> merge(const std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> &src)
{
	Eigen::Matrix<Eigen::Matrix<T, n, 1>, Eigen::Dynamic, Eigen::Dynamic> ret(src[0].rows(), src[0].cols());

	for (size_t x = 0; x < src[0].cols(); x++) {
		for (size_t y = 0; y < src[0].rows(); y++) {
			for (size_t c = 0; c < n; c++) {
				ret(y, x)(c) = src[c](y, x);
			}
		}
	}

	return ret;
}

    
}
#endif
