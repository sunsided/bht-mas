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
/// Loads a raw 8-bit unsigned single-channel image
/// </summary>
/// <param name="filepath">The filepath.</param>
/// <returns>image_t.</returns>
image_t Application::loadRawU8(const std::string filepath, const samples_t samples, const lines_t lines)
{
    // open the input file
    ifstream inputFile;
    inputFile.open(filepath, ios_base::in | ios_base::beg | ios_base::binary);
    if (!inputFile.is_open()) throw runtime_error("Could not open input file");

    // load the image data
    auto image = FloatImage::createFromU8Raw(inputFile, samples, lines);

    // close the input file
    inputFile.close();

    return std::move(image);
}

/// <summary>
/// Runs this instance.
/// </summary>
void Application::run()
{   
    OpenCvWindow& window = createWindow("Trololo");
    
    // load the image data
    const samples_t samples = 512;
    const lines_t lines = 512;
    auto image = loadRawU8("./images/bild0.raw", samples, lines);
    auto openCvImage = image->toOpenCv();
    window.showImage(openCvImage);

    cvWaitKey(0);
}