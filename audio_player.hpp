#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include "external/portaudio.h"

#include "folder.hpp"

class AudioPlayer {
    friend void fileFinishedPlayingChecker(AudioPlayer *player);
    friend int portaudioCallback(const void *input, void *output, unsigned long frame_count,
                                 const PaStreamCallbackTimeInfo* time_info,
                                 PaStreamCallbackFlags status_flags,
                                 void *user_data);
public:
    AudioPlayer(std::function<void ()> finished_playing_callback,
                std::function<void ()> time_changed_callback);
    AudioPlayer() = delete;
    ~AudioPlayer();
    void playFile(const FolderNode& node);
    void pause();
    void unpause();
    void togglePause();
    bool isPlaying();
    bool isPlayingNode(const FolderNode& node);
    void seek_MS(int milliseconds_from_start);
    unsigned int getPlayingTime_MS();
private:
    std::mutex finished_playing_lock;
    std::thread finished_playing_checker_thread;
    std::function<void ()> finished_playing_callback;
    std::function<void ()> time_changed_callback;
    unsigned int channels;
    unsigned int sample_rate;
    float *samples = nullptr;
    unsigned int sample_count;
    unsigned int current_sample;
    bool paused = true;
    PaStream *pa_stream = nullptr;
    ino_t currently_playing_inode_number = 0;
    int last_reported_time_ms;
};
