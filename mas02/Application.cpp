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
/// Converts an ENVI image to OpenCV
/// </summary>
/// <param name="image">The image.</param>
/// <returns>The converted image</returns>
IplImagePtr Application::enviToOpenCv(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands) const
{
    assert(bands == 1);
    
    cout << "Converting image for display ... ";
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, bands));

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        envi::line_t& line = image[y];
        uint_fast32_t lineOffset = y*samples;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(envi::samplecount_t x=0; x<samples; ++x)
        {
            char& pixel = displayImage->imageData[lineOffset+x];

            // pick the sample and convert it to 8-bit unsigned
            envi::sample_t sample = line[x];
            uint_fast8_t value = static_cast<uint_fast8_t>(sample);

            // assign sample (prediction friendly)
            pixel = value;

            // correct in case of problems
            if (sample < 0.0F) {
                pixel = 0;
            }
            if (sample > 255.0F) {
                pixel = (char)255;
            }
        }
    }
    cout << "done" << endl;

    return displayImage;
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
    IplImagePtr displayImage = enviToOpenCv(image, samples, lines, bands);

    // Display
    createWindow("Original");
    cvShowImage("Original", displayImage.get());

    cvWaitKey(0);
}