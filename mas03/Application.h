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
};

#endif