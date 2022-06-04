//
//  Codec2_0.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/27/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "Codec2_0.hpp"
#include"columbiaCode.hpp"
#include "Arithmetic.hpp"
#include <fstream>
#include<stdlib.h>
#include "codec.h"

void getNumForLastNum(int num, int stride, std::vector<int> &out)
{
    
    
    int num1=stride*stride-num;
    int split=stride/8;
    int x=num1/stride;
    int y=num1%stride;
    int prefixX=x/split;
    int lastX=x%split;
    int preFixY=y/split;
    int lastY=y%split;
    
//    out.resize(4);
    out[0]=prefixX;
    out[1]=lastX;
    out[2]=preFixY;
    out[3]=lastY;
}


void CodeDemo_2_0_TS(std::vector<blockCoeff> src)
{
    FILE * f;
    char *buf="/Users/jameswoods/Documents/project/pictureCodec/data/Demo2_0.dat";
    f=fopen(buf, "w");
    
    uint8_t head[5]={'w','j','m','y','y'};
    for(int i=0;i<5;i++)
    {
        fwrite(&head[i], 1, 1, f);

    }

    //write skipFlag;
    int count=0;
    
    for(int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> skipFlag;
        skipFlag.clear();
        entropyCodec::encode(src[i].TUSkip, skipFlag);
        for(int j=0;j<skipFlag.size();j++)
        {
            fwrite(&skipFlag[j], 1, 1, f);
            count++;
        }
    }
    
    std::cout<<"skipFlag num "<<count<<" bytes"<<std::endl;
    count=0;
    
    //write If SKipIndex
    for(int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> subBlockSkip;
        entropyCodec::encode(src[i].IfSkip, subBlockSkip);
        for(int j=0;j<subBlockSkip.size();j++)
        {
            fwrite(&subBlockSkip[j], 1, 1, f);
            count++;
        }
    }
    
    std::cout<<"Skip Index num "<<count<<" bytes"<<std::endl;

    count=0;
    // If Zero;
    for (int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> PositionZero;
        entropyCodec::encode(src[i].IfZero, PositionZero);
        for(int j=0;j<PositionZero.size();j++)
        {
            fwrite(&PositionZero[i], 1, 1, f);
            count++;
        }
    }
    std::cout<<"If Zero num "<<count<<" bytes"<<std::endl;

    
    count=0;
    // Sign
    for(int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> sign;
        entropyCodec::encode(src[i].Sign, sign);
        for(int j=0;j<sign.size();j++)
        {
            fwrite(&sign[i], 1, 1, f);
            count++;
        }
    }
    
    std::cout<<"Sign num "<<count<<" bytes"<<std::endl;
    count=0;

    //Ifgreathan1
    for(int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> greater1;
        entropyCodec::encode(src[i].IfGreaterThan1, greater1);
        for(int j=0;j<greater1.size();j++)
        {
            fwrite(&greater1[i], 1, 1, f);
            count++;
        }
    }
    
    std::cout<<"If greater than 1 num "<<count<<" bytes"<<std::endl;

    count=0;
    
    //ifGreater2
    for(int i=0;i<src.size();i++)
    {
        std::vector<uint8_t> greater2;
        entropyCodec::encode(src[i].IfGreaterThan2, greater2);
        for(int j=0;j<greater2.size();j++)
        {
            fwrite(&greater2[j], 1, 1, f);
            count++;
        }
    }
    
    std::cout<<"If greater than 2 num "<<count<<" bytes"<<std::endl;

    
//     lastNum Codec  error because of not split and use context;
    count=0;
    for(int i=0;i<src.size();i++)
    {
        
        std::vector<int> lastP(src[i].lastnumindex.size()*4);
        std::vector<int> temp(4);
        
        for(int j=0;j<src[i].lastnumindex.size();j++)
        {
            getNumForLastNum(src[i].lastnumindex[j], 32, temp);
            lastP[j*4]=temp[0];
            lastP[j*4+1]=temp[1];
            lastP[j*4+2]=temp[2];
            lastP[j*4+3]=temp[3];
        }
        
        std::vector<bool> codeClombia;
        std::vector<uint8_t> lastPostion;
        codeClombia =encodeColumbiaSeri(lastP, 1);
        entropyCodec::encode(codeClombia, lastPostion);
        for(int j=0;j<lastPostion.size();j++)
        {
            fwrite(&lastPostion[i], 1, 1, f);
            count++;
        }
    }

    std::cout<<"position num "<<count<<" bytes"<<std::endl;

    //last Abs Codec;

    count=0;
    for(int i=0;i<src.size();i++)
    {
        std::vector<bool> codeClombia;
        std::vector<uint8_t> abs_else;
        codeClombia =encodeColumbiaSeri(src[i].abs_else_num, 1);
        entropyCodec::encode(codeClombia, abs_else);
        for(int j=0;j<abs_else.size();j++)
        {
            fwrite(&abs_else[i], 1, 1, f);
            count++;
        }
    }
    fclose(f);
    
    std::cout<<"last Abs Codec num "<<count<<" bytes"<<std::endl;

}



