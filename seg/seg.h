#ifndef __LSC_SUPERPIXEL___
#define __LSC_SUPERPIXEL___

#include "imgtype.h"

namespace ljj {
    

void segM(const Mat_8UC3 &rgb, const size_t superpixel_num, const double ratio, Mat_16UC1 &label);

}
#endif
