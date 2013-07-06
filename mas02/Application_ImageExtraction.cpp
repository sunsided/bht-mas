#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <cmath>
#include <memory>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include "ENVIFileReader.h"
#include "Application.h"

using namespace std;
using namespace envi;

/// <summary>
/// Converts an ENVI image to OpenCV
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <returns>The converted image</returns>
IplImagePtr Application::enviToOpenCv(const image_t& image, const samplecount_t& sample_first, const samplecount_t& sample_last, const linecount_t& line_first, 
        const linecount_t& line_last, const bandcount_t& bands, const envi::sample_t& min, const envi::sample_t& max) const
{
    assert(bands == 1);
    assert(sample_last >= sample_first);
    assert(line_last >= line_first);
    assert (min < max);
   
    samplecount_t samples = sample_last - sample_first + 1;
    linecount_t lines = line_last - line_first + 1;
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, bands));
    samplecount_t step = displayImage->widthStep;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    const float lerp_scaling = 255.0F / (max - min);

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = image[y+line_first];
        uint_fast32_t lineOffset = y*(step);

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            char& pixel = displayImage->imageData[lineOffset+x];

            // pick the sample and convert it to 8-bit unsigned
            sample_t sample = line[x+sample_first];
            
            // lerp the value
            sample = (sample - min) * lerp_scaling;

            // assign sample (prediction friendly)
            uint_fast8_t value = static_cast<uint_fast8_t>(sample);
            pixel = value;

            // correct in case of problems
            if (sample < 0.0F) {
                pixel = 0;
            }
            if (sample > 255.0F) {
                pixel = (char)255;
            }
        }
    }
    return displayImage;
}