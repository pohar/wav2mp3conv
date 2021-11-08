#ifndef __WAV2MP3_H__
#define __WAV2MP3_H__

#ifdef _WIN32
#include <direntw.h>
#else
#include <dirent.h>
#endif
#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC // windows
#define PTW32_STATIC_LIB
#endif
#include <pthread.h>
#include <list>
#include <algorithm> // std::transform
#include <chrono>
#include <mutex>
#include <stdexcept>
#include "helperf.h"
#include "wavreader.h"
#include "mp3encoder.h"

struct thread_params
{
    explicit thread_params(unsigned int id, std::vector<std::string>& FileList) : thread_id{ id },
        WavFileList{ FileList } {}

    const unsigned int thread_id;
    std::vector<std::string>& WavFileList;
};

class ConversionCoordinator
{

public:
    explicit ConversionCoordinator(std::vector<std::string>&);
    void ConvertWavFiles();

private:
    static void* ConvertWavFile(void* args);
    std::vector<std::string>& WavFileList;
    std::mutex WavFileList_mutex;
    std::list<pthread_t> threads;
};

#endif // __WAV2MP3_H__
