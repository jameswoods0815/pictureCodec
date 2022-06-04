//
//  printData.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/20/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "printData.hpp"
#include <fstream>
#include<stdlib.h>

void printQuant(std::vector<std::vector<int>> src,int num)
{
    char buf[1000];
    char buf3[100];
    sprintf(buf3, "%d",num);
    buf[0]='\0';
    
    char *buf1="/Users/jameswoods/Documents/project/pictureCodec/data/";
    char *buf2=".txt";
    strcat(buf, buf1);
    strcat(buf, buf3);
    strcat(buf, buf2);

//    std::cout<<buf;
    FILE *f= std::fopen(buf,"w");
    for (int i=0;i<src.size();i++)
    {
        fprintf(f, "%d %d %d \n", src[i][0],src[i][1],src[i][2]);
    }
    fclose(f);
    
}
