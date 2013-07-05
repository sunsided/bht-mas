#pragma warning(push) // Disable deprecation
#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#pragma warning(pop) // enable deprecation

#include "ENVIFileReader.h"
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
    for(string& name: _windowNames)
    {
        cvDestroyWindow(name.c_str());
    }

    // purge window names
    _windowNames.clear();
}

/// <summary>   Creates a window. </summary>
///
/// <remarks>   Markus, 05.07.2013. </remarks>
///
/// <param name="name"> [in,out] The name. </param>
void Application::createWindow(const string& name)
{
    _windowNames.push_back(name);
    cvNamedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
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

    // load the image data
    const envi::samplecount_t samples = 5000;
    const envi::linecount_t lines = 2000;
    const envi::bandcount_t bands = 1;
    envi::ENVIFileReader reader(samples, lines, bands);

    cout << "Loading image ... ";
    envi::image_t image = reader.read(inputFile);
    cout << "done." << endl;

    // close the input file
    inputFile.close();

    // convert image to OpenCV image.
    cout << "Converting image for display ... ";
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, bands));
    for(envi::linecount_t y=0; y<lines; ++y)
    {
        for(envi::samplecount_t x=0; x<samples; ++x) 
        {
            // pick the sample and lerp it to 0..255
            envi::sample_t sample = image[y][x];
            sample = min(255.0F, max(0.0F, sample));
            uint_fast8_t value = static_cast<uint_fast8_t>(sample);

            // assign sample
            displayImage->imageData[y*samples+x] = value;
        }
    }
    cout << "done" << endl;

    // Display
    createWindow("Original");
    cvShowImage("Original", displayImage.get());

    cvWaitKey(0);
}