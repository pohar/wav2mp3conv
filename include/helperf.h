#ifndef __HELPERF_H__
#define __HELPERF_H__

#include <iostream>
#include <fstream>
#include <vector>

namespace HelperFunctions{

    //static unsigned is_little_endian_const;

    unsigned is_little_endian(void);
    float ReadFloat(std::ifstream &file);
    struct CoutMuter
    {
    public:
        CoutMuter(bool arg_enable_cout = 0) : enable_cout(arg_enable_cout) {}
        ~CoutMuter()
        {
            if (enable_cout)
            {
                std::cout << std::flush;
            }
        }

        template <typename T>
        CoutMuter &operator<<(T const &msg)
        {
            if (enable_cout)
            {
                std::cout << msg;
            }
            return *this;
        }

    private:
        bool enable_cout;
    };
    static CoutMuter coutm(1), errorm(1), debugm(1);
    unsigned int ReadInt32(std::ifstream &file);
    unsigned int ReadInt24(std::ifstream &file);
    unsigned int ReadInt16(std::ifstream &file);
    unsigned char ReadChar8(std::ifstream &file);
    std::string ReplaceExtension(const std::string inputfilename, const char *newextension);
    void ReadSamples_IEEE(unsigned int numsamples, const short numchannels, std::ifstream &file_handle, std::vector<float> &lbuffer, std::vector<float> &rbuffer);
    void ReadSamples_PCM(unsigned int numsamples, short numchannels, short num_bits, std::ifstream &file_handle, std::vector<int> &lbuffer, std::vector<int> &rbuffer);

}

#endif // __HELPERF_H__
