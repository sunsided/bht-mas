#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#pragma warning( disable : 4290 ) // disable throw() not implemented by MSVC

#include <cstdint>
#include <string>
#include <vector>

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
};

#endif