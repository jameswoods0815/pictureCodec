#include "s_dct.h"
#include <fstream>
#include <iostream>


void InitDctContext(char* src, DCTcontext * context)
{
    std::ifstream fi;
    context->DctContext.reserve(89440);
    float tem;
    
    fi.open(src, std::ios::binary);
    
    if (!fi) {
        std::cout << "can not open a file to read!!" << std::endl;
        return;
    }
    
    for (int i = 0; i < 89440; i++) {
        fi.read((char*)&tem, sizeof(float));
        context->DctContext.push_back(tem);
    }
    
    
    fi.close();
    int index=0;
    for (int i = 0; i < 64; i++) {
        index+= i*i;
        context->start[i]=index;
    }
    
    return;
}

void DCT(DCTcontext *context, std::vector<double> data, std::vector<double> &output)
{
    int length = data.size();
    if(length==0)
        return;
    int index=context->start[length-1];
    double tem;
    float *dctTable=&(context->DctContext[index]);
    
    int rowBase=0;
    for (int i = 0; i < length; i++) {
        tem = 0;
        for (int j = 0; j < length; j++) {
            tem += data.at(j)*dctTable[rowBase + j];
        }
        output[i]=tem;
        rowBase+=length;
    }
    output[0]*=5;

}


void destroyDCTcontext(DCTcontext *context)
{
    //这里不会写
}
