/*! \file wav2mp3.cpp
    \brief Source file of application.
*/

#include "wav2mp3.h"

/// Maximum number of threads
/** Maximum number of threads.
 * Consider the number of physical/virtual cores of actual CPU.
 * Set it to 1 for single threaded execution */
#define MAX_THREADS (6)

using namespace HelperFunctions;

/// Global mutex
/** Global mutex for threads can share the access of structure containig file list. */
std::mutex g_mutex;

/// Constructior of ConversionCoordinator
/** Constructior of ConversionCoordinator
 * It stores the list of files, and also counts the files tried and successfully converted */
ConversionCoordinator::ConversionCoordinator(std::vector<std::string> &FileList) : WavFileList(FileList),
                                                                                   files_tried(0),
                                                                                   files_success(0) {}

/// Function for converting wav files
/** Function for converting wav files to mp3
 * It pops the first file in the list and tries to convert it:
 * Steps: Reading wav file, Parsing wav file, if the header is correct, then reads sample data.
 * Finally it starts converting the file (depending on the format).
 * Another conversion is started until the wav file list is not empty.
 * Returns nullptr */
void *ConversionCoordinator::ConvertWavFile(void *args)
{
    std::unique_lock<std::mutex> guard(g_mutex);
    if (!guard.owns_lock())
    {
        guard.lock();
    }
    thread_params tp = *(reinterpret_cast<thread_params *>(args));
    debugm << "*** THREAD START: " << (int)tp.thread_id << "\n";

    while (tp.wav_file_list.size() > 0)
    {
        std::string wavfilename = tp.wav_file_list.back();
        tp.wav_file_list.pop_back();
        tp.files_tried++;
        guard.unlock();

        WavReader wav(wavfilename);
        try
        {
            wav.ParseWavFile2();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        if (wav.IsValid())
        {
            int numsamples = wav.GetNumSamples();
            std::string mp3filename = ReplaceExtension(wavfilename, "mp3");
            switch (wav.GetAudioFormat())
            {
            case (wav_header::Audio_Format_IDs::audio_format_PCM):
            {
                const int *leftpcmdata = wav.GetLeftPCMData();
                const int *rightpcmdata = wav.GetRightPCMData();
                MP3Encoder mp3enc(mp3filename, leftpcmdata, rightpcmdata, numsamples, wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
                if (mp3enc.GetBytesWritten() > 0)
                {
                    tp.files_success++;
                }
                break;
            }
            case (wav_header::Audio_Format_IDs::audio_format_IEEE):
            {
                const float *leftieeedata = wav.GetLeftIEEEData();
                const float *rightieedata = wav.GetRightIEEEData();
                MP3Encoder mp3enc(mp3filename, leftieeedata, rightieedata, numsamples, wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
                if (mp3enc.GetBytesWritten() > 0)
                {
                    tp.files_success++;
                }
                break;
            }
            default:
            {
                errorm << "Invalid audio format\n";
            }
            }
        }
        else
        {
            errorm << "Cannot parse wav file: " << wavfilename << "\n";
        }

        if (!guard.owns_lock())
        {
            guard.lock();
        }
    }

    if (guard.owns_lock())
    {
        guard.unlock();
    }
    debugm << "*** THREAD END: " << tp.thread_id << "\n";
    pthread_exit(NULL);

    return nullptr;
}

/// Function for converting all the wav files
/** Function for converting all wav file to mp3.
 * Each thread runs until there is another wav file in the list.
 * This function waits until all threads are finished (all files were converted) */
void ConversionCoordinator::ConvertWavFiles()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (unsigned int thread_id = 0; thread_id < MAX_THREADS; thread_id++)
    {
        thread_params tp{thread_id, WavFileList, files_tried, files_success};
        threads.emplace_back();
        auto ret_thread = pthread_create(&threads.back(), NULL, &ConversionCoordinator::ConvertWavFile, (void *)(&tp));
        if (ret_thread)
        {
            errorm << "Error:unable to create thread: " << ret_thread << "\n";
            continue;
        }
    }

    // wait for threads to finish
    for (auto &thread : threads)
    {
        try
        {
            auto ret = pthread_join(thread, NULL);
            if (ret)
            {
                errorm << "Error:unable to join thread: " << ret << "\n";
                ;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    return;
}

/// The main function of the application.
/** The main function, which starts the conversion.
 * It checks the arguments, and collects wav files into a list.
 * It passes the file list to ConversionCoordinator, which manages the conversion process. */
int main(int argc, char **argv)
{
    if (2 != argc)
    {
        coutm << "Incorrect number of arguments. 1st argument shall be path";
        return 1;
    }

    const std::string inputpath(argv[1]);

    auto dr = opendir(inputpath.c_str());
    if (NULL == dr)
    {
        errorm << "\nError occurred: directory not found!\n";
        return 1;
    }

    std::vector<std::string> wavfilelist;
    for (auto d = readdir(dr); d != NULL; d = readdir(dr))
    {
        std::string filename(d->d_name);
        if (4 < filename.size())
        {
            std::string extension(filename.substr(filename.size() - 4));
            std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
            if (extension.compare(".WAV") == 0)
            {
                wavfilelist.push_back(inputpath + "/" + filename);
            }
        }
    }
    closedir(dr);

    debugm << "Number of possible wav files found: " << wavfilelist.size() << "\n";
    if (0 == wavfilelist.size())
    {
        return 1;
    }

    auto start = std::chrono::system_clock::now();

    ConversionCoordinator convert(wavfilelist);
    try
    {
        convert.ConvertWavFiles();
    }
    catch (const std::runtime_error &d)
    {
        errorm << d.what() << "\n";
    }

    // endgame, runtime measurement
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    coutm << "elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
