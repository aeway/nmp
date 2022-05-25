#include "audio_player.hpp"
#include <stdexcept>
#include <cstring>

constexpr unsigned int TIME_CHANGED_INTERVAL_MS = 1000;

void fileFinishedPlayingChecker(AudioPlayer *player)
{
    player->finished_playing_lock.lock();
    while (true) {
        player->finished_playing_lock.lock();
        try {
            player->finished_playing_callback();
        } catch (std::runtime_error& e) { }
    }
}

int portaudioCallback(const void *input, void *output, unsigned long frame_count,
                             const PaStreamCallbackTimeInfo* time_info,
                             PaStreamCallbackFlags status_flags,
                             void *user_data)
{
    AudioPlayer& player = *(AudioPlayer*)user_data;
    
    if (player.paused || !player.samples)
        return 0;

    bool end_of_song = false;
    if (frame_count * player.channels + player.current_sample >= player.sample_count) {
        frame_count = (player.sample_count - player.current_sample) / player.channels;
        end_of_song = true;
    }
    
    memcpy(output, player.samples + player.current_sample, player.channels * frame_count * sizeof(float));
    player.current_sample += player.channels * frame_count;

    if (player.getPlayingTime_MS() - player.last_reported_time_ms >= TIME_CHANGED_INTERVAL_MS) {
        player.last_reported_time_ms = player.getPlayingTime_MS();
        player.time_changed_callback();
    }

    if (end_of_song) {
        player.finished_playing_lock.unlock();
        return paAbort;
    }
    
    return 0;
}


AudioPlayer::AudioPlayer(std::function<void ()> finished_playing_callback,
                         std::function<void ()> time_changed_callback)
    : finished_playing_checker_thread {fileFinishedPlayingChecker, this}
{
    AudioPlayer::finished_playing_callback = finished_playing_callback;
    AudioPlayer::time_changed_callback = time_changed_callback;

    if (Pa_Initialize() != paNoError)
        throw std::runtime_error("Couldn't initialize portaudio\n");
}

AudioPlayer::~AudioPlayer()
{
    Pa_Terminate();
}


void decode_audio_file(const std::string& filename, float **data_dest, unsigned int *sample_rate_dest,
                       unsigned int *sample_count_dest, unsigned int *channels_dest);

void AudioPlayer::playFile(const FolderNode& node)
{
    if (samples) {
        delete[] samples;
        samples = nullptr;
    }

    if (pa_stream) {
        Pa_CloseStream(pa_stream);
        pa_stream = nullptr;
    }

    decode_audio_file(node.path, &samples, &sample_rate, &sample_count, &channels);
    current_sample = 0;
    currently_playing_inode_number = node.inode_number;
    last_reported_time_ms = 0;
    
    if (Pa_OpenDefaultStream(&pa_stream,
                             0, // no input channels
                             channels,
                             paFloat32,
                             sample_rate,
                             paFramesPerBufferUnspecified,
                             portaudioCallback,
                             this
            ) != paNoError)
        throw std::runtime_error("Portaudio couldn't open default stream\nsample_rate: "
                                 + std::to_string(sample_rate) + "\n");
    
    unpause();
}

void AudioPlayer::pause()
{
    paused = true;
    Pa_StopStream(pa_stream);
}

void AudioPlayer::unpause()
{
    if (!isPlaying())
        return;
    
    if (Pa_StartStream(pa_stream) != paNoError)
        throw std::runtime_error("Portaudio couldn't start default stream");

    paused = false;
}

void AudioPlayer::togglePause()
{
    paused ? unpause() : pause();
}

bool AudioPlayer::isPlaying()
{
    return samples && current_sample < sample_count;
}

bool AudioPlayer::isPlayingNode(const FolderNode& node)
{
    return isPlaying() && currently_playing_inode_number == node.inode_number;
}

unsigned int AudioPlayer::getPlayingTime_MS()
{
    if (!isPlaying())
        return 0;
    return (int)(current_sample * 1000.0 / sample_rate / channels);
}
