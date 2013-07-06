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
/// Reads the image from a single-band unsigned 8-bit raw file
/// </summary>
/// <param name="stream">The input stream.</param>
/// <returns>The image</returns>
std::unique_ptr<FloatImage> FloatImage::createFromU8Raw(std::istream& stream, const samples_t& samples, const lines_t& lines)
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