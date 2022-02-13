/*! \file mp3encoder.h
    \brief Header file of encoding class.
*/
#ifndef __MP3ENCODER_H__
#define __MP3ENCODER_H__

#include <lame.h>
#include <string>
#include <vector>
#include "helperf.h"

using namespace HelperFunctions;

/// MP3Encoder class
/** The MP3Encoder class converts the sample data  in the incoming array(s)
 * to and mp3 file, and saves it to the disk.*/
class MP3Encoder
{
    enum Audio_Format_IDs
    {
        audio_format_PCM = 1,
        audio_format_IEEE = 3
    };

public:
    MP3Encoder(std::string &, const int *, const int *, int, int, int, int);
    MP3Encoder(std::string &, const float *, const float *, int, int, int, int);
    auto GetBytesWritten() const { return bytes_written; };

private:
    void Encode();
    const std::string filename;

    unsigned int buffer_size;
    const int *buffer_l;
    const int *buffer_r;
    const float *buffer_ieee_l;
    const float *buffer_ieee_r;
    short audio_format;
    int numsamples;
    int num_channels;
    int samplerate;
    int byte_rate;
    int bytes_written;

    std::vector<unsigned char> mp3buffer;
};

#endif //__MP3ENCODER_H__
