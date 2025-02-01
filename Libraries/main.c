#include "return_codes.h"
#include "use_ffmpeg.h"
#include "use_fftw.h"
#include <libavutil/log.h>

void clean_audio_data(Audio *audio)
{
	if (audio->block != NULL)
	{
		free(audio->block);
		audio->block = NULL;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "error format");
		return ERROR_ARGUMENTS_INVALID;
	}
	uint8_t result;
	av_log_set_level(AV_LOG_QUIET);
	Audio a1, a2;
	int32_t delta_samples = 0;
	uint8_t index = 0;

	result = readFileAudio(argv[1], &a1, argc);
	if (result != SUCCESS)
	{
		fprintf(stderr, "error read audio file");
		result = ERROR_FORMAT_INVALID;
		goto cleanUp;
	}
	result = readFileAudio(argc == 2 ? argv[1] : argv[2], &a2, argc);
	if (result != SUCCESS)
	{
		fprintf(stderr, "error read audio file");
		result = ERROR_FORMAT_INVALID;
		goto cleanUp;
	}
	if (a1.sample_rate != a2.sample_rate)
		perediscretization(&a1, &a2);

	take_sample(&a1, index);
	index += (argc == 2);
	take_sample(&a2, index);

	result = crossCorrelation(&a1, &a2, &delta_samples);
	if (result != SUCCESS)
		goto cleanUp;
	printf("delta: %d samples\n", delta_samples);
	printf("sample rate: %d Hz\n", a1.sample_rate);
	printf("delta time: %d ms\n", delta_samples * 1000 / a1.sample_rate);
	clean_audio_data(&a1);
	if (argc == 3)
		clean_audio_data(&a2);

cleanUp:
	return result;
}
