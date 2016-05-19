#ifndef DTMF_DATA_H
#define DTMF_DATA_H

/* Global parametrics */
#define DTMFSampleSize 256
#define DTMFSampleType int16_t
#define DTMFSampleRate 8000

/* DTMF frequencies (Hz) */
#define DTMF_NO_FREQ 0
#define DTMF_L0_FREQ 697
#define DTMF_L1_FREQ 770
#define DTMF_L2_FREQ 852
#define DTMF_L3_FREQ 941
#define DTMF_H0_FREQ 1209
#define DTMF_H1_FREQ 1336
#define DTMF_H2_FREQ 1477
#define DTMF_H3_FREQ 1633

/* DTMF frequency FFT bins */
#define DTMF_BIN(x) ((int16_t)((float)x / ((float)DTMFSampleRate / (float)DTMFSampleSize)) )
#define DTMF_L0_BIN DTMF_BIN(DTMF_L0_FREQ)
#define DTMF_L1_BIN DTMF_BIN(DTMF_L1_FREQ)
#define DTMF_L2_BIN DTMF_BIN(DTMF_L2_FREQ)
#define DTMF_L3_BIN DTMF_BIN(DTMF_L3_FREQ)
#define DTMF_H0_BIN DTMF_BIN(DTMF_H0_FREQ)
#define DTMF_H1_BIN DTMF_BIN(DTMF_H1_FREQ)
#define DTMF_H2_BIN DTMF_BIN(DTMF_H2_FREQ)
#define DTMF_H3_BIN DTMF_BIN(DTMF_H3_FREQ)

/* Result of the DTMF detection */
/* Code is the ASCII of the detected tones (space for none) */
/* toneX is the tone frequency in hz */
struct DTMFResult_t {
	int8_t code;
	int16_t toneA;
	int16_t toneB;
};

#endif
