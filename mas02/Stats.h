#ifndef _STATS_H_
#define _STATS_H_

#include <iostream>
#include <memory>

/// <summary>Data type used for statistics</summary>
typedef float stats_t;

struct Stats {

    /// <summary>
    /// The minimum value
    /// </summary>
    const stats_t min;

    /// <summary>
    /// The maximum value
    /// </summary>
    const stats_t max;

    /// <summary>
    /// The mean value
    /// </summary>
    const stats_t mean;

    /// <summary>
    /// The standard deviation
    /// </summary>
    const stats_t standard_deviation;

    /// <summary>
    /// Initializes a new instance of the <see cref="Stats"/> struct.
    /// </summary>
    /// <param name="min">The min.</param>
    /// <param name="max">The max.</param>
    /// <param name="mean">The mean.</param>
    /// <param name="std">The standard deviation.</param>
    Stats(const stats_t& min, const stats_t& max, const stats_t& mean, const stats_t& std)
        : min(min), max(max), mean(mean), standard_deviation(std)
    {}

    friend std::ostream& operator<< (std::ostream& stream, const Stats& stats);
    friend std::ostream& operator<< (std::ostream& stream, const std::shared_ptr<Stats>& stats);
};

#endif