#ifndef _FLOATIMAGE_H_
#define _FLOATIMAGE_H_

#pragma warning(disable: 4290)

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <memory>

typedef uint_fast16_t samples_t;
typedef uint_fast16_t lines_t;
typedef uint_fast8_t  bands_t;
typedef uint_fast32_t imagesize_t;

typedef float                         sample_t;
typedef std::unique_ptr<sample_t[]>   linedata_t;

class FloatImageLine
{
private:
    linedata_t _line;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="FloatImageLine"/> class.
    /// </summary>
    /// <param name="line">The line.</param>
    FloatImageLine(const samples_t& samples, bool zero = true) throw(std::runtime_error);

    /// <summary>
    /// Initializes a new instance of the <see cref="FloatImageLine"/> class.
    /// </summary>
    /// <param name="line">The line.</param>
    FloatImageLine(linedata_t&& line)
        : _line(std::move(line))
    {}

    /// <summary>
    /// Finalizes an instance of the <see cref="FloatImageLine"/> class.
    /// </summary>
    inline ~FloatImageLine()
    {
        _line.reset();
    }

    /// <summary>
    /// Gets the sample at the given position
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <returns>sample_t &.</returns>
    sample_t& sample(const samples_t& sample) const
    {
        return _line[sample];
    }
};

typedef std::unique_ptr<FloatImageLine>   line_t;
typedef std::unique_ptr<line_t[]>         imagedata_t;

class FloatImage
{
private:
    /// <summary>
    /// The image data
    /// </summary>
    imagedata_t _image;

public:
    /// <summary>
    /// The number of samples (width)
    /// </summary>
    const samples_t samples;
    
    /// <summary>
    /// The number of lines (height)
    /// </summary>
    const lines_t lines;
    
    /// <summary>
    /// The image size
    /// </summary>
    const imagesize_t size;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="FloatImage"/> class.
    /// </summary>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <param name="zero">If true the pixels will be initialized to zero.</param>
    FloatImage(const samples_t samples, const lines_t lines, bool zero = true) throw(std::runtime_error);

    /// <summary>
    /// Finalizes an instance of the <see cref="FloatImage"/> class.
    /// </summary>
    virtual ~FloatImage();

    /// <summary>
    /// Gets the given line
    /// </summary>
    /// <param name="line">The line.</param>
    /// <returns>Reference to the line.</returns>
    inline line_t& line(const lines_t& line) const 
    {
        return _image[line];
    }

    /// <summary>
    /// Gets the given line
    /// </summary>
    /// <param name="line">The line.</param>
    /// <returns>Reference to the line.</returns>
    inline sample_t& sample(const lines_t& line, const samples_t& sample) const 
    {
        return this->line(line)->sample(sample);
    }
};

#endif