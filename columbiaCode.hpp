//
//  columbiaCode.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/11/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef columbiaCode_hpp
#define columbiaCode_hpp

#include <stdio.h>
#include <vector>

// k is the exp num;

std::vector<bool> encodeColumbiaCode(unsigned int x, int k);
unsigned int decodeColumbiaCode(std:: vector <bool> code, int k);

std:: vector<bool> encodeColumbiaSeri(std::vector<int> src , int k);
std::vector<int> decodeColunmbiaSeri(std::vector<bool> code, int k);

#endif /* columbiaCode_hpp */
