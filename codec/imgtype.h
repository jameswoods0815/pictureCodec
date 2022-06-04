#ifndef __IMGTYPE_H__
#define __IMGTYPE_H__


#include <Eigen/Core>
namespace ljj{
    typedef Eigen::Matrix<uint8_t, 3, 1> Vec3b;
    typedef Eigen::Matrix<float, 5, 1> Vec5f;
    typedef Eigen::Matrix<double, 3, 1> Vec3d;
    typedef Eigen::Matrix<double, 5, 1> Vec5d;
    typedef Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic> Mat_bool;
    typedef Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> Mat_8UC1;
    typedef Eigen::Matrix<Vec3b, Eigen::Dynamic, Eigen::Dynamic> Mat_8UC3;
    typedef Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic> Mat_16UC1;
    typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> Mat_32SC1;
    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Mat_32FC1;
    typedef Eigen::Matrix<Vec5f, Eigen::Dynamic, Eigen::Dynamic> Mat_32FC5;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Mat_64FC1;
    typedef Eigen::Matrix<Vec3d, Eigen::Dynamic, Eigen::Dynamic> Mat_64FC3;
    typedef Eigen::Matrix<Vec5d, Eigen::Dynamic, Eigen::Dynamic> Mat_64FC5;

}

#endif
