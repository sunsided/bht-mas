#include "FloatImage.h"

using namespace std;

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
FloatImage::FloatImage(const samples_t samples, const lines_t lines, bool zero)
    : samples(samples), lines(lines), size(samples * lines)
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
}