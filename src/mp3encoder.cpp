#include "mp3encoder.h"

MP3Encoder::MP3Encoder(std::string &arg_filename,
                       const int *arg_buffer_l,
                       const int *arg_buffer_r,
                       int arg_num_samples,
                       int arg_num_channels,
                       int arg_sample_rate,
                       int arg_byte_rate) : filename(arg_filename),
                                            buffer_l(arg_buffer_l),
                                            buffer_r(arg_buffer_r),
                                            audio_format(Audio_Format_IDs::Audio_Format_PCM),
                                            numsamples(arg_num_samples),
                                            num_channels(arg_num_channels),
                                            samplerate(arg_sample_rate),
                                            byte_rate(arg_byte_rate),
                                            bytes_written(0)

{
    Encode();
}

MP3Encoder::MP3Encoder(std::string &arg_filename,
                       const float *arg_buffer_ieee_l,
                       const float *arg_buffer_ieee_r,
                       int arg_num_samples,
                       int arg_num_channels,
                       int arg_sample_rate,
                       int arg_byte_rate) : filename(arg_filename),
                                            buffer_ieee_l(arg_buffer_ieee_l),
                                            buffer_ieee_r(arg_buffer_ieee_r),
                                            audio_format(Audio_Format_IDs::Audio_Format_IEEE),
                                            numsamples(arg_num_samples),
                                            num_channels(arg_num_channels),
                                            samplerate(arg_sample_rate),
                                            byte_rate(arg_byte_rate),
                                            bytes_written(0)
{
    Encode();
}

void MP3Encoder::Encode()
{
    buffer_size = (unsigned int)(1.25 * numsamples + 7200); // method taken from Lame documentation
    mp3buffer.resize(buffer_size);

    lame_global_flags *gfp;
    gfp = lame_init();

    lame_set_num_channels(gfp, num_channels);
    lame_set_in_samplerate(gfp, samplerate);
    lame_set_brate(gfp, byte_rate);

    int ret_code = lame_init_params(gfp);

    debugm << "Lame settings channels: " << num_channels << " samplerate: " << samplerate << " byte rate: " << byte_rate << "\n";

    if (0 != ret_code)
    {
        errorm << "Error lame init:" << filename << "\n";
        return;
    }

    int ret_code_enc = 0;
    switch (audio_format)
    {
    case Audio_Format_IDs::Audio_Format_PCM:
    {
        try
        {
            if (num_channels == 1)
            {
                ret_code_enc = lame_encode_buffer_int(gfp, buffer_l, buffer_r, numsamples, mp3buffer.data(), buffer_size);
            }
            else
            {
                ret_code_enc = lame_encode_buffer_interleaved_int(gfp, buffer_l, numsamples, mp3buffer.data(), buffer_size);
            }
        }
        catch (std::exception const &d)
        {
            std::cerr << std::endl
                      << "Error encoding ieee_float: " << d.what() << std::endl
                      << std::flush;
            return;
        }
        catch (...)
        {
            std::cerr << std::endl
                      << "Some exception caught " << std::endl
                      << std::flush;
            return;
        }
        break;
    }
    case Audio_Format_IDs::Audio_Format_IEEE:
    {
        try
        {
            if (num_channels == 1)
            {
                ret_code_enc = lame_encode_buffer_ieee_float(gfp, buffer_ieee_l, buffer_ieee_r, numsamples, mp3buffer.data(), buffer_size);
            }
            else
            {
                ret_code_enc = lame_encode_buffer_interleaved_ieee_float(gfp, buffer_ieee_l, numsamples, mp3buffer.data(), buffer_size);
            }
        }
        catch (std::exception const &d)
        {
            std::cerr << std::endl
                      << "Error encoding ieee_float: " << d.what() << std::endl
                      << std::flush;
            return;
        }
        catch (...)
        {
            std::cerr << std::endl
                      << "Some exception caught " << std::endl
                      << std::flush;
            return;
        }
        break;
    }
    default:; // TODO invalid audio format
    }

    if (0 > ret_code_enc)
    {
        errorm << "Error " << ret_code_enc << " encoding file:" << filename << "\n";
        return;
    }
    debugm << "Encoding OK:" << ret_code_enc << "\n";

    std::ofstream outfile(filename, std::ofstream::binary);
    outfile.write(reinterpret_cast<const char *>(mp3buffer.data()), ret_code_enc);
    debugm << "bytes output to mp3buffer:" << ret_code_enc << "\n";
    bytes_written += ret_code_enc;

    int outputflush = lame_encode_flush(gfp, mp3buffer.data(), buffer_size); // TOTO replace flush with finish
    outfile.write(reinterpret_cast<const char *>(mp3buffer.data()), outputflush);
    debugm << "bytes output to mp3buffer:" << outputflush << "\n";
    bytes_written += outputflush;
    outfile.close();

    lame_close(gfp);
}
