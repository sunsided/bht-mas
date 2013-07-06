#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#pragma warning( disable : 4290 ) // disable throw() not implemented by MSVC

#include <cstdint>
#include <string>
#include <vector>

#include "ENVIFileReader.h"
#include "OpenCvImage.h"
#include "Stats.h"

/// <summary>Marks a variable as output</summary>
#define out

/// <summary>A single histogram bin</summary>
typedef float histogram_bin;

/// <summary>
/// Main application class
/// </summary>
class Application 
{
private:
    /// <summary>
    /// Vector of all created window names
    /// </summary>
    std::vector<const std::string> _windowNames;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="Application"/> class.
    /// </summary>
    Application();
    
    /// <summary>
    /// Finalizes an instance of the <see cref="Application"/> class.
    /// </summary>
    ~Application() ;
    
    /// <summary>
    /// Runs this instance.
    /// </summary>
    void run() throw(std::runtime_error);

private:

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in] The name. </param>
    inline void createWindow(const char* name)
    {
        createWindow(std::string(name));
    }

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in] The name. </param>
    void createWindow(const std::string& name);

    /// <summary>
    /// Converts an ENVI image to OpenCV
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <returns>The converted image</returns>
    IplImagePtr enviToOpenCv(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands) const
    {
        return enviToOpenCv(image, 0, samples-1, 0, lines-1, bands);
    }
    
    /// <summary>
    /// Converts an ENVI image to OpenCV
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <returns>The converted image</returns>
    IplImagePtr enviToOpenCv(const envi::image_t& image, const envi::samplecount_t& sample_first, const envi::samplecount_t& sample_last, const envi::linecount_t& line_first, 
        const envi::linecount_t& line_last, const envi::bandcount_t& bands) const;

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
    std::shared_ptr<Stats> calculateStatisticsNaive(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands) const;

    /// <summary>
    /// Naive calculation of the statistics using divide-and-conquer
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <param name="min">Output: The minimum value.</param>
    /// <param name="max">Output: The maximum value.</param>
    /// <param name="mean">Output: The mean value.</param>
    /// <param name="stdDev">Output: The standard deviation.</param>
    std::shared_ptr<Stats> calculateStatisticsNaiveDivideConquer(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands) const;

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
    std::shared_ptr<Stats> calculateStatisticsForward(const envi::image_t& image, const envi::samplecount_t& sample_first, const envi::samplecount_t& sample_last, const envi::linecount_t& line_first, 
        const envi::linecount_t& line_last, const envi::bandcount_t& bands ) const;

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
    std::shared_ptr<Stats> calculateStatisticsForward(const envi::image_t& image, const envi::samplecount_t& samples, const envi::samplecount_t& lines, const envi::bandcount_t& bands ) const {
        return calculateStatisticsForward(image, 0, samples-1, 0, lines-1, bands);
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
    std::shared_ptr<Stats> calculateStatistics(const envi::image_t& image, const envi::samplecount_t& sample_first, const envi::samplecount_t& sample_last, const envi::linecount_t& line_first, 
        const envi::linecount_t& line_last, const envi::bandcount_t& bands ) const 
    {
        return calculateStatisticsForward(image, sample_first, sample_last, line_first, line_last, bands);
    }

    /// <summary>
    /// Builds the histogram.
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="class_count">The number of classes.</param>
    /// <returns>The classes.</returns>
    std::unique_ptr<histogram_bin[]> buildHistogram(const envi::image_t& image, const envi::samplecount_t& samples, const envi::samplecount_t& lines, const envi::bandcount_t& bands, 
        const stats_t low_value, const stats_t high_value, const uint_fast8_t class_count = 10) const;
};

#endif