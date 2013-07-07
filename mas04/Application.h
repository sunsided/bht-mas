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
    /// Convolves the image with a dirac (unit) kernel
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <returns>The convolved image in OpenCV format.</returns>
    static IplImagePtr convolveDirac(const image_t& raw);

    /// <summary>
    /// Convolves the image with a box blur kernel
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <returns>The convolved image in OpenCV format.</returns>
    static IplImagePtr convolveBox(const image_t& raw);
    
    /// <summary>
    /// Convolves the image with a gaussian (low-pass) kernel
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <returns>The convolved image in OpenCV format.</returns>
    IplImagePtr convolveGaussian(const image_t& raw);

    /// <summary>
    /// Convolves the image with a laplacian (high-pass) kernel
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <returns>The convolved image in OpenCV format.</returns>
    static IplImagePtr convolveLaplacian(const image_t& raw);

    /// <summary>
    /// Convolves the image with a laplacian-of-gaussian (high-pass) kernel
    /// </summary>
    /// <param name="raw">The raw image.</param>
    /// <returns>The convolved image in OpenCV format.</returns>
    static IplImagePtr convolveLoG(const image_t& raw);

    /// <summary>
    /// Applies an additive white gaussian noise.
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="gain">The noise gain (implicit signal to noise ratio).</param>
    /// <param name="standard_deviation">The noise standard deviation.</param>
    static void applyAWGN(image_t& image, const float gain = 1.0F, const float standard_deviation = 1.0F);

    /// <summary>
    /// Applies salt-and-pepper noise
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="pepper_probability">The probability of pepper values (0..1).</param>
    /// <param name="salt_probability">The probability of salt values (0..1)</param>
    /// <param name="pepper_value">The pepper value (low value).</param>
    /// <param name="salt_value">The salt value (high value).</param>
    static void applySnP(image_t& image, const float pepper_probability = 0.01F, const float salt_probability = 0.01F, const sample_t& pepper_value = 0.0F, const sample_t& salt_value = 1.0F);

    /// <summary>
    /// Applies a median filter of the given size.
    /// </summary>
    /// <param name="raw">The image.</param>
    /// <param name="size">The kernel size, must be an odd number.</param>
    /// <returns>The filtered image.</returns>
    static image_t applyMedianFilter(const image_t& raw, const uint_fast8_t size = 3);
};

#endif