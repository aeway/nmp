#include <cstring>
#include <string>
#include <stdexcept>
#include <vector>
#include <inttypes.h>

#include <sndfile.h>
static void decode_by_sndfile(const std::string& filename, float **data_dest, unsigned int *sample_rate_dest,
                              unsigned int *sample_count_dest, unsigned int *channels_dest)
{
    SNDFILE *file;
    SF_INFO info;
    
    memset(&info, 0, sizeof(info));
    if ((file = sf_open(filename.c_str(), SFM_READ, &info)) == NULL)
        throw std::runtime_error("libsndfile unable to open file " + filename + ":\n"
                                 + sf_strerror(NULL) + "\n");
    
    *sample_rate_dest = info.samplerate;
    *sample_count_dest = info.channels * info.frames;
    *channels_dest = info.channels;
    
    *data_dest = new float[info.channels * info.frames];
    sf_readf_float(file, *data_dest, info.frames);
    
    sf_close(file);
}


#define MAX_INT (((uint32_t)~0) >> 1)
#define SOX_SAMPLE_TO_FLOAT(sample) ((double)(sample) / (double)(MAX_INT))

#include <sox.h>
static void decode_by_sox(const std::string& filename, float **data_dest, unsigned int *sample_rate_dest,
                          unsigned int *sample_count_dest, unsigned int *channels_dest)
{
    static int init_once = sox_format_init();

    // supress libsox's error output to stdout/stderr
    sox_get_globals()->verbosity = 0;

    sox_format_t *file = sox_open_read(filename.c_str(), NULL, NULL, NULL);
    if (!file)
        throw std::runtime_error("libsox unable to open file " + filename);

    *sample_count_dest = file->signal.length;
    *sample_rate_dest = file->signal.rate;
    *channels_dest = file->signal.channels;
    
    *data_dest = new float[*sample_count_dest];
    int all_read = 0;
    sox_sample_t buf[1024];
    size_t samples_read;
    while ((samples_read = sox_read(file, buf, 1024)) > 0) {
        for (int i = 0; i < samples_read; i++)
            (*data_dest)[all_read++] = SOX_SAMPLE_TO_FLOAT(buf[i]);
    }

    sox_close(file);
}


void decode_audio_file(const std::string& filename, float **data_dest, unsigned int *sample_rate_dest,
                       unsigned int *sample_count_dest, unsigned int *channels_dest)

{
    try {
        decode_by_sox(filename, data_dest, sample_rate_dest, sample_count_dest, channels_dest);        
    } catch (std::runtime_error& e) {
        decode_by_sndfile(filename, data_dest, sample_rate_dest, sample_count_dest, channels_dest);
    }
}
