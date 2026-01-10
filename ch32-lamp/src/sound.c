
// ****************************************************************************
//
//                         Music Player - Sound output
//
// ****************************************************************************

#include "../include.h"

// output buffer
sOutBuf OutBuf;

#if BABYBEAT		// 1 = use BabyBeat device
// volume
int Volume = 6;				// current volume
int VolMul = 769;			// current volume multiplier

// volume table
const int VolMulTab[VOLMAX+1] = {
	96,		// 0
	136,		// 1
	192,		// 2
	272,		// 3
	384,		// 4
	544,		// 5
	769,		// 6
	1088,		// 7
	1539,		// 8
	2177,		// 9
};
#else // BABYBEAT
#define VolMul	1088
#endif // BABYBEAT

// save sample to output buffer
void OutSamp(s16 samp)
{
	// prepare pointer to output buffer descriptor
	sOutBuf* o = &OutBuf;

	// convert 16-bit signed sample to unsigned ~11-bit sample (range 0..2176)
	u16 val = (((int)samp + 32768) * VolMul) >> 16;

	// waiting for ring buffer to be freed
	while (True)
	{
		int w = o->w;	// write index into ring buffer
		cb();
		int r = o->r;	// read index from ring buffer
		cb();
		int w2 = w + 1;	// next write index
		if (w2 >= OUTBUF_SIZE) w2 = 0; // wrap index to start of the buffer
		if (w2 != r)	// check if there si a free space in the ring buffer
		{
			o->buf[w] = val; // write data to ring buffer
			cb();
			o->w = w2;	// save new write index
			break;
		}
	}
}

// PWM max value for BTL inversion
#define PWM_MAX	2176

// timer handler
__attribute__((interrupt)) void TIM1_UP_IRQHandler(void)
{
	// clear interrupt request
	TIM1_UpIntClr();

	// prepare pointer to output buffer descriptor
	sOutBuf* o = &OutBuf;

	// prepare indices
	int w = o->w;	// write index into ring buffer
	int r = o->r;	// read index from ring buffer

	// check if we have sample to read from the ring buffer
	int s = o->old; // use old value if we have data drop-out
	if (r != w)
	{
		// prepare sample
		s = o->buf[r];	// get sample from the ring buffer
		o->old = s;

		// save new read index
		r++;		// next read index
		if (r >= OUTBUF_SIZE) r = 0; // wrap index to start of the buffer
		o->r = r;	// save new read index
	}

	// output samples (BTL mode: normal + inverted for 2x voltage swing)
#if BABYBEAT		// 1 = use BabyBeat device
	//   Timer1, mapping 2: PC6:CH1 (normal), PC7:CH2 (inverted)
	TIM1_Comp1(s);			// normal signal
	TIM1_Comp2(PWM_MAX - s);	// inverted signal
#else
	//   Timer1, mapping 9: PA1:CH2 (normal), PA2:CH3 (inverted)
	TIM1_Comp2(s);			// normal signal
	TIM1_Comp3(PWM_MAX - s);	// inverted signal
#endif
}

// sound initialize
void SndInit(void)
{
	// initialize output buffer
	memset(&OutBuf, 0, sizeof(OutBuf));

	// setup output pins (BTL mode: two pins for normal + inverted signal)
	RCC_AFIClkEnable();
#if BABYBEAT		// 1 = use BabyBeat device
	RCC_PCClkEnable();
#else
	RCC_PAClkEnable();
#endif
	GPIO_Mode(AUDIO_GPIO, GPIO_MODE_AF);
	GPIO_Mode(AUDIO_INV_GPIO, GPIO_MODE_AF);

	// Remap Timer 1 (default 0)
#if BABYBEAT		// 1 = use BabyBeat device
	//  2 ... PC5:ETR, PC6:CH1 (normal), PC7:CH2 (inverted), PC0:CH3, PD3:CH4
	GPIO_Remap_TIM1(2);

	// initialize PWM output: Timer 1, channel 1, divider 1, reload 2176, compare 0, high
	// System frequency: 48 MHz
	// Sample rate: divider 1, period 2177, result sample rate: 48000000/2177 = 22049 Hz
	TIM1_InitPWM(1, 1, 2176, 0, True);

	// initialize channel 2 (inverted signal)
	TIM1_Comp2(PWM_MAX);		// set compare value (inverted = max when normal = 0)
	TIM1_OC2Mode(TIM_COMP_PWM1);	// set compare mode
	TIM1_OC2PreEnable();		// enable preload compare register
	TIM1_CC2Enable();		// enable compare output

#else
	// 9 ... PB4:ETR, PA0:CH1, PA1:CH2 (normal), PA2:CH3 (inverted), PA3:CH4
	GPIO_Remap_TIM1(9);

	// initialize PWM output: Timer 1, channel 2, divider 1, reload 2176, compare 0, high
	// System frequency: 48 MHz
	// Sample rate: divider 1, period 2177, result sample rate: 48000000/2177 = 22049 Hz
	TIM1_InitPWM(2, 1, 2176, 0, True);

	// initialize channel 3 (inverted signal)
	TIM1_Comp3(PWM_MAX);		// set compare value (inverted = max when normal = 0)
	TIM1_OC3Mode(TIM_COMP_PWM1);	// set compare mode
	TIM1_OC3PreEnable();		// enable preload compare register
	TIM1_CC3Enable();		// enable compare output
#endif

	// set interrupt
	TIM1_UpIntEnable();		// enable update interrupt
	NVIC_IRQEnable(IRQ_TIM1_UP);	// enable interrupt service
}
