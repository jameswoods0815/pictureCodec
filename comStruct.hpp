//
//  comStruct.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/14/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef comStruct_hpp
#define comStruct_hpp

#include <stdio.h>

typedef struct {
    int splitFlag;
    int mode;
    int QP;
    int skipflag;
    int greaterthan1;
    int greaterthan2;
    int coeff;
    
} CUHeader;

typedef struct{
    int objectBit;
}Object;

#endif /* comStruct_hpp */
