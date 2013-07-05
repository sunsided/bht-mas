#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#pragma warning( disable : 4290 ) // disable throw() not implemented by MSVC

#include <string>
#include <vector>

#include "ENVIFileReader.h"
#include "OpenCvImage.h"

/// <summary>Marks a variable as output</summary>
#define out

/// <summary>Data type used for statistics</summary>
typedef float stats_t;

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
    IplImagePtr enviToOpenCv(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands) const;

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
    void calculateStatisticsNaive(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands, 
                                  out stats_t& min, out stats_t& max, out stats_t& mean, out stats_t& stdDev) const;

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
    void calculateStatisticsNaiveDivideConquer(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands, 
                                               out stats_t& min, out stats_t& max, out stats_t& mean, out stats_t& stdDev) const;

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
    void calculateStatisticsForward(const envi::image_t& image, const envi::samplecount_t& samples, const envi::linecount_t& lines, const envi::bandcount_t& bands, 
                                    out stats_t& min, out stats_t& max, out stats_t& mean, out stats_t& stdDev) const;
};

#endif