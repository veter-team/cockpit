#ifndef __TOEDGE_H
#define __TOEDGE_H

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

// Return an anaglyph image calculated from pair
// of images stored in a single frame (data)
void frameToEdge(unsigned char *data, 
		 size_t framewidth, 
		 size_t frameheight,
		 Mat &dst);

#endif // __TOEDGE_H
