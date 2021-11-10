#include "wavreader.h"

void WavReader::ParseWavFile()
{
#if 0
    wavfile_handle.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (wavfile_handle.is_open())
    {
        std::streampos filesize;
        filesize = wavfile_handle.tellg();
        // debugm << "filesize:" << filesize << "\n";
        if (filesize >= WAV_HEADER_MIN_SIZE)
        {
            // check header
            wavfile_handle.seekg(0, wavfile_handle.beg);
            wavheader.data_bytes = 0; // TODO find better condition
            wavfile_handle.read(wavheader.riff_header, sizeof(wavheader.riff_header));
            if (!std::strncmp(wavheader.riff_header, "RIFF", sizeof(wavheader.riff_header)))
            {
                wavheader.wav_size = ReadInt32(wavfile_handle);
                // debugm << "size in wav header:" << wavheader.wav_size << "\n";
                if (wavheader.wav_size <= (int(filesize) - 8)) // no equality check, because some wav files have extra data following the samples
                {
                    wavfile_handle.read(wavheader.wave_header, sizeof(wavheader.wave_header));
                    if (!std::strncmp(wavheader.wave_header, "WAVE", sizeof(wavheader.wave_header)))
                    {
                        wavfile_handle.read(wavheader.fmt_header, sizeof(wavheader.fmt_header));
                        while (/*(wavfile_handle.goodbit) &&  */ (0 < knownchunknames.count(std::string(wavheader.fmt_header).substr(0, 4))))
                        {
                            int chunk_size = ReadInt32(wavfile_handle);
                            // debugm << "chunk_size in wav header:" << chunk_size << "\n";

                            wavfile_handle.seekg(chunk_size, std::ios_base::cur); // skip chunk data
                            wavfile_handle.read(wavheader.fmt_header, sizeof(wavheader.fmt_header));
                        }
                        if (!std::strncmp(wavheader.fmt_header, "fmt ", sizeof(wavheader.fmt_header)))
                        {
                            wavheader.fmt_chunk_size = ReadInt32(wavfile_handle);
                            if (0 < valid_fmt_chunk_sizes.count(wavheader.fmt_chunk_size))
                            {
                                // debugm << "fmt_chunk_size in wav header:" << wavheader.fmt_chunk_size << "\n";
                                wavheader.audio_format = ReadInt16(wavfile_handle);

                                if ((wav_header::Audio_Format_PCM == wavheader.audio_format) | (wav_header::Audio_Format_IEEE == wavheader.audio_format))
                                {
                                    // debugm << "audio_format in wav header:" << wavheader.audio_format << "\n";
                                    wavheader.num_channels = ReadInt16(wavfile_handle);
                                    // debugm << "num_channels in wav header:" << wavheader.num_channels << "\n";

                                    wavheader.sample_rate = ReadInt32(wavfile_handle);
                                    // debugm << "sample_rate in wav header:" << wavheader.sample_rate << "\n";

                                    wavheader.byte_rate = ReadInt32(wavfile_handle);
                                    // debugm << "byte_rate in wav header:" << wavheader.byte_rate << "\n";
                                    if (wavheader.byte_rate < MAX_BITRATE)
                                    {
                                        wavheader.sample_alignment = ReadInt16(wavfile_handle);
                                        // debugm << "sample_alignment in wav header:" << wavheader.sample_alignment << "\n";

                                        if (0 < wavheader.sample_alignment)
                                        {
                                            wavheader.bit_depth = ReadInt16(wavfile_handle);
                                            // debugm << "bit_depth in wav header:" << wavheader.bit_depth << "\n";

                                            if ((18 == wavheader.fmt_chunk_size) | (40 == wavheader.fmt_chunk_size)) // 18,22,40 are the valid ftm chunk sizes
                                            {
                                                short cbSize = ReadInt16(wavfile_handle);

                                                if (22 == cbSize)
                                                {
                                                    wavfile_handle.seekg(cbSize, std::ios_base::cur); // skip extension data
                                                }
                                            }

                                            wavfile_handle.read(wavheader.data_header, sizeof(wavheader.data_header));
                                            while (0 < knownchunknames.count(std::string(wavheader.data_header)))
                                            {
                                                // debugm << "data_header is: LIST/fact/PEAK" << "\n";
                                                int chunk_size = ReadInt32(wavfile_handle);
                                                // debugm << "chunk_size in wav header:" << chunk_size << "\n";

                                                wavfile_handle.seekg(chunk_size, std::ios_base::cur); // skip chunk data
                                                wavfile_handle.read(wavheader.data_header, sizeof(wavheader.data_header));
                                            }

                                            if (!std::strncmp(wavheader.data_header, "data", sizeof(wavheader.data_header)))
                                            {
                                                wavheader.data_bytes = ReadInt32(wavfile_handle);
                                                // debugm << "data_bytes in wav header:" << wavheader.data_bytes << "\n";

                                                if (wavheader.data_bytes > 0)
                                                {
                                                    numsamples = wavheader.data_bytes / wavheader.sample_alignment;
                                                    // debugm << ">>> NumSamples=" << numsamples << "\n";

                                                    // read sample data
                                                    switch (wavheader.audio_format)
                                                    {
                                                    case (wav_header::Audio_Format_PCM):
                                                    {
                                                        ReadSamples_PCM(numsamples, wavheader.num_channels, wavheader.bit_depth, wavfile_handle, leftpcm, rightpcm);
                                                        if (((leftpcm.size()) != numsamples)/* || ((rightpcm.size() != numsamples) && (wavheader.num_channels == 2))*/)
                                                        {
                                                            debugm << "Not enough samples read (" << (leftpcm.size() ) << " , " << rightpcm.size() << ") expected:" << numsamples << "\n";
                                                            leftpcm.clear();
                                                            rightpcm.clear();
                                                        }
                                                        else
                                                        {
                                                            wavheader.is_valid = 1;
                                                        }
                                                        break;
                                                    }
                                                    case (wav_header::Audio_Format_IEEE):
                                                    {
                                                        ReadSamples_IEEE(numsamples, wavheader.num_channels, wavfile_handle, leftieee, rightieee);
                                                        if (((leftieee.size()) != numsamples)/* || ((rightieee.size() != numsamples) && (wavheader.num_channels == 2))*/)
                                                        {
                                                            debugm << "Not enough samples read (" << leftieee.size() << " , " << rightieee.size() << ") expected:" << numsamples << "\n";
                                                            leftieee.clear();
                                                            rightieee.clear();
                                                        }
                                                        else
                                                        {
                                                            wavheader.is_valid = 1;
                                                        }
                                                        break;
                                                    }
                                                    default:; // error, wrong format
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                errorm << "incorrect or unknown data_header: " << wavheader.data_header << "\n";
                                            }
                                        }
                                        else
                                        {
                                            errorm << "Invalid fmt_chunk_size in wav header:" << wavheader.fmt_chunk_size << "\n";
                                        }
                                    }
                                    else
                                    {
                                        errorm << "byte_rate is too high:" << wavheader.byte_rate << " skipping file"
                                               << "\n";
                                    }
                                }
                                else
                                {
                                    errorm << "Incorrect audio_format in wav header:" << wavheader.audio_format << "\n";
                                }
                            }
                            else
                            {
                                errorm << "incorrect sample_alignment"
                                       << "\n";
                            }
                        }
                        else
                        {
                            errorm << filename << " :incorrect fmt_header:[" << wavheader.fmt_header << "]"
                                   << "\n";
                        }
                    }
                    else
                    {
                        errorm << "incorrect wave_header"
                               << "\n";
                    }
                }
                else
                {
                    errorm << "incorrect filesize in header"
                           << "\n";
                }
            }
            else
            {
                errorm << "incorrect riff_header"
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
#endif // 0
}

bool WavReader::ReadRIFFChunk()
{
    bool retval = false;
    memcpy(wavheader.riff_header, "RIFF", sizeof(wavheader.riff_header));

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

bool WavReader::ReadWAVEChunk()
{
    bool retval = false;
    memcpy(wavheader.wave_header, "WAVE", sizeof(wavheader.wave_header));
    retval = true;
    return retval;
}

bool WavReader::ReadGenericChunk()
{
    bool retval = false;
    unsigned int chunk_size = ReadInt32(wavfile_handle);
    // debugm << "chunk_size in wav header:" << chunk_size << "\n";
    wavfile_handle.seekg(chunk_size, std::ios_base::cur); // skip chunk data
    retval = wavfile_handle.good();
    return retval;
}

bool WavReader::ReadfmtChunk()
{
    bool retval = false;
    wavheader.fmt_chunk_size = ReadInt32(wavfile_handle);
    if (0 < valid_fmt_chunk_sizes.count(wavheader.fmt_chunk_size))
    {
        // debugm << "fmt_chunk_size in wav header:" << wavheader.fmt_chunk_size << "\n";
        wavheader.audio_format = ReadInt16(wavfile_handle);

        if ((wav_header::Audio_Format_PCM == wavheader.audio_format) | (wav_header::Audio_Format_IEEE == wavheader.audio_format))
        {
            // debugm << "audio_format in wav header:" << wavheader.audio_format << "\n";
            wavheader.num_channels = ReadInt16(wavfile_handle);
            // debugm << "num_channels in wav header:" << wavheader.num_channels << "\n";

            wavheader.sample_rate = ReadInt32(wavfile_handle);
            // debugm << "sample_rate in wav header:" << wavheader.sample_rate << "\n";

            wavheader.byte_rate = ReadInt32(wavfile_handle);
            // debugm << "byte_rate in wav header:" << wavheader.byte_rate << "\n";
            if (wavheader.byte_rate < MAX_BITRATE)
            {
                wavheader.sample_alignment = ReadInt16(wavfile_handle);
                // debugm << "sample_alignment in wav header:" << wavheader.sample_alignment << "\n";

                wavheader.bit_depth = ReadInt16(wavfile_handle);
                // debugm << "bit_depth in wav header:" << wavheader.bit_depth << "\n";

                // TODO in valid chunk sizes
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
    // retval = stream.good(); // TODO ez a jobb vagy a retval=true?
    return retval;
}

bool WavReader::ReaddataChunk()
{
    bool retval = false;

    wavheader.data_bytes = ReadInt32(wavfile_handle);
    // debugm << "data_bytes in wav header:" << wavheader.data_bytes << "\n";

    if ((wavheader.data_bytes > 0) && (wavheader.sample_alignment != 0))
    {
        numsamples = wavheader.data_bytes / wavheader.sample_alignment;
        // debugm << ">>> NumSamples=" << numsamples << "\n";
        retval = true;
    }

    return retval;
}

void WavReader::ReadSamples()
{
    // read sample data
    switch (wavheader.audio_format)
    {
    case (wav_header::Audio_Format_PCM):
    {
        ReadSamples_PCM(numsamples, wavheader.num_channels, wavheader.bit_depth, wavfile_handle, leftpcm, rightpcm);
        if (leftpcm.size()  != (numsamples*wavheader.num_channels))
        {
            debugm << "Not enough samples read (" << leftpcm.size() << ") expected:" << numsamples* wavheader.num_channels << "\n";
            leftpcm.clear();
            rightpcm.clear();
        }
        else
        {
            wavheader.is_valid = 1;
        }
        break;
    }
    case (wav_header::Audio_Format_IEEE):
    {
        ReadSamples_IEEE(numsamples, wavheader.num_channels, wavfile_handle, leftieee, rightieee);
        if ((leftieee.size() != (numsamples * wavheader.num_channels) ))
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
    // wavfile_handle.close();
}

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
                else if (0 < knownchunknames.count(std::string(chunkname).substr(0, 4)))
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
#pragma warning(suppress : 4996)
       char *err=strerror(errno); // Get some info as to why
    }
}

WavReader::WavReader(std::string &filename) : filename(filename),
                                              numsamples(0),
                                              filesize(0)
{
}
