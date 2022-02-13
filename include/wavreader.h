/*! \file wavreader.h
    \brief Header file of wav file parsing class.
*/

#ifndef __WAVREADER_H__
#define __WAVREADER_H__

#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <cstring>
#include "helperf.h"

using namespace HelperFunctions;

/// Structure containing a wav header
/** Structure containing a wav header
 * It also contains the enum with the audio format IDs.
 * The sample data is not stored in this structure.
 * The validity bit is set according to the result of parsing the header of the wav file. */
struct wav_header
{
    enum Audio_Format_IDs
    {
        audio_format_PCM = 1,
        audio_format_IEEE = 3
    };

    // RIFF Header
    char riff_header[4] = ""; // Contains "RIFF"
    int wav_size = 0;         // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4] = ""; // Contains "WAVE"

    // Format Header
    char fmt_header[4] = ""; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size = 0;  // Should be 16 for PCM (further values: 18,40)
    short audio_format = 0;  // Should be 1 for PCM. 3 for IEEE Float
    short num_channels = 0;
    int sample_rate = 0;
    int byte_rate = 0;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment = 0; // num_channels * Bytes Per Sample
    short bit_depth = 0;        // Number of bits per sample

    // Data
    char data_header[4] = ""; // Contains "data"
    int data_bytes = 0;       // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes

    bool is_valid = 0;
};

/// Wav reader class
/** Wav reader class, it can parse the header and load sample data.
 * It also contains the enum with the audio format IDs.
 * The names of the known chunk types are stored in a set. Theoretically any kind of chunk could be read,
 * but there are exceptions. More chunk types can be added after testing.*/
class WavReader
{
    const int WAV_HEADER_MIN_SIZE = 44;
    const int MAX_BITRATE = 352800; // bitrates equal or bigger cause an assert in Lame library, when Encoding IEEE float format.

private:
    void ParseWavFile();
    bool ReadRIFFChunk();
    bool ReadWAVEChunk();
    bool ReadfmtChunk();
    bool ReaddataChunk();
    bool ReadGenericChunk();
    void ReadSamples();
    const std::string filename;
    std::ifstream wavfile_handle;
    wav_header wavheader;
    unsigned int numsamples;
    std::streampos filesize;
    std::vector<int> leftpcm;
    std::vector<int> rightpcm;
    std::vector<float> leftieee;
    std::vector<float> rightieee;
    const std::unordered_set<std::string> known_chunk_names{{"AFAn"}, {"bext"}, {"bext"}, {"fact"}, {"id3 "}, {"iXML"}, {"JUNK"}, {"LIST"}, {"LGWV"}, {"olym"}, {"PAD "}, {"PEAK"}};
    const std::unordered_set<int> valid_fmt_chunk_sizes{16, 18, 40};

public:
    WavReader(std::string &);
    void ParseWavFile2();
    auto GetLeftPCMData() const { return leftpcm.data(); }
    auto GetRightPCMData() const { return rightpcm.data(); }
    auto GetLeftIEEEData() const { return leftieee.data(); }
    auto GetRightIEEEData() const { return rightieee.data(); }
    auto GetNumChannels() const { return wavheader.num_channels; }
    auto GetSampleRate() const { return wavheader.sample_rate; }
    auto GetByteRate() const { return wavheader.byte_rate; }
    auto GetNumSamples() const { return numsamples; }
    auto GetAudioFormat() const { return wavheader.audio_format; }
    auto IsValid() const { return wavheader.is_valid; }
};

#endif // __WAVREADER_H__