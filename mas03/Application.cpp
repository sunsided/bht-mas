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
/// Correlates the specified raw image with the mask
/// </summary>
/// <param name="raw">The raw image.</param>
/// <param name="mask">The mask.</param>
/// <param name="candidate_x">The candidate x coordinate.</param>
/// <param name="candidate_y">The candidate y coordinate.</param>
/// <param name="min_coeff">The minimum correlation coefficient.</param>
/// <param name="max_coeff">The minimum correlation coefficient.</param>
/// <returns>image displaying the correlation coefficients.</returns>
image_t Application::correlate(const image_t& raw, const image_t& mask, samples_t& candidate_x, lines_t& candidate_y, sample_t& min_coeff, sample_t& max_coeff)
{
    // image to hold the coefficients
    image_t coeffs(new FloatImage(raw->samples, raw->lines, 1, true));

    const samples_t raw_samples     = raw->samples;
    const lines_t raw_lines         = raw->lines;
    const samples_t mask_samples    = mask->samples;
    const lines_t mask_lines        = mask->lines;

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
    const samples_t rightmost_exclusive = raw_samples - mask_samples;

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
    
    // find minimum and maximum coefficients
    min_coeff = FLT_MAX;
    max_coeff = FLT_MIN;
    candidate_x = 0;
    candidate_y = 0;
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

                candidate_x = x;
                candidate_y = y;

                // THEORY: filtering out values that are the local maximum within the neighbourhood of the mask's size yields all candidates
                // THEORY: applying median segmentation of the grey levels of all candidates yields strong candidates
            }
        }
    }

    return coeffs;
}

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
image_t Application::difference(const image_t& raw, const image_t& mask, out samples_t& candidate_x, out lines_t& candidate_y, out sample_t& min_diff, out sample_t& max_diff)
{
    // image to hold the coefficients
    image_t diffs(new FloatImage(raw->samples, raw->lines, 1, true));

    const samples_t raw_samples     = raw->samples;
    const lines_t raw_lines         = raw->lines;
    const samples_t mask_samples    = mask->samples;
    const lines_t mask_lines        = mask->lines;

    // OpenMP needs signed integral type
    typedef int_fast32_t omp_linecount_t;
    omp_linecount_t omp_lines = raw_lines;

    // iterate over all pixels
    const omp_linecount_t bottommost_exclusive = omp_lines - mask_lines;
    const samples_t rightmost_exclusive = raw_samples - mask_samples;

    #pragma omp parallel for
    for (omp_linecount_t y = 0; y < bottommost_exclusive; ++y)
    {
        line_t &diffs_line = diffs->line(y);

        for (samples_t x = 0; x < rightmost_exclusive; ++x)
        {
            // correlate the pixels in mask-space (in the area overlayed by the mask)
            sample_t block_difference = 0.0F;
            for (lines_t my = 0; my < mask_lines; ++my) // loop all lines in mask-space
            {
                const line_t &raw_line = raw->line(y+my);
                const line_t &mask_line = mask->line(my);

                for (samples_t mx = 0; mx < mask_samples; ++mx) // loop all samples in mask-space
                {
                    const sample_t &raw_sample = raw_line->sample(x+mx);
                    const sample_t &mask_sample = mask_line->sample(mx);
                    
                    block_difference += abs(raw_sample - mask_sample);
                }
            }

            // remember coefficients for later display
            diffs_line->sample(x) = block_difference;
        }
    }

    // find minimum and maximum coefficients
    min_diff = FLT_MAX;
    max_diff = FLT_MIN;
    candidate_x = 0;
    candidate_y = 0;
    for (omp_linecount_t y = 0; y < bottommost_exclusive; ++y)
    {
        line_t &coeffs_line = diffs->line(y);

        for (samples_t x = 0; x < rightmost_exclusive; ++x)
        {
            sample_t &sample = coeffs_line->sample(x);
            if (sample < min_diff)
            {
                min_diff = sample;

                candidate_x = x;
                candidate_y = y;

                // THEORY: filtering out values that are the local maximum within the neighbourhood of the mask's size yields all candidates
                // THEORY: applying median segmentation of the grey levels of all candidates yields strong candidates
            }
            if (sample > max_diff)
            {
                max_diff = sample;
            }
        }
    }

    return diffs;
}

/// <summary>
/// Marks the candidate in an OpenCV BGR image in red.
/// </summary>
/// <param name="image">The image.</param>
/// <param name="candidate_x">The candidate x position.</param>
/// <param name="candidate_y">The candidate y position.</param>
/// <param name="samples">The samples.</param>
/// <param name="lines">The lines.</param>
void Application::markCandidateInOpenCvBGR(IplImagePtr& image, const samples_t& candidate_x, const lines_t& candidate_y, const samples_t& samples, const lines_t& lines, const uint8_t pixel_offset)
{
    assert(pixel_offset >= 0 && pixel_offset <= 2);

    // draw horizontal line
    uint_fast32_t start_sample = candidate_x * 3;
    for (samples_t x=start_sample; x <start_sample + samples*3; x+=3)
    {
        char* top_line = &image->imageData[candidate_y * image->widthStep];
        char& pixel_top = top_line[x+pixel_offset];

        char* bottom_line = &image->imageData[(candidate_y + lines) * image->widthStep];
        char& pixel_bot = bottom_line[x+pixel_offset];

        pixel_top = static_cast<char>(255U);
        pixel_bot = static_cast<char>(255U);
    }

    // draw vertical line
    uint_fast32_t start_line = candidate_y*image->widthStep;
    for (lines_t y=start_line; y <start_line + lines*image->widthStep; y+=image->widthStep)
    {
        char& pixel_left = image->imageData[y+ start_sample + pixel_offset];
        char& pixel_right = image->imageData[y+ start_sample + samples*3 + pixel_offset];

        pixel_left = pixel_right = static_cast<char>(255U);
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

    sample_t min_coeff = FLT_MAX, max_coeff = FLT_MIN;
    samples_t corr_max_match_x = 0;
    samples_t corr_max_match_y = 0;
    auto corr_coeffs = correlate(raw, mask, corr_max_match_x, corr_max_match_y, min_coeff, max_coeff);
    cout << "done." << endl;

    cout << "correlation coefficient in range " << min_coeff << " .. " << max_coeff << endl;
    cout << "correlation maximum match at {" << to_string(corr_max_match_x) << ", " << to_string(corr_max_match_y) << "}" << endl;

    // display correlation coefficients
    OpenCvWindow& corr_coeffs_window = createWindow("correlation coefficients");
    auto corr_coeff_cv = corr_coeffs->toOpenCv(0.0F, max_coeff); // ignoring all negative correlation coefficients
    corr_coeffs_window.showImage(corr_coeff_cv);

    // === build difference ===

    cout << "Calculating difference coefficients ... ";
    
    min_coeff = FLT_MAX; max_coeff = FLT_MIN;
    samples_t diff_max_match_x = 0;
    samples_t diff_max_match_y = 0;
    auto diff_coeffs = difference(raw, mask, diff_max_match_x, diff_max_match_y, min_coeff, max_coeff);
    cout << "done." << endl;

    cout << "difference coefficient in range " << min_coeff << " .. " << max_coeff << endl;
    cout << "difference maximum match at {" << to_string(diff_max_match_x) << ", " << to_string(diff_max_match_y) << "}" << endl;

    // display correlation coefficients
    OpenCvWindow& diff_coeffs_window = createWindow("difference coefficients");
    auto diff_coeff_cv = diff_coeffs->toOpenCv(0.0F, max_coeff); // ignoring all negative correlation coefficients
    diff_coeffs_window.showImage(diff_coeff_cv);

    // === display raw picture ===

    // display raw picture
    OpenCvWindow& corr_coeffs_raw_window = createWindow("raw picture for correlation coefficients");
    auto raw_cv = raw->toOpenCvBGR();
    markCandidateInOpenCvBGRInRed(raw_cv, corr_max_match_x, corr_max_match_y, mask->samples, mask->lines);
    markCandidateInOpenCvBGRInGreen(raw_cv, diff_max_match_x, diff_max_match_y, mask->samples, mask->lines);
    corr_coeffs_raw_window.showImage(raw_cv);

    cvWaitKey(0);

    /*
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
    */
}