#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#pragma warning( disable : 4290 ) // disable throw() not implemented by MSVC

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "FloatImage.h"
#include "OpenCvImage.h"
#include "OpenCvWindow.h"

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
    std::vector<const OpenCvWindowPtr> _windows;

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
    inline OpenCvWindow& createWindow(const char* name)
    {
        return createWindow(std::string(name));
    }

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in] The name. </param>
    OpenCvWindow& createWindow(const std::string& name);

    /// <summary>
    /// Loads a raw 8-bit unsigned single-channel image
    /// </summary>
    /// <param name="filepath">The filepath.</param>
    /// <returns>image_t.</returns>
    inline static image_t loadRawU8(const char* filepath, const samples_t samples, const lines_t lines) throw(std::runtime_error)
    {
        return loadRawU8(std::string(filepath), samples, lines);
    }

    /// <summary>
    /// Loads a raw 8-bit unsigned single-channel image
    /// </summary>
    /// <param name="filepath">The filepath.</param>
    /// <returns>image_t.</returns>
    static image_t loadRawU8(const std::string filepath, const samples_t samples, const lines_t lines) throw(std::runtime_error);

    /// <summary>
    /// Correlates the specified raw image with the mask
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <param name="mask">The mask.</param>
    /// <param name="candidate_x">The candidate x coordinate.</param>
    /// <param name="candidate_y">The candidate y coordinate.</param>
    /// <param name="min_coeff">The minimum correlation coefficient.</param>
    /// <param name="max_coeff">The minimum correlation coefficient.</param>
    /// <returns>image displaying the correlation coefficients.</returns>
    static image_t correlate(const image_t& raw, const image_t& mask, out samples_t& candidate_x, out lines_t& candidate_y, out sample_t& min_coeff, out sample_t& max_coeff);

    /// <summary>
    /// Calculates the absolute differences between the image and the mask
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <param name="mask">The mask.</param>
    /// <param name="candidate_x">The candidate x coordinate.</param>
    /// <param name="candidate_y">The candidate y coordinate.</param>
    /// <param name="min_diff">The minimum difference.</param>
    /// <param name="max_diff">The minimum difference.</param>
    /// <returns>image displaying the differences.</returns>
    static image_t difference(const image_t& raw, const image_t& mask, out samples_t& candidate_x, out lines_t& candidate_y, out sample_t& min_diff, out sample_t& max_diff);

    /// <summary>
    /// Marks the candidate in an OpenCV BGR image.
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="candidate_x">The candidate x position.</param>
    /// <param name="candidate_y">The candidate y position.</param>
    /// <param name="samples">The samples.</param>
    /// <param name="lines">The lines.</param>
    /// <param name="pixel_offset">The color code; 2 = red, 1 = green, 0 = blue.</param>
    static void markCandidateInOpenCvBGR(IplImagePtr& image, const samples_t& candidate_x, const lines_t& candidate_y, const samples_t& samples, const lines_t& lines, const uint8_t pixel_offset = 2);

    /// <summary>
    /// Marks the candidate in an OpenCV BGR image in red.
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="candidate_x">The candidate x position.</param>
    /// <param name="candidate_y">The candidate y position.</param>
    /// <param name="samples">The samples.</param>
    /// <param name="lines">The lines.</param>
    static inline void markCandidateInOpenCvBGRInRed(IplImagePtr& image, const samples_t& candidate_x, const lines_t& candidate_y, const samples_t& samples, const lines_t& lines)
    {
        markCandidateInOpenCvBGR(image, candidate_x, candidate_y, samples, lines, 2);
    }

    /// <summary>
    /// Marks the candidate in an OpenCV BGR image in green.
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="candidate_x">The candidate x position.</param>
    /// <param name="candidate_y">The candidate y position.</param>
    /// <param name="samples">The samples.</param>
    /// <param name="lines">The lines.</param>
    static inline void markCandidateInOpenCvBGRInGreen(IplImagePtr& image, const samples_t& candidate_x, const lines_t& candidate_y, const samples_t& samples, const lines_t& lines)
    {
        markCandidateInOpenCvBGR(image, candidate_x, candidate_y, samples, lines, 1);
    }
};

#endif