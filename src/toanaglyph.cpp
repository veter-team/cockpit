#include "toanaglyph.h"


IplImage*
CopySubImage(IplImage *src,
             int xorigen, int yorigen,
             int width, int height)
{
  CvRect roi;
  IplImage *dest;

  roi.x = xorigen;
  roi.y = yorigen;
  roi.width = width;
  roi.height = height;

  // set ROI for source image
  cvSetImageROI(src, roi);
  // create destination image with ROI size
  dest = cvCreateImage(cvSize(roi.width, roi.height),
                       src->depth, src->nChannels);
  cvCopy(src, dest);
  cvResetImageROI(src);
  return dest;
}


IplImage* 
toAnaglyph(IplImage *imgLeft, IplImage *imgRight)
{
  IplImage *iplReturn;

  IplImage *l_R, * l_G, *l_B;
  IplImage *r_R, * r_G, *r_B;

  iplReturn = cvCreateImage(cvGetSize(imgLeft), 
                            imgLeft->depth, 
                            imgLeft->nChannels);

  l_R = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);
  l_G = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);
  l_B = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);
  r_R = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);
  r_G = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);
  r_B = cvCreateImage(cvGetSize(imgLeft), imgLeft->depth, 1);

  cvSplit(imgLeft, l_R, l_G, l_B, NULL);
  cvSplit(imgRight, r_R, r_G, r_B, NULL);

  //cvMerge(r_R, r_G, l_B, NULL, iplReturn);
  cvMerge(r_R, l_G, l_B, NULL, iplReturn);

  cvReleaseImage(&l_R);
  cvReleaseImage(&l_G);
  cvReleaseImage(&l_B);
  cvReleaseImage(&r_R);
  cvReleaseImage(&r_G);
  cvReleaseImage(&r_B);

  return iplReturn;
}


IplImage* 
frameToAnaglyph(unsigned char *data, size_t framewidth, size_t frameheight)
{
  IplImage *image = 
    cvCreateImageHeader(cvSize(framewidth, 
                               frameheight), 
                        IPL_DEPTH_8U, 
                        3);
  image->imageDataOrigin = (char*)data;
  image->imageData = (char*)data;

  int half_width = framewidth / 2;

  IplImage *imgLeft = 
    CopySubImage(image, 
                 0, 0, 
                 half_width, frameheight);
  IplImage *imgRight = 
    CopySubImage(image, 
                 half_width, 0, 
                 half_width, frameheight);

  IplImage* res_small = toAnaglyph(imgLeft, imgRight);
  IplImage* res = 
    cvCreateImage(cvSize(framewidth, frameheight),
                  res_small->depth, res_small->nChannels);

  cvResize(res_small, res, CV_INTER_LINEAR);

  cvReleaseImage(&imgLeft);
  cvReleaseImage(&imgRight);
  cvReleaseImage(&res_small);
  cvReleaseImageHeader(&image);

  return res;
}
