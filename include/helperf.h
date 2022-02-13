/*! \file helperf.h
    \brief Header file containing helper functions.
*/

#ifndef __HELPERF_H__
#define __HELPERF_H__

#include <iostream>
#include <fstream>
#include <vector>

/// Collection of helping utilities, functions.
/** These functions help during various tasks of the software. */
namespace HelperFunctions
{
    /// Checking endianness.
    /** Function for determining endianness. Returns true if little endian. */
    unsigned IsLittleEndian(void);

    /// Class for redirecting standard outputs.
    /** The standard outputs (console, error, debug) can be either muted of displayed (depending on platform)*/
    class CoutMuter
    {
    public:
        /// Constructor of CoutMuter.
        /** The argument decides if the message can be displayed, or it is muted.*/
        CoutMuter(bool arg_enable_cout = 0) : enable_cout(arg_enable_cout) {}

        /// Destructor of CoutMuter.
        /** Destructor of CoutMuter. It flushes the remaining parts of the message. */
        ~CoutMuter()
        {
            if (enable_cout)
            {
                std::cout << std::flush;
            }
        }

        /// Displaying a message.
        /** Depending on the previously enabled setting, the operator << can either display or mute the message. */
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
    static CoutMuter coutm(0);
    static CoutMuter errorm(1);
    static CoutMuter debugm(0);

    /// Reads a float value from the stream.
    /** Reads a float value from the given stream.
     * The previously determined endianness is taken into account.
     * The read is using a union for byte-level access to the float variable.*/
    float ReadFloat(std::ifstream &file);

    /// Reads an (unsigned) 32bit integer value from the stream.
    /** Reads an (unsigned) 32bit integer value from the given stream.
     * The previously determined endianness is taken into account.
     * The read is using a union for byte-level access to the integer variable.*/
    unsigned int ReadInt32(std::ifstream &file);

    /// Reads an (unsigned) 24bit integer value from the stream.
    /** Reads an (unsigned) 24bit integer value from the given stream.
     * The previously determined endianness is taken into account.
     * The read is using a union for byte-level access to the integer variable.*/
    unsigned int ReadInt24(std::ifstream &file);

    /// Reads an (unsigned) 16bit integer value from the stream.
    /** Reads an (unsigned) 16bit integer value from the given stream.
     * The previously determined endianness is taken into account.
     * The read is using a union for byte-level access to the integer variable.*/
    unsigned int ReadInt16(std::ifstream &file);

    /// Reads an (unsigned) 8bit char value from the stream.
    /** Reads an (unsigned) 8bit char value from the given stream.
     * The previously determined endianness is irrelevant here.*/
    unsigned char ReadChar8(std::ifstream &file);

    /// Replaces the extension in the filename.
    /** Replaces the extension in the given filename with the given extension.
     * It finds the last dot in the filename and appends the new extension (after the dot).
     * Note: In the current SW the extension was checked before the conversion replaces it,
     * so this function doesn't have to handle bad filenames. */
    std::string ReplaceExtension(const std::string inputfilename, const char *newextension);

    /// Reads IEEE format samples from stream to buffer.
    /** Reads the given number of IEEE (float) samples to the buffer from the stream.
     * The number of channels can be 1 or 2.
     * In case of multiple channels, all the samples are stored in the same buffer.
     */
    void ReadSamples_IEEE(unsigned int numsamples, const short numchannels, std::ifstream &file_handle, std::vector<float> &lbuffer);

    /// Reads PCM format samples from stream to buffer.
    /** Reads the given number of PCM samples to the buffer from the stream.
     * The number of channels can be 1 or 2.
     * The number of bits can be 8, 16, 24 or 32.
     * In case of multiple channels, all the samples are stored in the same buffer.
     */
    void ReadSamples_PCM(unsigned int numsamples, short numchannels, short num_bits, std::ifstream &file_handle, std::vector<int> &lbuffer);
}

#endif // __HELPERF_H__
