//
//  Arithmetic.hpp
//  pictureCodec
//
//  Created by jameswoods on 10/13/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef Arithmetic_hpp
#define Arithmetic_hpp

#include <iostream>
#include<vector>

namespace entropyCodec {
   
        int encode(std::vector<bool>data, std::vector<uint8_t> & code);
        void decode(std::vector<uint8_t> code, int length, std::vector<bool> & data);
        void testcodec();
        
}







#endif /* Arithmetic_hpp */
