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
/// Converts an ENVI image to OpenCV
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <returns>The converted image</returns>
IplImagePtr Application::enviToOpenCv(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands) const
{
    assert(bands == 1);
   
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, bands));

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = image[y];
        uint_fast32_t lineOffset = y*samples;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            char& pixel = displayImage->imageData[lineOffset+x];

            // pick the sample and convert it to 8-bit unsigned
            sample_t sample = line[x];
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
    return displayImage;
}


/// <summary>
/// Naive calculation of the statistics
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="min">Output: The minimum value.</param>
/// <param name="max">Output: The maximum value.</param>
/// <param name="mean">Output: The mean value.</param>
/// <param name="stdDev">Output: The standard deviation.</param>
void Application::calculateStatisticsNaive(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands, 
                                            out float& min, out float& max, out float& mean, out float& stdDev) const
{
    assert(bands == 1);

    min = FLT_MAX;
    max = FLT_MIN;
    mean = 0;
    stdDev = 0;

    const float invLines = 1.0F / lines;
    const float invSamples = 1.0F / samples;
    const float invSamplesA = 1.0F / (samples-1);

    // first run: gather min, max and mean
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        float lineSum = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            // update mean
            lineSum += sample;

            // update minimum
            if (sample < min) {
                min = sample;
            }

            // update maximum
            if (sample > max) {
                max = sample;
            }
        }

        // aggregate mean per line
        mean += lineSum * invSamples;
    }

    // adjust mean over all lines
    mean *= invLines;
    
    // second run: calculate variance
    float variance = 0;
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        float rowVariance = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            const float diff = sample - mean;
            rowVariance += (diff * diff);
        }

        // aggregate variance per line
        variance += rowVariance * invSamplesA; // Stichprobenvarianz, sample variance
    }

    // second run, part two: scale variance and calculate standard deviation
    variance *= invLines;
    stdDev = sqrt(variance);
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
    float min, max, mean, stddev;
    cout << "Calculating statistics (naive) ... ";
    calculateStatisticsNaive(image, samples, lines, bands, min, max, mean, stddev);
    cout << "done" << endl;
    cout << "Range " << min << " .. " << max << ", mean " << mean << " +/- " << stddev << endl;

    // convert image to OpenCV image.
    cout << "Converting image for display ... ";
    IplImagePtr displayImage = enviToOpenCv(image, samples, lines, bands);
    cout << "done" << endl;

    // Display
    createWindow("Original");
    cvShowImage("Original", displayImage.get());

    cvWaitKey(0);
}