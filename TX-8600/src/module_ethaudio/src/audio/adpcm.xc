#include "adpcm.h"

/* Intel ADPCM step variation table */
static const int IndexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static const int StepSizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
	

uint8_t adpcm_coder(int32_t Sample,struct adpcm_state &state)
{
	uint8_t code=0;
	int32_t tmpstep=0;
	int32_t diff=0;
	int32_t diffq=0;
	int32_t step=0;

	int32_t index;
	int32_t predsample;

	index=state.index;
	predsample=state.valprev;
	//
	step = StepSizeTable[index];

	/* 2. compute diff and record sign and absolut value */
	diff = Sample-predsample;
	if (diff < 0)
	{
	  code=8;
	  diff = -diff;
	}

	/* 3. quantize the diff into ADPCM code */
	/* 4. inverse quantize the code into a predicted diff */
	tmpstep = step;
	diffq = (step >> 3);

	if (diff >= tmpstep)
	{
	  code |= 0x04;
	  diff -= tmpstep;
	  diffq += step;
	}

	tmpstep = tmpstep >> 1;

	if (diff >= tmpstep)
	{
	  code |= 0x02;
	  diff -= tmpstep;
	  diffq+=(step >> 1);
	}

	tmpstep = tmpstep >> 1;

	if (diff >= tmpstep)
	{
	  code |=0x01;
	  diffq+=(step >> 2);
	}

	/* 5. fixed predictor to get new predicted sample*/
	if (code & 8)
	{
	  predsample -= diffq;
	}
	else
	{
	  predsample += diffq;
	}
	/* check for overflow*/
	if (predsample > 32767)
	{
	  predsample = 32767;
	}
	else if (predsample < -32768)
	{
	  predsample = -32768;
	}

	/* 6. find new stepsize index */
	index += IndexTable[code];
	/* check for overflow*/
	if (index <0)
	{
	  index = 0;
	}
	else if (index > 88)
	{
	  index = 88;
	}
	//
	state.valprev=predsample;
	state.index=index;
	/* 8. return new ADPCM code*/
	return (code & 0x0f);
}

