/*! \file wavreader.cpp
    \brief Source file of wav file parsing class.
*/

#include "wavreader.h"

/// Function for reading RIFF chunk
/** Function for reading RIFF chunk
 * It also checks the wav file size in the header.
 * returns true on success, false on failure */
bool WavReader::ReadRIFFChunk()
{
    bool retval = false;
    std::memcpy(wavheader.riff_header, "RIFF", sizeof(wavheader.riff_header));

    wavheader.wav_size = ReadInt32(wavfile_handle);
    if (wavheader.wav_size <= (int(filesize) - 8)) // no equality check, because some wav files have extra data following the samples
    {
        wavfile_handle.read(wavheader.wave_header, sizeof(wavheader.wave_header));
        if (!std::strncmp(wavheader.wave_header, "WAVE", sizeof(wavheader.wave_header)))
        {
            retval = true;
        }
        else
        {
            retval = false;
            errorm << "incorrect wave_header"
                   << "\n";
        }
    }
    else
    {
        retval = false;
        errorm << "incorrect filesize in header"
               << "\n";
    }
    return retval;
}

/// Function for reading WAVE chunk
/** Function for reading WAVE chunk
 * returns true */
bool WavReader::ReadWAVEChunk()
{
    bool retval = false;
    std::memcpy(wavheader.wave_header, "WAVE", sizeof(wavheader.wave_header));
    retval = true;
    return retval;
}

/// Function for reading a generic chunk
/** Function for reading a generic chunk
 * It reads the size of the chunk, and then reads (skips) the given number of bytes in the file.
 * returns true on success, false on failure */
bool WavReader::ReadGenericChunk()
{
    bool retval = false;
    unsigned int chunk_size = ReadInt32(wavfile_handle);
    // debugm << "chunk_size in wav header:" << chunk_size << "\n";
    wavfile_handle.seekg(chunk_size, std::ios_base::cur); // skip chunk data
    retval = wavfile_handle.good();
    return retval;
}

/// Function for reading fmt chunk
/** Function for reading fmt chunk, which contains the data format
 * It reads the size of the chunk (18,22,40 are the valid ftm chunk sizes),
 * audio format (which can be PCM or IEEE), number of channels, sample rate,
 * byte rate (which can not exceed MAX_BITRATE), sample alignment, bit depth.
 * Returns true on success (whole chunk read properly), false on failure */
bool WavReader::ReadfmtChunk()
{
    bool retval = false;
    wavheader.fmt_chunk_size = ReadInt32(wavfile_handle);
    if (0 < valid_fmt_chunk_sizes.count(wavheader.fmt_chunk_size))
    {
        wavheader.audio_format = ReadInt16(wavfile_handle);

        if ((wav_header::audio_format_PCM == wavheader.audio_format) | (wav_header::audio_format_IEEE == wavheader.audio_format))
        {
            wavheader.num_channels = ReadInt16(wavfile_handle);

            wavheader.sample_rate = ReadInt32(wavfile_handle);

            wavheader.byte_rate = ReadInt32(wavfile_handle);
            if (wavheader.byte_rate < MAX_BITRATE)
            {
                wavheader.sample_alignment = ReadInt16(wavfile_handle);

                wavheader.bit_depth = ReadInt16(wavfile_handle);

                if ((18 == wavheader.fmt_chunk_size) | (40 == wavheader.fmt_chunk_size)) // 18,22,40 are the valid ftm chunk sizes
                {
                    short cbSize = ReadInt16(wavfile_handle);

                    if (22 == cbSize)
                    {
                        wavfile_handle.seekg(cbSize, std::ios_base::cur); // skip extension data
                    }
                }

                retval = true;
            }
            else
            {
                errorm << "byte_rate is too high:" << wavheader.byte_rate << " skipping file"
                       << "\n";
            }
        }
    }
    // retval = stream.good(); // TODO decide if it is necessary
    return retval;
}

/// Function for reading data chunk
/** Function for reading data chunk
 * It calculates the number of samples, based of the previously read format.
 * Returns true on success (chunk read and number of samples read properly), false on failure */

bool WavReader::ReaddataChunk()
{
    bool retval = false;

    wavheader.data_bytes = ReadInt32(wavfile_handle);

    if ((wavheader.data_bytes > 0) && (wavheader.sample_alignment != 0))
    {
        numsamples = wavheader.data_bytes / wavheader.sample_alignment;
        retval = true;
    }

    return retval;
}

/// Function for reading sample data
/** Function for reading sample data
 * It can read PCM and IEEE data. If the read fails, it clears sample data.
 * If the read is successful, it sets the header to valid. */

void WavReader::ReadSamples()
{
    switch (wavheader.audio_format)
    {
    case (wav_header::audio_format_PCM):
    {
        ReadSamples_PCM(numsamples, wavheader.num_channels, wavheader.bit_depth, wavfile_handle, leftpcm);
        if (leftpcm.size() != (numsamples * wavheader.num_channels))
        {
            debugm << "Not enough samples read (" << leftpcm.size() << ") expected:" << numsamples * wavheader.num_channels << "\n";
            leftpcm.clear();
            rightpcm.clear();
        }
        else
        {
            wavheader.is_valid = 1;
        }
        break;
    }
    case (wav_header::audio_format_IEEE):
    {
        ReadSamples_IEEE(numsamples, wavheader.num_channels, wavfile_handle, leftieee);
        if ((leftieee.size() != (numsamples * wavheader.num_channels)))
        {
            debugm << "Not enough samples read (" << leftieee.size() << " , " << rightieee.size() << ") expected:" << numsamples * wavheader.num_channels << "\n";
            leftieee.clear();
            rightieee.clear();
        }
        else
        {
            wavheader.is_valid = 1;
        }
        break;
    }
    default:
    {
        errorm << "Wrong audio format\n";
    };
    }
}

/// Function for parsing a wav file
/** Function for parsing a wav file
 * If the file is found, all parts of the header is correct, and the sample data is read,
 * the number of samples is set.
 * No return value */

void WavReader::ParseWavFile2()
{
    wavfile_handle.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (wavfile_handle.is_open())
    {
        filesize = wavfile_handle.tellg();
        debugm << "filesize:" << filesize << "\n";
        if (filesize >= WAV_HEADER_MIN_SIZE)
        {
            // check header
            wavfile_handle.seekg(0, wavfile_handle.beg);
            char chunkname[4] = "";
            wavfile_handle.read(chunkname, sizeof(chunkname));
            while (wavfile_handle.good())
            {
                if (!std::strncmp(chunkname, "RIFF", sizeof(chunkname)))
                {
                    if (!ReadRIFFChunk())
                        return;
                }
                else if (!std::strncmp(chunkname, "WAVE", sizeof(chunkname)))
                {
                    if (!ReadWAVEChunk())
                        return;
                }
                else if (!std::strncmp(chunkname, "fmt ", sizeof(chunkname)))
                {
                    if (!ReadfmtChunk())
                        return;
                }
                else if (!std::strncmp(chunkname, "data", sizeof(chunkname)))
                {
                    if (ReaddataChunk())
                    {
                        ReadSamples();
                    }
                    else
                    {
                        return;
                    }
                }
                else if (0 < known_chunk_names.count(std::string(chunkname).substr(0, 4)))
                {
                    if (!ReadGenericChunk())
                        return;
                }
                else
                {
                    errorm << "Wav chunk unknown: " << chunkname << "\nSkipping the rest of the chunks"
                           << "\n";
                    return;
                }
                wavfile_handle.read(chunkname, sizeof(chunkname));
            }
            if (numsamples == 0)
            {
                errorm << "Wav chunks incomplete"
                       << "\n";
            }
        }
        else
        {
            errorm << "Header is too small"
                   << "\n";
        }
        wavfile_handle.close();
    }
    else
    {
        debugm << "file open error:" << filename << "\n";
    }
}

/// Constructior of WavReader
/** Constructior of WavReader
 * numsamples is set to 0 */
WavReader::WavReader(std::string &filename) : filename(filename),
                                              numsamples(0),
                                              filesize(0)
{
}
