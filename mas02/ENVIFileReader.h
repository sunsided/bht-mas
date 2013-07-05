#ifndef _ENVIFILEREADER_H_
#define _ENVIFILEREADER_H_

#pragma warning( disable : 4290 ) // disable throw() not implemented by MSVC

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>

namespace envi {

typedef uint_fast16_t   linecount_t;
typedef uint_fast16_t   samplecount_t;
typedef uint_fast8_t    bandcount_t;

typedef float                         sample_t;
typedef std::unique_ptr<sample_t[]>   line_t;
typedef std::unique_ptr<line_t[]>     image_t;

/// <summary>
/// Reader for band-sequential ENVI HDR files with data type 4, "float"
/// </summary>
class ENVIFileReader
{
private:
    /// <summary>
    /// The number of samples per line
    /// </summary>
    const linecount_t _samples;

    /// <summary>
    /// The number of lines per image
    /// </summary>
    const samplecount_t _lines;
    
    /// <summary>
    /// The number of bands per sample
    /// </summary>
    const bandcount_t _bands;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="ENVIFileReader"/> class.
    /// </summary>
    /// <param name="samples">The number of samples.</param>
    /// <param name="lines">The number of lines.</param>
    /// <param name="bands">The number of bands.</param>
    ENVIFileReader(samplecount_t samples, linecount_t lines, bandcount_t bands);
    
    /// <summary>
    /// Finalizes an instance of the <see cref="ENVIFileReader"/> class.
    /// </summary>
    ~ENVIFileReader(void);

    /// <summary>
    /// Reads the image from the given stream
    /// </summary>
    /// <param name="stream">The input stream.</param>
    image_t read(std::istream& stream) throw(std::runtime_error);
};

}

#endif