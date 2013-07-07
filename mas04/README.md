# Image Filtering

This project is about image-filtering through kernels.

## Methods used

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

### Highpass: Laplacian Filter

Using the 3x3 [laplacian kernel](http://en.wikipedia.org/wiki/Discrete_Laplace_operator)
    
    | 0 |  1 | 0 |
    | 1 | -4 | 1 |
    | 0 |  1 | 0 |

### Highpass: Laplacian-of-Gaussian Filter

The LoG kernel ([Marr-Hildreth algorithm](http://en.wikipedia.org/wiki/Marr%E2%80%93Hildreth_algorithm)) is the application of same-sized laplacian and gaussian kernels in sequence (in either combination, i.e. laplacian first, then gaussian or gaussian first, then laplacian). 

The kernel used was derived by multiplying the aforementioned laplacian filter kernel by the 3x3 [gaussian kernel](http://en.wikipedia.org/wiki/Gaussian_filter)
    
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
