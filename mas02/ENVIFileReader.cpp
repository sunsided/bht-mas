#include "ENVIFileReader.h"

using namespace std;
using namespace envi;

/// <summary>
/// Initializes a new instance of the <see cref="ENVIFileReader"/> class.
/// </summary>
ENVIFileReader::ENVIFileReader(samplecount_t samples, linecount_t lines, bandcount_t bands)
    : _samples(samples), _lines(lines), _bands(bands)
{
}


/// <summary>
/// Finalizes an instance of the <see cref="ENVIFileReader"/> class.
/// </summary>
ENVIFileReader::~ENVIFileReader(void)
{
}

/// <summary>
/// Reads the image from the given stream
/// </summary>
/// <param name="stream">The input stream.</param>
image_t ENVIFileReader::read(istream& stream)
{
    // create the line array
    image_t image = image_t(new line_t[_lines]);
    if (!image) throw runtime_error("not enough memory to create ENVI line array");

    // for each line, create the sample array
    for (linecount_t lineIndex = 0; lineIndex < _lines; ++lineIndex)
    {
        image[lineIndex].reset(new sample_t[_samples]);
        if (!image[lineIndex]) throw runtime_error("not enough memory to create ENVI sample array");
    }

    // read the stream
    // for each line, create the sample array
    for (linecount_t lineIndex = 0; lineIndex < _lines; ++lineIndex)
    {
        sample_t *line = image[lineIndex].get();
        char* pointer = reinterpret_cast<char*>(line);

        stream.read(pointer, _samples*sizeof(sample_t));
    }

    return image;
}