//
//  Codec2_0.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/27/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef Codec2_0_hpp
#define Codec2_0_hpp

#include <stdio.h>
#include <vector>
#include "codecAblock.hpp"

void CodeDemo_2_0_TS(std::vector<blockCoeff> src);
void getNumForLastNum(int num, int stride, std::vector<int> &out);
#endif /* Codec2_0_hpp */
