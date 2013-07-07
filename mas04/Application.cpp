#define USE_REAL_LENA 0

#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <random>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include "FloatImage.h"
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
    for(OpenCvWindowPtr& window: _windows)
    {
        window.reset();
    }

    // purge window names
    _windows.clear();
}

/// <summary>   Creates a window. </summary>
///
/// <remarks>   Markus, 05.07.2013. </remarks>
///
/// <param name="name"> [in,out] The name. </param>
OpenCvWindow& Application::createWindow(const string& name)
{
    OpenCvWindow* window = new OpenCvWindow(name);
    cvNamedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);

    _windows.emplace_back(OpenCvWindowPtr(window));

    return *window;
}

/// <summary>
/// Loads a raw 8-bit unsigned single-channel image
/// </summary>
/// <param name="filepath">The filepath.</param>
/// <returns>image_t.</returns>
image_t Application::loadRawU8(const std::string filepath, const samples_t samples, const lines_t lines)
{
    // open the input file
    ifstream inputFile;
    inputFile.open(filepath, ios_base::in | ios_base::beg | ios_base::binary);
    if (!inputFile.is_open()) throw runtime_error("Could not open input file");

    // load the image data
    auto image = FloatImage::createFromU8Raw(inputFile, samples, lines);

    // close the input file
    inputFile.close();

    return std::move(image);
}

/// <summary>
/// Convolves the image with a dirac (unit) kernel
/// </summary>
/// <param name="raw">The raw image.</param>
/// <returns>The convolved image in OpenCV format.</returns>
IplImagePtr Application::convolveDirac(const image_t& raw)
{
    // === create a 3x3 unit kernel ===
    
    auto kernel = FloatImage::create(3, 3, 1, true);
    kernel->set(1, 1, 1.0F);

    // === convolve with the kernel ===
    
    auto convolved = raw->convolve(kernel);

    // === convert to OpenCV ===
    
    return convolved->toOpenCv();
}

/// <summary>
/// Convolves the image with a box blur kernel
/// </summary>
/// <param name="raw">The raw image.</param>
/// <returns>The convolved image in OpenCV format.</returns>
IplImagePtr Application::convolveBox(const image_t& raw)
{
    // === create a 9x9 box blur kernel ===
    
    auto kernel = FloatImage::create(9, 9, 1, false);
    for (lines_t l=0; l<kernel->lines; ++l)
    {
        for (samples_t s=0; s<kernel->samples; ++s)
        {
            kernel->set(s, l, 1.0F);
        }
    }

    // === convolve with the kernel ===
    
    auto convolved = raw->convolve(kernel);

    // === convert to OpenCV ===
    
    return convolved->toOpenCv();
}

/// <summary>
/// Convolves the image with a laplacian (high-pass) kernel
/// </summary>
/// <param name="raw">The raw image.</param>
/// <returns>The convolved image in OpenCV format.</returns>
IplImagePtr Application::convolveLaplacian(const image_t& raw)
{
    // === create a 3x3 laplacian kernel ===
    
    auto kernel = FloatImage::create(3, 3, 1, true);
    kernel->set(0, 0, 0.0F);    kernel->set(0, 1, 1.0F);    kernel->set(0, 2, 0.0F);
    kernel->set(1, 0, 1.0F);    kernel->set(1, 1, -4.0F);    kernel->set(1, 2, 1.0F);
    kernel->set(2, 0, 0.0F);    kernel->set(2, 1, 1.0F);    kernel->set(2, 2, 0.0F);

    // === convolve with the kernel ===
    
    auto convolved = raw->convolve(kernel);

    // === convert to OpenCV ===
    
    return convolved->toOpenCv();
}

/// <summary>
/// Convolves the image with a laplacian-of-gaussian (high-pass) kernel
/// </summary>
/// <param name="raw">The raw image.</param>
/// <returns>The convolved image in OpenCV format.</returns>
IplImagePtr Application::convolveLoG(const image_t& raw)
{
    // === create a 3x3 laplacian kernel ===
    
    auto kernel = FloatImage::create(5, 5, 1, true);
    
    #define VALUE(x, y, v) kernel->set(x, y, static_cast<sample_t>(v))

    // example kernel taken from: http://kurse.fh-regensburg.de/cato/module/bildverarbeitung/pr/modul_5/pdf/hochpass_s4.pdf
    VALUE(0, 0, 0); VALUE(0, 1,-1); VALUE(0, 2,-2); VALUE(0, 3,-1); VALUE(0, 4, 0);
    VALUE(1, 0,-1); VALUE(1, 1, 0); VALUE(1, 2, 2); VALUE(1, 3, 0); VALUE(1, 4,-1);
    VALUE(2, 0,-2); VALUE(2, 1, 2); VALUE(2, 2, 8); VALUE(2, 3, 2); VALUE(2, 4,-2);
    VALUE(3, 0,-1); VALUE(3, 1, 0); VALUE(3, 2, 2); VALUE(3, 3, 0); VALUE(3, 4,-1);
    VALUE(4, 0, 0); VALUE(4, 1,-1); VALUE(4, 2,-2); VALUE(4, 3,-1); VALUE(4, 4, 0);
    
    #undef VALUE

    // === convolve with the kernel ===
    
    auto convolved = raw->convolve(kernel);

    // === convert to OpenCV ===
    
    return convolved->toOpenCv();
}


/// <summary>
/// Applies an additive white gaussian noise.
/// </summary>
/// <param name="image">The image.</param>
/// <param name="gain">The noise gain (implicit signal to noise ratio).</param>
/// <param name="standard_deviation">The noise standard deviation.</param>
void Application::applyAWGN(image_t& image, const float gain, const float standard_deviation)
{
    // create the noise distribution
    const float mean = 0.0F;
    normal_distribution<float> distribution(mean, standard_deviation);

    // the generator used
    default_random_engine generator;

    // apply the noise
    const samples_t samples = image->samples;
    const lines_t lines = image->lines;
    for (lines_t lineIndex = 0; lineIndex < lines; ++lineIndex)
    {
        line_t& line = image->line(lineIndex);

        // THEORY: Band agnostic, since samples should contain all bands
        for(samples_t x=0; x<samples; ++x)
        {
            sample_t& sample = line->sample(x);
            
            const sample_t noise = static_cast<sample_t>(gain * distribution(generator));
            sample += noise;
        }
    }
}

/// <summary>
/// Applies salt-and-pepper noise
/// </summary>
/// <param name="image">The image.</param>
/// <param name="pepper_probability">The probability of pepper values (0..1).</param>
/// <param name="salt_probability">The probability of salt values (0..1)</param>
/// <param name="pepper_value">The pepper value (low value).</param>
/// <param name="salt_value">The salt value (high value).</param>
void Application::applySnP(image_t& image, const float pepper_probability, const float salt_probability, const sample_t& pepper_value, const sample_t& salt_value)
{
    assert (salt_probability + pepper_probability <= 1.0F);

    // create the noise distribution
    uniform_real_distribution<float> distribution(0.0F, 1.0F);

    // the generator used
    default_random_engine generator;

    // define the thresholds
    const float pepper_threshold = 0.0F + pepper_probability;
    const float salt_threshold   = 1.0F - salt_probability;

    // apply the noise
    const samples_t samples = image->samples;
    const lines_t lines = image->lines;
    for (lines_t lineIndex = 0; lineIndex < lines; ++lineIndex)
    {
        line_t& line = image->line(lineIndex);

        // THEORY: Band agnostic, since samples should contain all bands
        for(samples_t x=0; x<samples; ++x)
        {
            sample_t& sample = line->sample(x);
            
            const float noise_percentage = distribution(generator);
            if (noise_percentage <= pepper_threshold)
            {
                sample = pepper_value;
            }
            else if (noise_percentage >= salt_threshold)
            {
                sample = salt_value;
            }
            
        }
    }
}

/// <summary>
/// Runs this instance.
/// </summary>
void Application::run()
{   
    // === load the raw data ===

    const samples_t     raw_samples = 512;
    const lines_t       raw_lines = 512;

#if USE_REAL_LENA
    auto raw = loadRawU8("./images/lena.raw", raw_samples, raw_lines);
    raw->flipVertical();
#else
    auto raw = loadRawU8("./images/lenaml.raw", raw_samples, raw_lines);
#endif
    
    // === display raw picture ===

    OpenCvWindow& window_raw = createWindow("raw picture");
    auto raw_cv = raw->toOpenCv();
    window_raw.showImage(raw_cv);
    cvWaitKey(1);

    // === apply noise ===

    const float awgn_gain = 0.5F; // encodes the signal-to-noise ratio
    const float awgn_standard_deviation = 0.125F;
    applyAWGN(raw, awgn_gain, awgn_standard_deviation);

    const float snp_salt = 1.0F;
    const float snp_pepper = 0.0F;
    const float snp_salt_probability = 0.01F;
    const float snp_pepper_probability = 0.01F;
    applySnP(raw, snp_pepper_probability, snp_salt_probability, snp_salt, snp_pepper);

    // === display noisy picture ===

    OpenCvWindow& window_noise = createWindow("noisy picture");
    auto noise_cv = raw->toOpenCv();
    window_noise.showImage(noise_cv);
    cvWaitKey(1);

    // === display dirac convolved picture ===

    OpenCvWindow& window_dirac = createWindow("dirac");
    auto dirac_cv = convolveDirac(raw);
    window_dirac.showImage(dirac_cv);
    cvWaitKey(1);

    // === display box convolved picture ===

    OpenCvWindow& window_box = createWindow("box");
    auto box_cv = convolveBox(raw);
    window_box.showImage(box_cv);
    cvWaitKey(1);

    // === display laplace convolved picture ===

    OpenCvWindow& window_laplace = createWindow("laplacian");
    auto laplacian_cv = convolveLaplacian(raw);
    window_laplace.showImage(laplacian_cv);
    cvWaitKey(1);
    
    // === display LoG convolved picture ===

    OpenCvWindow& window_log = createWindow("laplacian-of-gaussian");
    auto log_cv = convolveLoG(raw);
    window_log.showImage(log_cv);
    cvWaitKey(1);

    cvWaitKey(0);
}