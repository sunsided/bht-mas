#ifndef _FLOATIMAGE_H_
#define _FLOATIMAGE_H_

#pragma warning(disable: 4290)

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <memory>

#include "OpenCvImage.h"

typedef uint_fast16_t samples_t;
typedef uint_fast16_t lines_t;
typedef uint_fast8_t  bands_t;
typedef uint_fast32_t imagesize_t;

typedef float                         sample_t;
typedef std::unique_ptr<sample_t[]>   linedata_t;

class FloatImageLine
{
private:
    /// <summary>The inverse of 255.</summary>
    static const float inverse255;

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
    /// Gets a pointer to the samples.
    /// </summary>
    /// <returns>sample_t *.</returns>
    inline sample_t* get_samples() const 
    {
        return _line.get();
    }

    /// <summary>
    /// Gets the sample at the given position
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <returns>sample_t &.</returns>
    inline sample_t& sample(const samples_t& sample) const
    {
        return _line[sample];
    }

    /// <summary>
    /// Gets the sample at the given position
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <returns>sample_t &.</returns>
    inline sample_t& operator[](const samples_t& sample) const
    {
        return _line[sample];
    }
    
    /// <summary>
    /// Sets a sample while lerp'ing it to the range 0..1
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <param name="value">The value.</param>
    inline void lerpSet(const samples_t& sample, const uint8_t value)
    {
        _line[sample] = static_cast<float>(value) * inverse255;
    }

    /// <summary>
    /// Sets a sample while lerp'ing it to the given range
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <param name="value">The value.</param>
    /// <param name="to_min">The target range's minimum value.</param>
    /// <param name="to_max">The target range's maximum value.</param>
    inline void lerpSet(const samples_t& sample, const uint8_t value, const sample_t& to_min, const sample_t& to_max)
    {
        _line[sample] = static_cast<float>(value) * inverse255 * (to_max - to_min) - to_min;
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
    /// The number of color bands
    /// </summary>
    const bands_t bands;

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
    FloatImage(const samples_t& samples, const lines_t& lines, const bands_t& bands, bool zero = true) throw(std::runtime_error);

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

    /// <summary>
    /// Gets the sample at the given position
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <returns>sample_t &.</returns>
    inline line_t& operator[](const lines_t& line) const
    {
        return _image[line];
    }

    /// <summary>
    /// Sets the specified sample to the given value.
    /// </summary>
    /// <param name="sample">The sample.</param>
    /// <param name="line">The line.</param>
    /// <param name="value">The value.</param>
    inline void set(const samples_t& sample, const lines_t& line, const sample_t& value)
    {
        _image[line]->sample(sample) = value;
    }

    /// <summary>
    /// Converts a float image to OpenCV
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <returns>The converted image</returns>
    inline IplImagePtr toOpenCv(const sample_t& min, const sample_t& max) const
    {
        return toOpenCv(0, samples-1, 0, lines-1, min, max);
    }

    /// <summary>
    /// Converts a float image to OpenCV
    /// </summary>
    /// <param name="image">The image.</param>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    /// <returns>The converted image</returns>
    IplImagePtr toOpenCv(const samples_t& sample_first, const samples_t& sample_last, const lines_t& line_first, const lines_t& line_last, const sample_t& min, const sample_t& max) const;

    /// <summary>
    /// Reads the image from a single-band unsigned 8-bit raw file
    /// </summary>
    /// <param name="stream">The input stream.</param>
    /// <returns>The image</returns>
    static std::unique_ptr<FloatImage> createFromU8Raw(std::istream& stream, const samples_t& samples, const lines_t& lines);

};

#endif