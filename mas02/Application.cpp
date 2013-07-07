#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include "ENVIFileReader.h"
#include "Application.h"

using namespace std;
using namespace envi;

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
    const samplecount_t samples = 5000;
    const linecount_t lines = 2000;
    const bandcount_t bands = 1;
    ENVIFileReader reader(samples, lines, bands);

    cout << "Loading image ... ";
    image_t image = reader.read(inputFile);
    cout << "done." << endl;

    // close the input file
    inputFile.close();

    // calculate naive statistics
    cout << endl << "Calculating statistics (naive) ... ";
    auto stats = calculateStatisticsNaive(image, samples, lines, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // calculate naive d/c statistics
    cout << endl << "Calculating statistics (naive divide-and-conquer) ... ";
    stats = calculateStatisticsNaiveDivideConquer(image, samples, lines, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // calculate forward statistics
    cout << endl << "Calculating statistics (forward d&q) ... ";
    stats = calculateStatisticsForward(image, samples, lines, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // low- and high density regions
    samplecount_t region_width = 500;
    linecount_t   region_height = 500;

    samplecount_t low_x = 200;
    linecount_t   low_y = 200;

    samplecount_t hi_x  = 2000;
    linecount_t   hi_y  = 800;

    // calculate low-density statistics
    cout << endl << "Calculating statistics for low-density region ... ";
    auto low_stats = calculateStatistics(image, low_x, low_x+region_width, low_y, low_x+region_height, bands);
    cout << "done" << endl;
    cout << low_stats << endl;

    // calculate high-density statistics
    cout << endl << "Calculating statistics for high-density region ... ";
    auto high_stats = calculateStatistics(image, hi_x, hi_x+region_height, hi_y, hi_y+region_height, bands);
    cout << "done" << endl;
    cout << high_stats << endl;

    // convert image to OpenCV image.
    cout << endl << "Converting low and high density regions for display ... ";
    IplImagePtr lowDensityRegion = enviToOpenCv(image, low_x, low_x+region_width, low_y, low_x+region_height, bands, low_stats->min, low_stats->max);
    IplImagePtr highDensityRegion = enviToOpenCv(image, hi_x, hi_x+region_height, hi_y, hi_y+region_height, bands, high_stats->min, high_stats->max);
    IplImagePtr displayImage = enviToOpenCv(image, samples, lines, bands, stats->min, stats->max);
    cout << "done" << endl;
    
    // calculate the histogram
    cout << endl << "Building histogram ... ";
    auto histogram = buildHistogram(image, samples, lines, bands, 0.0F, 4096.0F, 10);
    cout << "done" << endl;

    cout << endl << "Histogram:" << endl;
    for (uint8_t i=0; i<10; ++i)
    {
        float value = histogram[i] * 100.0F;
        cout << "class " << to_string(i) << "\tvalue: " << to_string(value) << "%" << endl;
    }
    
    // scaling
    const float scaleFactor = 5;
    cout << endl << "Scaling image ... ";
    auto scaled = scaleDownLinear(image, samples, lines, bands, scaleFactor);
    cout << "done" << endl;

    cout << "Converting scaled image ... ";
    auto cvscaled       = enviToOpenCv(scaled, static_cast<samplecount_t>(samples/scaleFactor), static_cast<linecount_t>(lines/scaleFactor), bands, stats->min, stats->max);
    auto cvscaledLow    = enviToOpenCv(scaled, static_cast<samplecount_t>(samples/scaleFactor), static_cast<linecount_t>(lines/scaleFactor), bands, stats->min, stats->max/4);
    auto cvscaledHigh   = enviToOpenCv(scaled, static_cast<samplecount_t>(samples/scaleFactor), static_cast<linecount_t>(lines/scaleFactor), bands, stats->max*1/4, stats->max);
    cout << "done" << endl;
    

    // Display
    createWindow("Original");
    cvShowImage("Original", displayImage.get());

    createWindow("Scaled");
    cvShowImage("Scaled", cvscaled.get());
    cvSaveImage("./mas02_scaled.jpg", cvscaled.get());
    
    createWindow("Scaled Low-Range");
    cvShowImage("Scaled Low-Range", cvscaledLow.get());
    cvSaveImage("./mas02_scaled_lowrange.jpg", cvscaledLow.get());
    
    createWindow("Scaled High-Range");
    cvShowImage("Scaled High-Range", cvscaledHigh.get());
    cvSaveImage("./mas02_scaled_highrange.jpg", cvscaledHigh.get());
    
    createWindow("Low Density");
    cvShowImage("Low Density", lowDensityRegion.get());
    cvSaveImage("./mas02_lowdensity.jpg", lowDensityRegion.get());

    createWindow("High Density");
    cvShowImage("High Density", highDensityRegion.get());
    cvSaveImage("./mas02_highdensity.jpg", highDensityRegion.get());

    cvWaitKey(0);
}