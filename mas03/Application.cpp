#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>

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
/// Runs this instance.
/// </summary>
void Application::run()
{   
    // === load the raw data ===

    const samples_t     raw_samples = 512;
    const lines_t       raw_lines = 512;
    
    vector<string> raw_image_paths;
    raw_image_paths.push_back("./images/bild0.raw");
    raw_image_paths.push_back("./images/bild1.raw");
    raw_image_paths.push_back("./images/bild2.raw");
    raw_image_paths.push_back("./images/bild3.raw");
    raw_image_paths.push_back("./images/bild4.raw");
    raw_image_paths.push_back("./images/bild5.raw");
    raw_image_paths.push_back("./images/bild6.raw");

    vector<image_t> raw_images;
    for (string path : raw_image_paths)
    {
        auto image = loadRawU8(path, raw_samples, raw_lines);
        raw_images.push_back(std::move(image));
    }
    
    // === load the mask data ===

    const samples_t     mask_samples = 32;
    const lines_t       mask_lines = 32;
    auto mask = loadRawU8("./images/mask_32_32.raw", mask_samples, mask_lines);


    // === correlate ===
    cout << "Calculating correlation coefficients ... ";

    image_t& raw = raw_images[2];

    // image to hold the coefficients
    image_t coeffs(new FloatImage(raw_samples, raw_lines, 1, false));

    // get mask mean value
    const sample_t invMaskCount = 1.0F / (static_cast<sample_t>(mask_lines * mask_samples));
    sample_t mask_mean = 0.0F;
    for (lines_t my = 0; my < mask_lines; ++my) // loop all lines in mask-space
    {
        line_t &mask_line = raw->line(my);
        for (samples_t mx = 0; mx < mask_samples; ++mx) // loop all samples in mask-space
        {
            sample_t &mask_sample = mask_line->sample(mx);
            mask_mean += mask_sample;
        }
    }
    mask_mean *= invMaskCount;

    // OpenMP needs signed integral type
    typedef int_fast32_t omp_linecount_t;
    omp_linecount_t omp_lines = raw_lines;

    // iterate over all pixels
    const omp_linecount_t bottommost_exclusive = omp_lines - mask_lines;
    const sample_t rightmost_exclusive = raw_samples - mask_samples;

    #pragma omp parallel for
    for (omp_linecount_t y = 0; y < bottommost_exclusive; ++y)
    {
        line_t &coeffs_line = coeffs->line(y);

        for (samples_t x = 0; x < rightmost_exclusive; ++x)
        {
            // calculate mean values of the input image in mask-space
            // that is, in the area overlayed by the mask
            sample_t raw_mean = 0.0F;
            for (lines_t my = 0; my < mask_lines; ++my) // loop all lines in mask-space
            {
                const line_t &raw_line = raw->line(y+my);

                for (samples_t mx = 0; mx < mask_samples; ++mx) // loop all samples in mask-space
                {
                    const sample_t &raw_sample = raw_line->sample(x+mx);
                    raw_mean += raw_sample;
                }
            }
            raw_mean *= invMaskCount;

            // correlate the pixels in mask-space (in the area overlayed by the mask)
            sample_t block_cross_variance = 0.0F;
            sample_t block_raw_variance = 0.0F;
            sample_t block_mask_variance = 0.0F;
            for (lines_t my = 0; my < mask_lines; ++my) // loop all lines in mask-space
            {
                const line_t &raw_line = raw->line(y+my);
                const line_t &mask_line = mask->line(my);

                for (samples_t mx = 0; mx < mask_samples; ++mx) // loop all samples in mask-space
                {
                    const sample_t &raw_sample = raw_line->sample(x+mx);
                    const sample_t &mask_sample = mask_line->sample(mx);
                    
                    // calculate the errors
                    const sample_t raw_error = (raw_sample - raw_mean);
                    const sample_t mask_error = (mask_sample - mask_mean);
                    
                    // calculate the cross-variance (raw <-> mask)
                    const sample_t cross_variance = raw_error * mask_error;
                    block_cross_variance += cross_variance;

                    // calculate the variances
                    const sample_t raw_variance = raw_error * raw_error;
                    const sample_t mask_variance = mask_error * mask_error;
                    block_raw_variance += raw_variance;
                    block_mask_variance += mask_variance;
                }
            }

            // calculate the product of raw and mask standard deviations
            const sample_t sigma_raw_mask = sqrt(block_raw_variance * block_mask_variance);
            // sample_t sigma_raw_mask = (block_raw_variance * block_mask_variance); // THEORY: leaving away the square root yields the same qualitative result by gaining performance

            // calculate the correlation coefficient
            const sample_t corr_coeff = block_cross_variance / sigma_raw_mask;

            // remember coefficients for later display
            coeffs_line->sample(x) = corr_coeff;
        }
    }

    cout << "done." << endl;

    // find minimum and maximum coefficients
    sample_t min_coeff = FLT_MAX, max_coeff = FLT_MIN;
    samples_t max_match_x = 0;
    samples_t max_match_y = 0;
    for (omp_linecount_t y = 0; y < bottommost_exclusive; ++y)
    {
        line_t &coeffs_line = coeffs->line(y);

        for (samples_t x = 0; x < rightmost_exclusive; ++x)
        {
            sample_t &sample = coeffs_line->sample(x);
            if (sample < min_coeff)
            {
                min_coeff = sample;
            }
            if (sample > max_coeff)
            {
                max_coeff = sample;

                max_match_x = x;
                max_match_y = y;
            }
        }
    }

    cout << "correlation coefficient in range " << min_coeff << " .. " << max_coeff << endl;
    cout << "maximum match at {" << to_string(max_match_x) << ", " << to_string(max_match_y) << "}" << endl;

    // display correlation coefficients
    OpenCvWindow& corr_coeffs_window = createWindow("correlation coefficients");
    auto corr_coeff_cv = coeffs->toOpenCv(0.0F, max_coeff); // ignoring all negative correlation coefficients
    corr_coeffs_window.showImage(corr_coeff_cv);

    // display raw picture
    OpenCvWindow& corr_coeffs_raw_window = createWindow("raw picture for correlation coefficients");
    auto raw_cv = raw->toOpenCv();
    corr_coeffs_raw_window.showImage(raw_cv);

    cvWaitKey(0);

    // === animate ===

    OpenCvWindow& window = createWindow("Animation");
    bool break_loop = false;

    while (!break_loop)
    for (image_t& image : raw_images)
    {
        auto openCvImage = image->toOpenCv(); // todo prepare the OpenCV images
        window.showImage(openCvImage);
        int key = cvWaitKey(33);
        
        // leave display loop
        break_loop = key >= 0;
        if (break_loop) break;
    }
}