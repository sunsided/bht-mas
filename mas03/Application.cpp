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
    // load the image data
    const samples_t     raw_samples = 512;
    const lines_t       raw_lines = 512;
    
    vector<string> raw_image_paths;
    raw_image_paths.push_back("./images/bild0.raw");
    raw_image_paths.push_back("./images/bild1.raw");
    raw_image_paths.push_back("./images/bild2.raw");
    raw_image_paths.push_back("./images/bild3.raw");
    raw_image_paths.push_back("./images/bild4.raw");
    raw_image_paths.push_back("./images/bild5.raw");
    raw_image_paths.push_back("./images/bild6.raw");

    vector<image_t> raw_images;
    for (string path : raw_image_paths)
    {
        auto image = loadRawU8(path, raw_samples, raw_lines);
        raw_images.push_back(std::move(image));
    }
    
    OpenCvWindow& window = createWindow("Trololo");
    bool break_loop = false;

    while (!break_loop)
    for (image_t& image : raw_images)
    {
        auto openCvImage = image->toOpenCv();
        window.showImage(openCvImage);
        int key = cvWaitKey(33);
        
        // leave display loop
        break_loop = key >= 0;
        if (break_loop) break;
    }
}