#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <cmath>
#include <memory>

#include "Application.h"

using namespace std;
using namespace envi;

/// <summary>
/// Builds the histogram.
/// </summary>
/// <param name="image">The image.</param>
/// <param name="class_count">The number of classes.</param>
/// <returns>The classes.</returns>
unique_ptr<histogram_bin[]> Application::buildHistogram(const envi::image_t& image, const samplecount_t& samples, const samplecount_t& lines, const bandcount_t& bands, const stats_t low_value, const stats_t high_value, const uint_fast8_t class_count) const
{
    assert (bands == 1);
    assert (high_value >= low_value);

    const stats_t width = high_value - low_value;
    const stats_t invWidth = 1.0F / width;

    const stats_t f_count = static_cast<stats_t>(class_count);
    const stats_t step = width / f_count;

    // create a buffer of lines classes per line
    typedef histogram_bin* histogram_t;
    typedef histogram_t*   line_histograms_t;

    line_histograms_t line_histograms = new histogram_t[lines];
    for (linecount_t y=0; y<lines; ++y)
    {
        line_histograms[y] = new histogram_bin[class_count];
        for (uint_fast8_t c=0; c<class_count; ++c)
        {
            line_histograms[y][c] = 0;
        }
    }

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = image[y];
        auto& histogram = line_histograms[y];

        float lineSum = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            sample_t sample = line[x];

            // adjust for lower boundary
            sample = sample - low_value;

            // adjust for upper boundary
            sample *= invWidth;
            
            // lerp to class and add
            uint_fast8_t c = static_cast<uint_fast8_t>(floor(sample * f_count));

            // skip value if it is outside our histogram bounds
            if (sample < 0.0F || sample > 1.0F)
            {
                continue;
            }

            ++histogram[c];
        }
    }
    
    // initialize the composite histogram
    unique_ptr<histogram_bin[]> histogram(new float[class_count]);
    for (uint_fast8_t c=0; c<class_count; ++c)
    {
        histogram[c] = 0;
    }

    // aggregate the line histograms
    float count = 0;
    for (linecount_t y=0; y<lines; ++y)
    {
        // fetch the histogram of the current line and aggregate it
        auto& line_hist = line_histograms[y];
        for (uint_fast8_t c=0; c<class_count; ++c)
        {
            histogram[c] += line_hist[c];
            count += line_hist[c];
        }

        // delete the current line, we don't need it anymore
        delete[] line_hist;
    }

    // delete the line histogram table
    delete[] line_histograms;

    // scale composite histogram
    const float invCount = 1.0F / count;
    for (uint_fast8_t c=0; c<class_count; ++c)
    {
        histogram[c] *= invCount;
    }

    return histogram;
}
