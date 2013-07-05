#ifndef _OPENCV_IMAGE_H_
#define _OPENCV_IMAGE_H_

#include <memory>

#pragma warning(push) // Disable deprecation
#pragma warning(disable: 4996) // Disable deprecation

#include <opencv/cv.h>

#pragma warning(pop) // enable deprecation


// custom delete für IplImage-Pointer
struct IplImageDeleter 
{
    void operator()(IplImage* image) const {
        if (!image) return;
        cvReleaseImage(&image); 
    }
};

// alias for IplImage-Pointer
typedef std::unique_ptr<IplImage, IplImageDeleter> IplImagePtr;

#endif