//
//  quantization.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/20/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef quantization_hpp
#define quantization_hpp

#include <stdio.h>

#include "imgtype.h"
#include <vector>
void quant (std::vector<ljj::Vec3d> src, std::vector<std::vector<int>> & out, float QPY,float QPCb,float QPCr  );
void dequant(std::vector<std::vector<int>> src, std::vector<ljj::Vec3d> & out,float QPY, float QPCb,float QPCr);

void deQuMat(ljj::Mat_64FC3 &src, float QPY,float QPCb, float QPCr);
#endif /* quantization_hpp */
