#pragma warning(push) // Disable deprecation
#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <iostream>
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
    samplecount_t step = displayImage->widthStep;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = image[y];
        uint_fast32_t lineOffset = y*(step);

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
shared_ptr<Stats> Application::calculateStatisticsNaive(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands
                                            ) const
{
    assert(bands == 1);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;

    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);

    // first run: gather min, max and mean
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        stats_t lineSum = 0;

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
    stats_t variance = 0;
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        stats_t rowVariance = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            const stats_t diff = sample - mean;
            rowVariance += (diff * diff);
        }

        // aggregate variance per line
        variance += rowVariance * invSamplesA; // Stichprobenvarianz, sample variance
    }

    // second run, part two: scale variance and calculate standard deviation
    variance *= invLines;
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
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
shared_ptr<Stats> Application::calculateStatisticsNaiveDivideConquer(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands
                                                        ) const
{
    assert(bands == 1);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;

    const stats_t count = static_cast<stats_t>(samples * lines);
    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);

    // array for intermediate results of each line
    unique_ptr<stats_t[]> intermediate(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_min(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_max(new stats_t[lines]);

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    // first run: gather min, max and mean
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y];
        stats_t lineSum = 0;
        stats_t lineMin = FLT_MAX;
        stats_t lineMax = FLT_MIN;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            // update mean
            lineSum += sample;

            // update minimum
            if (sample < lineMin) {
                lineMin = sample;
            }

            // update maximum
            if (sample > lineMax) {
                lineMax = sample;
            }
        }

        // aggregate mean per line
        intermediate[y] = lineSum;
        intermediate_min[y] = lineMin;
        intermediate_max[y] = lineMax;
    }

    // conquer intermediate results
    for(linecount_t y=0; y<lines; ++y)
    {
        // update minimum
        stats_t& lineMin = intermediate_min[y];
        if (lineMin < min) {
            min = lineMin;
        }

        // update maximum
        stats_t& lineMax = intermediate_max[y];
        if (lineMax > max) {
            max = lineMax;
        }

        // update mean
        mean += intermediate[y];
    }
    mean *= invLines * invSamples;

    // release memory
    intermediate_max.reset();
    intermediate_min.reset();
    
    // second run: calculate variance
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y];
        stats_t rowVariance = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            const stats_t diff = sample - mean;
            rowVariance += (diff * diff);
        }

        // aggregate variance per line
        intermediate[y] = rowVariance;
    }

    // second run, part two: scale variance and calculate standard deviation
    // conquer intermediate results
    stats_t variance = 0;
    for(linecount_t y=0; y<lines; ++y)
    {
        variance += intermediate[y];
    }
    variance *= invLines * invSamplesA; // Stichprobenvarianz, sample variance
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
}


/// <summary>
/// Forward-calculation of the statistics with divide-and-conquer
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="min">Output: The minimum value.</param>
/// <param name="max">Output: The maximum value.</param>
/// <param name="mean">Output: The mean value.</param>
/// <param name="stdDev">Output: The standard deviation.</param>
shared_ptr<Stats> Application::calculateStatisticsForward(const image_t& image, const samplecount_t& sample_first, const samplecount_t& sample_last, const linecount_t& line_first, const linecount_t& line_last, const bandcount_t& bands
                                                        ) const
{
    assert(bands == 1);
    assert(sample_last >= sample_first);
    assert(line_last >= line_first);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;
    stats_t variance = 0;

    samplecount_t samples = sample_last - sample_first + 1;
    linecount_t lines = line_last - line_first + 1;

    const stats_t count = static_cast<stats_t>(samples * lines);
    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);
    
    stats_t sum = 0;
    stats_t squareSum = 0;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    // array for intermediate results of each line
    unique_ptr<stats_t[]> intermediate_sum(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_ssq(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_min(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_max(new stats_t[lines]);

    // single run: gather min, max, mean and standard deviation
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y+line_first];
        stats_t lineSum = 0;
        stats_t lineSumSq = 0;
        stats_t lineMin = FLT_MAX;
        stats_t lineMax = FLT_MIN;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=sample_first; x<=sample_last; ++x)
        {
            const sample_t& sample = line[x];
            
            lineSum += sample;
            lineSumSq += sample * sample;

            // update minimum
            if (sample < lineMin) {
                lineMin = sample;
            }

            // update maximum
            if (sample > lineMax) {
                lineMax = sample;
            }
        }

        // store
        intermediate_sum[y] = lineSum;
        intermediate_ssq[y] = lineSumSq;
        intermediate_min[y] = lineMin;
        intermediate_max[y] = lineMax;
    }

    // conquer intermediate results
    for(linecount_t y=0; y<lines; ++y)
    {
        // update minimum
        stats_t& lineMin = intermediate_min[y];
        if (lineMin < min) {
            min = lineMin;
        }

        // update maximum
        stats_t& lineMax = intermediate_max[y];
        if (lineMax > max) {
            max = lineMax;
        }

        // update mean
        sum += intermediate_sum[y];
        squareSum += intermediate_ssq[y];
    }

    // augment
    mean = sum / count;
    variance = (squareSum - (mean*sum))/(count-1);

    // and finalize
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
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
    cout << "Calculating statistics (naive) ... ";
    auto stats = calculateStatisticsNaive(image, samples, lines, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // calculate naive d/c statistics
    cout << "Calculating statistics (naive divide-and-conquer) ... ";
    stats = calculateStatisticsNaiveDivideConquer(image, samples, lines, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // calculate forward statistics
    cout << "Calculating statistics (forward d&q) ... ";
    stats = calculateStatisticsForward(image, 0, samples-1, 0, lines-1, bands);
    cout << "done" << endl;
    cout << stats << endl;

    // convert image to OpenCV image.
    cout << "Converting image for display ... ";
    IplImagePtr displayImage = enviToOpenCv(image, samples, lines, bands);
    cout << "done" << endl;

    // Display
    createWindow("Original");
    cvShowImage("Original", displayImage.get());

    cvWaitKey(0);
}