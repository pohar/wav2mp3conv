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
    explicit thread_params(unsigned int id, std::vector<std::string>& FileList, unsigned int &arg_files_tried, unsigned int &arg_files_success) : thread_id{ id },
        WavFileList{ FileList },
        files_tried{ arg_files_tried },
        files_success{ arg_files_success }  {}

    const unsigned int thread_id;
    std::vector<std::string>& WavFileList;
    unsigned int &files_tried;
    unsigned int &files_success;
};

class ConversionCoordinator
{

public:
    explicit ConversionCoordinator(std::vector<std::string>&);
    void ConvertWavFiles();
    auto GetFilesTried() const { return files_tried; };
    auto GetFilesSuccess() const { return files_success; };
private:
    static void* ConvertWavFile(void* args);
    std::vector<std::string>& WavFileList;
    std::mutex WavFileList_mutex;
    std::list<pthread_t> threads;
    unsigned int files_tried;
    unsigned int files_success;
};

#endif // __WAV2MP3_H__
