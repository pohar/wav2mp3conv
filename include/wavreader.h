#ifndef __WAVREADER_H__
#define __WAVREADER_H__

#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <cstring>
#include "helperf.h"

using namespace HelperFunctions;

typedef struct wav_header
{
    enum Audio_Format_IDs
    {
        Audio_Format_PCM = 1,
        Audio_Format_IEEE = 3
    };

    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM (further values: 18,40)
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth;        // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
    // data_bytes    == NumSamples * num_channels * bit_depth/8

    bool is_valid = 0;
} wav_header;

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
    const std::unordered_set<std::string> knownchunknames{{"AFAn"}, {"bext"}, {"bext"}, {"fact"}, {"id3 "}, {"iXML"}, {"JUNK"}, {"LIST"}, {"LGWV"}, {"olym"}, {"PAD "}, {"PEAK"}};
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