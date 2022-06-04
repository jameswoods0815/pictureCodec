// dct_test.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <fstream>
#include <vector>
#include "s_dct.h"
#include "s_idct.h"
#include <time.h>
using namespace std;
#include "codec.h"

int main1()
{
    int length = 64;
    vector<double> data( length,200);
    vector<double> output(length);
    
    //    data[1]=180;
    //    data[2]=230;
    
    DCTcontext context;
    char *str = "/Users/jameswoods/Documents/project/pictureCodec/DCT/table.bin";
    InitDctContext(str, &context);
    DCT(&context, data, output);
    destroyDCTcontext(&context);
    
    std::vector<int> idata(length);
    
    for (int i=0;i<length;i++)
    {
        idata[i]=int(output[i]);
    }
    
    IDCTcontext icontext;
    char *istr = "/Users/jameswoods/Documents/project/pictureCodec/DCT/itable.bin";
    InitIDctContext(istr, &icontext);
    
    
    double ioutput[64];
    
    IDCT(&icontext, idata, ioutput);
    
    vector<double>data1=data;
    ljj::dct(data1);
    
    {
        double start=clock();
        for(int i=0;i<10000;i++)
        {
//            DCT(&context, data, output);
            IDCT(&icontext, idata, ioutput);
            
        }
        
        double end=clock();
        
        std::cout << " our  encode  and decode takes time: " << (end - start) / 1000.0 << "ms" << std::endl;
    }
    
        {
            double start=clock();
            for(int i=0;i<10000;i++)
            {
//                ljj::dct(data1);
                ljj::idct(data1);
            }
    
            double end=clock();
    
            std::cout << " fftw encode  and decode takes time: " << (end - start) / 1000.0 << "ms" << std::endl;
        }
    
    destroyIDCTcontext(&icontext);
    
    return 0;
    
}

