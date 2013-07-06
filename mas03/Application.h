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
};

#endif