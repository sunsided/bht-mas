# Image Statistics

This project is about dynamic memory management and simple statistics (minimum and maximum, mean, standard deviation) of an image. This project also covers radiometric transformations in the context of high dynamic range imaging.

The HDR image used is `ENVI HDR` format, which - in this case - is simply a stream of 4-byte (single precision) floating point numbers. Loading is done within the `ENVIFileReader` class.

## Simple statistics

The methods for generating image statistics can be found within the `Application_Statistics.cpp` file.

Three methods are used:

* *naïve standard deviation*
* *naïve standard deviation with divide-and-conquer*
* *forward standard deviation*

### Naïve standard deviation

Calculates the standard deviation by iterating over the value set once to generate the mean value, then iterating again. To prevent overflows, values are collected per line and then aggregated. 

### Naïve standard deviation with divide-and-conquer

Is like the first approach except that it splits the calculations into separable parts by storing per line values in a dedicated vector.

This vector is then post-processed at the end during the conquer phase.

### Forward standard deviation

Calculates the mean value and standard deviation on-the-fly by adjusting the previous result. This method is faster but less numerically stable due to rounding errors in floating point operations.

## Histogram

The histogram is calculated in `Application_Histogram.cpp`. The approach here is horribly slow due to the assumption that no a-priory knowledge of image is given. Some math operations in the linear interpolation part could be optimized if the radiometric resolution (i.e. value range) would be known beforehand.