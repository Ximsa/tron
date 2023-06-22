
#include <stdint.h>

#define SAMPLE_RATE 44100
#define SOUND_MAX_FREQ 1193182
#define SOUND_MAX_LENGTH 64
#define SOUND_VOLUME 16384

// https://en.wikipedia.org/wiki/Piano_key_frequencies#List
// Convert a divisor D into a frequency as 1193182 / D.
#define SOUND_TERMINATOR	0x0000
#define NOTE_REST		0x0000	// 0x0028 29829.550 Hz
#define NOTE_C3			0x2394	// 131.004 Hz
#define NOTE_D3			0x1fb5	// 146.998 Hz
#define NOTE_E3			0x1c3f	// 165.009 Hz
#define NOTE_F3			0x1aa2	// 175.005 Hz
#define NOTE_G3			0x17c8	// 195.989 Hz
#define NOTE_A3			0x1530	// 219.982 Hz
#define NOTE_B3			0x12df	// 246.984 Hz
#define NOTE_C4			0x11ca	// 262.007 Hz
#define NOTE_B4			0x0974	// 493.050 Hz
#define NOTE_C5			0x08e9	// 523.096 Hz
#define NOTE_D5			0x07f1	// 586.907 Hz
#define NOTE_E5			0x0713	// 658.853 Hz
#define NOTE_F5			0x06ad	// 698.176 Hz
#define NOTE_G5			0x05f2	// 783.957 Hz
#define NOTE_A5			0x054b	// 880.577 Hz

// Convert a divisor D into a frequency as 1193182 / D.
#define SOUND_FREQ_94HZ		0x3195	//  94.003 Hz
#define SOUND_FREQ_95HZ		0x310f	//  95.006 Hz
#define SOUND_FREQ_96HZ		0x308c	//  96.008 Hz
#define SOUND_FREQ_97HZ		0x300c	//  97.007 Hz
#define SOUND_FREQ_98HZ		0x2f8f	//  98.003 Hz
#define SOUND_FREQ_99HZ		0x2f14	//  99.003 Hz
#define SOUND_FREQ_100HZ	0x2e9b	// 100.007 Hz
#define SOUND_FREQ_105HZ	0x2c63	// 105.006 Hz
#define SOUND_FREQ_110HZ	0x2a5f	// 110.001 Hz
#define SOUND_FREQ_125HZ	0x2549	// 125.006 Hz
#define SOUND_FREQ_130HZ	0x23da	// 130.005 Hz
#define SOUND_FREQ_146HZ	0x1fec	// 146.009 Hz
#define SOUND_FREQ_150HZ	0x1f12	// 150.010 Hz
#define SOUND_FREQ_160HZ	0x1d21	// 160.008 Hz
#define SOUND_FREQ_200HZ	0x174d	// 200.031 Hz
#define SOUND_FREQ_210HZ	0x1631	// 210.030 Hz
#define SOUND_FREQ_220HZ	0x152f	// 220.022 Hz
#define SOUND_FREQ_230HZ	0x1443	// 230.033 Hz
#define SOUND_FREQ_240HZ	0x136b	// 240.029 Hz
#define SOUND_FREQ_250HZ	0x12a4	// 250.038 Hz
#define SOUND_FREQ_300HZ	0x0f89	// 300.021 Hz
#define SOUND_FREQ_400HZ	0x0ba6	// 400.128 Hz
#define SOUND_FREQ_600HZ	0x07c4	// 600.192 Hz
#define SOUND_FREQ_900HZ	0x052d	// 900.515 Hz
