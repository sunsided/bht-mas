# Image Filtering

This project is about image-filtering through kernels.

## Filter Kernels and Median Filter

All filter kernels are weighted on-the-fly.

### Allpass Filter

Using the 3x3 discrete dirac (unit) filter kernel
    
    | 0 | 0 | 0 |
    | 0 | 1 | 0 |
    | 0 | 0 | 0 |
    
### Lowpass: Box Filter

Using the 3x3 box filter kernel
    
    | 1 | 1 | 1 |
    | 1 | 1 | 1 |
    | 1 | 1 | 1 |
    
(well, actually a 9x9 filter but I'm too lazy here)

### Lowpass: Gaussian Filter

The 5x5 [gaussian kernel](http://en.wikipedia.org/wiki/Gaussian_filter) used in this project is
    
    |  1 |  4 |  7 |  4 |  1 |
    |  4 | 16 | 26 | 16 |  4 |
    |  7 | 26 | 41 | 26 |  7 |
	|  4 | 16 | 26 | 16 |  4 |
    |  1 |  4 |  7 |  4 |  1 |

The coefficients used here are taken from [Gaussian Smoothing](http://homepages.inf.ed.ac.uk/rbf/HIPR2/gsmooth.htm) and represent a gaussian kernel with a standard deviation of 1. Note again that this filter is weighted - scaled by the sum of its coefficients - in the convolution method itself.

Another 3x3 gaussian filter is used in the *Laplacian-of-Gaussian* section below. The kernel described there could have been used here but resulted in ludicrously bad smoothing especially in the context of salt-and-pepper noise.

### Highpass: Laplacian Filter

Using the 3x3 [laplacian kernel](http://en.wikipedia.org/wiki/Discrete_Laplace_operator)
    
    | 0 |  1 | 0 |
    | 1 | -4 | 1 |
    | 0 |  1 | 0 |

### Highpass: Laplacian-of-Gaussian Filter

The LoG kernel ([Marr-Hildreth algorithm](http://en.wikipedia.org/wiki/Marr%E2%80%93Hildreth_algorithm)) is the application of same-sized laplacian and gaussian kernels in sequence (in either combination, i.e. laplacian first, then gaussian or gaussian first, then laplacian). 

The kernel used was derived by multiplying the aforementioned laplacian filter kernel by the 3x3 gaussian filter kernel
    
    | 1 | 2 | 1 |
    | 2 | 4 | 2 |
    | 1 | 2 | 1 |

resulting in the following 5x5 LoG kernel
    
    |  0 | -1 | -2 | -1 |  0 |
    | -1 |  0 |  2 |  0 | -1 |
    | -2 |  2 |  8 |  2 | -2 |
    | -1 |  0 |  2 |  0 | -1 |
    |  0 | -1 | -2 | -1 |  0 |
    
Since I was too lazy to calculate it myself, I simply used the kernel as it was described [here](http://kurse.fh-regensburg.de/cato/module/bildverarbeitung/pr/modul_5/pdf/hochpass_s4.pdf).

### Median Filter

Implemented by moving a `N`-by-`N` filter window (with `N` being any odd number) over the picture, sorting all values within this window and picking the median. Sorting is done (as requested in the exercise) by using bubble sort.

## Noise processes

### Additive white gaussian noise

The AWGN noise is realized using `std::normal_distribution<float>` and the `std::default_random_engine` with configurable standard deviation and a gain (signal-to-noise ratio).

Values taken from the normal distribution are scaled by the given gain and then added to the sample value. Values smaller than zero or larger than one are truncated during image conversion for OpenCV.

### Salt-and-pepper noise

The SnP noise is realized using `std::uniform_real_distribution<float>` and the `std::default_random_engine` with configurable pepper and salt probabilities.

The salt/pepper decision is based on a 0..1 ranged real value drawn from the uniform distribution by comparing it to a threshold derived from the probability, i.e.

	draw value in 0 .. 1 from uniform distribution
	if value is smaller than pepper probability 
		apply pepper
	if value is larger than (1 - salt probability)
		apply salt