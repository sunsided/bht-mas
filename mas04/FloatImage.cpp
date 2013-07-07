#include "FloatImage.h"

using namespace std;

/// <summary>The inverse of 255.</summary>
const float FloatImageLine::inverse255 = 1.0F / 255.0F;

/// <summary>
/// Initializes a new instance of the <see cref="FloatImageLine"/> class.
/// </summary>
/// <param name="line">The line.</param>
FloatImageLine::FloatImageLine(const samples_t& samples, bool zero)
{
    _line.reset(new sample_t[samples]);
    if (!_line) throw runtime_error("not enough memory to create float image (sample array)");

    // initialize to zero if requested
    if (zero)
    {
        for(samples_t sample = 0; sample < samples; ++sample)
        {
            _line[sample] = 0.0F;
        }
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="FloatImage"/> class.
/// </summary>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <param name="zero">If true the pixels will be initialized to zero.</param>
FloatImage::FloatImage(const samples_t& samples, const lines_t& lines, const bands_t& bands, bool zero)
    : samples(samples), lines(lines), bands(bands), size(samples * lines)
{
    // create the line array
    imagedata_t image = imagedata_t(new line_t[lines]);
    if (!image) throw runtime_error("not enough memory to create float image (line array)");

    // for each line, create the sample array
    for (lines_t lineIndex = 0; lineIndex < lines; ++lineIndex)
    {
        image[lineIndex].reset(new FloatImageLine(samples, zero));
        if (!image[lineIndex]) throw runtime_error("not enough memory to create float image (sample array)");
    }
    
    // move ownership
    _image = std::move(image);
}

/// <summary>
/// Finalizes an instance of the <see cref="FloatImage"/> class.
/// </summary>
FloatImage::~FloatImage()
{
    _image.reset();
}

/// <summary>
/// Converts a float image to OpenCV
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <returns>The converted image</returns>
IplImagePtr FloatImage::toOpenCv(const samples_t& sample_first, const samples_t& sample_last, const lines_t& line_first, const lines_t& line_last, const sample_t& min, const sample_t& max) const
{
    assert(bands == 1);
    assert(sample_last >= sample_first);
    assert(line_last >= line_first);
    assert(min < max);
   
    samples_t samples = sample_last - sample_first + 1;
    lines_t lines = line_last - line_first + 1;
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, bands));
    samples_t step = displayImage->widthStep;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    const float lerp_scaling = 255.0F / (max - min);

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = _image[y+line_first];
        uint_fast32_t lineOffset = y*(step);

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samples_t x=0; x<samples; ++x)
        {
            char& pixel = displayImage->imageData[lineOffset+x];

            // pick the sample and convert it to 8-bit unsigned
            const sample_t input_sample = line->sample(x+sample_first);
            
            // lerp the value
            sample_t sample = (input_sample - min) * lerp_scaling;

            // assign sample (prediction friendly)
            pixel = static_cast<uint_fast8_t>(sample);

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

/// <summary>
/// Converts a float image to OpenCV
/// </summary>
/// <param name="image">The image.</param>
/// <param name="samples">The number of samples.</param>
/// <param name="lines">The number of lines.</param>
/// <param name="bands">The number of bands.</param>
/// <returns>The converted image</returns>
IplImagePtr FloatImage::toOpenCvBGR(const samples_t& sample_first, const samples_t& sample_last, const lines_t& line_first, const lines_t& line_last, const sample_t& min, const sample_t& max) const
{
    assert(bands == 1);
    assert(sample_last >= sample_first);
    assert(line_last >= line_first);
    assert(min < max);
   
    samples_t samples = sample_last - sample_first + 1;
    lines_t lines = line_last - line_first + 1;
    IplImagePtr displayImage(cvCreateImage(cvSize(samples, lines), IPL_DEPTH_8U, 3));
    samples_t step = displayImage->widthStep;

    typedef int_fast32_t omp_linecount_t; // OpenMP needs signed integral type
    omp_linecount_t omp_lines = lines;

    const float lerp_scaling = 255.0F / (max - min);

    #pragma omp parallel for
    for(omp_linecount_t y=0; y<omp_lines; ++y)
    {
        line_t& line = _image[y+line_first];
        uint_fast32_t lineOffset = y*(step);
        uint_fast32_t target_x = 0;

        // TODO: when multiple bands are needed, implement another loop or specific behaviour for regular band counts (1, 3, 4)
        for(samples_t x=0; x<samples; ++x)
        {
            char& pixel_b = displayImage->imageData[lineOffset + target_x++];
            char& pixel_g = displayImage->imageData[lineOffset + target_x++];
            char& pixel_r = displayImage->imageData[lineOffset + target_x++];

            // pick the sample and convert it to 8-bit unsigned
            const sample_t input_sample = line->sample(x+sample_first);
            
            // lerp the value
            sample_t sample = (input_sample - min) * lerp_scaling;

            // assign sample (prediction friendly)
            pixel_r = static_cast<uint_fast8_t>(sample);
            pixel_b = pixel_r;
            pixel_g = pixel_r;

            // correct in case of problems
            if (sample < 0.0F) {
                pixel_r = 0;
            }
            if (sample > 255.0F) {
                pixel_r = (char)255;
            }
        }
    }
    return displayImage;
}

/// <summary>
/// Reads the image from a single-band unsigned 8-bit raw file
/// </summary>
/// <param name="stream">The input stream.</param>
/// <returns>The image</returns>
unique_ptr<FloatImage> FloatImage::createFromU8Raw(istream& stream, const samples_t& samples, const lines_t& lines)
{
    FloatImage* image = new FloatImage(samples, lines, 1, false);

    // read the stream
    // for each line, create the sample array
    for (lines_t lineIndex = 0; lineIndex < lines; ++lineIndex)
    {
        line_t& line = image->line(lineIndex);
        
         // there is only a single band
        for(samples_t x=0; x<samples; ++x)
        {
            char pixel;
            stream.read(&pixel, sizeof(uint8_t));

            line->lerpSet(x, static_cast<uint8_t>(pixel));
        }
    }

    return unique_ptr<FloatImage>(image);
}

/// <summary>
/// Creates an image
/// </summary>
/// <returns>The image</returns>
unique_ptr<FloatImage> FloatImage::create(const samples_t& samples, const lines_t& lines, const bands_t& bands, const bool zero)
{
    return unique_ptr<FloatImage>(new FloatImage(samples, lines, bands, zero));
}

/// <summary>
/// Flips the image vertically (in-place)
/// </summary>
void FloatImage::flipVertical() 
{
    const lines_t halfLines = lines/2; // TODO: should work for odd line numbers, but better test that
    for (lines_t lineIndex = 0; lineIndex < halfLines; ++lineIndex)
    {
        line_t& top = _image[lineIndex];
        line_t& bottom = _image[lines - lineIndex - 1];
        
        // swap the line pointers
        std::swap(top, bottom);
    }
}

/// <summary>
/// Convolves the image with the given kernel.
/// </summary>
/// <param name="kernel">The kernel.</param>
/// <returns>The convolved image.</returns>
unique_ptr<FloatImage> FloatImage::convolve(const unique_ptr<FloatImage>& kernel) const
{
    assert (bands == 1);
    assert ((kernel->samples & 0x1) == 0x1); // kernel width must be odd
    assert ((kernel->lines & 0x1) == 0x1);   // kernel height must be odd

    // image to hold the convolved image
#if _DEBUG
    const bool initialize = true;
#else
    const bool initialize = false;
#endif
    image_t target(new FloatImage(samples, lines, bands, initialize));

    const samples_t raw_samples     = samples;
    const lines_t raw_lines         = lines;
    const samples_t kernel_samples  = kernel->samples;
    const lines_t kernel_lines      = kernel->lines;
    
    const ssamples_t kernel_halfsamples = static_cast<slines_t>(kernel_samples) / 2;
    const slines_t kernel_halflines = static_cast<slines_t>(kernel_lines) / 2;

    // OpenMP needs signed integral type
    typedef int_fast32_t omp_linecount_t;
    omp_linecount_t omp_lines = raw_lines;

    #pragma omp parallel for
    for (omp_linecount_t y = 0; y < omp_lines; ++y)
    {
        line_t &target_line = target->line(y);

        // loop over all samples
        for (samples_t x = 0; x < raw_samples; ++x)
        {
            // correlate the pixels in mask-space (in the area overlayed by the mask)
            sample_t sample_value = 0.0F;
            sample_t kernel_sum = 0.0F;

            for (lines_t my = 0; my < kernel_lines; ++my) // loop all lines in kernel-space
            {
                // calculate the image line
                slines_t raw_y = y + my - kernel_halflines;

                // grab the kernel and image lines assuming they're valid
                const line_t &kernel_line = kernel->line(my);
                const line_t &raw_line = _image[raw_y];
                
                // branch prediction will (have to) save us.
                // THEORY: Operation might be faster if we handle special cases for the edges and corners (i.e. image boundary overlaps)
                if (raw_y < 0) continue;
                if (static_cast<lines_t>(raw_y) >= lines) continue;
                
                // loop over pixels (in kernel-space)
                for (samples_t mx = 0; mx < kernel_samples; ++mx)
                {
                    // calculate the image line
                    slines_t raw_x = x + mx - kernel_halfsamples;

                    // grab the kernel and image lines assuming they're valid
                    const sample_t &kernel_sample = kernel_line->sample(mx);
                    const sample_t &raw_sample = raw_line->sample(raw_x);

                    // branch prediction to the rescue
                    // THEORY: same as above
                    if (raw_x < 0) continue;
                    if (static_cast<samples_t>(raw_x) >= samples) continue;

                    // convolve
                    sample_value += kernel_sample * raw_sample;
                    kernel_sum += kernel_sample;
                }
            }

            // set value (adjust to effective summed kernel values)
            target_line->sample(x) = sample_value / kernel_sum;
        }
    }
    
    return target;
}