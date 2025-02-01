#include "return_codes.h"
#include "use_ffmpeg.h"
#include <libavutil/log.h>
#include <libswresample/swresample.h>

uint8_t perediscretization(Audio *audio1, Audio *audio2)
{
	Audio *min_smpl = (audio1->codec_context->sample_rate < audio2->codec_context->sample_rate) ? audio1 : audio2;
	Audio *max_smpl = (audio1->codec_context->sample_rate > audio2->codec_context->sample_rate) ? audio1 : audio2;
	min_smpl->swr_ctx = swr_alloc();
	if (!min_smpl->swr_ctx)
	{
		fprintf(stderr, "swr context couldn't be allocated\n");
		return ERROR_ARGUMENTS_INVALID;
	}

	if (swr_alloc_set_opts2(
			&min_smpl->swr_ctx,
			&min_smpl->codec_context->ch_layout,
			AV_SAMPLE_FMT_DBLP,
			max_smpl->codec_context->sample_rate,
			&min_smpl->codec_context->ch_layout,
			min_smpl->codec_context->sample_fmt,
			min_smpl->codec_context->sample_rate,
			0,
			NULL) < 0)
	{
		fprintf(stderr, "swr context couldn't be set\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (swr_init(min_smpl->swr_ctx) < 0)
	{
		fprintf(stderr, "swr couldn't be initialized\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	return SUCCESS;
}

uint8_t add_ell_in_array(Audio *audio, double value)
{
	if (audio->size == audio->max_size)
	{
		audio->max_size = audio->max_size * 2;
		double *newBlock = realloc(audio->block, audio->max_size * sizeof(double));
		if (!newBlock)
		{
			fprintf(stderr, "memory couldn't be allocated\n");
			return ERROR_NOTENOUGH_MEMORY;
		}
		audio->block = newBlock;
	}
	audio->block[audio->size] = value;
	audio->size++;
	return SUCCESS;
}

uint8_t take_sample(Audio *audio, uint8_t index)
{
	uint8_t result = SUCCESS;
	int32_t readStatus;
	uint8_t *pointer[8];
	while (av_read_frame(audio->format_context, audio->packet) >= 0)
	{
		if (audio->packet->stream_index != audio->audio_stream_index)
			continue;
		readStatus = avcodec_send_packet(audio->codec_context, audio->packet);
		if (readStatus < 0)
		{
			fprintf(stderr, "couldn't send the packet");
			avcodec_free_context(&audio->codec_context);
			avformat_close_input(&audio->format_context);
			return ERROR_ARGUMENTS_INVALID;
		}
		while (readStatus >= 0)
		{
			readStatus = avcodec_receive_frame(audio->codec_context, audio->frame);
			if (readStatus == AVERROR(EAGAIN) || readStatus == AVERROR_EOF)
			{
				break;
			}
			else if (readStatus < 0)
			{
				fprintf(stderr, "couldn't receive the frame");
				result = ERROR_ARGUMENTS_INVALID;
				goto cleanUp;
			}
			pointer[0] = malloc(audio->frame->nb_samples * sizeof(double));
			pointer[1] = malloc(audio->frame->nb_samples * sizeof(double));
			if (!pointer[0] || !pointer[1])
			{
				fprintf(stderr, "memory couldn't be allocated\n");
				result = ERROR_NOTENOUGH_MEMORY;
				goto cleanUp;
			}
			if (swr_convert(
					audio->swr_ctx,
					(uint8_t **)pointer,
					audio->frame->nb_samples,
					(const uint8_t **)audio->frame->data,
					audio->frame->nb_samples) < 0)
			{
				fprintf(stderr, "swr couldn't be converted\n");
				result = ERROR_ARGUMENTS_INVALID;
				goto cleanUp;
			}

			for (size_t i = 0; i < audio->frame->nb_samples; i++)
				if (add_ell_in_array(audio, ((double *)pointer[index])[i]) != SUCCESS)
				{
					result = ERROR_NOTENOUGH_MEMORY;
					goto cleanUp;
				}
		}
		av_packet_unref(audio->packet);
	}
cleanUp:
	avcodec_free_context(&audio->codec_context);
	avformat_close_input(&audio->format_context);
	av_frame_free(&audio->frame);
	return result;
}

uint8_t readFileAudio(const char *filename, Audio *audio, uint8_t argc)
{
	audio->format_context = avformat_alloc_context();
	if (!audio->format_context)
	{
		fprintf(stderr, "format context couldn't be allocated\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	int32_t res = avformat_open_input(&audio->format_context, filename, NULL, NULL);
	if (res != 0)
	{
		if (res == AVERROR(ENOMEM))
		{
			fprintf(stderr, "Not enough memory\n");
			return ERROR_NOTENOUGH_MEMORY;
		}
		else if (res == AVERROR(EIO) || res == AVERROR(EOVERFLOW) || res == AVERROR(ENOSYS) || res == AVERROR(ENOENT) ||
				 res == AVERROR(EINVAL) || res == AVERROR(EAGAIN) || res == AVERROR_EOF)
		{
			fprintf(stderr, "I/O error\n");
			return ERROR_ARGUMENTS_INVALID;
		}
		fprintf(stderr, "Error with avformat_open_input'\n");
		return ERROR_CANNOT_OPEN_FILE;
	}
	if (avformat_find_stream_info(audio->format_context, NULL) < 0)
	{
		fprintf(stderr, "Cannot find stream information\n");
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	audio->audio_stream_index = av_find_best_stream(audio->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audio->audio_stream_index < 0)
	{
		fprintf(stderr, "Cannot find a video stream in the input file\n");
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}

	audio->codec_params = audio->format_context->streams[audio->audio_stream_index]->codecpar;
	if (audio->codec_params == NULL)
	{
		fprintf(stderr, "Cannot find the codec parameters\n");
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	audio->codec = avcodec_find_decoder(audio->codec_params->codec_id);
	enum AVCodecID avcf = audio->codec_params->codec_id;
	if (!audio->codec ||
		(avcf != AV_CODEC_ID_MP2 && avcf != AV_CODEC_ID_MP3 && avcf != AV_CODEC_ID_AAC && avcf != AV_CODEC_ID_OPUS && avcf != AV_CODEC_ID_FLAC))
	{
		fprintf(stderr, "Unsupported codec\n");
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}

	audio->codec_context = avcodec_alloc_context3(audio->codec);
	if (!audio->codec_context)
	{
		fprintf(stderr, "codec context couldn't be allocated\n");
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (avcodec_parameters_to_context(audio->codec_context, audio->codec_params) < 0)
	{
		fprintf(stderr, "codec parameters couldn't be copied to codec context\n");
		avcodec_free_context(&audio->codec_context);
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (avcodec_open2(audio->codec_context, audio->codec, NULL) < 0)
	{
		fprintf(stderr, "codec couldn't be opened\n");
		avcodec_free_context(&audio->codec_context);
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	audio->frame = av_frame_alloc();
	if (!audio->frame)
	{
		fprintf(stderr, "frame couldn't be allocated\n");
		avcodec_free_context(&audio->codec_context);
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (audio->codec_context->ch_layout.nb_channels < 2 && argc == 2)
	{
		fprintf(stderr, "error with number of channels");
		return ERROR_ARGUMENTS_INVALID;
	}
	enum AVSampleFormat sf = audio->codec_context->sample_fmt;
	if (sf != AV_SAMPLE_FMT_S16 && sf != AV_SAMPLE_FMT_S16P && sf != AV_SAMPLE_FMT_S32 && sf != AV_SAMPLE_FMT_S32P &&
		sf != AV_SAMPLE_FMT_S64 && sf != AV_SAMPLE_FMT_S64P && sf != AV_SAMPLE_FMT_FLT && sf != AV_SAMPLE_FMT_FLTP &&
		sf != AV_SAMPLE_FMT_DBL && sf != AV_SAMPLE_FMT_DBLP)
	{
		fprintf(stderr, "error with sample format");
		avcodec_free_context(&audio->codec_context);
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}

	audio->swr_ctx = swr_alloc();
	if (!audio->swr_ctx)
	{
		fprintf(stderr, "swr context couldn't be allocated\n");
		avcodec_free_context(&audio->codec_context);
		avformat_close_input(&audio->format_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (swr_alloc_set_opts2(
			&audio->swr_ctx,
			&audio->codec_context->ch_layout,
			AV_SAMPLE_FMT_DBLP,
			audio->codec_context->sample_rate,
			&audio->codec_context->ch_layout,
			audio->codec_context->sample_fmt,
			audio->codec_context->sample_rate,
			0,
			NULL) < 0)
	{
		fprintf(stderr, "swr context couldn't be set\n");
		avcodec_free_context(&audio->codec_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (swr_init(audio->swr_ctx) < 0)
	{
		fprintf(stderr, "swr context couldn't be initialized\n");
		avcodec_free_context(&audio->codec_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	audio->packet = av_packet_alloc();
	if (!audio->packet)
	{
		fprintf(stderr, "packet couldn't be allocated\n");
		avcodec_free_context(&audio->codec_context);
		return ERROR_ARGUMENTS_INVALID;
	}
	audio->sample_rate = audio->codec_context->sample_rate;
	audio->size = 0;
	audio->max_size = 512;
	audio->block = malloc(audio->max_size * sizeof(double));
	if (!audio->block)
	{
		fprintf(stderr, "memory couldn't be allocated\n");
		avcodec_free_context(&audio->codec_context);
		return ERROR_NOTENOUGH_MEMORY;
	}
	return SUCCESS;
}
