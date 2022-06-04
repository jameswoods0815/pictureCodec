//
//  codecAblock.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/22/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef codecAblock_hpp
#define codecAblock_hpp

#include <stdio.h>
#include<vector>
typedef struct {
    bool skipFlag; //if all zero, skip flag true
    int  firstNum;
    bool ElseFlag;
    std::vector<bool> ElseStatus; //check every place is zeros or not
    std::vector<bool> coeffFlag;
    std::vector<unsigned int> coeff;
} codeUint;

typedef struct {
    std::vector<bool> skipFlag; //if the TU can skip
    std::vector<int> firstNum; //last num position
     std::vector<bool> elseFlag; //
    std::vector<bool> elseStatus;
    std::vector<bool>coeffFlag;
    std::vector<unsigned int >coeff;
}TransformCoeff;

typedef struct {
    bool IfSkip;
    std:: vector<bool> IfZero;
    std::vector<bool> Sign;
    std::vector<bool> IfGreaterThan1;
    std::vector<bool> IfGreaterThan2;
    std::vector<int> abs_else_num;
    
}subblockContext;


typedef struct {
    std::vector<bool> TUSkip;// all block skip
    std::vector<int> lastnumindex; //last num in the sequence;
    std::vector<bool> IfSkip; //subblock index;
    std:: vector<bool> IfZero; // postion of index
    std::vector<bool> Sign; // the sgin of coeff
    std::vector<bool> IfGreaterThan1; // num bigger than 1
    std::vector<bool> IfGreaterThan2; //num bigger than 2
    std::vector<int> abs_else_num; // abs num else
    
}blockCoeff;



void encodeAblock(std::vector<int> src,codeUint &output);
void decodeAblock(codeUint src  ,std::vector<int> &output,int size);

void encodeVector(std::vector<int> src, TransformCoeff & out, std::vector<int>seg);
void decodeVector(TransformCoeff src, std::vector<int>seg, std::vector<int>  & out);
void  testEncodeAndDecode(void);

void encodeCoeff(std::vector<int> src, blockCoeff &out, std::vector<int>seg);

void testEncodeAsubBlokc();
#endif /* codecAblock_hpp */
