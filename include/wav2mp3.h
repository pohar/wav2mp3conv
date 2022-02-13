/*! \file wav2mp3.h
    \brief Header file of application.
*/

#ifndef __WAV2MP3_H__
#define __WAV2MP3_H__

#ifdef _WIN32
#include <direntw.h>
#else
#include <dirent.h>
#endif
#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC
#define PTW32_STATIC_LIB
#endif
#include <pthread.h>
#include <list>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include "helperf.h"
#include "wavreader.h"
#include "mp3encoder.h"

/// Structure containing thread parameters and info
/** Structure containing thread parameters and info
 * It stores the id of the thread, the actual list of files to convert,
 * the number of files that were tried to convert and the number of successful conversions.*/
struct thread_params
{
    explicit thread_params(unsigned int id, std::vector<std::string> &FileList, unsigned int &arg_files_tried, unsigned int &arg_files_success) : thread_id{id},
                                                                                                                                                  wav_file_list{FileList},
                                                                                                                                                  files_tried{arg_files_tried},
                                                                                                                                                  files_success{arg_files_success} {}

    const unsigned int thread_id;
    std::vector<std::string> &wav_file_list;
    unsigned int &files_tried;
    unsigned int &files_success;
};

/// Class for coordinating the conversions
/** Class for coordinating the conversions
 * It tries to convert the list of wav files to mp3.
 * It also count the tried and successfully converted files. */
class ConversionCoordinator
{

public:
    explicit ConversionCoordinator(std::vector<std::string> &);
    void ConvertWavFiles();
    auto GetFilesTried() const { return files_tried; };
    auto GetFilesSuccess() const { return files_success; };

private:
    static void *ConvertWavFile(void *args);
    std::vector<std::string> &WavFileList;
    std::mutex WavFileList_mutex;
    std::list<pthread_t> threads;
    unsigned int files_tried;
    unsigned int files_success;
};

#endif // __WAV2MP3_H__
