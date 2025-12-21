
// ****************************************************************************
//
//                    Music Player - IMA ADPCM decompression
//
// ****************************************************************************

#include "../include.h"

// IMA ADPCM tables
#define ADPCM_MINVAL	-32768		// IMA ADPCM minimal value
#define ADPCM_MAXVAL	+32767		// IMA ADPCM maximal value
#define ADPCM_STEPS	89		// IMA ADPCM number of steps

const s8 ADPCM_TabInx[16] =	// IMA ADPCM table of index change steps
{
	-1,	-1,	-1,	-1,	// +0 .. +3, decrease step size
	2,	4,	6,	8,	// +4 .. +7, increase step size
	-1,	-1,	-1,	-1,	// -0 .. -3, decrease step size
	2,	4,	6,	8,	// -4 .. -7, increase step size
};

const s16 ADPCM_StepSize[ADPCM_STEPS] =	// IMA ADPCM table of step sizes
{
	7,	8,	9,	10,	11,	12,	13,	14,
	16,	17,	19,	21,	23,	25,	28,	31,
	34,	37,	41,	45,	50,	55,	60,	66,
	73,	80,	88,	97,	107,	118,	130,	143,
	157,	173,	190,	209,	230,	253,	279,	307,
	337,	371,	408,	449,	494,	544,	598,	658,
	724,	796,	876,	963,	1060,	1166,	1282,	1411,
	1552,	1707,	1878,	2066,	2272,	2499,	2749,	3024,
	3327,	3660,	4026,	4428,	4871,	5358,	5894,	6484,
	7132,	7845,	8630,	9493,	10442,	11487,	12635,	13899,
	15289,	16818,	18500,	20350,	22385,	24623,	27086,	29794,
	32767
};

// ADPCM decompressor
sAdpcm Adpcm;

// ADPCM decompression (returns key code)
u8 Decomp()
{
	u8 b;
	int val;

	// pointer to descriptors
	sOutBuf* o = &OutBuf;
	sAdpcm* a = &Adpcm;

	while (True)
	{

#if BABYBEAT		// 1 = use BabyBeat device
		// keyboard
		u8 key = KeyGet();
		if (key != NOKEY)
		{
			switch (key)
			{
			// key "Play"
			case KEY_PLAY:
			case KEY_PLAY_LONG:
				a->pause = !a->pause;
				break;

			// key "VolUp", short
			case KEY_VOLUP:
				if (Volume < VOLMAX)
				{
					Volume++;
					VolMul = VolMulTab[Volume];
				}
				break;

			// key "VolDn", short
			case KEY_VOLDN:
				if (Volume > 0)
				{
					Volume--;
					VolMul = VolMulTab[Volume];
				}
				break;

			// seek forward "Next", long
			case KEY_NEXT_LONG:
				a->pause = False;
				InSeek(a->sampinx + SAMPSKIP);
				break;

			// seek backward "Prev", long
			case KEY_PREV_LONG:
				a->pause = False;
				InSeek(a->sampinx - SAMPSKIP);
				break;

			// other keys
			default:
				return key;
			}
		}
#endif // BABYBEAT

		// pause
		if (a->pause)
		{
			// add value to output
			OutSamp(a->prevval);
			continue;
		}

		// preamble block (mono: 4 bytes)
		if (a->sampcnt <= 0)
		{
			// get current value from block header
			b = GetByte();
			val = (s16)(b + GetByte()*256);
			a->prevval = (s16)val;
			a->stepinx = (s8)GetByte();
			GetByte(); // skip 3rd byte

			// set counter of samples per block
			a->sampcnt = a->sampblock; // new sample counter
			a->odd = False; // odd sub-sample
		}
		else
		{
			u8 delta;

			// get next sample delta
			if (a->odd) // odd sub-sample (higher 4 bits)
			{
				delta = (u8)(a->subsample >> 4);
			}
			else // even sub-sample (lower 4 bits)
			{
				delta = GetByte();
				a->subsample = delta;
				delta &= 0x0f;
			}

			// flip odd/even flag
			a->odd = !a->odd;

			// get step size
			s8 stepinx = a->stepinx;
			s16 step = ADPCM_StepSize[stepinx];

			// shift step index
			stepinx += ADPCM_TabInx[delta];

			// limit step index
			if ((uint)stepinx >= (uint)ADPCM_STEPS)
			{
				if (stepinx < 0)
					stepinx = 0;
				else
					stepinx = ADPCM_STEPS - 1;
			}
			a->stepinx = stepinx;

			// get next difference
			s16 dif = step >> 3;
			if ((delta & B0) != 0) dif += step >> 2;
			if ((delta & B1) != 0) dif += step >> 1;
			if ((delta & B2) != 0) dif += step ;
			if ((delta & B3) != 0) dif = -dif; // sign correction

			// add difference to previous value
			val = a->prevval + dif;
			if (val > ADPCM_MAXVAL) val = ADPCM_MAXVAL;
			if (val < ADPCM_MINVAL) val = ADPCM_MINVAL;
			a->prevval = (s16)val;
		}

		// sample counter
		a->sampcnt--;

		// add value to output
		OutSamp(a->prevval);

		// shift sample index
		a->sampinx++;

		// end of file (or read error) - skip to next file
		if ((a->sampinx >= a->sampnum) || InBuf.err) return KEY_NEXT;
	}
}
