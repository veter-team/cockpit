#include "toedge.h"


void frameToEdge(unsigned char *data, 
		 size_t framewidth, 
		 size_t frameheight,
		 Mat &dst)
{
  Mat src(frameheight, framewidth, CV_8UC4, data);
  Mat src_gray;
  Mat detected_edges;

  const int lowThreshold = 15;
  int ratio = 3;
  int kernel_size = 3;

  // Create a matrix of the same type and size as src (for dst)
  dst.create(src.size(), src.type());

  // Convert the image to grayscale
  cvtColor(src, src_gray, CV_BGRA2GRAY);

  // Reduce noise with a kernel 3x3
  blur(src_gray, detected_edges, Size(3,3));

  // Canny detector
  Canny(detected_edges, detected_edges, 
	lowThreshold, lowThreshold * ratio, 
	kernel_size);

  // Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo(dst, detected_edges);
}
