#include "wav2mp3.h"

#define MAX_THREADS (1)

using namespace HelperFunctions;

std::mutex g_mutex;

ConversionCoordinator::ConversionCoordinator(std::vector<std::string> &FileList) : WavFileList(FileList)
{
}

void *ConversionCoordinator::ConvertWavFile(void *args)
{
    std::unique_lock<std::mutex> guard(g_mutex);
    if (!guard.owns_lock())
    {
        guard.lock();
    }
    thread_params tp = *(reinterpret_cast<thread_params *>(args));
    debugm << "*** THREAD START: " << (int)tp.thread_id << "\n";

    while (tp.WavFileList.size() > 0)
    {
        std::string wavfilename = tp.WavFileList.back();
        tp.WavFileList.pop_back();
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
            case (wav_header::Audio_Format_IDs::Audio_Format_PCM):
            {
                const int *leftpcmdata = wav.GetLeftPCMData();
                const int *rightpcmdata = wav.GetRightPCMData();
                MP3Encoder mp3enc(mp3filename, leftpcmdata, rightpcmdata, numsamples, wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
                break;
            }
            case (wav_header::Audio_Format_IDs::Audio_Format_IEEE):
            {
                const float *leftieeedata = wav.GetLeftIEEEData();
                const float *rightieedata = wav.GetRightIEEEData();
                MP3Encoder mp3enc(mp3filename, leftieeedata, rightieedata, numsamples, wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
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
    pthread_exit(NULL); // TODO

    return nullptr;
}

void ConversionCoordinator::ConvertWavFiles()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (unsigned int thread_id = 0; thread_id < MAX_THREADS; thread_id++)
    {
        thread_params tp{thread_id, WavFileList};
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

int main(int argc, char **argv)
{
    if (2 != argc)
    {
        coutm << "Incorrect number of arguments. 1st argument shall be path";
        return 1;
    }

    const std::string inputpath(argv[1]);
    debugm << "inputpath=" << inputpath << "\n";

    // collect files
    auto dr = opendir(inputpath.c_str());
    if (dr != NULL)
    {
        std::vector<std::string> WavFileList;
        for (auto d = readdir(dr); d != NULL; d = readdir(dr))
        {
            std::string filename(d->d_name);
            if (filename.size() > 4)
            {
                std::string extension(filename.substr(filename.size() - 4));
                std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
                if (extension.compare(".WAV") == 0)
                {
                    debugm << "WAV: " << filename << "\n";
                    WavFileList.push_back(inputpath + "/" + filename); // TODO  make windows compatible
                }
            }
        }
        closedir(dr);

        debugm << "Number of possible wav files found: " << WavFileList.size() << "\n";
        if (0 == WavFileList.size())
        {
            return 1;
        }

        auto start = std::chrono::system_clock::now();

        ConversionCoordinator convert(WavFileList);
        try
        {
            convert.ConvertWavFiles();
        }
        catch (const std::runtime_error &d)
        {
            errorm << d.what() << "\n";
        }

        // endgame
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        coutm << "elapsed time: " << elapsed_seconds.count() << "s\n";
    }
    else
    {
        errorm << "\nError occurred: directory not found!\n";
        return 1;
    }

    return 0;
}
