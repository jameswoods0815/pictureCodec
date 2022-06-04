#include "s_idct.h"
#include <fstream>
#include <iostream>


void InitIDctContext(char* src, IDCTcontext * context)
{
    std::ifstream fi;
    context->IDctContext.reserve(89440);
    float tem;
    
    fi.open(src, std::ios::binary);
    
    if (!fi) {
        std::cout << "can not open a file to read!!" << std::endl;
        return;
    }
    
    for (int i = 0; i < 89440; i++) {
        fi.read((char*)&tem, sizeof(float));
        context->IDctContext.push_back(tem);
    }
    
    
    fi.close();
    
    int index=0;
    for (int i = 0; i < 64; i++) {
        index+= i*i;
        context->start[i]=index;
    }
    
    return;
}
void IDCT(IDCTcontext *context, std::vector<int> data, double * output)
{
    int length = data.size();
    if(length==0)
        return;
    int index = context->start[length-1];
    float * idctTable= &(context->IDctContext[index]);
    int tempCount=0;
    int ne1=0,pe1=0;
    for(int i=1;i<length;i++)
    {
        if(data[i])
        {
            if(data[i]==1)
            {
                context->num1[pe1]=i;
                pe1++;
            }
            else if (data[i]==-1)
            {
                context->numNe1[ne1]=i;
                ne1++;
            }
            else {
            context->indexMe[tempCount]=i;
            tempCount++;
            }
        }
    }
    
   
    if(data[0])
    {
        double temp=data[0]/5.0;
        for(int i=0;i<length;i++)
        {
            output[i]=temp;
        }
    }
    else{
        memset(output, 0, sizeof(double)*length);
    }


    int rowBase=0;
    for (int i = 0; i < length; i++) {
        
        for (int j = 0; j < tempCount; j++) {
            output[i] += data.at(context->indexMe[j])*idctTable[rowBase + context->indexMe[j]];
        }
        
        for(int j=0;j<pe1;j++){
            output[i] +=idctTable[rowBase + context->num1[j]];
        }
        for(int j=0;j<ne1;j++){
            output[i] -=idctTable[rowBase + context->numNe1[j]];
        }
        rowBase+=length;
    }
    
}



void destroyIDCTcontext(IDCTcontext *context)
{
}
