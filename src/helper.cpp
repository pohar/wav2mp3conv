/*! \file helper.cpp
    \brief Source file containing helper functions.
*/
#include "helperf.h"

namespace HelperFunctions
{
    /// The function for deciding endianness.
    /** The function for deciding endianness of the architecture.
     * It checks if the LSB is in the very first byte. */
    unsigned IsLittleEndian(void)
    {
        const union
        {
            unsigned u;
            unsigned char c[4];
        } one = {1};
        return one.c[0];
    }

    /// Exception handler class.
    /** Exception handler class.
     *  */
    class ExceptionHandler : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    float ReadFloat(std::ifstream &file)
    {
        float ret = 0;

        union
        {
            float f;
            unsigned char c[4];
        } c2f = {0};

        if (IsLittleEndian())
        {
            file >> c2f.c[0];
            file >> c2f.c[1];
            file >> c2f.c[2];
            file >> c2f.c[3];
        }
        else
        {
            file >> c2f.c[3];
            file >> c2f.c[2];
            file >> c2f.c[1];
            file >> c2f.c[0];
        }
        ret = c2f.f;

        return ret;
    }

    unsigned int ReadInt32(std::ifstream &file)
    {
        unsigned int ret = 0;

        if (IsLittleEndian())
        {
            char c[4];
            file.read(c, 4);
            for (unsigned int i = 0; i < sizeof(int); i++)
            {
                unsigned int add = (unsigned char)c[i];
                add <<= (8 * i);
                ret += add;
            }
        }
        else
        {
            unsigned char c;
            for (unsigned int i = 0; i < sizeof(int); i++)
            {
                file >> c;
                ret = (ret << 8) + c;
            }
        }
        return ret;
    }

    unsigned int ReadInt24(std::ifstream &file)
    {
        unsigned int ret = 0;

        if (IsLittleEndian())
        {
            char c[3];
            file.read(c, 3);
            for (unsigned int i = 0; i < 3; i++)
            {
                unsigned int add = (unsigned char)c[i];
                add <<= (8 * i);
                ret += add;
            }
        }
        else
        {
            unsigned char c;
            for (unsigned int i = 0; i < sizeof(3); i++)
            {
                file >> c;
                ret = (ret << 8) + c;
            }
        }

        return ret;
    }

    unsigned int ReadInt16(std::ifstream &file)
    {
        unsigned int ret = 0;

        if (IsLittleEndian())
        {
            char c[2];
            file.read(c, 2);
            for (int i = 0; i < 2; i++)
            {
                unsigned int add = (unsigned char)c[i];
                add <<= (8 * i);
                ret += add;
            }
        }
        else
        {
            unsigned char c;
            for (int i = 0; i < 2; i++)
            {
                file >> c;
                ret = (ret << 8) + c;
            }
        }
        return ret;
    }

    unsigned char ReadChar8(std::ifstream &file)
    {
        unsigned char ret = 0;

        char c;
        file.read(&c, 1);
        ret = c;
        return ret;
    }

    std::string ReplaceExtension(const std::string inputfilename, const char *newextension)
    {
        std::size_t dotpos = inputfilename.find_last_of(".");
        std::string outputfilename = inputfilename;

        if (std::string::npos == dotpos)
        {
            throw ExceptionHandler("Filename without extension\n");
        }

        outputfilename = inputfilename.substr(0, dotpos + 1); // +1 to include the dot itself
        outputfilename.append(newextension);
        return outputfilename;
    }

    void ReadSamples_IEEE(unsigned int numsamples, const short numchannels, std::ifstream &file_handle, std::vector<float> &lbuffer)
    {
        for (unsigned int sample = 0; sample < numsamples; sample++)
        {
            float f;
            f = ReadFloat(file_handle);
            lbuffer.push_back(f);
            if (2 == numchannels)
            {
                f = ReadFloat(file_handle);
                lbuffer.push_back(f);
            }
        }
    }

    void ReadSamples_PCM(unsigned int numsamples, short numchannels, short num_bits, std::ifstream &file_handle, std::vector<int> &lbuffer)
    {
        for (unsigned int sample = 0; sample < numsamples; sample++)
        {
            int s, sl, sr;
            switch (num_bits)
            {
            case 8:
                s = (int)ReadChar8(file_handle) << 23; // TODO it shall be 24 bits, but it makes the output distorted. Somehow 23 works fine.
                lbuffer.push_back(s);
                if (2 == numchannels)
                {
                    s = (int)ReadChar8(file_handle) << 23;
                    lbuffer.push_back(s);
                }
                break;
            case 16:
                s = (int)ReadInt16(file_handle) << 16;
                lbuffer.push_back(s);
                if (2 == numchannels)
                {
                    s = (int)ReadInt16(file_handle) << 16;
                    lbuffer.push_back(s);
                }
                break;
            case 24:
                sl = (int)ReadInt24(file_handle) << 8;
                lbuffer.push_back(sl);
                if (2 == numchannels)
                {
                    sr = (int)ReadInt24(file_handle) << 8;
                    lbuffer.push_back(sr);
                }
                break;
            case 32:
                s = (int)ReadInt32(file_handle);
                lbuffer.push_back(s);
                if (2 == numchannels)
                {
                    s = (int)ReadInt32(file_handle);
                    lbuffer.push_back(s);
                }
                break;
            default:
            {
                throw ExceptionHandler("Invalid number of bits during reading samples.\n");
            }
            }
        }
    }
}
// end of HelperFunctions
