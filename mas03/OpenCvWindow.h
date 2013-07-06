#ifndef _OPENCV_WINDOW_H_
#define _OPENCV_WINDOW_H_

#include <memory>
#include <string>

#pragma warning(push) // Disable deprecation
#pragma warning(disable: 4996) // Disable deprecation

#include <opencv/cv.h>
#include <opencv/highgui.h>

#pragma warning(pop) // enable deprecation

#include "OpenCvImage.h"

// custom delete für IplImage-Pointer
struct OpenCvWindow 
{
    friend struct OpenCvWindowDeleter;

private:
    const std::string _windowName;
    bool _destroyed;

public:
    inline explicit OpenCvWindow(const std::string name) : _windowName(name), _destroyed(false) {}

    /// <summary>
    /// Shows the image.
    /// </summary>
    /// <param name="image">The image.</param>
    inline void showImage(const IplImagePtr& image) const
    {
        assert (!_destroyed);
        cvShowImage(_windowName.c_str(), image.get());
    }

private:
    virtual ~OpenCvWindow() {}
};


// custom delete für IplImage-Pointer
struct OpenCvWindowDeleter 
{
    void operator()(OpenCvWindow* window) const {
        if (!window || window->_destroyed) return;
        window->_destroyed = true;
        cvDestroyWindow(window->_windowName.c_str());
    }
};

// alias for IplImage-Pointer
typedef std::unique_ptr<OpenCvWindow, OpenCvWindowDeleter> OpenCvWindowPtr;

#endif