//
//  Arithmetic.cpp
//  pictureCodec
//
//  Created by jameswoods on 10/13/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "Arithmetic.hpp"
#include <iostream>
using namespace std;

namespace entropyCodec {
    

void adapt(int bit,uint32_t  & prob);


int const kProbBits=12;
uint32_t const kProbMax=1u<<kProbBits;
int const inertia=4;


int encode(std::vector<bool>data, std::vector<uint8_t> & code)
{
    //init prob as 1/2
    uint32_t prob=kProbMax/2;
    uint32_t low=0;
    uint32_t high=(~0u);
    
    code.clear();
    for (int i=0;i<data.size();i++)
    {
        //encode data
        uint32_t x=low+((uint64_t(high-low)*prob)>>kProbBits);
        if(data[i])
            high=x;
        else
            low=x+1;
        
        while((low^high)<(1u<<24))
        {
            code.push_back(low>>24);
            low<<=8;
            high=(high<<8)|0xff;
        }

        
        //refresh model
        
        adapt(data[i], prob);
    }
    
    //flush all the otherdata
    
    for(int i=0;i<4;i++)
    {
        code.push_back(low>>24);
        low<<=8;
    }
    return code.size();
    
}
void decode(std::vector<uint8_t> code, int length, std::vector<bool> & data)
{
    //init prob
    uint32_t prob=kProbMax/2;
    uint32_t low=0;
    uint32_t high=~(0u);
    uint32_t nowCode=0;
    int readptr=0;
    
    
    // init nowcode;
    for(int i=0;i<4;i++)
    {
        nowCode=(nowCode<<8)|code[readptr++];
    }
    
    // clear data
    data.clear();
    
    // get all the data
    for (int i=0;i<length;i++)
    {
        //decode data
        int bit ;
        uint32_t x=low+((uint64_t(high-low)*prob)>>kProbBits);
    
        if(nowCode<=x)
        {
            high=x;
            bit=1;
        }
        else
        {
            low=x+1;
            bit=0;
        }
        
        while ((low^high)<(1u<<24))
        {
            nowCode=(nowCode<<8)|code[readptr++];
            low<<=8;
            high=(high<<8)|0xff;
        }
        data.push_back(bit);
        
        //refresh model
        adapt(bit, prob);
        
    }
    
}


void  testcodec()
{
    // init data;
    vector<bool> data;
    
    srand(2345);
    
    for (size_t chunk = 0; chunk < 50; ++chunk)
    {
        int threshold = rand();
        for (size_t i = 0; i < 400000; ++i)
            data.push_back(rand() < 0.025*threshold);
    }
    
    //encode data;
    vector <uint8_t> code;
  int size=  encode(data, code);
    
    float rate1=0;
    for (int i =0;i<data.size();i++)
    {
        rate1+=data[i];
    }
    rate1=rate1/data.size();
    
    cout <<"code size="<<size<<" Bytes and ratio=" <<size*8.0/data.size() <<"  rate of 1="<<rate1<<endl;
    //decode
    vector<bool> dedata;
    decode(code, data.size(), dedata);
    
    if(dedata!=data)
    {
        cout<<"error decode!"<<endl;
    }
    else {
        cout<<"decode OK"<<endl;
    }
    
    
}

//probility refress model;

void adapt(int bit,uint32_t  & prob)
{
    if(bit)
    {
        prob+=(kProbMax-prob)>>inertia;
    }
    else{
        prob-=prob>>inertia;
    }
}
}