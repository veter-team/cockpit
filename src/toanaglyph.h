/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __TOANAGLYPH_H
#define __TOANAGLYPH_H

#include <cv.h>

// Return an anaglyph image calculated from pair
// of images stored in a single frame (data)
IplImage* frameToAnaglyph(unsigned char *data, 
                          size_t framewidth, 
                          size_t frameheight);

#endif // __TOANAGLYPH_H
