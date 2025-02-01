#include "use_fftw.h"

#include "return_codes.h"
#include "use_ffmpeg.h"

#include <fftw3.h>

fftw_plan check_plan(fftw_plan plan)
{
	if (!plan)
	{
		fprintf(stderr, "fftw_plan uncorrect");
		return NULL;
	}
	return plan;
}

uint8_t crossCorrelation(const Audio *a1, const Audio *a2, int *delta_samples)
{
	uint8_t result = SUCCESS;
	size_t n = a1->size > a2->size ? a1->size : a2->size;
	fftw_complex *a1fftw, *a2fftw, *correlation, *memory;
	double *doubleMemory, *inFirst, *inSecond, *res;

	doubleMemory = fftw_alloc_real(sizeof(double) * n * 3);
	memory = fftw_alloc_complex(sizeof(fftw_complex) * n * 3);
	if (!doubleMemory || !memory)
	{
		fprintf(stderr, "Not enough memory");
		return ERROR_NOTENOUGH_MEMORY;
	}
	inFirst = doubleMemory, inSecond = doubleMemory + n, res = doubleMemory + 2 * n;

	memset(inFirst, 0, n * sizeof(inFirst[0]));
	memcpy(inFirst, a1->block, (a1->size > n ? n : a1->size) * sizeof(inFirst[0]));
	memset(inSecond, 0, n * sizeof(inSecond[0]));
	memcpy(inSecond, a2->block, (a2->size > n ? n : a2->size) * sizeof(inSecond[0]));

	a1fftw = memory, a2fftw = memory + n, correlation = memory + 2 * n;
	fftw_plan plan1, plan2, plan3;

	plan1 = fftw_plan_dft_r2c_1d((int32_t)n, inFirst, a1fftw, FFTW_ESTIMATE);
	plan2 = fftw_plan_dft_r2c_1d((int32_t)n, inSecond, a2fftw, FFTW_ESTIMATE);
	if (!check_plan(plan1) || !check_plan(plan2))
	{
		fprintf(stderr, "Not enough memory");
		result = ERROR_NOTENOUGH_MEMORY;
		goto cleanUp;
	}

	fftw_execute(plan1);
	fftw_execute(plan2);

	for (size_t i = 0; i < n; i++)
	{
		correlation[i][0] = a1fftw[i][0] * a2fftw[i][0] + a1fftw[i][1] * a2fftw[i][1];
		correlation[i][1] = -a1fftw[i][0] * a2fftw[i][1] + a1fftw[i][1] * a2fftw[i][0];
	}
	plan3 = fftw_plan_dft_c2r_1d((int32_t)n, correlation, res, FFTW_ESTIMATE);
	if (!check_plan(plan3))
	{
		fprintf(stderr, "Not enough memory");
		result = ERROR_NOTENOUGH_MEMORY;
		goto cleanUp;
	}
	fftw_execute(plan3);

	int32_t maxVal = 0;
	for (size_t i = 0; i < n; i++)
	{
		if (res[i] > res[maxVal])
			maxVal = (int32_t)i;
	}
	*delta_samples = maxVal;
	if (*delta_samples > n >> 1)
		*delta_samples -= (int32_t)n;

cleanUp:
	fftw_free(doubleMemory);
	fftw_free(memory);
	return result;
}
