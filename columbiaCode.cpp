//
//  columbiaCode.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/11/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "columbiaCode.hpp"
#include<math.h>
#include <iostream>

//

std::vector<bool> encodeColumbiaCode(unsigned int x, int k)
{
    std::vector<bool> ret;
    std::vector<bool> temp;
    int src=x;
    
    // delete k bits and plus 1;
    int xd=(x>>k)+1;
    int zero=log2(xd);
    
   
    //push tail;
    for (int i=0;i<k;i++)
    {
        ret.push_back(src&0x1);
        src=src>>1;
    }
    //push xd;
    for (int i=0;;i++)
    {
        if(xd<1)
        {
            break;
        }
        ret.push_back(xd&0x1);
        xd=xd>>1;
    }
    
    //push zero
    for(int i=0;i<zero;i++)
    {
        ret.push_back(0);
    }
    
    for(int i=ret.size()-1;i>-1;i--)
    {
        temp.push_back(ret[i]);
    }
    
    return  temp;
}
unsigned int decodeColumbiaCode(std:: vector <bool> code, int k)
{
    int ret;
    
    int zero=0;
    for (int i=0;i<code.size();i++)
    {
        if(!code[i])
        {
            zero++;
        }
        else{
            break;
        }
    }
    
    int value=0;
    int i=0;
    for(i=zero+1;i<2*zero+k;i++)
    {
        value+=code[i];
        value=value<<1;
    }
    value+=code[i];
    
    std::cout<<" value "<< value<<"  ";
    
    ret=(1<<(zero+k))-(1<<k)+value;
    
    return ret;
}

std:: vector<bool> encodeColumbiaSeri(std::vector<int> src, int k)
{
    std:: vector<bool> ret;
    std:: vector<bool> temp;

    
    for(int i=0;i<src.size();i++)
    {
        temp= encodeColumbiaCode(src[i], k);
        ret.insert(ret.end(), temp.begin(),temp.end());
    }
    
    return ret;
    
}
std::vector<int> decodeColunmbiaSeri(std::vector<bool> code, int k)
{
    std::vector<int> result;
    
    
    for(int ptr=0;ptr<code.size();)
    {
    
    int ret;
    
    int zero=0;
    for (int i=ptr;i<code.size();i++)
    {
        if(!code[i])
        {
            zero++;
        }
        else{
            break;
        }
    }
    
    int value=0;
    int i=0;
    for(i=ptr+zero+1;i<ptr+2*zero+k;i++)
    {
        value+=code[i];
        value=value<<1;
    }
    value+=code[i];
    
//    std::cout<<" value "<< value<<"  ";
    
    ret=(1<<(zero+k))-(1<<k)+value;
    result.push_back(ret);
    ptr+=zero+zero+k+1;
    }
    
    return  result;
}

