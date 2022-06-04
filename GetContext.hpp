//
//  GetContext.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/14/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef GetContext_hpp
#define GetContext_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>

// test result is OK

void getContext(cv::Mat mask, cv::Mat restore,uint8_t * up, uint8_t  * left, int x, int y, int width,int height);
void testContext(void);

#endif /* GetContext_hpp */
