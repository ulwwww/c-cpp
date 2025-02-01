#ifndef LAB_2_USE_FFMPEG_H
#define LAB_2_USE_FFMPEG_H
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

typedef struct
{
	int32_t audio_stream_index;
	double *block;
	uint32_t size;
	uint32_t max_size;
	int32_t sample_rate;
	AVFormatContext *format_context;
	const AVCodec *codec;
	AVCodecParameters *codec_params;
	AVCodecContext *codec_context;
	AVPacket *packet;
	SwrContext *swr_ctx;
	AVFrame *frame;
} Audio;

uint8_t add_ell_in_array(Audio *audio, double value);
uint8_t readFileAudio(const char *filename, Audio *audio, uint8_t argc);
uint8_t perediscretization(Audio *audio1, Audio *audio2);
uint8_t take_sample(Audio *audio, uint8_t index);

#endif
