/**
 * ============================================================================
 * Sound Synthesis Module
 * ============================================================================
 *
 * PWM-based sound output supporting single tones and melodies.
 * Uses Timer 1 channel 4 for PWM generation at 1 MHz time base.
 *
 * Based on CH32LibSDK by Miroslav Nemecek (Panda38)
 * https://github.com/Panda381/CH32LibSDK
 *
 * ============================================================================
 */

#include "../include.h"

#if USE_SOUND == 2  // use sound support 1=tone, 2=melody
// pointer to current melody
const sMelodyNote* volatile SoundMelodyPtr;

// pointer to next melody
const sMelodyNote* volatile SoundMelodyNext;

// remaining length of current tone (0 = no melody, -1 = start next melody)
volatile s16 SoundMelodyLen = 0;

// Game sound samples
const sMelodyNote SoundSamp1[] = {
    { 1, NOTE_C6 },
    { 1, NOTE_CS6 },
    { 1, NOTE_D6 },
    { 1, NOTE_DS6 },
    { 1, NOTE_E6 },
    { 1, NOTE_F6 },
    { 1, NOTE_FS6 },
    { 1, NOTE_G6 },
    { 1, NOTE_GS6 },
    { 1, NOTE_A6 },
    { 1, NOTE_AS6 },
    { 1, NOTE_B6 },
    { 1, NOTE_C7 },
    { 0, 0 },
};

const sMelodyNote SoundSamp2[] = {
    { 1, NOTE_C7 },
    { 1, NOTE_B6 },
    { 1, NOTE_AS6 },
    { 1, NOTE_A6 },
    { 1, NOTE_GS6 },
    { 1, NOTE_G6 },
    { 1, NOTE_FS6 },
    { 1, NOTE_F6 },
    { 1, NOTE_E6 },
    { 1, NOTE_DS6 },
    { 1, NOTE_D6 },
    { 1, NOTE_CS6 },
    { 1, NOTE_C6 },

    { 1, NOTE_B5 },
    { 1, NOTE_AS5 },
    { 1, NOTE_A5 },
    { 1, NOTE_GS5 },
    { 1, NOTE_G5 },
    { 1, NOTE_FS5 },
    { 1, NOTE_F5 },
    { 1, NOTE_E5 },
    { 1, NOTE_DS5 },
    { 1, NOTE_D5 },
    { 1, NOTE_CS5 },
    { 1, NOTE_C5 },
    { 0, 0 },
};

const sMelodyNote SoundSamp3[] = {
    { 1, NOTE_C4 },
    { 1, NOTE_CS4 },
    { 1, NOTE_D4 },
    { 1, NOTE_DS4 },
    { 1, NOTE_E4 },
    { 1, NOTE_F4 },
    { 1, NOTE_FS4 },
    { 1, NOTE_G4 },
    { 1, NOTE_GS4 },
    { 1, NOTE_A4 },
    { 1, NOTE_AS4 },
    { 1, NOTE_B4 },
    { 1, NOTE_C5 },

    { 1, NOTE_B4 },
    { 1, NOTE_AS4 },
    { 1, NOTE_A4 },
    { 1, NOTE_GS4 },
    { 1, NOTE_G4 },
    { 1, NOTE_FS4 },
    { 1, NOTE_F4 },
    { 1, NOTE_E4 },
    { 1, NOTE_DS4 },
    { 1, NOTE_D4 },
    { 1, NOTE_CS4 },
    { 1, NOTE_C4 },
    { 0, 0 },
};

const sMelodyNote SoundSamp4[] = {
    { 1, NOTE_C5 },
    { 1, NOTE_B5 },
    { 1, NOTE_CS5 },
    { 1, NOTE_AS5 },
    { 1, NOTE_D5 },
    { 1, NOTE_A5 },
    { 1, NOTE_DS5 },
    { 1, NOTE_GS5 },
    { 1, NOTE_E5 },
    { 1, NOTE_G5 },
    { 1, NOTE_F5 },
    { 1, NOTE_FS5 },
    { 1, NOTE_F5 },
    { 0, 0 },
};

// Initialize sound output on PC4 (TIM1_CH4)
void SoundInit()
{
    RCC_AFIClkEnable();
    RCC_PCClkEnable();
    GPIO_Mode(PC4, GPIO_MODE_AF);
    GPIO_Remap_TIM1(0);  // TIM1_CH4 on PC4

    TIM1_ClkEnable();
    TIM1_Reset();
    TIM1_InMode(TIM_INMODE_INT);
    TIM1_Div(SND_TIM_DIV);  // Prescaler for 1 MHz time base
    TIM1_Load(1000);        // Default period
    TIM1_Comp4(500);        // 50% duty cycle
    TIM1_DirUp();
    TIM1_Update();
    TIM1_AutoReloadEnable();
    TIM1_OC4Mode(TIM_COMP_PWM1);
    TIM1_OC4PreEnable();
    TIM1_OCEnable();
    TIM1_Enable();
}

// Terminate sound output
void SoundTerm()
{
    TIM1_Disable();
    TIM1_Reset();
    GPIO_PinReset(PC4);
}

// Play tone with given divider (use SND_GET_DIV or NOTE_* constants)
void PlayTone(u32 div)
{
    TIM1_Load(div);
    TIM1_Comp4(div >> 1);  // 50% duty cycle
    TIM1_CC4Enable();
}

// Stop playing tone or melody
void StopSound()
{
#if USE_SOUND == 2
    SoundMelodyLen = 0;
#endif
    TIM1_CC4Disable();
}

#endif // USE_SOUND

#if USE_SOUND == 2  // use sound support 1=tone, 2=melody
// Sound scan melody
void SoundScan()
{
    // get churrent melody
    int len = SoundMelodyLen;
    if (len == 0) return; // no melody

    // pointer to current melody
    const sMelodyNote* ptr;

    // start next melody
    if (len < 0)
    {
        // get pointer to next melody
        ptr = SoundMelodyNext;
    }

    // continue melody - decrease counter of current tone
    else
    {
        len--;
        SoundMelodyLen = (s16)len; // save new tone length

        // continue current tone
        if (len > 0) return;

        // shift current melody pointer
        ptr = SoundMelodyPtr;   // get pointer to current melody
        ptr++;          // shift pointer to next tone
    }

    // save new pointer to current melody
    SoundMelodyPtr = ptr;

    // get length of next note
    len = ptr->len; // get length of the note
    SoundMelodyLen = (s16)len;  // save length of new note

    // end of melody
    if (len == 0)
    {
        // disable compare output
        TIM1_CC4Disable();
    }

    // start new note
    else
    {
        // get note divider
        u16 div = ptr->div;

        // pause
        if (div == 0)
        {
            // disable compare output
            TIM1_CC4Disable();
        }
        else
        {
            // start playing tone
            PlayTone(div);
        }
    }
}

// play melody
//  melody = pointer to array of notes sMelodyNote (terminated with NOTE_STOP)
void PlayMelody(const sMelodyNote* melody)
{
    // set next melody
    SoundMelodyNext = melody;
    cb();

    // request to restart melody
    SoundMelodyLen = -1;
}
#endif // USE_SOUND
