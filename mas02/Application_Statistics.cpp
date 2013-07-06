#pragma warning(disable: 4996) // Disable deprecation

#include <algorithm>
#include <cmath>
#include <memory>

#include "Application.h"
#include "Stats.h"

using namespace std;
using namespace envi;

/// <summary>
/// Naive calculation of the statistics
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="min">Output: The minimum value.</param>
/// <param name="max">Output: The maximum value.</param>
/// <param name="mean">Output: The mean value.</param>
/// <param name="stdDev">Output: The standard deviation.</param>
shared_ptr<Stats> Application::calculateStatisticsNaive(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands
                                            ) const
{
    assert(bands == 1);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;

    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);

    // first run: gather min, max and mean
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        stats_t lineSum = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            // update mean
            lineSum += sample;

            // update minimum
            if (sample < min) {
                min = sample;
            }

            // update maximum
            if (sample > max) {
                max = sample;
            }
        }

        // aggregate mean per line
        mean += lineSum * invSamples;
    }

    // adjust mean over all lines
    mean *= invLines;
    
    // second run: calculate variance
    stats_t variance = 0;
    for(linecount_t y=0; y<lines; ++y)
    {
        const line_t& line = image[y];
        stats_t rowVariance = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            const stats_t diff = sample - mean;
            rowVariance += (diff * diff);
        }

        // aggregate variance per line
        variance += rowVariance * invSamplesA; // Stichprobenvarianz, sample variance
    }

    // second run, part two: scale variance and calculate standard deviation
    variance *= invLines;
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
}

/// <summary>
/// Naive calculation of the statistics
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="min">Output: The minimum value.</param>
/// <param name="max">Output: The maximum value.</param>
/// <param name="mean">Output: The mean value.</param>
/// <param name="stdDev">Output: The standard deviation.</param>
shared_ptr<Stats> Application::calculateStatisticsNaiveDivideConquer(const image_t& image, const samplecount_t& samples, const linecount_t& lines, const bandcount_t& bands
                                                        ) const
{
    assert(bands == 1);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;

    const stats_t count = static_cast<stats_t>(samples * lines);
    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);

    // array for intermediate results of each line
    unique_ptr<stats_t[]> intermediate(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_min(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_max(new stats_t[lines]);

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    // first run: gather min, max and mean
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y];
        stats_t lineSum = 0;
        stats_t lineMin = FLT_MAX;
        stats_t lineMax = FLT_MIN;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            // update mean
            lineSum += sample;

            // update minimum
            if (sample < lineMin) {
                lineMin = sample;
            }

            // update maximum
            if (sample > lineMax) {
                lineMax = sample;
            }
        }

        // aggregate mean per line
        intermediate[y] = lineSum;
        intermediate_min[y] = lineMin;
        intermediate_max[y] = lineMax;
    }

    // conquer intermediate results
    for(linecount_t y=0; y<lines; ++y)
    {
        // update minimum
        stats_t& lineMin = intermediate_min[y];
        if (lineMin < min) {
            min = lineMin;
        }

        // update maximum
        stats_t& lineMax = intermediate_max[y];
        if (lineMax > max) {
            max = lineMax;
        }

        // update mean
        mean += intermediate[y];
    }
    mean *= invLines * invSamples;

    // release memory
    intermediate_max.reset();
    intermediate_min.reset();
    
    // second run: calculate variance
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y];
        stats_t rowVariance = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=0; x<samples; ++x)
        {
            const sample_t& sample = line[x];
            
            const stats_t diff = sample - mean;
            rowVariance += (diff * diff);
        }

        // aggregate variance per line
        intermediate[y] = rowVariance;
    }

    // second run, part two: scale variance and calculate standard deviation
    // conquer intermediate results
    stats_t variance = 0;
    for(linecount_t y=0; y<lines; ++y)
    {
        variance += intermediate[y];
    }
    variance *= invLines * invSamplesA; // Stichprobenvarianz, sample variance
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
}


/// <summary>
/// Forward-calculation of the statistics with divide-and-conquer
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="min">Output: The minimum value.</param>
/// <param name="max">Output: The maximum value.</param>
/// <param name="mean">Output: The mean value.</param>
/// <param name="stdDev">Output: The standard deviation.</param>
shared_ptr<Stats> Application::calculateStatisticsForward(const image_t& image, const samplecount_t& sample_first, const samplecount_t& sample_last, const linecount_t& line_first, const linecount_t& line_last, const bandcount_t& bands
                                                        ) const
{
    assert(bands == 1);
    assert(sample_last >= sample_first);
    assert(line_last >= line_first);

    stats_t min = FLT_MAX;
    stats_t max = FLT_MIN;
    stats_t mean = 0;
    stats_t stdDev = 0;
    stats_t variance = 0;

    samplecount_t samples = sample_last - sample_first + 1;
    linecount_t lines = line_last - line_first + 1;

    const stats_t count = static_cast<stats_t>(samples * lines);
    const stats_t invLines = 1.0F / lines;
    const stats_t invSamples = 1.0F / samples;
    const stats_t invSamplesA = 1.0F / (samples-1);
    
    stats_t sum = 0;
    stats_t squareSum = 0;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    // array for intermediate results of each line
    unique_ptr<stats_t[]> intermediate_sum(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_ssq(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_min(new stats_t[lines]);
    unique_ptr<stats_t[]> intermediate_max(new stats_t[lines]);

    // single run: gather min, max, mean and standard deviation
    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        const line_t& line = image[y+line_first];
        stats_t lineSum = 0;
        stats_t lineSumSq = 0;
        stats_t lineMin = FLT_MAX;
        stats_t lineMax = FLT_MIN;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samplecount_t x=sample_first; x<=sample_last; ++x)
        {
            const sample_t& sample = line[x];
            
            lineSum += sample;
            lineSumSq += sample * sample;

            // update minimum
            if (sample < lineMin) {
                lineMin = sample;
            }

            // update maximum
            if (sample > lineMax) {
                lineMax = sample;
            }
        }

        // store
        intermediate_sum[y] = lineSum;
        intermediate_ssq[y] = lineSumSq;
        intermediate_min[y] = lineMin;
        intermediate_max[y] = lineMax;
    }

    // conquer intermediate results
    for(linecount_t y=0; y<lines; ++y)
    {
        // update minimum
        stats_t& lineMin = intermediate_min[y];
        if (lineMin < min) {
            min = lineMin;
        }

        // update maximum
        stats_t& lineMax = intermediate_max[y];
        if (lineMax > max) {
            max = lineMax;
        }

        // update mean
        sum += intermediate_sum[y];
        squareSum += intermediate_ssq[y];
    }

    // augment
    mean = sum / count;
    variance = (squareSum - (mean*sum))/(count-1);

    // and finalize
    stdDev = sqrt(variance);

    // up, up and away
    return shared_ptr<Stats>(new Stats(min, max, mean, stdDev));
}
