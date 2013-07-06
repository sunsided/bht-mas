#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include "FloatImage.h"
#include "Application.h"

using namespace std;

/// <summary>
/// Initializes a new instance of the <see cref="Application"/> class.
/// </summary>
Application::Application() {}
    
/// <summary>
/// Finalizes an instance of the <see cref="Application"/> class.
/// </summary>
Application::~Application() 
{
    // close all windows
    for(OpenCvWindowPtr& window: _windows)
    {
        window.reset();
    }

    // purge window names
    _windows.clear();
}

/// <summary>   Creates a window. </summary>
///
/// <remarks>   Markus, 05.07.2013. </remarks>
///
/// <param name="name"> [in,out] The name. </param>
OpenCvWindow& Application::createWindow(const string& name)
{
    OpenCvWindow* window = new OpenCvWindow(name);
    cvNamedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);

    _windows.emplace_back(OpenCvWindowPtr(window));

    return *window;
}

/// <summary>
/// Runs this instance.
/// </summary>
void Application::run()
{
    // open the input file
    string filename = "./images/rued_corr_flt.img";
    ifstream inputFile;
    inputFile.open(filename, ios_base::in | ios_base::beg | ios_base::binary);
    if (!inputFile.is_open()) throw runtime_error("Could not open input file");
    
    // close the input file
    inputFile.close();
    
    unique_ptr<FloatImage> image(new FloatImage(100, 100, true));
    image->set(50, 50, 1.0f);

    auto openCvImage = image->toOpenCv(0, 1.0F);

    OpenCvWindow& window = createWindow("Trololo");
    window.showImage(openCvImage);

    cvWaitKey(0);
}